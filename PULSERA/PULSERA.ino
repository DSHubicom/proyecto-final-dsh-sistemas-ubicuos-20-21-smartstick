#include "Adafruit_FONA.h"
#include <WiFi.h>
// standard pins for the shield, adjust as necessary
#define FONA_RX 2
#define FONA_TX 3
#define FONA_RST 4

// We default to using software serial. If you want to use hardware serial
// (because softserial isnt supported) comment out the following three lines
// and uncomment the HardwareSerial line
#include <SoftwareSerial.h>
SoftwareSerial fonaSS (FONA_TX, FONA_RX);
SoftwareSerial *fonaSerial = &fonaSS;

Adafruit_FONA fona = Adafruit_FONA(FONA_RST);

const int CUTOFF = -70;

void tone(byte pin, int freq) {
  ledcSetup(0, 2000, 8); // setup beeper
  ledcAttachPin(pin, 0); // attach beeper
  ledcWriteTone(0, freq); // play tone
}

void setup() {

  // Poner el módulo WiFi en modo station y desconectar de cualquier red a la que pudiese estar previamente conectado
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  pinMode(5, OUTPUT);

  while (! Serial);

  Serial.begin(115200);
  Serial.println(F("Adafruit FONA 808 & 3G GPS demo"));
  Serial.println(F("Initializing FONA... (May take a few seconds)"));

  fonaSerial->begin(4800);
  if (! fona.begin(*fonaSerial)) {
    Serial.println(F("Couldn't find FONA"));
    while (1);
  }
  Serial.println(F("FONA is OK"));
  // Try to enable GPRS


  Serial.println(F("Enabling GPS..."));
  if (!fona.enableGPS(true))
    Serial.println(F("Failed to turn off"));
}

void searchDevice() {
  //Get the Wifi networks of the enviroment
  Serial.println("Escaneando el entorno...");
  int n = WiFi.scanNetworks();
  Serial.print(n);
  Serial.println(" redes en el rango del dispositivo");
  for (int i = 0; i < n; ++i) {
    //Print SSID (name) y RSSI (power) for each found network
    Serial.println((int)WiFi.RSSI(i));
    Serial.println(WiFi.SSID(i));
    //Check if SSID is the name of our device network and the value of RSSI
    if (WiFi.SSID(i) == "yourAP" and (int)WiFi.RSSI(i) < CUTOFF) {
      //Tone if the device is far away
      tone(5, 2000);
    } else {
      if (WiFi.SSID(i) == "yourAP") {
        //Stop tone if the device is near
        tone(5, 0);
      }
    }
  }
}

void getGPSlocation() {
  float latitude, longitude, speed_kph, heading, speed_mph, altitude;

  // if you ask for an altitude reading, getGPS will return false if there isn't a 3D fix
  boolean gps_success = fona.getGPS(&latitude, &longitude, &speed_kph, &heading, &altitude);

  if (gps_success) {
    //We are only interested in lat and long
    Serial.print("GPS lat:");
    Serial.println(latitude, 6);
    Serial.print("GPS long:");
    Serial.println(longitude, 6);
    /*Serial.print("GPS speed KPH:");
      Serial.println(speed_kph);
      Serial.print("GPS speed MPH:");
      speed_mph = speed_kph * 0.621371192;
      Serial.println(speed_mph);
      Serial.print("GPS heading:");
      Serial.println(heading);
      Serial.print("GPS altitude:");
      Serial.println(altitude);*/
  } else {
    Serial.println("Waiting for FONA GPS 3D fix...");
  }
}

void loop() {
  getGPSlocation();
  searchDevice();
}
