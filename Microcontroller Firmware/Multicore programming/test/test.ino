#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include "DHT.h"
#include "MQUnifiedsensor.h"// MQ135 Enable
#include <HardwareSerial.h>

#define BLYNK_TEMPLATE_ID "TMPLqo4MJG8x"
#define BLYNK_DEVICE_NAME "IoT"
#define BLYNK_AUTH_TOKEN "gkp3KEPrshlwgZ00bzexG33Jd64ayy_U"

char ssid[]= "MZ-Faculty";
char pass[]= "uit91258";


#ifndef ESP32
#pragma message(THIS EXAMPLE IS FOR ESP32 ONLY!)
#error Select ESP32 board.
#endif

//Definitions
#define dhtPin 17
#define DHTTYPE DHT22
#define mqPin 36
#define lcd_tx 1 //tx0
#define lcd_rx 3 //rx0
#define litter 16 //rx2
#define pump 15 //d15
#define valve 02 //d2

#define board ("ESP-32")          // Development Board
#define Voltage_Resolution 3.3    // VCC
#define pin 15                    // Analog Input 0 of ESP32
#define type "MQ-135"             // MQ135
#define ADC_Bit_Resolution 12     // For ESP32
#define RatioMQ135CleanAir 3.6    // RS / R0 = 3.6 ppm

float NH4 = 3;
int h, t, nh;


MQUnifiedsensor MQ135(board, Voltage_Resolution, ADC_Bit_Resolution, pin, type);
DHT dht(dhtPin, DHTTYPE);


void setup(){
  Serial.begin(115200);
  xTaskCreatePinnedToCore(
    Task1Code, //function to implement task
    "Task1",  //name of task
    10000,  //stack size in words
    NULL, //Input params of task
    0, //priority of task
    NULL, //task handle
    0 //cpu core
  );
  xTaskCreatePinnedToCore(
    Task2Code, //function to implement task
    "Task2",  //name of task
    10000,  //stack size in words
    NULL, //Input params of task
    0, //priority of task
    NULL, //task handle
    1 //cpu core
  );
    Serial.begin(9600);
  // mySerial.begin(115200);

//Set math model to calculate the PPM concentration and the value of constants
  MQ135.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ135.setA(110.47); MQ135.setB(-2.862); // Configure the equation to to calculate C02 concentration
  dht.begin();
    /*
    Exponential regression:
  GAS      | a      | b
  CO       | 605.18 | -3.937  
  Alcohol  | 77.255 | -3.18 
  CO2      | 110.47 | -2.862
  Toluen   | 44.947 | -3.445
  NH4      | 102.2  | -2.473
  Aceton   | 34.668 | -3.369
  */
  
  MQ135.init(); 
  Serial.print("Calibrating please wait.");
  float calcR0 = 0;
  for(int i = 1; i<=10; i ++)
  {
    MQ135.update(); // Update data, the arduino will read the voltage from the analog pin
    calcR0 += MQ135.calibrate(RatioMQ135CleanAir);
    Serial.print(".");
  }
  MQ135.setR0(calcR0/10);
  // Serial.println("  done!.");
  
  if(isinf(calcR0)) {Serial.println("Warning: Conection issue, R0 is infinite (Open circuit detected) please check your wiring and supply"); while(1);}
  if(calcR0 == 0){Serial.println("Warning: Conection issue found, R0 is zero (Analog pin shorts to ground) please check your wiring and supply"); while(1);}
  MQ135.serialDebug(true);
  MQ135.setA(102.2); MQ135.setB(-2.473); // Configure the equation to to calculate NH4 concentration

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, "blynk.cloud", 80);

  delay(1000);
}


void Task1Code(void * pvParameters){

  for(;;){
  int currentTime = millis();
  int prevTime;
  if(currentTime-prevTime >= 3000){
      prevTime = currentTime - millis();
      if(NH4 == 3){
        NH4 = 4;
      }
      else if(NH4 == 4){
        NH4 = 3;
      }
  }
  //code for task1 infinite loop like loop()
  MQ135.update(); // Update data, the arduino will read the voltage from the analog pin
  MQ135.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup
  h = dht.readHumidity();
  t = dht.readTemperature();
  Blynk.run();
  Blynk.virtualWrite(V0, t);
  Blynk.virtualWrite(V1, h);
  Blynk.virtualWrite(V2,nh);

  }
}
void Task2Code(void * pvParameters){
  for(;;){
    //code for task1 infinite loop like loop()
    //temprature page
Serial.print("n5.val=");
  Serial.print(t);
Serial.write(0xff);
Serial.write(0xff);
Serial.write(0xff);
  // float bar = 100*(t/125+4);
  Serial.print("j0.val=");
  Serial.print(t);
Serial.write(0xff);
Serial.write(0xff);
Serial.write(0xff);
//humidity page
Serial.print("x4.val=");
  Serial.print(h);
Serial.write(0xff);
Serial.write(0xff);
Serial.write(0xff);
Serial.print("z0.val=");
  Serial.print((h*100)/360);
Serial.write(0xff);
Serial.write(0xff);
Serial.write(0xff);
//Ammonia Page
Serial.print("n6.val=");
nh=NH4;
  Serial.print(nh);
Serial.write(0xff);
Serial.write(0xff);
Serial.write(0xff);
  }
}

void loop(){

}