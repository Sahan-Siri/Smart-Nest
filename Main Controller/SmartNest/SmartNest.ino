#include <FS.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <WiFi.h>
#include <EEPROM.h> // Include EEPROM library

#define Button 15
#define EEPROM_SIZE 512 // Define EEPROM size
#define EMAIL_ADDRESS 0 // EEPROM address to store the Email

WiFiManager wm;
String Email;
volatile bool resetFlag = false;

void IRAM_ATTR Reset() {
  resetFlag = true;
}

void saveEmailToEEPROM(String email) {
  for (int i = 0; i < email.length(); ++i) {
    EEPROM.write(EMAIL_ADDRESS + i, email[i]);
  }
  EEPROM.write(EMAIL_ADDRESS + email.length(), '\0'); // Add null terminator
  EEPROM.commit(); // Save changes to EEPROM
}

String readEmailFromEEPROM() {
  char email[50]; // Adjust size as needed
  for (int i = 0; i < 50; ++i) {
    email[i] = EEPROM.read(EMAIL_ADDRESS + i);
    if (email[i] == '\0') break; // Stop reading if null terminator is encountered
  }
  return String(email);
}

void setup() {
  pinMode(Button, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(Button), Reset, FALLING); // Use FALLING for INPUT_PULLUP
  pinMode(2, OUTPUT);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA); // Explicitly set mode, ESP defaults to STA+AP

  EEPROM.begin(EEPROM_SIZE); // Initialize EEPROM

  // Retrieve stored email from EEPROM
  Email = readEmailFromEEPROM();
  wm.setDebugOutput(false);

  WiFiManagerParameter custom_text_box("Email", "Enter your E-mail here", "", 50);
  wm.addParameter(&custom_text_box);
  
  bool res = wm.autoConnect("SmartNest", "password"); // Password protected AP

  if (!res) {
    Serial.println("Failed to connect");
  } else {
    Serial.println("Connected... yeey :)");
  }

  if (Email==""){
    Email = custom_text_box.getValue();
    int atIndex = Email.indexOf('@');
    if (atIndex != -1) {
      Email = Email.substring(0, atIndex);
      }
      saveEmailToEEPROM(Email);
    }
}

void loop() {
  if (resetFlag) {
    resetFlag = false; // Reset the flag
    Serial.println("Resetting WiFiManager settings...");
    wm.resetSettings();
    Email="";
    saveEmailToEEPROM(Email);
    digitalWrite(2, LOW);
    delay(1000); // Delay to ensure settings are reset before restarting
    ESP.restart();
  }
  
  if (WiFi.isConnected()) {
    digitalWrite(2, HIGH);
  } else {
    digitalWrite(2, LOW);
  }

  Serial.println(Email);
  delay(1000); // Small delay to avoid flooding the Serial output
}
