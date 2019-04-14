//Importing Libraries

#include <DHT.h>
#include <DHT_U.h>
#include <LiquidCrystal_PCF8574.h>//LCD
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>//Barometer
#include <SoftwareSerial.h>//Wifi
#include <Wire.h> 
//Constants

#define DHTPIN 7     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)


// Setted wifi name and passwrod also included write API key for ThinkSpeak.

SoftwareSerial esp8266(9,10);
String AP = "Samarkand";       // Wifi Username
String PASS = "TimurIsTheBest335580"; // Wifi Password
String API = "DH2O636ZA35XHELM";//Thingspeak API key
String HOST = "184.106.153.149";// Thingspeak host name
String PORT = "80";//Thingspeak port

String Light = "field1";
int countTrueCommand;
int countTimeCommand;
boolean found = false;
int valSensor = 1;
int light;
int pin = 8; 

void setup()
{
  Serial.begin(9600);
  
  // When device is connected for the first time Dislpaying not gets displayed
  
  //wifi and thingspeak
  esp8266.begin(115200);
  sendCommand("AT",5,"OK");
  sendCommand("AT+CWMODE=1",5,"OK");
  sendCommand("AT+CWJAP=\""+ AP +"\",\""+ PASS +"\"",20,"OK");
  
  //Light sensor Input
  pinMode(A0,INPUT);
 
  
}
void loop()
{
  
 
  
  light=analogRead(A0);
  Serial.println("Light = " + String(light));
  

 
    String getData1 = "GET /update?api_key="+ API +"&"+ Light +"="+String(light);

    sendCommand("AT+CIPMUX=1",5,"OK");
    sendCommand("AT+CIPSTART=0,\"TCP\",\""+ HOST +"\","+ PORT,15,"OK");
    sendCommand("AT+CIPSEND=0," +String(getData1.length()+4),4,">");
    esp8266.println(getData1);
    countTrueCommand++;
    sendCommand("AT+CIPCLOSE=0",5,"OK");
   
}

//Setting up the wifi

void sendCommand(String command, int maxTime, char readReplay[])
{
  Serial.print(countTrueCommand);
  Serial.print(". at command => ");
  Serial.print(command);
  Serial.print(" ");
  while(countTimeCommand < (maxTime*1))
  {
    esp8266.println(command);//at+cipsend
    if(esp8266.find(readReplay))//ok
    {
      found = true;
      break;
      }
    countTimeCommand++;
  }

  if(found == true)
  {
    Serial.println("OYI");
    countTrueCommand++;
    countTimeCommand = 0;
  }
  if(found == false)
  {
    Serial.println("Fail");
    countTrueCommand = 0;
    countTimeCommand = 0;
  }
  found = false;
}       

 



 









 
