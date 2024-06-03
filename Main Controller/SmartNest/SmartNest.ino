#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <EEPROM.h> // Include EEPROM library
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"


#define Button 15
#define EEPROM_SIZE 64 // Define EEPROM size
#define PATH_ADDRESS 0 // EEPROM address to store the Path
#define API_KEY "AIzaSyAKF2apBkqBW3pKeMt0GMj2MXmkSoebQks"
#define DATABASE_URL "https://smartnest0-default-rtdb.firebaseio.com/" 

WiFiManager wm;
String Email;
String Location;
String Socket;
String Path;

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;

volatile bool resetFlag = false;

void IRAM_ATTR Reset() {
  resetFlag = true;
}

void saveStringToEEPROM(int address, String data) {
  for (int i = 0; i < data.length(); ++i) {
    EEPROM.write(address + i, data[i]);
  }
  EEPROM.write(address + data.length(), '\0'); // Add null terminator
  EEPROM.commit(); // Save changes to EEPROM
}

String readStringFromEEPROM(int address) {
  char data[50]; // Adjust size as needed
  for (int i = 0; i < 50; ++i) {
    data[i] = EEPROM.read(address + i);
    if (data[i] == '\0') break; // Stop reading if null terminator is encountered
  }
  return String(data);
}

void setup() {
  pinMode(Button, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(Button), Reset, FALLING); // Use FALLING for INPUT_PULLUP
  pinMode(2, OUTPUT);
  pinMode(22, OUTPUT);
  pinMode(23,INPUT);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA); // Explicitly set mode, ESP defaults to STA+AP

  EEPROM.begin(EEPROM_SIZE); // Initialize EEPROM

  // Retrieve stored variables from EEPROM
  Path=readStringFromEEPROM(PATH_ADDRESS);
  wm.setDebugOutput(false);

  WiFiManagerParameter Email_Box("Email", "Enter your E-mail here", "", 50);
  WiFiManagerParameter Location_Box("Location", "Location Name", "", 50);
  WiFiManagerParameter Name_Box("Name", "Name the Socket", "", 50);
  wm.addParameter(&Email_Box);
  wm.addParameter(&Location_Box);
  wm.addParameter(&Name_Box);
  
  bool res = wm.autoConnect("SmartNest", "password"); // Password protected AP
  if (!res) {
    Serial.println("Failed to connect");
  } else {
    Serial.println("Connected... yeey :)");
  }

  // Save parameters if they are empty
  if (Path == "/0") {
    Email = Email_Box.getValue();
    Location = Location_Box.getValue();
    Socket = Name_Box.getValue();

    int atIndex = Email.indexOf('@');
    if (atIndex != -1) {
      Email = Email.substring(0, atIndex);
    }
    atIndex = Location.indexOf(' ');
    if (atIndex != -1) {
      Location = Location.substring(0, atIndex);
    }
    atIndex = Socket.indexOf(' ');
    if (atIndex != -1) {
      Socket = Socket.substring(0, atIndex);
    }
    Path=Email+"/"+Location+" "+Socket;
    saveStringToEEPROM(PATH_ADDRESS, Path);
  }
  config.api_key = API_KEY;
  auth.user.email = "Device01@smartnest.com";
  auth.user.password = "Smart@1234";
  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;
  Firebase.begin(&config, &auth);

  signupOK = true;
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  Firebase.reconnectWiFi(true);

}

void loop() {
  if (resetFlag) {
    resetFlag = false; // Reset the flag
    Serial.println("Resetting WiFiManager settings...");
    wm.resetSettings();
    Path = "/0";
    saveStringToEEPROM(PATH_ADDRESS, Path);
    digitalWrite(2, LOW);
    delay(1000); // Delay to ensure settings are reset before restarting
    ESP.restart();
  }
  
  if (WiFi.isConnected()) {
    digitalWrite(2, HIGH);
  } else {
    digitalWrite(2, LOW);
  }
  Serial.println(Path);
  if (Firebase.ready() && signupOK) {
    if (Firebase.RTDB.getString(&fbdo, Path + "/Switch")) {
      String switchValue = fbdo.stringData();
      switchValue = switchValue.substring(2, switchValue.length() - 2); // Remove extra quotes
      Serial.println("Switch Value: " + switchValue);
      if (switchValue == "On") {
        digitalWrite(22, HIGH);
      } else {
        digitalWrite(22, LOW);
      }
    }
    if (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0) {
      sendDataPrevMillis = millis();
      Firebase.RTDB.setFloat(&fbdo, Path + "/Power", int(0.01 + random(5, 25)));
      Firebase.RTDB.setFloat(&fbdo, Path + "/Voltage", int(0.01 + random(228, 235)));
      Firebase.RTDB.setFloat(&fbdo, Path + "/Current", int(0.01 + random(1, 5)));
    }
  }
}
