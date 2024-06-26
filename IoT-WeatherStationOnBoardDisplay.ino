#include <M5Core2.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h> 
#include "M5UnitENV.h"

// Change the following constants to work with your enviroments
const char *ssid                  = "SSID"; // Your WiFi SSID
const char *password              = "PASSWORD"; // Your WiFi Password
const char *ApiKey                = "APIKEY"; // Your OpenWeatherMap API key
const char *OpenWeatherMapApiCall = "https://api.openweathermap.org/data/2.5/weather?id=2702259&units=metric&appid=APIKEY"; // Change "APIKEY" to your OpenWeatherMap API key 
const char *CityId                = "2702259"; // Your City ID, this one is for Kalmar, Sweden
const char *DatacakeApiCall       = "APICALL"; // Your Datacake API call
const char *DatacakeDeviceId      = "DEVICEID"; // Your Datacake Device ID

RTC_TimeTypeDef TimeStruct;
SHT3X sht3x;
String state;

// A nice welcoming message
void welcomeMessage() {
  M5.Lcd.setTextDatum(MC_DATUM);
  M5.Lcd.drawString("Hello World!", 160, 120, 2);
}

// A method for connecting the device to WiFi
void wifiConnect() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      M5.Lcd.print(".");
      Serial.println(".");
  }
  M5.Lcd.println("\nWiFi connected");
  Serial.println("\nWiFi connected");
}

// A method for initiating the ENV III sensor
void enviiiInit() {
  if (!sht3x.begin(&Wire, SHT3X_I2C_ADDR, 32, 33, 400000U)) {
      Serial.println("Couldn't find SHT3X");
  }
}

// A method for getting data from the ENV III sensor and printing it in the Serial Monitor and on the Device Display
void enviiiGetAndPrint() {
  if (sht3x.update()) {
    float temp = sht3x.cTemp;
    float humidity = sht3x.humidity;
    Serial.println("-----SHT3X-----");
    Serial.print("Temperature: ");
    Serial.print(temp);
    Serial.println(" degrees C");
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println("% rH");
    Serial.println("-----------------\r\n");

    M5.Lcd.print("Temp: ");
    if (temp > 20) {
      M5.Lcd.setTextColor(RED,BLACK);
    } else if (temp > 10 && temp < 20) {
      M5.Lcd.setTextColor(GREEN,BLACK);
    } else if (temp < 10) {
      M5.Lcd.setTextColor(BLUE,BLACK);
    }
    M5.Lcd.print(temp);
    M5.Lcd.setTextColor(WHITE,BLACK);
    M5.Lcd.println(" *C");
    M5.Lcd.println("-----------------\r\n");
    M5.Lcd.print("Humidity: ");
    M5.Lcd.setTextColor(BLUE,BLACK);
    M5.Lcd.print(humidity);
    M5.Lcd.setTextColor(WHITE,BLACK);
    M5.Lcd.println(" %");
    M5.Lcd.println("-----------------\r\n");
  }
}

