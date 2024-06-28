//Pico_Scope3 3 channel scope with dual 8 bit R2R ladder DAC AWG
// 6/01/2024
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include <hardware/timer.h>
#include <hardware/irq.h>
#include "pico/multicore.h"

#define ALARM_NUM 1
#define ALARM_IRQ TIMER_IRQ_1

uint8_t scopea[16384]; // 14336/8 ~ 1700 max samples
//  uint8_t digin[4096];
uint8_t awgouta[4096];
uint8_t awgoutb[4096];

short led = 25;
short awgon;
//short awgonb;
short addr;
short data;
short awgdata;
short awgres;
short n;
short m;
uint32_t tg;
short sync = 0;
short point = 0;
float Step;
unsigned int at, at2, st, st2, stReal;
short bmax=4096;
short amax=4096;
short bs=1024;
uint16_t tbs=2048;
uint16_t rbs=3072;
uint16_t fbs=4096;
uint16_t vbs=5120;
uint16_t sbs=6144;
uint16_t ebs=7168;
uint16_t gbs=8192;
short dy=1;
uint16_t ns=1024;
uint16_t ms=1024;
int pwmf = 500;
int pwid = 500;

static void alarm_irq(void) {
  hw_clear_bits(&timer_hw->intr, 1u << ALARM_NUM);
  alarm_in_us_arm(at);
  if (awgon == 1){
    if (n >= ns) n = 0;
    if (m >= ms) m = 0;
    awgdata = (awgoutb[m] << 8) + awgouta[n];
    gpio_put_masked(0x0FFFF, awgdata);
    n++;
    m++;
  }
  else {
    n = 0;
    m = 0;
  }
}

static void alarm_in_us_arm(uint32_t delay_us) {
  uint64_t target = timer_hw->timerawl + delay_us;
  timer_hw->alarm[ALARM_NUM] = (uint32_t) target;
}

static void alarm_in_us(uint32_t delay_us) {
  hw_set_bits(&timer_hw->inte, 1u << ALARM_NUM);
  irq_set_exclusive_handler(ALARM_IRQ, alarm_irq);
  irq_set_enabled(ALARM_IRQ, true);
  alarm_in_us_arm(at);
}

void setup1() {
  at=20;
  awgon = 0; // default AWG off
  alarm_in_us(at);
}

void loop1() {
  
}
  
void setup() {
  
  Serial.begin(2000000);
  pinMode(led, OUTPUT);
  pinMode(0, OUTPUT);
  pinMode(1, OUTPUT);
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(14, OUTPUT);
  pinMode(15, OUTPUT);
  pinMode(16, INPUT);
  pinMode(17, INPUT);
  pinMode(18, INPUT);
  pinMode(19, INPUT);
  pinMode(20, INPUT);
  pinMode(21, INPUT);
  analogWriteFreq(pwmf);
  analogWriteRange(1000);
  adc_init();
  adc_gpio_init(26); // ADC0
  adc_gpio_init(27); // ADC1
  adc_gpio_init(28); // ADC2
  adc_gpio_init(29); // ADC3
  adc_select_input(0); 
  //alarm_in_us(at);
} 

