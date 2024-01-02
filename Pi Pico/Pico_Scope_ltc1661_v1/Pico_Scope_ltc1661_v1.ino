//Pico_Scope3 3 channel scope with AWG using LTC1661 dual 8/10/12 bit SPI DAC
// 12/30/2023 
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include <hardware/timer.h>
#include <hardware/irq.h>
#include <SPI.h>
#include "pico/multicore.h"

#define ALARM_NUM 1
#define ALARM_IRQ TIMER_IRQ_1

int awgouta[4096];
int awgoutb[4096];

int led = 25;
int wavea;
int waveb;
int cyclea;
int ampla;
int offseta;
int cycleb;
int amplb;
int offsetb;
int awgona;
int awgonb;
int addr;
int data;
int awgdata;
int awgres;
int n;
int m;
uint32_t tg;
int sync = 0;
float Step;
unsigned int at, at2, st, st2, stReal;
int bmax=2048;
int bs=1024;
int ns=1024;
int ms=1024;
int pwmf = 500;
int pwid = 500;

static void alarm_irq(void) {
  hw_clear_bits(&timer_hw->intr, 1u << ALARM_NUM);
  alarm_in_us_arm(at);
  if (awgona == 1){ // send data to DAC A
    if (n >= ns) n = 0;
    SPI.transfer16(awgouta[n]);
    n++;
  } else {
    n = 0;
  }
  if (awgonb == 1) { // send data to DAC B
    if (m >= ms) m = 0;
    SPI.transfer16(awgoutb[m]);
    m++;
  }
  else {
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
  wavea = 1; // default to sine shape (for now)
  cyclea = 8; // default is 8 cycles in 1024 samples
  ampla = 127; // default is 127 or full 0 to 255 peak to prak
  offseta = 128; // default is 128 or mid range 0 to 255
  waveb = 1; // default to sine shape (for now)
  cycleb = 8; // default is 8 cycles in 1024 samples
  amplb = 127; // default is 127 or full 0 to 255 peak to prak
  offsetb = 128; // default is 128 or mid range 0 to 255
  awgona = 0; // default AWG A off
  awgonb = 0; // default AWG B off
  makewavea(); 
}

void loop1() {
  
}
  
void setup() {
  
  Serial.begin(2000000);
  pinMode(led, OUTPUT);
  
  analogWriteFreq(pwmf);
  analogWriteRange(1000);
  adc_init();
  adc_gpio_init(26); // ADC0
  adc_gpio_init(27); // ADC1
  adc_gpio_init(28); // ADC2
  adc_gpio_init(29); // ADC3
  adc_select_input(0);
  SPI.begin(true);
  SPI.beginTransaction(SPISettings(25000000, MSBFIRST, SPI_MODE0));
  alarm_in_us(at); 
}

void loop() {
  uint8_t scopeahi[4096];
  uint8_t scopealow[4096];
  uint8_t scopebhi[4096];
  uint8_t scopeblow[4096];
  uint8_t scopechi[4096];
  uint8_t scopeclow[4096];
  uint8_t digin[4096];
  
  int measa;
  int measb;
  int measc;
  int digmeas;
  int VDD;
  char c, c2;
  uint32_t ta, TotalReal, StartReal;
  st=11;
  awgres = 4095;
  while (true) { 
    if (Serial.available()){
      c=Serial.read();
      switch (c){
        case 'I': // read back FW Rev
          Serial.println ("Pi Pico Scope mcp 3.0");
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
        case 'M': // change number of AWG A samples 
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
          awgouta[addr] = data | 0b0001000000000000; // address DACA 2X REF
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
          awgoutb[addr] = data | 0b1010000000000000; // address DACB 2X REF
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
          break;
        case 'W': // change waveform shape ch a
          wavea = Serial.parseInt();
          makewavea();
          break;
        case 'A': // change waveform amplitude 0 to 127 ch a
          ampla = Serial.parseInt();
          makewavea();
          break;
        case 'C': // change number of waveform cycles ch a
          cyclea = Serial.parseInt();
          makewavea();
          break;
        case 'O': // change waveform offset 0 to 255 ch a
          offseta = Serial.parseInt();
          makewavea();
          break;
        case 'w': // change waveform shape ch b
          waveb = Serial.parseInt();
          makewaveb();
          break;
        case 'a': // change waveform amplitude 0 to 127 ch b
          amplb = Serial.parseInt();
          makewaveb();
          break;
        case 'n': // change number of waveform cycles ch b
          cycleb = Serial.parseInt();
          makewaveb();
          break;
        case 'o': // change waveform offset 0 to 255 ch b
          offsetb = Serial.parseInt();
          makewaveb();
          break;
        case 'G': // enable - disable AWG A output
          c2 = Serial.read();
          if(c2=='o'){
            awgona = 1;
          }else{
            awgona = 0;
          }
          break;
        case 'g': // enable - disable AWG B output
          c2 = Serial.read();
          if(c2=='o'){
            awgonb = 1;
          }else{
            awgonb = 0;
          }
          break;
        case 'R': // Reset AWG start point at start of aquire
          c2 = Serial.read();
          if(c2=='o'){
            sync = 1;
          }else{
            sync = 0;
          }
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
        case '0': // do scope ch a single capture
          // if sync is on reset start of awg buffer pointer
          if (sync > 0 ) {
            n = 0;
            m = 0;
            delayMicroseconds(st*2);
          }
          ta = time_us_32();
          adc_select_input(0);
          StartReal = ta;
          for (int k = 0; k < bs; k++){ // Fill Buffer
            measa = adc_read();
            scopeahi[k] = (measa & 0xFF00) >> 8;
            scopealow[k] = measa & 0xFF;
            digmeas = gpio_get_all() >> 3;
            digin[k] = digmeas & 0x0F;
            ta+=st;
            while (ta>time_us_32());
          }
          TotalReal=time_us_32()-StartReal;
          //stReal=TotalReal/bs; // calculate the average time for each reading
          digitalWrite(led, HIGH); // Toggel LED High while sending data
          Serial.print("stReal= ");
          Serial.println(TotalReal);
          for (int k = 0; k < bs; k++){ // Dunp Buffer over serial
            Serial.write(scopeahi[k]);
            Serial.write(scopealow[k]);
            Serial.write(digin[k]);
          }
          digitalWrite(led, LOW);
          break;
        case '1': // do scope ch a and b single capture
          // if sync is on reset start of awg buffer pointer
          if (sync > 0 ) {
            n = 0;
            m = 0;
            delayMicroseconds(st*2);
          }
          ta = time_us_32();
          StartReal = ta;
          for (int k = 0; k < bs; k++){ // Fill Buffer
            adc_select_input(0);
            measa = adc_read();
            scopeahi[k] = (measa & 0xFF00) >> 8;
            scopealow[k] = measa & 0xFF;
            adc_select_input(1);
            measb = adc_read();
            scopebhi[k] = (measb & 0xFF00) >> 8;
            scopeblow[k] = measb & 0xFF;
            digmeas = gpio_get_all() >> 3;
            digin[k] = digmeas & 0x0F;
            ta+=st;
            while (ta>time_us_32());
          }
          TotalReal=time_us_32()-StartReal;
          //stReal=TotalReal/bs; // calculate the average time for each reading
          digitalWrite(led, HIGH); // Toggel LED High while sending data
          Serial.print("stReal= ");
          Serial.println(TotalReal);
          for (int k = 0; k < bs; k++){ // Dunp Buffer over serial
            Serial.write(scopeahi[k]);
            Serial.write(scopealow[k]);
            Serial.write(scopebhi[k]);
            Serial.write(scopeblow[k]);
            Serial.write(digin[k]);
          }
          digitalWrite(led, LOW);
          break;
        case '2': // do scope ch a and c single capture
          // if sync is on reset start of awg buffer pointer
          if (sync > 0 ) {
            n = 0;
            m = 0;
            delayMicroseconds(st*2);
          }
          ta = time_us_32();
          StartReal = ta;
          for (int k = 0; k < bs; k++){ // Fill Buffer
            adc_select_input(0);
            measa = adc_read();
            scopeahi[k] = (measa & 0xFF00) >> 8;
            scopealow[k] = measa & 0xFF;
            adc_select_input(2);
            measc = adc_read();
            scopechi[k] = (measc & 0xFF00) >> 8;
            scopeclow[k] = measc & 0xFF;
            digmeas = gpio_get_all() >> 3;
            digin[k] = digmeas & 0x0F;
            ta+=st;
            while (ta>time_us_32());
          }
          TotalReal=time_us_32()-StartReal;
          //stReal=TotalReal/bs; // calculate the average time for each reading
          digitalWrite(led, HIGH); // Toggel LED High while sending data
          Serial.print("stReal= ");
          Serial.println(TotalReal);
          for (int k = 0; k < bs; k++){ // Dunp Buffer over serial
            Serial.write(scopeahi[k]);
            Serial.write(scopealow[k]);
            Serial.write(scopechi[k]);
            Serial.write(scopeclow[k]);
            Serial.write(digin[k]);
          }
          digitalWrite(led, LOW);
          break;
        case '3': // do scope ch b and c single capture
          // if sync is on reset start of awg buffer pointer
          if (sync > 0 ) {
            n = 0;
            m = 0;
            delayMicroseconds(st*2);
          }
          ta = time_us_32();
          StartReal = ta;
          for (int k = 0; k < bs; k++){ // Fill Buffer
            adc_select_input(1);
            measb = adc_read();
            scopebhi[k] = (measb & 0xFF00) >> 8;
            scopeblow[k] = measb & 0xFF;
            adc_select_input(2);
            measc = adc_read();
            scopechi[k] = (measc & 0xFF00) >> 8;
            scopeclow[k] = measc & 0xFF;
            digmeas = gpio_get_all() >> 3;
            digin[k] = digmeas & 0x0F;
            ta+=st;
            while (ta>time_us_32());
          }
          TotalReal=time_us_32()-StartReal;
          //stReal=TotalReal/bs; // calculate the average time for each reading
          digitalWrite(led, HIGH); // Toggel LED High while sending data
          Serial.print("stReal= ");
          Serial.println(TotalReal);
          for (int k = 0; k < bs; k++){ // Dunp Buffer over serial
            Serial.write(scopebhi[k]);
            Serial.write(scopeblow[k]);
            Serial.write(scopechi[k]);
            Serial.write(scopeclow[k]);
            Serial.write(digin[k]);
          }
          digitalWrite(led, LOW);
          break;
        case '4': // do scope ch a b and c single capture
          // if sync is on reset start of awg buffer pointer
          if (sync > 0 ) {
            n = 0;
            m = 0;
            delayMicroseconds(st*2);
          }
          ta = time_us_32();
          StartReal = ta;
          for (int k = 0; k < bs; k++){ // Fill Buffer
            adc_select_input(0);
            measa = adc_read();
            scopeahi[k] = (measa & 0xFF00) >> 8;
            scopealow[k] = measa & 0xFF;
            adc_select_input(1);
            measb = adc_read();
            scopebhi[k] = (measb & 0xFF00) >> 8;
            scopeblow[k] = measb & 0xFF;
            adc_select_input(2);
            measc = adc_read();
            scopechi[k] = (measc & 0xFF00) >> 8;
            scopeclow[k] = measc & 0xFF;
            digmeas = gpio_get_all() >> 3;
            digin[k] = digmeas & 0x0F;
            ta+=st;
            while (ta>time_us_32());
          }
          TotalReal=time_us_32()-StartReal;
          //stReal=TotalReal/bs; // calculate the average time for each reading
          digitalWrite(led, HIGH); // Toggel LED High while sending data
          Serial.print("stReal= ");
          Serial.println(TotalReal);
          for (int k = 0; k < bs; k++){ // Dunp Buffer over serial
            Serial.write(scopeahi[k]);
            Serial.write(scopealow[k]);
            Serial.write(scopebhi[k]);
            Serial.write(scopeblow[k]);
            Serial.write(scopechi[k]);
            Serial.write(scopeclow[k]);
            Serial.write(digin[k]);
          }
          digitalWrite(led, LOW);
          break;
        case '5': // do scope ch b single capture
          // if sync is on reset start of awg buffer pointer
          if (sync > 0 ) {
            n = 0;
            m = 0;
            delayMicroseconds(st*2);
          }
          ta = time_us_32();
          adc_select_input(1);
          StartReal = ta;
          for (int k = 0; k < bs; k++){ // Fill Buffer
            measb = adc_read();
            scopebhi[k] = (measb & 0xFF00) >> 8;
            scopeblow[k] = measb & 0xFF;
            digmeas = gpio_get_all() >> 3;
            digin[k] = digmeas & 0x0F;
            ta+=st;
            while (ta>time_us_32());
          }
          TotalReal=time_us_32()-StartReal;
          //stReal=TotalReal/bs; // calculate the average time for each reading
          digitalWrite(led, HIGH); // Toggel LED High while sending data
          Serial.print("stReal= ");
          Serial.println(TotalReal);
          for (int k = 0; k < bs; k++){ // Dunp Buffer over serial
            Serial.write(scopebhi[k]);
            Serial.write(scopeblow[k]);
            Serial.write(digin[k]);
          }
          digitalWrite(led, LOW);
          break;
        case '6': // do scope ch c single capture
          // if sync is on reset start of awg buffer pointer
          if (sync > 0 ) {
            n = 0;
            m = 0;
            delayMicroseconds(st*2);
          }
          ta = time_us_32();
          adc_select_input(2);
          StartReal = ta;
          for (int k = 0; k < bs; k++){ // Fill Buffer
            measc = adc_read();
            scopechi[k] = (measc & 0xFF00) >> 8;
            scopeclow[k] = measc & 0xFF;
            digmeas = gpio_get_all() >> 3;
            digin[k] = digmeas & 0x0F;
            ta+=st;
            while (ta>time_us_32());
          }
          TotalReal=time_us_32()-StartReal;
          //stReal=TotalReal/bs; // calculate the average time for each reading
          digitalWrite(led, HIGH); // Toggel LED High while sending data
          Serial.print("stReal= ");
          Serial.println(TotalReal);
          for (int k = 0; k < bs; k++){ // Dunp Buffer over serial
            Serial.write(scopechi[k]);
            Serial.write(scopeclow[k]);
            Serial.write(digin[k]);
          }
          digitalWrite(led, LOW);
          break;
      }
    }
  }
}

float nextVal (float curr, int min, int max) {
    // Handle situations where you want to turn around.
    if ((curr == min) || (curr == max)) {
        Step = -(Step);
        return curr + Step;
    }
    // Handle situation where you would exceed your bounds (just
    //   go to the bound).
    if (curr + Step < min) return min;
    if (curr + Step > max) return max;
    // Otherwise, just move within the bounds.
    return curr + Step;
}

void makewavea() {
  int Vmin = offseta - ampla;
  if(Vmin > 255){
    Vmin=255;
  }
  if(Vmin < 0){
    Vmin=0;
  }
  int Vmax = offseta + ampla;
  if(Vmax > 255){
    Vmax=255;
  }
  if(Vmax < 0){
    Vmax=0;
  }
  float Curr = Vmin;
  Step = ((Vmax-Vmin)/255.0)*(cyclea/2.0);

  if (wavea == 1) { // make a sine wave shape
    for (int j = 0; j < ns; j++){
      int temp = offseta + (ampla * sin(2*cyclea*PI*j/ns));
      if(temp > 255){
        temp=255;
      }
      if(temp < 0){
        temp=0;
      }
      awgouta[j] = temp;
    }
  } else if(wavea == 2){ // make a triangle wave shape
    for (int j = 0; j < ns; j++){
      float temp = nextVal(Curr, Vmin, Vmax);
      Curr = temp;
      if(temp > 255){
        temp=255;
      }
      if(temp < 0){
        temp=0;
      }
      awgouta[j] = int(temp);
    }
  } else { // make a DC value = offset
    for (int j = 0; j < ns; j++){
      awgouta[j] = offseta;
    }
  }
}

void makewaveb() {
  int Vmin = offsetb - amplb;
  if(Vmin > 255){
    Vmin=255;
  }
  if(Vmin < 0){
    Vmin=0;
  }
  int Vmax = offsetb + amplb;
  if(Vmax > 255){
    Vmax=255;
  }
  if(Vmax < 0){
    Vmax=0;
  }
  float Curr = Vmin;
  Step = ((Vmax-Vmin)/255.0)*(cycleb/2.0);

  if (waveb == 1) { // make a sine wave shape
    for (int j = 0; j < ns; j++){
      int temp = offsetb + (amplb * sin(2*cycleb*PI*j/ns));
      if(temp > 255){
        temp=255;
      }
      if(temp < 0){
        temp=0;
      }
      awgoutb[j] = temp;
    }
  } else if(waveb == 2){ // make a triangle wave shape
    for (int j = 0; j < ns; j++){
      float temp = nextVal(Curr, Vmin, Vmax);
      Curr = temp;
      if(temp > 255){
        temp=255;
      }
      if(temp < 0){
        temp=0;
      }
      awgoutb[j] = int(temp);
    }
  } else { // make a DC value = offset
    for (int j = 0; j < ns; j++){
      awgoutb[j] = offsetb;
    }
  }
}

