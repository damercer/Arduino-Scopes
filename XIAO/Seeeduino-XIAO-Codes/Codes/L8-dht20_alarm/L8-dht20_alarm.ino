#include "DHT.h"
#include <Arduino.h>
#include <U8x8lib.h>//here we use the U8x8lib.h
#define DHTTYPE DHT20
DHT dht(DHTTYPE);
int buzzerPin = A3;
U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);
void setup() { 
  pinMode(buzzerPin , OUTPUT);//initialize the buzzer pin as an output
  Wire.begin();//
  dht.begin();//DHT begins
  u8x8.begin();//u8x8 begins
  u8x8.setPowerSave(0);  //power-saving mode is turned off, 1 is turned on, and nothing will be seen on the screen after power-saving mode is turned on
  u8x8.setFlipMode(1);
}
void loop() { 
  float temp, humi;//defines variables to store readings.
  temp = dht.readTemperature();//read the temperature value and store it in temp
  humi = dht.readHumidity();//read the humidity value and store it in the humi
  if (temp > 30 || humi < 40) {  //when the temperature is greater than 30 or the humidity is less than 40, the buzzer shall sound an alarm if one of the conditions is met
  tone(buzzerPin, 200, 200);
  }
  
  u8x8.setFont(u8x8_font_chroma48medium8_r);//set display font
  u8x8.setCursor(0, 33);//sets the position of the draw cursor (0, 33)
  u8x8.print("Temp:");//show temp at (0, 33)
  u8x8.print(temp);//display real-time temperature values
  u8x8.print("C");//displays the unit "c" of temperature
  u8x8.setCursor(0,50);
  u8x8.print("Humidity:");
  u8x8.print(humi);
  u8x8.print("%");
  u8x8.refreshDisplay();
  delay(200);
}
