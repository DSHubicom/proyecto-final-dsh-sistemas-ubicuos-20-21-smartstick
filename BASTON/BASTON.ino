#include "Adafruit_VL53L0X.h"
#include <Arduino.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "WiFi.h"
#include "esp_wifi.h"
#include <RTClib.h>
#include <HTTPClient.h>

const char* ssid = "yourAP";
const char* password =  "yourPassword";

//const char *ssid     = "MiTelefono";
//const char *password = "123456789";

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

int seconds = 0;

Adafruit_VL53L0X lox = Adafruit_VL53L0X();

const int PIN = 2;

DateTime t0;

const int BOTON = 25;
int val;
int sec;


void tone(byte pin, int freq) {
  ledcSetup(0, 2000, 8); // setup beeper
  ledcAttachPin(pin, 0); // attach beeper
  ledcWriteTone(0, freq); // play tone
}

void setup() {

  pinMode(25, INPUT_PULLUP);
  pinMode(BOTON, INPUT);
  pinMode(2, OUTPUT);

  Serial.begin(115200);
  // wait until serial port opens for native USB devices
  while (! Serial) {
    delay(1);
  }
  //Start and test Adafruit VL53L0X
  Serial.println("Adafruit VL53L0X test");
  if (!lox.begin()) {
    Serial.println(F("Failed to boot VL53L0X"));
    while (1);
  }
  Serial.println(F("VL53L0X API Started\n\n"));

  //Initialization of activity time
  DateTime t1 (2020, 12, 31, 0, 0, 0);
  t0 = t1;
  //Connect device to the other ESP32
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  //Initialization of seconds counter
  sec = 0;
}


void checkProcess(int measure, int state) {
  if (state != 4) {  //Check if status is not the "out of range value"
    //Print distance in mm
    Serial.print("Distance (mm): "); Serial.println(measure);
    //Do different checks to see if it should tone(2000) or not(0)
    if (measure < 100) {
      tone(2, 2000);
    } else {
      if (t0.second() <= 30) {
        tone(2, 2000);
      } else {
        tone(2, 0);
      }

    }
  } else {
    if (t0.second() <= 30) {
      tone(2, 2000);
    } else {
      tone(2, 0);
    }
    Serial.println(" out of range ");
  }
}

void getButton(){
  //HTTP GET request to the button server
  HTTPClient http;

  http.begin("http://192.168.4.1/button");
  int httpCode = http.GET();
  //Check the answer code
  if (httpCode > 0) {
    String payload = http.getString();
    Serial.print("Valor baston: ");
    Serial.println(payload);
    //Update the button value
    val = payload.toInt();
  }

  else {
    Serial.println("Error on HTTP request");
  }
  //Finish HTTP request
  http.end();
}

void loop() {
  delay(750);
  //Check if button value is HIGH to reset activity time
  if  (val == HIGH) {
    DateTime t2 (2020, 12, 31, 0, 0, 0);
    t0 = t2;
  }

  //Check how many millis have passed since last time and update the activity time
  if ((millis() / 1000) >= (sec + 1)) {
    sec = (millis() / 1000);
    DateTime t1 = t0 - TimeSpan(0, 0, 0, 1);
    t0 = t1;
  }
  //Print the inactivity time
  Serial.print("Inactividad:");
  Serial.println(t0.second()-30);

  //Read distance by sensor
  VL53L0X_RangingMeasurementData_t measure;
  Serial.print("Reading a measurement... ");
  lox.rangingTest(&measure, false);

  //Check the reading and the inactivity time, and tone if it is necessary
  checkProcess(measure.RangeMilliMeter, measure.RangeStatus);

  //Get button value with HTTP request
  getButton();
}
