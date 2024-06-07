#include <WiFiManager.h>
#include <EEPROM.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define Button 15
#define EEPROM_SIZE 100
#define PATH_ADDRESS 0
#define API_KEY "AIzaSyAKF2apBkqBW3pKeMt0GMj2MXmkSoebQks"
#define DATABASE_URL "https://smartnest0-default-rtdb.firebaseio.com/" 

char Path[100];
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;

volatile bool resetFlag = false;

void IRAM_ATTR Reset() {
  resetFlag = true;
}

void saveStringToEEPROM(int address, const char* data) {
  for (int i = 0; i < strlen(data); ++i) {
    EEPROM.write(address + i, data[i]);
  }
  EEPROM.write(address + strlen(data), '\0');
  EEPROM.commit();
}

void readStringFromEEPROM(int address, char* data) {
  for (int i = 0; i < 50; ++i) {
    data[i] = EEPROM.read(address + i);
    if (data[i] == '\0') break;
  }
}

void setup() {
  pinMode(Button, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(Button), Reset, FALLING);
  pinMode(22, OUTPUT);
  pinMode(2, OUTPUT);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  EEPROM.begin(EEPROM_SIZE);
  WiFiManager wm;
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
  readStringFromEEPROM(PATH_ADDRESS, Path);
  if (strcmp(Path, "/0") == 0) {
    char Email[50];
    char Location[50];
    char Socket[50];
    strncpy(Email, Email_Box.getValue(), sizeof(Email) - 1);
    strncpy(Location, Location_Box.getValue(), sizeof(Location) - 1);
    strncpy(Socket, Name_Box.getValue(), sizeof(Socket) - 1);

    int atIndex = strcspn(Email, "@");
    if (atIndex != strlen(Email)) {
      Email[atIndex] = '\0';
    }
    atIndex = strcspn(Location, " ");
    if (atIndex != strlen(Location)) {
      Location[atIndex] = '\0';
    }
    atIndex = strcspn(Socket, " ");
    if (atIndex != strlen(Socket)) {
      Socket[atIndex] = '\0';
    }
    snprintf(Path, sizeof(Path), "%s/%s %s", Email, Location, Socket);
    saveStringToEEPROM(PATH_ADDRESS, Path);
  }

  config.api_key = API_KEY;
  auth.user.email = "Device01@smartnest.com";
  auth.user.password = "Smart@1234";
  config.database_url = DATABASE_URL;

  config.token_status_callback = tokenStatusCallback;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  signupOK = true;
}

void loop() {
  if (resetFlag) {
    resetFlag = false;
    Serial.println("Resetting WiFiManager settings...");
    WiFiManager wm;
    wm.resetSettings();
    strcpy(Path, "/0");
    saveStringToEEPROM(PATH_ADDRESS, Path);
    digitalWrite(2, LOW);
    delay(1000);
    ESP.restart();
  }

  if (WiFi.isConnected()) {
    digitalWrite(2, HIGH);
  } else {
    digitalWrite(2, LOW);
  }
  Serial.println(F(Path));

  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();
    Firebase.RTDB.setFloat(&fbdo, String(Path) + "/Power", int(0.01 + random(5, 25)));
    Firebase.RTDB.setFloat(&fbdo, String(Path) + "/Voltage", int(0.01 + random(228, 235)));
    Firebase.RTDB.setFloat(&fbdo, String(Path) + "/Current", int(0.01 + random(1, 13)));
    fbdo.clear();
  }

  Serial.println(F(ESP.getFreeHeap()));
}
