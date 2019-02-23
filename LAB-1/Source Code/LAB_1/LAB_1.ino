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
DHT dht(DHTPIN, DHTTYPE); //// Initialize DHT sensor for normal 16mhz Arduino
//LCD
LiquidCrystal_PCF8574 lcd(0x27);  // set the LCD address to 0x27 for a 16 chars and 2 line display 

// Setted wifi name and passwrod also included write API key for ThinkSpeak.

SoftwareSerial esp8266(9,10);
String AP = "Supu";       // Wifi Username
String PASS = "12345678"; // Wifi Password
String API = "O5R2BXR7OMESYUM9";//Thingspeak API key
String HOST = "184.106.153.149";// Thingspeak host name
String PORT = "80";//Thingspeak port

/*

  -Thingspeak Parameters.

  -These fileds are used to send the data to the Thingspeak.

*/

String Temp = "field1";
String Hum = "field2";
String BTemp = "field3";
String BPres = "field4";
String BAlt = "field5";
String Light = "field6";
String Dust = "field7";
String Pluse = "field8";

int countTrueCommand;
int countTimeCommand;
boolean found = false;
int valSensor = 1;

//BMP
Adafruit_BMP280 bmp; // I2C
//Temperature and Humidity Sensors variables

int chk;
float hum;  //Stores humidity value
float temp; //Stores temperature value
int light;
int pin = 8; 

//Dust Sensors

unsigned long duration;
unsigned long starttime;
unsigned long sampletime_ms = 2000;
unsigned long lowpulseoccupancy = 0;
float ratio = 0;
float concentration = 0; 

//Plus Sensor

int pulsePin = 0;                 // Pulse Sensor purple wire connected to analog pin 0
// Volatile Variables, used in the interrupt service routine!
volatile int BPM;                   // int that holds raw Analog in 0. updated every 2mS
volatile int Signal;                // holds the incoming raw data
volatile int IBI = 600;             // int that holds the time interval between beats! Must be seeded!
volatile boolean Pulse = false;     // "True" when heartbeat is detected. "False" when not a "live beat".
volatile boolean QS = false;        // becomes true when Arduino finds a beat.
// Regards Serial OutPut  -- Set This Up to your needs

static boolean serialVisual = true;   // Set to 'false' by Default.  Re-set to 'true' to see Arduino Serial Monitor ASCII Visual Pulse
volatile int rate[10];                    // array to hold last ten IBI values
volatile unsigned long sampleCounter = 0;          // used to determine pulse timing
volatile unsigned long lastBeatTime = 0;           // used to find IBI
volatile int P =512;                      // used to find peak in pulse wave, seeded
volatile int T = 512;                     // used to find trough in pulse wave, seeded
volatile int thresh = 525;                // used to find instant moment of heart beat, seeded
volatile int amp = 100;                   // used to hold amplitude of pulse waveform, seeded
volatile boolean firstBeat = true;        // used to seed rate array so we startup with reasonable BPM
volatile boolean secondBeat = false;      // used to seed rate array so we startup with reasonable BPM

