#include <Arduino.h>
#include <U8x8lib.h>//use u8x8 library
#include <PCF8563.h>// RTC library
PCF8563 pcf;//define variable pcf
#include <Wire.h>
#include "DHT.h" // DHT library
#define DHTPIN 0  //the number of DHT pin
#define DHTTYPE DHT11 //DHT11
DHT dht(DHTPIN, DHTTYPE);
U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);
void setup() {
  Serial.begin(9600);
  u8x8.begin();//initialize the u8x8 library
  u8x8.setFlipMode(1);
  Wire.begin();
  pcf.init();//initialize clock
  pcf.stopClock();//stop clock
  //set the current time and date:
  pcf.setYear(21);
  pcf.setMonth(3);
  pcf.setDay(5);
  pcf.setHour(18);
  pcf.setMinut(53);
  pcf.setSecond(0);
  pcf.startClock();//start the clock
}

void loop() {
  float temp, humi;//defines variables to store readings.
  temp = dht.readTemperature();//read the temperature value and store it in temp
  humi = dht.readHumidity();//read the humidity value and store it in the humi
  Time nowTime = pcf.getTime();//get current time
  u8x8.setFont(u8x8_font_chroma48medium8_r); //set the font for display.
 
 //The current date, time, temperature and humidity are display in different coordinates on that OLED screen
  u8x8.setCursor(0, 0);
  u8x8.print(nowTime.day);
  u8x8.print("/");
  u8x8.print(nowTime.month);
  u8x8.print("/");
  u8x8.print("20");
  u8x8.print(nowTime.year);
  u8x8.setCursor(0, 1);
  u8x8.print(nowTime.hour);
  u8x8.print(":");
  u8x8.print(nowTime.minute);
  u8x8.print(":");
  u8x8.println(nowTime.second);
  delay(1000);
  u8x8.setCursor(0, 2);
  u8x8.print("Temp:");
  u8x8.print(temp);
  u8x8.print("C");
  u8x8.setCursor(0,3);
  u8x8.print("Humidity:");
  u8x8.print(humi);
  u8x8.print("%");
  u8x8.refreshDisplay();
  delay(200);
}
