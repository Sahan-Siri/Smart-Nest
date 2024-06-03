#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <EEPROM.h> // Include EEPROM library
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define Button 15
#define EEPROM_SIZE 512 // Define EEPROM size
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
<<<<<<< HEAD
unsigned long lastReconnectAttempt = 0;
const unsigned long reconnectInterval = 30000; // Try reconnecting every 30 seconds
=======
int count = 0;
bool signupOK = false;
String uid;
>>>>>>> parent of 710a645 (Update SmartNest.ino)

bool signupOK = false;
volatile bool resetFlag = false;
String Plug_Stat = "";

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
<<<<<<< HEAD
  pinMode(22, OUTPUT);
  pinMode(23, INPUT);
=======
>>>>>>> parent of 710a645 (Update SmartNest.ino)
  Serial.begin(115200);
  WiFi.mode(WIFI_STA); // Explicitly set mode, ESP defaults to STA+AP

  EEPROM.begin(EEPROM_SIZE); // Initialize EEPROM

  // Retrieve stored variables from EEPROM
  Path = readStringFromEEPROM(PATH_ADDRESS);
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
  if (Path == "") {
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
    Path = Email + "/" + Location + " " + Socket;
    saveStringToEEPROM(PATH_ADDRESS, Path);
  }
  config.api_key = API_KEY;
  auth.user.email = "Device01@smartnest.com";
  auth.user.password = "Smart@1234";
  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;
<<<<<<< HEAD
=======
  uid = auth.token.uid.c_str();
 // Serial.println(uid);
  Firebase.begin(&config, &auth);
  uid = auth.token.uid.c_str();
  //Serial.print("User UID: ");
  //Serial.println(uid);
>>>>>>> parent of 710a645 (Update SmartNest.ino)
  signupOK = true;
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  if (resetFlag) {
    resetFlag = false; // Reset the flag
    Serial.println("Resetting WiFiManager settings...");
    wm.resetSettings();
    Path = "";
    saveStringToEEPROM(PATH_ADDRESS, Path);
    digitalWrite(2, LOW);
    delay(1000); // Delay to ensure settings are reset before restarting
    ESP.restart();
  }
  
  if (!WiFi.isConnected()) {
    digitalWrite(2, LOW);
    unsigned long now = millis();
    if (now - lastReconnectAttempt > reconnectInterval) {
      lastReconnectAttempt = now;
      // Attempt to reconnect
      Serial.println("Attempting to reconnect to WiFi...");
      WiFi.reconnect();
    }
    return; // Skip the rest of the loop if not connected
  } else {
    digitalWrite(2, HIGH);
  }

  Serial.println(Path);
<<<<<<< HEAD
  if (Firebase.ready() && signupOK) {
    if (Firebase.RTDB.getString(&fbdo, Path + "/Switch")) {
      String switchValue = fbdo.stringData();
      switchValue = switchValue.substring(2, switchValue.length() - 2); // Remove extra quotes
      Serial.println("Switch Value: " + switchValue);
      if (Plug_Stat != switchValue) {
        if (switchValue == "On") {
          digitalWrite(22, HIGH);
          Plug_Stat = switchValue;
        } else {
          digitalWrite(22, LOW);
          Plug_Stat = switchValue;
        }
      }
    } else {
      Serial.println("Failed to get Switch value, reason: " + fbdo.errorReason());
    }

    if (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0) {
      sendDataPrevMillis = millis();

      // Error handling for data upload
      if (!Firebase.RTDB.setFloat(&fbdo, Path + "/Power", int(0.01 + random(5, 25)))) {
        Serial.println("Failed to set Power data, reason: " + fbdo.errorReason());
      }
      if (!Firebase.RTDB.setFloat(&fbdo, Path + "/Voltage", int(0.01 + random(228, 235)))) {
        Serial.println("Failed to set Voltage data, reason: " + fbdo.errorReason());
      }
      if (!Firebase.RTDB.setFloat(&fbdo, Path + "/Current", int(0.01 + random(1, 5)))) {
        Serial.println("Failed to set Current data, reason: " + fbdo.errorReason());
      }
=======
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();
    // Write an Int number on the database path test/int
    if (Firebase.RTDB.setFloat(&fbdo, Path+"/Power",int(0.01+random(5,25)))){
    }
    if (Firebase.RTDB.setFloat(&fbdo, Path+"/Voltage",int( 0.01 + random(228,235)))){
    }
    if (Firebase.RTDB.setFloat(&fbdo, Path+"/Current",int( 0.01 + random(1,5)))){
>>>>>>> parent of 710a645 (Update SmartNest.ino)
    }
  } else {
    Serial.println("Firebase not ready or signup not OK");
  }
  delay(100);
}
