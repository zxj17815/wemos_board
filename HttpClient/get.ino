#include <ArduinoJson.h>

/**
   BasicHTTPSClient.ino

    Created on: 20.08.2018

*/

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
// Fingerprint for demo URL, expires on June 2, 2021, needs to be updated well before this date
const uint8_t fingerprint[20] = {0x40, 0xaf, 0x00, 0x6b, 0xec, 0x90, 0x22, 0x41, 0x8e, 0xa3, 0xad, 0xfa, 0x1a, 0xe8, 0x25, 0x41, 0x1d, 0x1a, 0x54, 0xb3};

ESP8266WiFiMulti WiFiMulti;

void setup()
{

    Serial.begin(9600);
    // Serial.setDebugOutput(true);

    Serial.println();
    Serial.println();
    Serial.println();

    for (uint8_t t = 4; t > 0; t--)
    {
        Serial.printf("[SETUP] WAIT %d...\n", t);
        Serial.flush();
        delay(1000);
    }
    digitalWrite(LED_BUILTIN, HIGH);
    WiFi.mode(WIFI_STA);
    WiFiMulti.addAP("shebei", "shebei2018");
    while (WiFiMulti.run() != WL_CONNECTED)
    {
        delay(1500);
        Serial.println("WiFi connecting...");
    }
    digitalWrite(LED_BUILTIN, LOW);
    Serial.println("WiFi connected");
}

void loop()
{
    // wait for WiFi connection
    if (WiFiMulti.run() == WL_CONNECTED)
    {

        std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);

        client->setFingerprint(fingerprint);
        // Or, if you happy to ignore the SSL certificate, then use the following line instead:
        client->setInsecure();

        HTTPClient https;

        Serial.print("[HTTPS] begin...\n");
        if (https.begin(*client, "https://restapi.amap.com/v3/weather/weatherInfo?city=330481&key=4f1d511d26bbbbba8785af2b24101850"))
        { // HTTPS

            Serial.print("[HTTPS] GET...\n");
            // start connection and send HTTP header
            int httpCode = https.GET();

            // httpCode will be negative on error
            if (httpCode > 0)
            {
                // HTTP header has been send and Server response header has been handled
                Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

                // file found at server
                if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
                {
                    String payload = https.getString();
                    Serial.println(payload);
                    DynamicJsonDocument doc(1024);
                    DeserializationError error = deserializeJson(doc, payload);
                    if (error)
                    {
                        Serial.print(F("deserializeJson() failed: "));
                        Serial.println(error.f_str());
                    }
                    String lives = doc["lives"][0]["weather"];
                    Serial.println(lives);
                }
            }
            else
            {
                Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
                Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
            }

            https.end();
        }
        else
        {
            Serial.printf("[HTTPS] Unable to connect\n");
        }
    }

    Serial.println("Wait 30s before next round...");
    delay(30000);
}