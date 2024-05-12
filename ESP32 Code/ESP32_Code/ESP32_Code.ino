#include <WiFi.h>
#include <Firebase_ESP_Client.h>

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "AndroidAP7697"
#define WIFI_PASSWORD "sahan@@@@1234"

// Insert Firebase project API Key
#define API_KEY "AIzaSyAKF2apBkqBW3pKeMt0GMj2MXmkSoebQks"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://smartnest0-default-rtdb.firebaseio.com/" 

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;
String uid;
#define Switch=D2;
String Stat;

void setup(){
  //Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  //Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    //Serial.print(".");
    delay(300);
  }
  //Serial.println();
  //Serial.print("Connected with IP: ");
  //Serial.println(WiFi.localIP());
  //Serial.println();
  pinMode(22,OUTPUT);
  /* Assign the api key (required) */
  config.api_key = API_KEY;
  auth.user.email = "Device01@smartnest.com";
  auth.user.password = "Smart@1234";
  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;
  uid = auth.token.uid.c_str();
 // Serial.println(uid);
  Firebase.begin(&config, &auth);
  uid = auth.token.uid.c_str();
  //Serial.print("User UID: ");
  //Serial.println(uid);
  signupOK = true;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  Firebase.reconnectWiFi(true);
}

void loop(){
  if (Firebase.RTDB.getString(&fbdo, "sandunmax422/Kitchen Plug 02/Switch")){
    String switchValue = fbdo.stringData();
    switchValue = switchValue.substring(2, switchValue.length() - 2); // Remove extra quotes
    //Serial.println("Switch Value: " + switchValue);
    if (switchValue=="On"){
      //Serial.println("Ok");
      digitalWrite(22,HIGH);
    }
    else{
      //Serial.println("GO");
      digitalWrite(22,LOW);
    }
  }
  else {
    //Serial.println("FAILED to get Switch value");
    //Serial.println("REASON: " + fbdo.errorReason());
  }
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();
    // Write an Int number on the database path test/int
    if (Firebase.RTDB.setFloat(&fbdo, "sandunmax422/Kitchen Plug 02/Power",int(0.01+random(5,25)))){
      //Serial.println("PASSED");
      //Serial.println("PATH: " + fbdo.dataPath());
      //Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      //Serial.println("FAILED");
      //Serial.println("REASON: " + fbdo.errorReason());
    }
    count++;
    
    // Write an Float number on the database path test/float
    if (Firebase.RTDB.setFloat(&fbdo, "sandunmax422/Kitchen Plug 02/Voltage",int( 0.01 + random(228,235)))){
      //Serial.println("PASSED");
      //Serial.println("PATH: " + fbdo.dataPath());
      //Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      //Serial.println("FAILED");
      //Serial.println("REASON: " + fbdo.errorReason());
    }
  }
}