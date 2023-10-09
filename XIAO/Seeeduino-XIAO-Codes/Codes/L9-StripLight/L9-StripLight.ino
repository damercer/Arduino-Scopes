#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif
#define LIGHT_PIN 7
#define PIXEL_PIN 0
#define NUMPIXELS 30

int readValue = 0;
 Adafruit_NeoPixel strip(NUMPIXELS , PIXEL_PIN , NEO_GRB + NEO_KHZ800);
void setup() { 
   strip.begin(); 
   pinMode(LIGHT_PIN , INPUT); 
}

void loop() {
    strip.clear(); 
    readValue = analogRead(A7);
    if(readValue > 100){
    rainbow(10);
     }else {
     strip.clear();  
     strip.show(); 
}
}
void rainbow(int wait) {
  for(long firstPixelHue = 0; firstPixelHue < 3*65536; firstPixelHue += 256) {
    for(int i=0; i<strip.numPixels(); i++) { 
      int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
    }
    strip.show(); 
    delay(wait);  
  }
}
