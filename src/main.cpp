#include <Arduino.h>
#include <ArduinoJson.h>
#include "secrets.h"

////////////////////////////
//        ARDUINO_JSON  ////
////////////////////////////
DynamicJsonDocument rcv(512);
DynamicJsonDocument snd(512);
////////////////////////////////
///      FIRBASE        ////////
////////////////////////////////
#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

FirebaseData fbdo;
FirebaseData fbdo1;

FirebaseAuth auth;
FirebaseConfig config;
String path = "/Test/Json";
String path1 = "/Test/value";

void streamCallback(FirebaseStream data)
{
  // Serial.println("Stream Data1 available...");
  // Serial.println("STREAM PATH: " + data.streamPath());
  // Serial.println("EVENT PATH: " + data.dataPath());
  // Serial.println("DATA TYPE: " + data.dataType());
  // Serial.println("EVENT TYPE: " + data.eventType());
  // Serial.print("VALUE: ");
  // printResult(data);
  // Serial.println();
  if (data.dataPath() == "/m")
  {
    snd["m"] = data.stringData();
  }
  if (data.dataPath() == "/f")
  {
    snd["f"] = data.stringData();
  }
  serializeJson(snd, Serial2);
}
void streamTimeoutCallback(bool timeout)
{
  if (timeout)
  {
    Serial.println();
    Serial.println("Stream timeout, resume streaming...");
    Serial.println();
  }
}

void setup()
{
  Serial2.begin(9600);
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /////////////////////
  //Firebase
  ////////////////////
  /* Assign the project host and api key (required) */
  config.host = FIREBASE_PROJECT_HOST;
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

#if defined(ESP8266)
  //Set the size of WiFi rx/tx buffers in the case where we want to work with large data.
  fbdo.setBSSLBufferSize(1024, 1024);
#endif

  //Set the size of HTTP response buffers in the case where we want to work with large data.
  fbdo.setResponseSize(1024);

#if defined(ESP8266)
  //Set the size of WiFi rx/tx buffers in the case where we want to work with large data.
  fbdo1.setBSSLBufferSize(1024, 1024);
#endif

  //Set the size of HTTP response buffers in the case where we want to work with large data.
  fbdo1.setResponseSize(1024);
  if (!Firebase.RTDB.beginStream(&fbdo1, path1.c_str()))
  {
    Serial.println("------------------------------------");
    Serial.println("Can't begin stream connection...");
    Serial.println("REASON: " + fbdo1.errorReason());
    Serial.println("------------------------------------");
    Serial.println();
  }

  Firebase.RTDB.setStreamCallback(&fbdo1, streamCallback, streamTimeoutCallback);
  Firebase.RTDB.setwriteSizeLimit(&fbdo, "tiny");
}

void loop()
{
  if (Serial2.available() > 0)
  {
    deserializeJson(rcv, Serial2);
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Match with other script
    String type = rcv["type"];
    int person = rcv["person"];
    float temp = rcv["temp"];
    float flame = rcv["flame"];
    float gas = rcv["gas"];
    int prod = rcv["prod"];
    
    
    if (type == "null")
    {
      Serial2.flush();
    }
    else
    {
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<Firebase
      String jsonStr = "";
      FirebaseJson json1;
      FirebaseJsonData jsonObj;
      json1.set("person", person);
      json1.set("temp", temp);
      json1.set("flame", flame);
      json1.set("gas", gas);
      json1.set("prod", prod);

      json1.toString(jsonStr, true);
      Serial.println(jsonStr);
      if (Firebase.RTDB.set(&fbdo, path.c_str(), &json1))
      {
        Serial.println("[UPLOAD] PASSED");
      }
      else
      {
        Serial.println("[UPLOAD] FAILED");
        Serial.println("REASON: " + fbdo.errorReason());
        Serial.println("------------------------------------");
        Serial.println();
      }
    }

    // serializeJson(rcv, Serial);
    Serial.println();
    //   // Serial.println(time);
    //   // Serial.println(data1);
    //   // Serial.println(data2);
    // Serial.write(Serial2.read());
  }
}