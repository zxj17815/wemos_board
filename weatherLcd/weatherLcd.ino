/**
   BasicHTTPSClient.ino

    Created on: 20.08.2018

*/
#include <map>
#include <string>
#include <iostream>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h> // i2c LCD i/o class header
// Fingerprint for demo URL, expires on June 2, 2021, needs to be updated well before this date
const uint8_t fingerprint[20] = {0x40, 0xaf, 0x00, 0x6b, 0xec, 0x90, 0x22, 0x41, 0x8e, 0xa3, 0xad, 0xfa, 0x1a, 0xe8, 0x25, 0x41, 0x1d, 0x1a, 0x54, 0xb3};

// declare i2c address and constructor for specified i2c address
const int i2c_addr = 0x27;
hd44780_I2Cexp lcd(i2c_addr); // use device at this address

// LCD geometry
const int LCD_COLS = 16;
const int LCD_ROWS = 2;
int Count = 1;
int flag = 0;

const std::map<std::string, const char *> winddirectionMap = {
    {"无风向", "no direction"},
    {"东北", "northeast"},
    {"东", "east"},
    {"东南", "southeast"},
    {"南", "south"},
    {"西南", "southwest"},
    {"西", "west"},
    {"西北", "northwest"},
    {"北", "north"},
    {"旋转不定", "uncertainty"},
};

const char *humidity;
const char *temperature;
const char *windpower;
const char *wind;

int switch_flag_1h = 3600;
int switch_flag_30s = 0;

ESP8266WiFiMulti WiFiMulti;

void setup()
{
    Serial.begin(9600);
    int status;
    status = lcd.begin(LCD_COLS, LCD_ROWS);
    if (status) // non zero status means it was unsuccesful
    {
        // begin() failed so blink error code using the onboard LED if possible
        hd44780::fatalError(status); // does not return
    }
    // Print a message to the LCD
    lcd.print("WiFi connecting...");
    digitalWrite(LED_BUILTIN, HIGH);
    WiFi.mode(WIFI_STA);
    WiFiMulti.addAP("shebei", "shebei2018");
    while (WiFiMulti.run() != WL_CONNECTED)
    {
        delay(2000);
    }
    digitalWrite(LED_BUILTIN, LOW);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi connected");
    lcd.setCursor(0, 1);
    delay(2000);
    lcd.print("Finding message");

    char *hum = "东北";
    Serial.println(winddirectionMap.at(hum));
}

void loop()
{
    // wait for WiFi connection
    delay(1000);
    if (switch_flag_1h == 3600)
    {
        switch_flag_1h = 0;
        switch_flag_30s = 0;
        if (WiFiMulti.run() == WL_CONNECTED)
        {
            std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);

            client->setFingerprint(fingerprint);
            // Or, if you happy to ignore the SSL certificate, then use the following line instead:
            client->setInsecure();

            HTTPClient https;
            if (https.begin(*client, "https://restapi.amap.com/v3/weather/weatherInfo?city=330481&key=key"))
            { // HTTPS
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
                        humidity = (const char *)doc["lives"][0]["humidity"];
                        temperature = (const char *)doc["lives"][0]["temperature"];
                        windpower = (const char *)doc["lives"][0]["windpower"];
                        wind = (const char *)doc["lives"][0]["winddirection"];
                        Serial.println(humidity);
                        Serial.println(temperature);
                        Serial.println(windpower);
                        Serial.println(wind);
                        lcd.clear();
                        lcd.setCursor(0, 0);
                        lcd.print("humidity:");
                        lcd.print(humidity);
                        lcd.write(0x25);
                        lcd.print("hr");
                        lcd.setCursor(0, 1);
                        lcd.print("temperature:");
                        lcd.print(temperature);
                        lcd.write(0xDF);
                        lcd.write("C");
                    }
                }
                else
                {
                    Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
                    Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
                    lcd.setCursor(0, 1);
                    lcd.print("Http Error");
                }

                https.end();
            }
            else
            {
                Serial.printf("[HTTPS] Unable to connect\n");
                lcd.setCursor(0, 1);
                lcd.print("WiFi Error");
            }
        }
    }
    else
    {
        switch_flag_30s++;
        if (switch_flag_30s == 15)
        {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("humidity:");
            lcd.print(humidity);
            lcd.write(0x25);
            lcd.print("hr");
            lcd.setCursor(0, 1);
            lcd.print("temperature:");
            lcd.print(temperature);
            lcd.write(0xDF);
            lcd.write("C");
        }
        if (switch_flag_30s == 30)
        {
            Serial.println(windpower);
            std::string direction(wind);
            Serial.println(winddirectionMap.at(direction));
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("wind:");
            lcd.print(winddirectionMap.at(direction));
            lcd.setCursor(0, 1);
            lcd.print("windpower:");
            if (windpower == "≤3")
            {
                lcd.print(0x3c);
                lcd.print(0x3d);
                lcd.print("3");
            }
            else
            {
                lcd.print(windpower);
            }
            switch_flag_30s = 0;
        }
    }
}