#include<Arduino.h>
#include<U8g2lib.h>//使用U8g2库

//判断使用SPI还是I2C协议 
#ifdef U8X8_HAVE_HW_SPI
#include<SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include<Wire.h>
#endif
 
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
//设置构造函数，定义显示类型，控制器，RAM缓冲区大小和通信协议
void draw(void) {
 // 使用u8g2.drawCircle函数在OLED上绘制圆形
  u8g2.drawCircle(20, 25, 10, U8G2_DRAW_ALL);
}
 
void setup(void) {
 u8g2.begin();//初始化U8g2库
}
 
void loop(void) {
 //图片循环显示
 u8g2.firstPage();
 do {
     draw();//使用draw函数
    } while( u8g2.nextPage() );

 delay(1000);
}
