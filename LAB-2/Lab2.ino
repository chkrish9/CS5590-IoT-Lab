/Importing Libraries

#include <DHT.h>
#include <DHT_U.h>
#include <LiquidCrystal_PCF8574.h>//LCD
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>//Barometer
#include <SoftwareSerial.h>//Wifi
#include <Wire.h> 

//BMP
Adafruit_BMP280 bmp; // I2C
int light;

void setup() {  
Serial.begin(9600);
pinMode(A0,INPUT);
 //barometer
  if (!bmp.begin()) { 
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
    while (1);
  } 
}

void loop() {
  int NData = 2;       
  light=analogRead(A0);  
  String str_Payload;
  str_Payload += NData;
  str_Payload += "_" + String(light)+"l";
  str_Payload += "_" + String(bmp.readTemperature()).substring(0,2)+"t";
  str_Payload += "_";
  byte Payload[str_Payload.length()];
  int LengthFrameAPI = 18 + sizeof(Payload);
  int LengthPayload = sizeof(Payload);  
  Serial.print(str_Payload);
  delay(2000);
}