int switchState1 = 0;
int switchState2 = 0;
int lcdOn = 0;
int pushed = 0; 
//Initial board setup code
void setup()
{
  Serial.begin(9600);
  //Starting the Temperature and Humidity Sensors.
  dht.begin();
  // When device is connected for the first time Dislpaying not gets displayed
  lcd.setBacklight(255);
  lcd.begin(16, 2);
  lcd.print("circuitdigest.com");
  lcd.setCursor(0,1);
  lcd.print("Connecting...");
  //wifi and thingspeak
  esp8266.begin(115200);
  sendCommand("AT",5,"OK");
  sendCommand("AT+CWMODE=1",5,"OK");
  sendCommand("AT+CWJAP=\""+ AP +"\",\""+ PASS +"\"",20,"OK");
  //barometer
  if (!bmp.begin()) { 
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
    while (1);
  }
  //Light sensor Input
  pinMode(A0,INPUT);
  //Dust sensor Input
  pinMode(8,INPUT);
  starttime = millis();
  pinMode(12, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(2, INPUT);
  pinMode(3, INPUT);
  interruptSetup();
}
void loop()
{
  lcd.clear();
  digitalWrite(12, LOW);//Green
  digitalWrite(4, LOW);//Red
  switchState1 = digitalRead(2);
  int  switchState2 = digitalRead(3);
  int BACKLIGHT_ON = digitalRead(6);
  //if button state is off then we are keep LEDs off except GREEN LED 
  if(switchState2 == HIGH  && lcdOn == LOW){  
  pushed = 1 - pushed;
  delay(2000);       
  }
  if(pushed == HIGH){
    lcd.setBacklight(BACKLIGHT_ON);
  }else{
    lcd.setBacklight(255);
  }
   if(switchState1 == HIGH){
  lcd.setBacklight(255);
  }
  delay(2000);
  //Read data and store it to variables hum and temp
  hum = dht.readHumidity();
  temp= dht.readTemperature();
  //Print temp and humidity values to serial monitor
  Serial.print("Humidity: ");
  Serial.print(hum);
  Serial.print(" %, Temp: ");
  Serial.print(temp);
  Serial.println(" Celsius");
  /*Printing Temperature and Humidity sensor data to LCD*/

  lcd.setCursor(0,0);
  lcd.print("Temp & Humidity");
  lcd.setCursor(0,1);
  lcd.print("Sensor");
  delay(2000); //Delay 2 sec.
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Humidity: ");
  lcd.print(hum);
  lcd.print(" %");
  lcd.setCursor(0,1);
  lcd.print("Temp: ");
  lcd.print(temp);
  lcd.print( (char)223);
  lcd.print("C");
  delay(2000);
  if(temp > 21){
    digitalWrite(12, HIGH);//Green
  }else{
  digitalWrite(4, HIGH);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Warning: Weather");
  lcd.setCursor(0,1);
  lcd.print("is cold outside!");
  delay(2000);
  }
  delay(5000); //Delay 2 sec.
  lcd.clear();
  digitalWrite(12, LOW);//Green
  digitalWrite(4, LOW);//Red
  //Barometer Sensor
  Serial.print("Barometer sensor");
  lcd.print("Barometer sensor");
  delay(2000);
  Serial.print(F("Temperature = "));
  Serial.print(bmp.readTemperature());
  Serial.println(" *C");

  /*Printing Barometer Temperature sensor data to LCD*/

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Temp:");
  lcd.print(bmp.readTemperature());
  lcd.print(" *C");
  if(bmp.readTemperature() > 21){
    digitalWrite(12, HIGH);//Green      
  }else{
    digitalWrite(4, HIGH);
    lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Warning: Weather");
  lcd.setCursor(0,1);
  lcd.print("is cold outside!");
  delay(2000);
  }

  delay(2000);
  lcd.clear();
  digitalWrite(12, LOW);//Green
  digitalWrite(4, LOW);//Red
  Serial.print(F("Pressure = "));
  Serial.print(bmp.readPressure());
  Serial.println(" Pa");
  /*Printing Barometer Pressure sensor data to LCD*/
  lcd.print("Pres: ");
  lcd.print(bmp.readPressure());
  lcd.setCursor(0,1);
  lcd.print(" Pa");
  if(bmp.readPressure() > 21){
    digitalWrite(12, HIGH);//Green
  }else{
    digitalWrite(4, HIGH);
    lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Warning: Pres");
  lcd.setCursor(0,1);
  lcd.print("is low outside!");
  delay(2000);
  }
  delay(2000);
  lcd.clear();
  digitalWrite(12, LOW);//Green
  digitalWrite(4, LOW);//Red
  Serial.print(F("Approx altitude = "));
  Serial.print(bmp.readAltitude(1013.25)); // this should be adjusted to your local forcase
  Serial.println(" m");
  /*Printing Barometer Altitude sensor data to LCD*/

  lcd.setCursor(0,0);
  lcd.print("Alt: ");
  lcd.print(bmp.readAltitude(1013.25));
  lcd.print(" m");
  Serial.println();
  if(bmp.readAltitude(1013.25) > 21){
    digitalWrite(12, HIGH);//Green
  }else{
    digitalWrite(4, HIGH);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Warning: Alt");
  lcd.setCursor(0,1);
  lcd.print("is too low");
  delay(2000);
  }
  delay(2000);

  //Light Sensor
  light=analogRead(A0);
  Serial.println("Light = " + String(light));
  /*Printing Light sensor data to LCD*/

  lcd.clear();
  digitalWrite(12, LOW);//Green
  digitalWrite(4, LOW);//Red
  lcd.print("Light sensor");
  delay(2000);
  lcd.clear();
  lcd.print("Light: ");
  lcd.print(String(light));
  delay(2000);
  if(light > 21){
    digitalWrite(12, HIGH);//Green
  }else{
    digitalWrite(4, HIGH);
    lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Warning: Light");
  lcd.setCursor(0,1);
  lcd.print("is too high!");
  delay(2000);
  }
  //Dust Sensor
  duration = pulseIn(pin, LOW);
  lowpulseoccupancy = lowpulseoccupancy+duration;
  if ((millis()-starttime) >= sampletime_ms) //if the sampel time = = 30s
  {
    ratio = lowpulseoccupancy/(sampletime_ms*10.0);
    concentration = 1.1*pow(ratio,3)-3.8*pow(ratio,2)+520*ratio+0.62;
    /*Printing Dust sensor data to Serial Monitor*/

    Serial.print("Concentration = ");
    Serial.print(concentration);
    Serial.println(" pcs/0.01cf");
    Serial.println("\n");
    lowpulseoccupancy = 0;
    starttime = millis();
    /*Printing Dust sensor data to LCD*/
    lcd.clear();
    digitalWrite(12, LOW);//Green
    digitalWrite(4, LOW);//Red
    lcd.print("Pulse sensor");
  delay(2000);
  lcd.clear();
//  updatebeat();
  lcd.print("BPM = ");
  lcd.print(BPM);
  delay(2000);
   if(BPM<80){ //Red
      digitalWrite(4, HIGH);
      digitalWrite(12, LOW);
      Serial.println("below 80");
       lcd.setCursor(0,0);
        lcd.print("Warning: Pulse");      
        lcd.setCursor(0,1);
        lcd.print("is too high");
        delay(2000);
    }

    else if(BPM>100){ //Red
        digitalWrite(12, LOW);
        digitalWrite(4, HIGH);
        Serial.println("above 100");
        lcd.clear();        
        lcd.setCursor(0,0);
        lcd.print("Warning: Pulse");      
        lcd.setCursor(0,1);
        lcd.print("is too high");
        delay(2000);
    }
    lcd.clear();
    lcd.print("Dust sensor");
    delay(2000);
    lcd.clear();
    lcd.print("Dust: ");
    lcd.print(concentration);
    delay(2000);
    if(concentration < 1){
      digitalWrite(12, HIGH);//Green
    }else{
      digitalWrite(4, HIGH);// Red
      lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Warning: Dust");
  lcd.setCursor(0,1);
  lcd.print("is high outside!");
  delay(2000);
    }
    lcd.print(" pcs/0.01cf");
    /*

    Sending the below data to ThingSpeak:

    -Light.

    -Dust.

   */
    String getData1 = "GET /update?api_key="+ API +"&"+ Light +"="+String(light)+"&"+ Dust +"="+String(concentration)+"&"+ Temp +"="+String(temp) +"&"+ Hum +"="+String(hum)+"&"+ BTemp +"="+String(bmp.readTemperature())+"&"+ BPres +"="+String(bmp.readPressure())+"&"+ BAlt +"="+String(bmp.readAltitude(1013.25))+"&"+Pluse+"="+String(BPM);

    sendCommand("AT+CIPMUX=1",5,"OK");
    sendCommand("AT+CIPSTART=0,\"TCP\",\""+ HOST +"\","+ PORT,15,"OK");
    sendCommand("AT+CIPSEND=0," +String(getData1.length()+4),4,">");
    esp8266.println(getData1);
    countTrueCommand++;
    sendCommand("AT+CIPCLOSE=0",5,"OK");
  }else{
    String getData1 = "GET /update?api_key="+ API +"&"+ Light +"="+String(light)+"&"+ Temp +"="+String(temp) +"&"+ Hum +"="+String(hum)+"&"+ BTemp +"="+String(bmp.readTemperature())+"&"+ BPres +"="+String(bmp.readPressure())+"&"+ BAlt +"="+String(bmp.readAltitude(1013.25))+"&"+Pluse+"="+String(BPM);

    sendCommand("AT+CIPMUX=1",5,"OK");
    sendCommand("AT+CIPSTART=0,\"TCP\",\""+ HOST +"\","+ PORT,15,"OK");
    sendCommand("AT+CIPSEND=0," +String(getData1.length()+4),4,">");
    esp8266.println(getData1);
    countTrueCommand++;
    sendCommand("AT+CIPCLOSE=0",5,"OK");
  }
  lcd.clear();
  digitalWrite(12, LOW);//Green
  digitalWrite(4, LOW);//Red
  /*

    Sending the below data to ThingSpeak:

    -Temperature.

    -Humidity.

    -Barometer Temperature.

    -Pressure.

    -Altitude.

  */
  String getData = "GET /update?api_key="+ API +"&"+ Light +"="+String(light)+"&"+ Dust +"="+String(concentration)+"&"+ Temp +"="+String(temp) +"&"+ Hum +"="+String(hum)+"&"+ BTemp +"="+String(bmp.readTemperature())+"&"+ BPres +"="+String(bmp.readPressure())+"&"+ BAlt +"="+String(bmp.readAltitude(1013.25))+"&"+Pluse+"="+String(BPM);

  sendCommand("AT+CIPMUX=1",5,"OK");
  sendCommand("AT+CIPSTART=0,\"TCP\",\""+ HOST +"\","+ PORT,15,"OK");
  sendCommand("AT+CIPSEND=0," +String(getData.length()+4),4,">");
  esp8266.println(getData);
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

ISR(TIMER2_COMPA_vect){                      // triggered when Timer2 counts to 124

  cli();                                     // disable interrupts while we do this

  Signal = analogRead(pulsePin);              // read the Pulse Sensor

  sampleCounter += 2;                         // keep track of the time in mS

  int N = sampleCounter - lastBeatTime;       // monitor the time since the last beat to avoid noise

    //  find the peak and trough of the pulse wave

  if(Signal < thresh && N > (IBI/5)*3){      // avoid dichrotic noise by waiting 3/5 of last IBI

    if (Signal < T){                         // T is the trough

      T = Signal;                            // keep track of lowest point in pulse wave

    }

  }

  if(Signal > thresh && Signal > P){        // thresh condition helps avoid noise

    P = Signal;                             // P is the peak

  }                                         // keep track of highest point in pulse wave

  //  NOW IT'S TIME TO LOOK FOR THE HEART BEAT

  // signal surges up in value every time there is a pulse

  if (N > 250){                                   // avoid high frequency noise

    if ( (Signal > thresh) && (Pulse == false) && (N > (IBI/5)*3) ){      

      Pulse = true;                               // set the Pulse flag when there is a pulse

      //digitalWrite(blinkPin,HIGH);                // turn on pin 13 LED

      IBI = sampleCounter - lastBeatTime;         // time between beats in mS

      lastBeatTime = sampleCounter;               // keep track of time for next pulse

      if(secondBeat){                        // if this is the second beat

        secondBeat = false;                  // clear secondBeat flag

        for(int i=0; i<=9; i++){             // seed the running total to get a realistic BPM at startup

          rate[i] = IBI;                    

       }

      }

      if(firstBeat){                         // if it's the first time beat is found

        firstBeat = false;                   // clear firstBeat flag

        secondBeat = true;                   // set the second beat flag

        sei();                               // enable interrupts again

        return;                              // IBI value is unreliable so discard it

      } 

      word runningTotal = 0;                  // clear the runningTotal variable  

      for(int i=0; i<=8; i++){                // shift data in the rate array

        rate[i] = rate[i+1];                  // and drop the oldest IBI value

        runningTotal += rate[i];              // add up the 9 oldest IBI values

      }

      rate[9] = IBI;                          // add the latest IBI to the rate array

      runningTotal += rate[9];                // add the latest IBI to runningTotal

      runningTotal /= 10;                     // average the last 10 IBI values

      BPM = 60000/runningTotal;               // how many beats can fit into a minute? that's BPM!

      QS = true;                              // set Quantified Self flag

      // QS FLAG IS NOT CLEARED INSIDE THIS ISR

      if(BPM<80){ //Red

//        digitalWrite(4, HIGH);

//        digitalWrite(12, LOW);

        Serial.println("below 80");

      }

      else if(BPM>100){ //Red

//          digitalWrite(12, HIGH);

//          digitalWrite(4, LOW);

          Serial.println("above 100");

      }

    

    }                     

  }

  if (Signal < thresh && Pulse == true){   // when the values are going down, the beat is over

    //digitalWrite(blinkPin,LOW);            // turn off pin 13 LED

    Pulse = false;                         // reset the Pulse flag so we can do it again

    amp = P - T;                           // get amplitude of the pulse wave

    thresh = amp/2 + T;                    // set thresh at 50% of the amplitude

    P = thresh;                            // reset these for next time

    T = thresh;

  }

  if (N > 2500){                           // if 2.5 seconds go by without a beat

    thresh = 512;                          // set thresh default

    P = 512;                               // set P default

    T = 512;                               // set T default

    lastBeatTime = sampleCounter;          // bring the lastBeatTime up to date      

    firstBeat = true;                      // set these to avoid noise

    secondBeat = false;                    // when we get the heartbeat back

  }

  sei();   

  // enable interrupts when youre done!

}// end

 

void interruptSetup(){   

  TCCR2A = 0x02;     // DISABLE PWM ON DIGITAL PINS 3 AND 11, AND GO INTO CTC MODE

  TCCR2B = 0x06;     // DON'T FORCE COMPARE, 256 PRESCALER

  OCR2A = 0X7C;      // SET THE TOP OF THE COUNT TO 124 FOR 500Hz SAMPLE RATE

  TIMSK2 = 0x02;     // ENABLE INTERRUPT ON MATCH BETWEEN TIMER2 AND OCR2A

  sei();             // MAKE SURE GLOBAL INTERRUPTS ARE ENABLED    

}



void updatebeat(){

  String cmd = "AT+CIPSTART=\"TCP\",\"";

  cmd += HOST;

  cmd += "\",80";

  Serial.println(cmd);

  esp8266.println(cmd);

  delay(2000);

  if(esp8266.find("Error")){

    return;

  }

  // update messages

  //cmd = msg ;

  cmd += "&field1="; 

  cmd += BPM;

  cmd += "\r\n";

  Serial.print("AT+CIPSEND=");

  esp8266.print("AT+CIPSEND=");

  Serial.println(cmd.length());

  esp8266.println(cmd.length());

// if(esp8266.find(">")){

    Serial.print(cmd);

    esp8266.print(cmd);

}



 
