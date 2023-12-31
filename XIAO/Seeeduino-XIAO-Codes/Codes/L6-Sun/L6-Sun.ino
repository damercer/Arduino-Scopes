#include<Arduino.h>
#include<U8g2lib.h>
 
#ifdef U8X8_HAVE_HW_SPI
#include<SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include<Wire.h>
#endif
 
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
 
// Copy the contents of your .xbm file below
#define sun_width 64
#define sun_height 64
static const unsigned char sun_bits[] PROGMEM = {
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0C,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x0E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1F,0x00,0x00,0x00,
0x00,0x00,0x00,0x80,0x1F,0x00,0x00,0x00,0x00,0x00,0x60,0xC0,0x19,0x00,0x00,0x00,0x00,0x18,0xE0,0xE0,0x39,0x00,0x00,0x00,0x00,0x78,0xE0,0xF3,0xF0,0x00,0x00,0x00,
0x00,0xF8,0xE1,0x7F,0xE0,0x01,0x06,0x00,0x00,0xF8,0x67,0x1F,0xC0,0xFF,0x07,0x00,0x00,0xF8,0x7F,0x00,0x00,0xFF,0x07,0x00,0x00,0x70,0x7C,0x00,0x00,0x00,0x07,0x00,
0x00,0x70,0x00,0xE0,0x07,0x00,0x07,0x00,0x00,0x60,0x00,0xFE,0x3F,0x00,0x07,0x00,0x00,0x60,0x80,0xFF,0xFF,0x00,0x07,0x00,0x00,0x60,0xE0,0x07,0xF0,0x03,0x0F,0x00,
0x00,0x60,0xF8,0x00,0xC0,0x07,0x7E,0x00,0x00,0x70,0x78,0x00,0x00,0x0F,0xFC,0x03,0x00,0x7C,0x3C,0x00,0x00,0x1C,0xE0,0x03,0x00,0x3E,0x1E,0x00,0x00,0x3C,0xF0,0x01,
0x00,0x1E,0x0F,0x00,0x00,0x78,0x70,0x00,0x00,0x0C,0x07,0x00,0x00,0x70,0x38,0x00,0x00,0x9C,0x03,0x00,0x00,0xE0,0x1C,0x00,0x00,0x9C,0x03,0x00,0x00,0xE0,0x1C,0x00,
0x00,0x98,0x03,0x00,0x00,0xE0,0x0C,0x00,0x00,0x98,0x03,0x00,0x00,0xE0,0x0C,0x00,0x00,0x9E,0xC1,0x03,0xE0,0xC1,0x0C,0x00,0x00,0x9E,0xE1,0x07,0xF0,0xC3,0x1C,0x00,
0xC0,0x8F,0xF1,0x07,0xF0,0xC7,0x1C,0x00,0xC0,0x8F,0x63,0x06,0x30,0xE7,0x38,0x00,0x00,0x9F,0x03,0x00,0x00,0xE0,0x70,0x00,0x00,0xBC,0x03,0x00,0x00,0xE0,0xF0,0x00,
0x00,0xB0,0x03,0x00,0x00,0xE0,0xF8,0x01,0x00,0x70,0x07,0x20,0x0C,0x70,0xFC,0x03,0x00,0x30,0x0F,0x70,0x0E,0x78,0xFE,0x03,0x00,0x70,0x1E,0xF0,0x0F,0x3C,0x0E,0x00,
0x00,0x70,0x3C,0xE0,0x07,0x1E,0x07,0x00,0x00,0x70,0xF8,0x80,0x81,0x0F,0x06,0x00,0x00,0x38,0xF0,0x01,0xC0,0x07,0x06,0x00,0x00,0x38,0xE0,0x0F,0xF8,0x03,0x0E,0x00,
0x00,0xF8,0x87,0xFF,0x7F,0x00,0x0E,0x00,0x00,0xF8,0x0F,0xFC,0x1F,0xF0,0x1F,0x00,0x00,0x00,0x1E,0x00,0x00,0xF8,0x1F,0x00,0x00,0x00,0x3C,0x00,0x00,0x7C,0x1F,0x00,
0x00,0x00,0xB8,0x3F,0xC0,0x1F,0x00,0x00,0x00,0x00,0xF8,0x7F,0xF0,0x1F,0x00,0x00,0x00,0x00,0xFC,0xF9,0xF0,0x1E,0x00,0x00,0x00,0x00,0x7C,0xE0,0x39,0x18,0x00,0x00,
0x00,0x00,0x1C,0xC0,0x1D,0x18,0x00,0x00,0x00,0x00,0x00,0xC0,0x1F,0x10,0x00,0x00,0x00,0x00,0x00,0x80,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x07,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};
 
void draw(void) {
 // graphic commands to redraw the complete screen should be placed here  
 u8g2.drawXBMP( 42, 0, sun_width, sun_height, sun_bits);
}
 
void setup(void) {
 u8g2.begin();
}
 
void loop(void) {
 // picture loop
 u8g2.firstPage();
 do {
     draw();
    } while( u8g2.nextPage() );
 
 // rebuild the picture after some delay
 delay(1000);
}
