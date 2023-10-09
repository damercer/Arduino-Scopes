#include "DHT.h"
#include <Arduino.h>
#include <U8x8lib.h>
 
#define DHTPIN 0     // what pin we're connected to
#define DHTTYPE DHT11   // DHT 11 
DHT dht(DHTPIN, DHTTYPE);
int buzzerPin = A3;
U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);
 
void setup() {
  Serial.begin(9600); 
  Serial.println("DHTxx test!");
  dht.begin();
  u8x8.begin();
  u8x8.setPowerSave(0);  
  u8x8.setFlipMode(1);
  pinMode(buzzerPin , OUTPUT);
}
 
void loop() {
  float temp, humi;
  temp = dht.readTemperature();
  humi = dht.readHumidity();
if (temp > 30 || humi < 40) {
   tone(buzzerPin, 200, 200);
}
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.setCursor(0, 33);
  u8x8.print("Temp:");
  u8x8.print(temp);
  u8x8.print("C");
  u8x8.setCursor(0,50);
  u8x8.print("Humidity:");
  u8x8.print(humi);
  u8x8.print("%");
  u8x8.refreshDisplay();
  delay(200);
}