void loop() {
  
  int measa;
  int measb;
  int measc;
  int digmeas;
  int VDD;
  char c, c2;
  uint32_t ta, TotalReal, StartReal;
  st=11;
  awgres = 255;
  at=20;
  short Ain1;
  Ain1 = 0; // 26; // pin number for A1
  short Ain2;
  Ain2 = 1; // 27; // pin number for A2
  short Ain3;
  Ain3 = 2; // 28; // pin number for A3
  short Ain4;
  Ain4 = 3; // 29; // pin number for A4

  while (true) { 
    if (Serial.available()){
      c=Serial.read();
      switch (c){
        case 'I': // read back FW Rev
          Serial.println ("Pi Pico Scope R2R 3.0");
          break;
        case 'T': // change the AWG value of at in uSec both channels must be same rate
          at2 = Serial.parseInt();
          if(at2>0){
            at=at2;
          }
          break;
        case 'N': // change number of AWG A samples 
          ns = Serial.parseInt();
          if(ns>bmax){
            ns=bmax;
          }
          break;
        case 'M': // change number of AWG B samples 
          ms = Serial.parseInt();
          if(ms>bmax){
            ms=bmax;
          }
          break;
        case 'L': // load AWG A Buffer data
          addr = Serial.parseInt();
          if(addr > 2047){
              addr=2047;
            }
            if(addr < 0){
              addr=0;
            }
          c2 = Serial.read();
          if(c2=='D'){
            data = Serial.parseInt();
            if(data > awgres){
              data=awgres;
            }
            if(data < 0){
              data=0;
            }
          } else {
            data = 0;
          }
          awgouta[addr] = data; // | 0b0111000000000000; // address DACA
          break;
        case 'l': // load AWG B Buffer data
          addr = Serial.parseInt();
          if(addr > 2047){
              addr=2047;
            }
            if(addr < 0){
              addr=0;
            }
          c2 = Serial.read();
          if(c2=='D'){
            data = Serial.parseInt();
            if(data > awgres){
              data=awgres;
            }
            if(data < 0){
              data=0;
            }
          } else {
            data = 0;
          }
          awgoutb[addr] = data; //| 0b1111 0000 0000 0000; // address DACB
          break;
        case 't': // change the Scope value of dt in uSec
          st2 = Serial.parseInt();
          if(st2>0){
            st=st2;
          }
          break;
        case 'b': // change number of samples to capture
          bs = Serial.parseInt();
          if(bs>bmax){
            bs=bmax;
          }
          tbs = bs * 2;
          rbs = bs * 3;
          fbs = bs * 4;
          vbs = bs * 5;
          sbs = bs * 6;
          ebs = bs * 7;
          gbs = bs * 8;
          break;
        case 'G': // enable - disable AWG A output
          c2 = Serial.read();
          if(c2=='o'){
            awgon = 1;
          }else{
            awgon = 0;
          }
          break;
        case 'g': // enable - disable AWG A output
          c2 = Serial.read();
          if(c2=='o'){
            awgon = 1;
          }else{
            awgon = 0;
          }
          break;
        case 'R': // Reset AWG start point at start of aquire
          sync = Serial.parseInt();
          break;
        case 'r': // Set AWG start address when case R > 0
          point = Serial.parseInt();
          break;
        case 's': // enable - disable PWM output
          c2 = Serial.read();
          if(c2=='o'){
            analogWrite (22, pwid); 
          }else{
            analogWrite (22, 0); 
          }
          break;
        case 'p': // change analog write pwm frequency value
          pwmf = Serial.parseInt();
          analogWriteFreq( pwmf );
          break;
        case 'm': // change pwm duty cycle % 
          pwid = Serial.parseInt(); // range from 0 (0%) to 1000 (100%)
          break;
        case 'V': // Read back the 3.3 V supply voltage divider 1/3
          adc_select_input(3);
          sleep_ms(1);
          VDD=adc_read();
          Serial.print ("V=");
          Serial.println ((int) VDD);
          break;
        case 'A': // set input pin value for scope channel Ain1
          Ain1 = Serial.parseInt();
          break;
        case 'B': // set input pin value for scope channel Ain2
          Ain2 = Serial.parseInt();
          break;
        case 'C': // set input pin value for scope channel Ain3
          Ain3 = Serial.parseInt();
          break;
        case 'D': // set input pin value for scope channel Ain4
          Ain4 = Serial.parseInt();
          break;
        case '1': // do scope ch a single capture
        // if sync is on reset start of awg buffer pointer
          if (sync > 0 ) {
            if (awgon == 1){
              awgon = 0;
              while ( n != point) {
               n = point;
                m = point;
              }
              delayMicroseconds(6);
              awgon = 1;
              delayMicroseconds(sync);
            }
          }
          ta = time_us_32();
          adc_select_input(Ain1);
          StartReal = ta;
          for (int i = 0; i < bs; i++){ // Fill Buffer
            measa = adc_read(); // analogRead(A1);
            scopea[i] = (measa & 0xFF00) >> 8;
            scopea[i+bs] = measa & 0xFF;
            ta+=st;
            while (ta>time_us_32());
          }
          TotalReal=time_us_32()-StartReal;
          //stReal=TotalReal/bs; // calculate the average time for each reading
          digitalWrite(led, HIGH); // Toggel LED High while sending data
          Serial.print("stReal= ");
          Serial.println(TotalReal); // report total time for bs samples
          // dump buffer over serial
          Serial.write(scopea, tbs);
          Serial.println("");
          //
          digitalWrite(led, LOW);
          break;
          //
        case '2': // do scope ch a and b single capture
        // if sync is on reset start of awg buffer pointer
          if (sync > 0 ) {
            if (awgon == 1){
              awgon = 0;
              while ( n != point) {
               n = point;
                m = point;
              }
              delayMicroseconds(6);
              awgon = 1;
              delayMicroseconds(sync);
            }
          }
          ta = time_us_32();
          StartReal = ta;
          for (int i = 0; i < bs; i++){ // Fill Buffer
            adc_select_input(Ain1);
            measa = adc_read();
            adc_select_input(Ain2);
            measb = adc_read();
            scopea[i] = (measa & 0xFF00) >> 8;
            scopea[i+bs] = measa & 0xFF;
            scopea[i+tbs] = (measb & 0xFF00) >> 8;
            scopea[i+rbs] = measb & 0xFF;
            ta+=st;
            while (ta>time_us_32());
          }
          TotalReal=time_us_32()-StartReal;
          //stReal=TotalReal/bs; // calculate the average time for each reading
          digitalWrite(led, HIGH); // Toggel LED High while sending data
          Serial.print("stReal= ");
          Serial.println(TotalReal);
          // Dump Buffer over serial
          Serial.write(scopea, fbs);
          Serial.println("");
          //
          digitalWrite(led, LOW);  // turn the LED off (HIGH is the voltage level)
          break;
          //
        case '3': // do scope ch a b and c single capture
        // if sync is on reset start of awg buffer pointer
          if (sync > 0 ) {
            if (awgon == 1){
              awgon = 0;
              while ( n != point) {
               n = point;
                m = point;
              }
              delayMicroseconds(6);
              awgon = 1;
              delayMicroseconds(sync);
            }
          }
          ta = time_us_32();
          StartReal = ta;
          for (int i = 0; i < bs; i++){ // Fill Buffer
            adc_select_input(Ain1);
            measa = adc_read();
            adc_select_input(Ain2);
            measb = adc_read();
            adc_select_input(Ain3);
            measc = adc_read();
            scopea[i] = (measa & 0xFF00) >> 8;
            scopea[i+bs] = measa & 0xFF;
            scopea[i+tbs] = (measb & 0xFF00) >> 8;
            scopea[i+rbs] = measb & 0xFF;
            scopea[i+fbs] = (measc & 0xFF00) >> 8;
            scopea[i+vbs] = measc & 0xFF;
            ta+=st;
            while (ta>time_us_32());
          }
          TotalReal=time_us_32()-StartReal;
          //stReal=TotalReal/bs; // calculate the average time for each reading
          digitalWrite(led, HIGH); // Toggel LED High while sending data
          Serial.print("stReal= ");
          Serial.println(TotalReal);
          // Dump Buffer over serial
          Serial.write(scopea, sbs);
          Serial.println("");
          //
          digitalWrite(led, LOW);  // turn the LED off (HIGH is the voltage level)
          break;
      }
    }
  }
}

