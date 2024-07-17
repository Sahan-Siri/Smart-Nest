#include <Wire.h>
#include <WiFiManager.h>
#include <EEPROM.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define Button 7
#define EEPROM_SIZE 100
#define PATH_ADDRESS 0
#define API_KEY "AD API"
#define DATABASE_URL "ADD URL"

String Path;
String Old="Off";
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
WiFiManager wm;

float number1, number2, number3;
byte buffer[12]; // 3 floats * 4 bytes each = 12 bytes
int bufferIndex = 0;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;

volatile bool resetFlag = false;
unsigned long buttonPressTime = 0;
bool buttonPressed = false;

void IRAM_ATTR handleButtonPress() {
  if (digitalRead(Button) == LOW) { // Button is pressed
    if (!buttonPressed) {
      buttonPressed = true;
      buttonPressTime = millis();
    }
  } else { // Button is released
    buttonPressed = false;
  }
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

float roundToTwoDecimalPlaces(float value) {
  return roundf(value * 100.0) / 100.0;
}

void setup() {
  pinMode(Button, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(Button), handleButtonPress, CHANGE);
  pinMode(6, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(10, INPUT);
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
    String Email = Email_Box.getValue();
    String Location = Location_Box.getValue();
    String Socket = Name_Box.getValue();

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
  config.database_url = DATABASE_URL;
  config.token_status_callback = tokenStatusCallback;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  signupOK = true;
  Wire.begin(8); // Join I2C bus with address #8
  Wire.onReceive(receiveEvent); // Register event
  if (digitalRead(10)==0){
    Firebase.RTDB.setString(&fbdo, Path + "/Switch","On");
    Old="On";
  }
  if (digitalRead(10)==1){
    Firebase.RTDB.setString(&fbdo, Path + "/Switch","Off");
    Old="Off";
  }
}

void loop() {
  if (buttonPressed && (millis() - buttonPressTime >= 3000)) {
    resetFlag = true;
    buttonPressed = false;
  }

  if (resetFlag) {
    resetFlag = false;
    Serial.println("Resetting WiFiManager settings...");
    wm.resetSettings();
    Path = "/0";
    saveStringToEEPROM(PATH_ADDRESS, Path);
    digitalWrite(6, LOW);
    delay(1000);
    ESP.restart();
  }

  if (WiFi.isConnected()) {
    digitalWrite(6, HIGH);
  } else {
    digitalWrite(6, LOW);
  }
  Serial.println(Path);

  if (Firebase.ready() && signupOK) {
    if (Old=="On" && digitalRead(10)==1){
      Firebase.RTDB.setString(&fbdo, Path + "/Switch","Off");
      Old="Off";
    }
    if (Old=="Off" && digitalRead(10)==0){
      Firebase.RTDB.setString(&fbdo, Path + "/Switch","On");
      Old="On";
    }
    if (Firebase.RTDB.getString(&fbdo, Path + "/Switch")) {
      String switchValue = fbdo.stringData();
      switchValue = switchValue.substring(2, switchValue.length() - 2);
      Serial.println(switchValue);
      if ((switchValue == "On") && (switchValue!=Old) && digitalRead(10)==1) {
        digitalWrite(5, HIGH);
        delay(100);
        digitalWrite(5, LOW);
        Old=switchValue;
      } 
      if ((switchValue == "Off") && (switchValue!=Old) && digitalRead(10)==0) {
        digitalWrite(5, HIGH);
        delay(100);
        digitalWrite(5, LOW);
        Old=switchValue;
      }
       
    } else {
      Serial.println("Failed to get switch value: " + fbdo.errorReason());
      ESP.restart();
    }

    if (millis() - sendDataPrevMillis > 5000 || sendDataPrevMillis == 0) {
      sendDataPrevMillis = millis();
      float roundedNumber1 = roundToTwoDecimalPlaces(number1);
      float roundedNumber2 = roundToTwoDecimalPlaces(number2);
      float roundedNumber3 = roundToTwoDecimalPlaces(number3);

      if (Old=="Off"){
        roundedNumber1=0.00;
        roundedNumber2=0.00;
        roundedNumber3=0.00;
      }

      if (Firebase.RTDB.setFloat(&fbdo, Path + "/Power", roundedNumber1)) {
        Serial.println(roundedNumber1);
        Serial.println("Power value set successfully");
      } else {
        Serial.println("Failed to set Power value: " + fbdo.errorReason());
        ESP.restart();
      }

      if (Firebase.RTDB.setFloat(&fbdo, Path + "/Voltage", roundedNumber2)) {
        Serial.println(roundedNumber2);
        Serial.println("Voltage value set successfully");
      } else {
        Serial.println("Failed to set Voltage value: " + fbdo.errorReason());
        ESP.restart();
      }

      if (Firebase.RTDB.setFloat(&fbdo, Path + "/Current", roundedNumber3)) {
        Serial.println(roundedNumber3);
        Serial.println("Current value set successfully");
      } else {
        Serial.println("Failed to set Current value: " + fbdo.errorReason());
        ESP.restart();
      }
    }
  } else {
    Serial.println("Firebase not ready or sign-up not completed");
    ESP.restart();
  }

  delay(1000);
  fbdo.clear();
}

void receiveEvent(int howMany) {
  while (Wire.available() && bufferIndex < 12) {
    buffer[bufferIndex++] = Wire.read();
  }
  if (bufferIndex >= 12) { // We have received 12 bytes
    bufferIndex = 0;
    // Cast bytes to float variables
    number1 = *((float*)&buffer[0]);
    number2 = *((float*)&buffer[4]);
    number3 = *((float*)&buffer[8]);
  }
}