// A method for getting data from the OpenWeatherMap API and printing it in the Serial Monitor and on the Device Display
void getAndPrintOutsideConditions() {
  if (WiFi.status() == WL_CONNECTED) {
 
    HTTPClient http;
 
    http.begin(OpenWeatherMapApiCall);
    int httpCode = http.GET();
 
    if (httpCode > 0) { 
 
      String payload = http.getString();

      DynamicJsonDocument jsonBuffer(1024);

      DeserializationError error = deserializeJson(jsonBuffer, payload);
      if (error) {
        Serial.print("Deserialization failed with code: ");
        Serial.println(error.c_str());
        return;
      }

      JsonArray array = jsonBuffer["weather"].as<JsonArray>();

      for(JsonVariant v : array) {
          Serial.println(v.as<int>());
      }

      float weatherID = (float)(jsonBuffer["weather"][0]["id"]);
      String description = (jsonBuffer["weather"][0]["description"]);  
      float temp = (float)(jsonBuffer["main"]["temp"]);
      int   humidity = jsonBuffer["main"]["humidity"];
      
      Serial.printf("Temperature = %.2f°C\r\n", temp);
      Serial.println(description);
      Serial.printf("Humidity    = %d %%\r\n", humidity);
      Serial.println(weatherID);

      M5.Lcd.println(description);
      M5.Lcd.println("-----------------\r\n");
      M5.Lcd.print("Temp = ");
      if (temp > 20) {
        M5.Lcd.setTextColor(RED,BLACK);
      } else if (temp > 10 && temp < 20) {
        M5.Lcd.setTextColor(GREEN,BLACK);
      } else if (temp < 10) {
        M5.Lcd.setTextColor(BLUE,BLACK);
      }
      M5.Lcd.print(temp);
      M5.Lcd.setTextColor(WHITE,BLACK);
      M5.Lcd.println(" *C");
      M5.Lcd.println("-----------------\r\n");
      M5.Lcd.print("Humidity = ");
      M5.Lcd.setTextColor(BLUE,BLACK);
      M5.Lcd.print(humidity);
      M5.Lcd.setTextColor(WHITE,BLACK);
      M5.Lcd.println(" %");
      M5.Lcd.println("-----------------\r\n");
    }
 
    http.end();
  }
}

// A method for posting the ENV III sensor data to Datacake
void postSensorData() {
  if (WiFi.status() == WL_CONNECTED) {

    if (sht3x.update()) {
      float temp = sht3x.cTemp;
      float humidity = sht3x.humidity;
 
      HTTPClient http;
  
      http.begin(DatacakeApiCall);
      
      // Prepare JSON document
      DynamicJsonDocument doc(2048);
      doc["device"] = DatacakeDeviceId;
      doc["temp"] = temp;
      doc["humidity"] = humidity;

      // Serialize JSON document
      String json;
      serializeJson(doc, json);

      http.POST(json);

      // Read response
      Serial.println(http.getString());

      http.end();
    }
  }  

}

// Method for setting the Core2s RTC, REMOVE CALL FROM SETUP/LOOP ONCE SET!
void setRtc() {

  TimeStruct.Hours   = 0;
  TimeStruct.Minutes = 0;
  TimeStruct.Seconds = 0;
  M5.Rtc.SetTime(&TimeStruct);

}

void setup() {
    M5.begin();

    M5.Lcd.setTextSize(3);
    M5.Lcd.setTextColor(WHITE,BLACK);

    welcomeMessage();
    wifiConnect();

    delay(1000);

    M5.Lcd.clear();

    enviiiInit();

    
}
void loop() {

  M5.update();
  M5.Lcd.setTextSize(3);
  M5.Lcd.setCursor(0, 0);

  if (state.equals("A")){
    M5.Lcd.println("Outside:");
    M5.Lcd.println("");
    getAndPrintOutsideConditions();
  } else if (state.equals("B")){
    M5.Lcd.println("Inside:");
    M5.Lcd.println("");
    enviiiGetAndPrint();
  } else if (state.equals("C")){
    M5.Lcd.setTextSize(5);
    M5.Lcd.setCursor(90, 20);
    M5.Lcd.print("Time:");

    M5.Lcd.setCursor(40, 100);
    M5.Rtc.GetTime(&TimeStruct);
    M5.Lcd.printf("%02d:%02d:%02d",TimeStruct.Hours, TimeStruct.Minutes, TimeStruct.Seconds);
  } else {
    M5.Lcd.println("Outside:");
    M5.Lcd.println("");
    getAndPrintOutsideConditions();
  }

  if (M5.BtnA.wasPressed()) {
    M5.Lcd.clear();
    state = "A";
  } else if(M5.BtnB.wasPressed()){
    M5.Lcd.clear();
    state = "B";
  } else if(M5.BtnC.wasPressed()){
    M5.Lcd.clear();
    state = "C";
  }

  postSensorData();

  delay(1000);
}
