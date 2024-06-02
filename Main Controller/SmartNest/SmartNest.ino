#include <FS.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <WiFi.h>
#include <EEPROM.h> // Include EEPROM library

#define Button 15
#define EEPROM_SIZE 512 // Define EEPROM size
#define PATH_ADDRESS 0 // EEPROM address to store the Path

WiFiManager wm;
String Email;
String Location;
String Socket;
String Path;

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
    Path=Email+"/"+Location+"/"+Socket;
    saveStringToEEPROM(PATH_ADDRESS, Path);
  }
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
  
  if (WiFi.isConnected()) {
    digitalWrite(2, HIGH);
  } else {
    digitalWrite(2, LOW);
  }
  Serial.println(Path);
  delay(1000); // Small delay to avoid flooding the Serial output
}
