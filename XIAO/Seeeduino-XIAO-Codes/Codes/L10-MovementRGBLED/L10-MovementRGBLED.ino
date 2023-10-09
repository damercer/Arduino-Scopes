#include "LIS3DHTR.h"
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> 
#endif
#ifdef SOFTWAREWIRE
    #include <SoftwareWire.h>
    SoftwareWire myWire(3, 2);
    LIS3DHTR<SoftwareWire> LIS; 
    #define WIRE myWire
#else
    #include <Wire.h>
    LIS3DHTR<TwoWire> LIS;    
    #define WIRE Wire
#endif
#define PIXEL_PIN 0
#define PIXEL_COUNT 30 
Adafruit_NeoPixel strip(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
    Serial.begin(9600);
    while (!Serial) {};
    LIS.begin(WIRE, 0x19);
    delay(100);
    LIS.setOutputDataRate(LIS3DHTR_DATARATE_50HZ);
    strip.begin(); 
    strip.show(); 
}
void loop() {
    if (!LIS) {
        Serial.println("LIS3DHTR didn't connect.");
        while (1);
        return;
    }
 
 if ((abs(LIS.getAccelerationX()) > 0.2)) {
  theaterChase(strip.Color(127, 0, 0), 50);//红
  }
 if ((abs(LIS.getAccelerationY()) > 0.2)) {
  theaterChase(strip.Color(0, 127, 0), 50); // 绿色
  }
 if ((abs(LIS.getAccelerationZ()) > 1.0)) {
  theaterChase(strip.Color(0, 0, 127), 50); // 蓝色
  }
  else
  {
    strip.clear(); 
    strip.show();
  }

    //3 axis
    Serial.print("x:"); Serial.print(LIS.getAccelerationX()); Serial.print("  ");
    Serial.print("y:"); Serial.print(LIS.getAccelerationY()); Serial.print("  ");
    Serial.print("z:"); Serial.println(LIS.getAccelerationZ());
 
    delay(500);
}
void theaterChase(uint32_t color, int wait) {
  for(int a=0; a<10; a++) {  
    for(int b=0; b<3; b++) { 
      strip.clear();   
      for(int c=b; c<strip.numPixels(); c += 3) {
        strip.setPixelColor(c, color);
      }
      strip.show(); 
      delay(wait);
    }
  }
}
