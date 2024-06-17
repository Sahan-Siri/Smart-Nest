#include <FirebaseClient.h>
#include <WiFiClientSecure.h>

#define WIFI_SSID "SLT_FIBRE"
#define WIFI_PASSWORD "2828282828"

// The API key can be obtained from Firebase console > Project Overview > Project settings.
#define API_KEY "AIzaSyAKF2apBkqBW3pKeMt0GMj2MXmkSoebQks"

// User Email and password that already registerd or added in your project.
#define USER_EMAIL "device01@smartnest.com"
#define USER_PASSWORD "Smart@1234"
#define DATABASE_URL "https://smartnest0-default-rtdb.firebaseio.com/"

void authHandler();


DefaultNetwork network; // initilize with boolean parameter to enable/disable network reconnection

UserAuth user_auth(API_KEY, USER_EMAIL, USER_PASSWORD);

FirebaseApp app;

WiFiClientSecure ssl_client;


using AsyncClient = AsyncClientClass;

AsyncClient aClient(ssl_client, getNetwork(network));

RealtimeDatabase Database;

AsyncResult aResult_no_callback;

void setup()
{
    pinMode(2,OUTPUT);
    Serial.begin(115200);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
    }
    ssl_client.setInsecure();

    initializeApp(aClient, app, getAuth(user_auth), aResult_no_callback);

    authHandler();

    app.getApp<RealtimeDatabase>(Database);

    Database.url(DATABASE_URL);
    aClient.setAsyncResult(aResult_no_callback);
}

void loop()
{
   if (WiFi.isConnected()) {
    digitalWrite(2, HIGH);
  } else {
    digitalWrite(2, LOW);
  }
      Serial.print("Get int... ");
    int v1 = Database.get<int>(aClient, "/test/int");
    if (aClient.lastError().code() == 0){
        Serial.println(v1);
    } 
    else{
      ESP.restart();
    }
    authHandler();
    Database.loop();
}

void authHandler(){
    unsigned long ms = millis();
    while (app.isInitialized() && !app.ready() && millis() - ms < 120 * 1000)
    {
        JWT.loop(app.getAuth());
    }
}