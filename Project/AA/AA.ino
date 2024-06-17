#include <WiFiManager.h>
#include <EEPROM.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define Button 21
#define EEPROM_SIZE 100
#define PATH_ADDRESS 0
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
  EEPROM.write(address + data.length(), '\0');
  EEPROM.commit();
}

String readStringFromEEPROM(int address) {
  char data[50];
  for (int i = 0; i < 50; ++i) {
    data[i] = EEPROM.read(address + i);
    if (data[i] == '\0') break;
  }
  return String(data);
}

void setup() {
  pinMode(Button, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(Button), Reset, FALLING);
  pinMode(2, OUTPUT);
  pinMode(8, OUTPUT);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  EEPROM.begin(EEPROM_SIZE);

  Path = readStringFromEEPROM(PATH_ADDRESS);
  wm.setDebugOutput(false);

  WiFiManagerParameter Email_Box("Email", "Enter your E-mail here", "", 50);
  WiFiManagerParameter Location_Box("Location", "Location Name", "", 50);
  WiFiManagerParameter Name_Box("Name", "Name the Socket", "", 50);
  wm.addParameter(&Email_Box);
  wm.addParameter(&Location_Box);
  wm.addParameter(&Name_Box);

  bool res = wm.autoConnect("SmartNest", "password");
  if (!res) {
    Serial.println("Failed to connect");
  } else {
    Serial.println("Connected... yeey :)");
  }

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
    Path = Email + "/" + Location + " " + Socket;
    saveStringToEEPROM(PATH_ADDRESS, Path);
  }

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("ok");
    signupOK = true;
  } else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

}

void loop() {

  if (WiFi.isConnected()) {
    digitalWrite(8, HIGH);
  } else {
    digitalWrite(8, LOW);
  }
  Serial.println(Path);
    if (resetFlag) {
      resetFlag = false;
      Serial.println("Resetting WiFiManager settings...");
      wm.resetSettings();
      Path = "/0";
      saveStringToEEPROM(PATH_ADDRESS, Path);
      digitalWrite(8, LOW);
      delay(1000);
      ESP.restart();
  }

    if (Firebase.ready() && signupOK) {
      if (Firebase.RTDB.getInt(&fbdo, Path + "/Switch")) {
        int switchValue = fbdo.intData();
        Serial.println(switchValue);
  delay(1000);
}
