//ItsyBitsy M4 Express Scope3 1/2/3 channel scope 1 internal awg 
// (12/26/2023)
#include <Adafruit_ZeroTimer.h>
#include "Adafruit_NeoPixel.h"

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN 47 // On Trinket or Gemma, suggest changing this to 1

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS 1 // Popular NeoPixel ring size
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
#ifndef MULTIPLE_PIN_OPS_H
#define MULTIPLE_PIN_OPS_H
//
//Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

void multiplePinMode(const int *pins, const int numberOfPins, uint8_t mode);
void multipleDigitalWrite(const int *pins, const int numberOfPins, uint16_t number);
uint64_t multipleDigitalRead(const int *pins, const int numberOfPins);
void multipleAnalogRead(const int *pins, const int numberOfPins, int *values);
void multipleAnalogWrite(const int *pins, const int numberOfPins, uint8_t* values);

#endif // MULTIPLE_PIN_OPS_H
// timer tester
Adafruit_ZeroTimer zerotimer = Adafruit_ZeroTimer(3);

uint8_t scopea[2048];
uint8_t scopeb[2048];
uint8_t scopec[2048]; 
uint8_t digin[1024];
unsigned int awgouta[2048];
unsigned int awgoutb[2048];
int wavea;
int cyclea;
int ampla;
int offseta;
int awgona;
int awgonb;
int awgpwnon;
int awgres;
int addr;
int data;
int awgdata;
int n;
int m;
int sync = 0;
float Step;
unsigned int at, at2, st, st2, stReal;
int bmax=2048;
int bs=1024;
int ns=1024;
int ms=1024;
int pwmf = 500;
int pwid = 500;
//
void TC3_Handler() {
  Adafruit_ZeroTimer::timerHandler(3);
}
static inline void RGB_LED_off() {
  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  pixels.clear(); // Set all pixel colors to 'off'
  //strip.setPixelColor(0, strip.Color(0, 0, 255));
  //strip.show();
}
//
static inline void syncADC(const Adc *hw, uint32_t reg) {
  // Taken from `hri_adc_d51.h`
  while (((Adc *)hw)->SYNCBUSY.reg & reg) {}
}
void ADC_init() {
  /* Increase the ADC clock by setting the PRESCALER from default DIV128 to a
  smaller divisor. This is needed for DAQ rates larger than ~20 kHz on SAMD51
  and DAQ rates larger than ~10 kHz on SAMD21. Setting too small divisors will
  result in ADC errors. Keep as large as possible to increase ADC accuracy.
  
  SAMD51:
    ADC source clock is default at 48 MHz (Generic Clock Generator 1).
    \.platformio\packages\framework-arduino-samd-adafruit\cores\arduino\wiring.c

    Max ADC input clock frequency    : 100 MHz    (datasheet table 54-8)
    Max ADC internal clock frequency :  16 MHz    (datasheet table 54-24)
  */
  // Turn off ADC
  ADC0->CTRLA.reg = 0;
  syncADC(ADC0, ADC_SYNCBUSY_SWRST | ADC_SYNCBUSY_ENABLE);

  // Load the factory calibration
  uint32_t biascomp =
      (*((uint32_t *)ADC0_FUSES_BIASCOMP_ADDR) & ADC0_FUSES_BIASCOMP_Msk) >>
      ADC0_FUSES_BIASCOMP_Pos;
  uint32_t biasr2r =
      (*((uint32_t *)ADC0_FUSES_BIASR2R_ADDR) & ADC0_FUSES_BIASR2R_Msk) >>
      ADC0_FUSES_BIASR2R_Pos;
  uint32_t biasref =
      (*((uint32_t *)ADC0_FUSES_BIASREFBUF_ADDR) & ADC0_FUSES_BIASREFBUF_Msk) >>
      ADC0_FUSES_BIASREFBUF_Pos;

  ADC0->CALIB.reg = ADC_CALIB_BIASREFBUF(biasref) | ADC_CALIB_BIASR2R(biasr2r) |
                    ADC_CALIB_BIASCOMP(biascomp);
  // No sync needed according to `hri_adc_d51.h`

  // The ADC clock must remain below 16 MHz, see SAMD51 datasheet table 54-24.
  // Hence, don't go below DIV4 @ 48 MHz.
  ADC0->CTRLA.bit.PRESCALER = ADC_CTRLA_PRESCALER_DIV16_Val;
  syncADC(ADC0, ADC_SYNCBUSY_MASK);

  // AnalogRead resolution
  ADC0->CTRLB.bit.RESSEL = ADC_CTRLB_RESSEL_16BIT_Val;
  syncADC(ADC0, ADC_SYNCBUSY_MASK);

  // Sample averaging
  ADC0->AVGCTRL.bit.SAMPLENUM = ADC_AVGCTRL_SAMPLENUM_1_Val;
  syncADC(ADC0, ADC_SYNCBUSY_MASK);
  ADC0->AVGCTRL.bit.ADJRES = 1; // 2^N, must match `ADC0->AVGCTRL.bit.SAMPLENUM`
  syncADC(ADC0, ADC_SYNCBUSY_MASK);

  // Sampling length, larger means increased max input impedance
  // default 5, stable 14 @ DIV16 & SAMPLENUM_4
  ADC0->SAMPCTRL.bit.OFFCOMP = 0; // When set to 1, SAMPLEN must be set to 0
  syncADC(ADC0, ADC_SYNCBUSY_MASK);
  ADC0->SAMPCTRL.bit.SAMPLEN = 4;
  syncADC(ADC0, ADC_SYNCBUSY_MASK);

  ADC0->INPUTCTRL.bit.DIFFMODE = 0;
  syncADC(ADC0, ADC_SYNCBUSY_MASK);
  ADC0->INPUTCTRL.bit.MUXPOS = g_APinDescription[A1].ulADCChannelNumber;
  syncADC(ADC0, ADC_SYNCBUSY_MASK);
  ADC0->INPUTCTRL.bit.MUXNEG = ADC_INPUTCTRL_MUXNEG_GND_Val;
  syncADC(ADC0, ADC_SYNCBUSY_MASK);

  ADC0->REFCTRL.bit.REFSEL = ADC_REFCTRL_REFSEL_INTVCC1_Val; // VDDANA
  syncADC(ADC0, ADC_SYNCBUSY_MASK);
  ADC0->REFCTRL.bit.REFCOMP = 0;
  syncADC(ADC0, ADC_SYNCBUSY_MASK);

  // Turn on ADC
  ADC0->CTRLA.bit.ENABLE = 1;
  syncADC(ADC0, ADC_SYNCBUSY_SWRST | ADC_SYNCBUSY_ENABLE);

}
//
int16_t ADC_read_signal(int pin) {
  ADC0->INPUTCTRL.bit.MUXPOS = pin; // Selection new conversion pin
  ADC0->SWTRIG.bit.START = 1;       // Request start conversion
  syncADC(ADC0, ADC_SYNCBUSY_MASK); // Make sure conversion has been initiated
  __asm__("nop\n\t");               // Tiny delay before we can poll RESRDY
  __asm__("nop\n\t");
  __asm__("nop\n\t");
  while (!ADC0->INTFLAG.bit.RESRDY) {} // Wait for result ready to be read
  return ADC0->RESULT.reg;             // Read result
}
//
void multiplePinMode(const int *pins, const int numberOfPins, uint8_t mode) {
  for (uint8_t i  = 0; i < numberOfPins; i++) {
    if (mode == INPUT_PULLUP) pinMode(pins[i], INPUT_PULLUP);
    else if (mode == OUTPUT) pinMode(pins[i], OUTPUT);
    else pinMode(pins[i], INPUT);
  }
}

uint64_t multipleDigitalRead(const int *pins, const int numberOfPins) {
  uint16_t value = 0;
  for (uint8_t i = 0; i < numberOfPins; i++) {
    if (digitalRead(pins[i])) {
      value += (uint16_t) (1 << i);
    }
  }
  return value;
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
// DAC output timer function fixed length of 1024 for now
void updatedac() {
  if (awgona) {
    if (n >= ns) n = 0;
    analogWrite(A0, awgouta[n]);
    n++;
  } else {
    analogWrite(A0, 0);
    n = 0;
  }
  if (awgonb) {
    if (m >= ms) m = 0;
    analogWrite(A1, awgoutb[m]); // pwm(A10, pwmf, awgoutb[m]);
      m++;
  } else {
    analogWrite(A1, 0);
    m = 0;
  }
}
// internal fill waveform array
void makewavea() {
  int Vmin = offseta - ampla;
  if(Vmin > 1023){
    Vmin=1023;
  }
  if(Vmin < 0){
    Vmin=0;
  }
  int Vmax = offseta + ampla;
  if(Vmax > 1023){
    Vmax=1023;
  }
  if(Vmax < 0){
    Vmax=0;
  }
  float Curr = Vmin;
  Step = ((Vmax-Vmin)/1023.0)*(cyclea/2.0);

  if (wavea == 1) { // make a sine wave shape
    for (int j = 0; j < ns; j++){
      int temp = offseta + (ampla * sin(2*cyclea*PI*j/ns));
      if(temp > 1023){
        temp=1023;
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
      if(temp > 1023){
        temp=1023;
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

void setup() {
  Serial.begin(2000000);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  RGB_LED_off();
  wavea = 1; // default to sine shape (for now)
  cyclea = 8; // default is 8 cycles in 1024 samples
  ampla = 500; // default is 127 or full 0 to 255 peak to prak
  offseta = 512; // default is 128 or mid range 0 to 255
  awgona = 0; // default AWG a off
  awgonb = 0; // default AWG b off
  awgres = 4095;
  makewavea();   
  int measa;
  int measb;
  int measc;
  int digmeas;
  //int VDD;
  char c, c2;
  uint32_t ta, TotalReal, StartReal;
  st=11;
  at=25;
  int Ain1;
  Ain1 = 2; // pin number for A1
  int Ain2;
  Ain2 = 3; // pin number for A2
  int Ain3;
  Ain3 = 4; // pin number for A3
  //
  //pinMode(A1, INPUT);
  //pinMode(A2, INPUT);
  //pinMode(A3, INPUT);
  //pinMode(A4, INPUT);
  //pinMode(A5, INPUT);
  //pinMode(A6, INPUT);
  //pinMode(A7, INPUT);
  //pinMode(A8, INPUT);
  //pinMode(A9, INPUT);
  //const int numberOfInputPins = 6;
  //const int inputPins[numberOfInputPins] = {A6, A7, A8, A9}; // pins to use as inputs
  //multiplePinMode(inputPins, numberOfInputPins, INPUT);
  // Set up the flexible divider/compare
  uint16_t divider  = 1;
  uint16_t compare = 0;
  tc_clock_prescaler prescaler = TC_CLOCK_PRESCALER_DIV1;
  // start AWG timer
  zerotimer.enable(false);
  //Timer3.initialize(at);
  compare = 4800/4;
  zerotimer.configure(prescaler,       // prescaler
          TC_COUNTER_SIZE_16BIT,       // bit width of timer/counter
          TC_WAVE_GENERATION_MATCH_FREQ // frequency or PWM mode
          );

  zerotimer.setCompare(0, compare);
  //Timer3.attachInterrupt(updatedac);
  zerotimer.setCallback(true, TC_CALLBACK_CC_CHANNEL0, updatedac);
  //
  //m0tweak::cpuFrequency(64);
  //m0tweak::adcPrecision(12);
  analogWriteResolution(12);
  //analogReadResolution(12);
  ADC_init();
  //ADC->CTRLB.reg = ADC_CTRLB_PRESCALER_DIV32;
  //pinMode(A3, INPUT); 
  while (true) { 
    if (Serial.available()){
      c=Serial.read();
      switch (c){
        case 'I': // read back FW Rev
          Serial.println ("Bitsy M4 Scope 3.0");
          break;
        case 'T': // change the AWG value of at in uSec both channels must be same rate
          at2 = Serial.parseInt();
          if(at2>0){
            at=at2;
            //zerotimer.enable(false);
            compare = 48*at;
            zerotimer.setCompare(0, compare);
            //Timer3.detachInterrupt();
            //Timer3.initialize(at);
            //zerotimer.enable(true);
            //Timer3.attachInterrupt(updatedac);
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
        case 'L': // load DAC AWG A Buffer data
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
          awgouta[addr] = data;
          break;
        case 'l': // load PWM AWG Buffer data
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
          awgoutb[addr] = data;
          break;
        case 't': // change the Scope value of st in uSec
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
        case 'G': // enable - disable AWG output
          c2 = Serial.read();
          if(c2=='o'){
            awgona = 1;
            n = 0;
            zerotimer.configure(prescaler,       // prescaler
              TC_COUNTER_SIZE_16BIT,       // bit width of timer/counter
              TC_WAVE_GENERATION_MATCH_FREQ // frequency or PWM mode
              );
            zerotimer.setCompare(0, compare);
            zerotimer.setCallback(true, TC_CALLBACK_CC_CHANNEL0, updatedac);
            zerotimer.enable(true);
            //
          }else{
            awgona = 0;
            zerotimer.enable(false);
            //Timer3.detachInterrupt();
          }
          break;
        case 'g': // enable - disable AWG output
          c2 = Serial.read();
          if(c2=='o'){
            awgonb = 1;
            n = 0;
            zerotimer.configure(prescaler,       // prescaler
              TC_COUNTER_SIZE_16BIT,       // bit width of timer/counter
              TC_WAVE_GENERATION_MATCH_FREQ // frequency or PWM mode
              );
            zerotimer.setCompare(0, compare);
            zerotimer.setCallback(true, TC_CALLBACK_CC_CHANNEL0, updatedac);
            zerotimer.enable(true);
            //
          }else{
            awgonb = 0;
            zerotimer.enable(false);
            //Timer3.detachInterrupt();
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
        case 's': // enable - disable PWM output awgpwnon
          c2 = Serial.read();
          if(c2=='o'){
            analogWrite(A5, pwid); //pwm(A10, pwmf, pwid);
          }else{
            analogWrite(A5, 0); //pwm(A10, pwmf, 0);
          }
          break;
        case 'p': // change analog write pwm frequency value
          pwmf = Serial.parseInt();
          analogWrite(A5, pwid); // pwm(A10, pwmf, pwid);
          break;
        case 'm': // change pwm duty cycle % 
          pwid = Serial.parseInt(); // range from 0 (0%) to 1000 (100%)
          analogWrite(A5, pwid); // pwm(A10, pwmf, pwid);
          break;
        case '0': // do scope ch a single capture
        // if sync is on reset start of awg buffer pointer
          if (sync > 0 ) {
            n = 0;
            m = 0;
            delayMicroseconds(10);
          }
          ta = micros();
          StartReal = ta;
          for (int n = 0; n < bs; n++){ // Fill Buffer
            measa = ADC_read_signal(Ain1); // analogRead(A1);
            scopea[n] = (measa & 0xFF00) >> 8;
            scopea[n+bs] = measa & 0xFF;
            digmeas = 0; // digitalRead(A6);
            digin[n]= digmeas & 0x3F;
            ta+=st;
            while (ta>micros());
          }
          TotalReal=micros()-StartReal;
          //stReal=TotalReal/bs; // calculate the average time for each reading
          digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
          Serial.print("stReal= ");
          Serial.println(TotalReal);
          // dump buffer over serial
          Serial.write(scopea, sizeof(scopea));
          Serial.write(digin, sizeof(digin));
          Serial.println("");
          //
          digitalWrite(LED_BUILTIN, LOW);  // turn the LED off (HIGH is the voltage level)
          break;
          //
        case '1': // do scope ch a and b single capture
        // if sync is on reset start of awg buffer pointer
          if (sync > 0 ) {
            n = 0;
            m = 0;
            delayMicroseconds(10);
          }
          ta = micros();
          StartReal = ta;
          for (int n = 0; n < bs; n++){ // Fill Buffer
            measa = ADC_read_signal(Ain1); // analogRead(A1);
            measb = ADC_read_signal(Ain2); // analogRead(A2);
            digmeas = 0; // digitalRead(A6);
            scopea[n] = (measa & 0xFF00) >> 8;
            scopea[n+bs] = measa & 0xFF;
            scopeb[n] = (measb & 0xFF00) >> 8;
            scopeb[n+bs] = measb & 0xFF;
            digin[n]= digmeas & 0x3F;
            ta+=st;
            while (ta>micros());
          }
          TotalReal=micros()-StartReal;
          //stReal=TotalReal/bs; // calculate the average time for each reading
          digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
          Serial.print("stReal= ");
          Serial.println(TotalReal);
          // Dump Buffer over serial
          Serial.write(scopea, sizeof(scopea));
          Serial.write(scopeb, sizeof(scopeb));
          Serial.write(digin, sizeof(digin));
          Serial.println("");
          //
          digitalWrite(LED_BUILTIN, LOW);  // turn the LED off (HIGH is the voltage level)
          break;
          //
        case '2': // do scope ch a and c single capture
        // if sync is on reset start of awg buffer pointer
          if (sync > 0 ) {
            n = 0;
            m = 0;
            delayMicroseconds(10);
          }
          ta = micros();
          StartReal = ta;
          for (int n = 0; n < bs; n++){ // Fill Buffer
            measa = ADC_read_signal(Ain1); // analogRead(A1);
            measc = ADC_read_signal(Ain3); // analogRead(A3);
            digmeas = 0; // digitalRead(A6);
            scopea[n] = (measa & 0xFF00) >> 8;
            scopea[n+bs] = measa & 0xFF;
            scopec[n] = (measc & 0xFF00) >> 8;
            scopec[n+bs] = measc & 0xFF;
            digin[n]= digmeas & 0x3F;
            ta+=st;
            while (ta>micros());
          }
          TotalReal=micros()-StartReal;
          //stReal=TotalReal/bs; // calculate the average time for each reading
          digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
          Serial.print("stReal= ");
          Serial.println(TotalReal);
          // Dump Buffer over serial
          Serial.write(scopea, sizeof(scopea));
          Serial.write(scopec, sizeof(scopec));
          Serial.write(digin, sizeof(digin));
          Serial.println("");
          //
          digitalWrite(LED_BUILTIN, LOW);  // turn the LED off (HIGH is the voltage level)
          Serial.println("");
          break;
          //
        case '3': // do scope ch b and c single capture
        // if sync is on reset start of awg buffer pointer
          if (sync > 0 ) {
            n = 0;
            m = 0;
            delayMicroseconds(10);
          }
          ta = micros();
          StartReal = ta;
          for (int n = 0; n < bs; n++){ // Fill Buffer
            measb = ADC_read_signal(Ain2); // analogRead(A2);
            measc = ADC_read_signal(Ain3); // analogRead(A3);
            digmeas = 0; // digitalRead(A6);
            scopeb[n] = (measb & 0xFF00) >> 8;
            scopeb[n+bs] = measb & 0xFF;
            scopec[n] = (measc & 0xFF00) >> 8;
            scopec[n+bs] = measc & 0xFF;
            digin[n]= digmeas & 0x3F;
            ta+=st;
            while (ta>micros());
          }
          TotalReal=micros()-StartReal;
          //stReal=TotalReal/bs; // calculate the average time for each reading
          digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
          Serial.print("stReal= ");
          Serial.println(TotalReal);
          // Dump Buffer over serial
          Serial.write(scopeb, sizeof(scopeb));
          Serial.write(scopec, sizeof(scopec));
          Serial.write(digin, sizeof(digin));
          Serial.println("");
          //
          digitalWrite(LED_BUILTIN, LOW);  // turn the LED off (HIGH is the voltage level)
          break;
          //
        case '4': // do scope ch a b and c single capture
        // if sync is on reset start of awg buffer pointer
          if (sync > 0 ) {
            n = 0;
            m = 0;
            delayMicroseconds(10);
          }
          ta = micros();
          StartReal = ta;
          for (int n = 0; n < bs; n++){ // Fill Buffer
            measa = ADC_read_signal(Ain1); // analogRead(A1);
            measb = ADC_read_signal(Ain2); // analogRead(A2);
            measc = ADC_read_signal(Ain3); // analogRead(A3);
            digmeas = 0; // digitalRead(A6);
            scopea[n] = (measa & 0xFF00) >> 8;
            scopea[n+bs] = measa & 0xFF;
            scopeb[n] = (measb & 0xFF00) >> 8;
            scopeb[n+bs] = measb & 0xFF;
            scopec[n] = (measc & 0xFF00) >> 8;
            scopec[n+bs] = measc & 0xFF;
            digin[n]= digmeas & 0x3F;
            ta+=st;
            while (ta>micros());
          }
          TotalReal=micros()-StartReal;
          //stReal=TotalReal/bs; // calculate the average time for each reading
          digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
          Serial.print("stReal= ");
          Serial.println(TotalReal);
          // Dump Buffer over serial
          Serial.write(scopea, sizeof(scopea));
          Serial.write(scopeb, sizeof(scopeb));
          Serial.write(scopec, sizeof(scopec));
          Serial.write(digin, sizeof(digin));
          Serial.println("");
          //
          digitalWrite(LED_BUILTIN, LOW);  // turn the LED off (HIGH is the voltage level)
          break;
        case '5': // do scope ch b single capture
        // if sync is on reset start of awg buffer pointer
          if (sync > 0 ) {
            n = 0;
            m = 0;
            delayMicroseconds(10);
          }
          ta = micros();
          StartReal = ta;
          for (int n = 0; n < bs; n++){ // Fill Buffer
            measb = ADC_read_signal(Ain2); // analogRead(A2);
            scopeb[n] = (measb & 0xFF00) >> 8;
            scopeb[n+bs] = measb & 0xFF;
            digmeas = 0; // digitalRead(A6);
            digin[n]= digmeas & 0x3F;
            ta+=st;
            while (ta>micros());
          }
          TotalReal=micros()-StartReal;
          //stReal=TotalReal/bs; // calculate the average time for each reading
          digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
          Serial.print("stReal= ");
          Serial.println(TotalReal);
          // Dump Buffer over serial
          Serial.write(scopeb, sizeof(scopeb));
          Serial.write(digin, sizeof(digin));
          Serial.println("");
          //
          digitalWrite(LED_BUILTIN, LOW);  // turn the LED off (HIGH is the voltage level)
          break;
          //
        case '6': // do scope ch c single capture
        // if sync is on reset start of awg buffer pointer
          if (sync > 0 ) {
            n = 0;
            m = 0;
            delayMicroseconds(10);
          }
          ta = micros();
          StartReal = ta;
          for (int n = 0; n < bs; n++){ // Fill Buffer
            measc = ADC_read_signal(Ain3); // analogRead(A3);
            scopec[n] = (measc & 0xFF00) >> 8;
            scopec[n+bs] = measc & 0xFF;
            digmeas = 0; // digitalRead(A6);
            digin[n]= digmeas & 0x3F;
            ta+=st;
            while (ta>micros());
          }
          TotalReal=micros()-StartReal;
          //stReal=TotalReal/bs; // calculate the average time for each reading
          digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
          Serial.print("stReal= ");
          Serial.println(TotalReal);
          // Dump Buffer over serial
          Serial.write(scopec, sizeof(scopec));
          Serial.write(digin, sizeof(digin));
          Serial.println("");
          //
          digitalWrite(LED_BUILTIN, LOW);  // turn the LED off (HIGH is the voltage level)
          break;
          //
        case '7': // do scope ch a single capture
        // if sync is on reset start of awg buffer pointer
          if (sync > 0 ) {
            n = 0;
            m = 0;
            delayMicroseconds(10);
          }
          ta = micros();
          StartReal = ta;
          for (int n = 0; n < bs; n++){ // Fill Buffer
            measa = ADC_read_signal(Ain1); // analogRead(A1);
            scopea[n] = (measa & 0xFF00) >> 8;
            scopea[n+bs] = measa & 0xFF;
            digmeas = 0; // multipleDigitalRead(inputPins, numberOfInputPins); // returns corresponding decimal number reading fom input pins
            digin[n]= digmeas & 0x3F;
            ta+=st;
            while (ta>micros());
          }
          TotalReal=micros()-StartReal;
          //stReal=TotalReal/bs; // calculate the average time for each reading
          digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
          Serial.print("stReal= ");
          Serial.println(TotalReal);
          // Dump Buffer over serial
          Serial.write(scopea, sizeof(scopea));
          Serial.write(digin, sizeof(digin));
          Serial.println("");
          //
          digitalWrite(LED_BUILTIN, LOW);  // turn the LED off (HIGH is the voltage level)
          break;
        case '8': // do scope ch a and b single capture
        // if sync is on reset start of awg buffer pointer
          if (sync > 0 ) {
            n = 0;
            m = 0;
            delayMicroseconds(10);
          }
          ta = micros();
          StartReal = ta;
          for (int n = 0; n < bs; n++){ // Fill Buffer
            measa = ADC_read_signal(Ain1); // analogRead(A1);
            scopea[n] = (measa & 0xFF00) >> 8;
            scopea[n+bs] = measa & 0xFF;
            measb = ADC_read_signal(Ain2); // analogRead(A2);
            scopeb[n] = (measb & 0xFF00) >> 8;
            scopeb[n+bs] = measb & 0xFF;
            digmeas = 0; // multipleDigitalRead(inputPins, numberOfInputPins); // returns corresponding decimal number reading fom input pins
            digin[n]= digmeas & 0x3F;
            ta+=st;
            while (ta>micros());
          }
          TotalReal=micros()-StartReal;
          //stReal=TotalReal/bs; // calculate the average time for each reading
          digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
          Serial.print("stReal= ");
          Serial.println(TotalReal);
          // Dump Buffer over serial
          Serial.write(scopea, sizeof(scopea));
          Serial.write(scopeb, sizeof(scopeb));
          Serial.write(digin, sizeof(digin));
          Serial.println("");
          //
          digitalWrite(LED_BUILTIN, LOW);  // turn the LED off (HIGH is the voltage level)
          break;
        case '9': // do scope ch a b and c single capture
        // if sync is on reset start of awg buffer pointer
          if (sync > 0 ) {
            n = 0;
            m = 0;
            delayMicroseconds(10);
          }
          ta = micros();
          StartReal = ta;
          for (int n = 0; n < bs; n++){ // Fill Buffer
            measa = ADC_read_signal(Ain1); // analogRead(A1);
            scopea[n] = (measa & 0xFF00) >> 8;
            scopea[n+bs] = measa & 0xFF;
            measb = ADC_read_signal(Ain2); // analogRead(A2);
            scopeb[n] = (measb & 0xFF00) >> 8;
            scopeb[n+bs] = measb & 0xFF;
            measc = ADC_read_signal(Ain3); // analogRead(A3);
            scopec[n] = (measc & 0xFF00) >> 8;
            scopec[n+bs] = measc & 0xFF;
            digmeas = 0; // multipleDigitalRead(inputPins, numberOfInputPins); // returns corresponding decimal number reading fom input pins
            digin[n]= digmeas & 0x3F;
            ta+=st;
            while (ta>micros());
          }
          TotalReal=micros()-StartReal;
          //stReal=TotalReal/bs; // calculate the average time for each reading
          digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
          Serial.print("stReal= ");
          Serial.println(TotalReal);
          Serial.write(scopea, sizeof(scopea));
          Serial.write(scopeb, sizeof(scopeb));
          Serial.write(scopec, sizeof(scopec));
          Serial.write(digin, sizeof(digin));
          Serial.println("");
          //
          digitalWrite(LED_BUILTIN, LOW);  // turn the LED off (HIGH is the voltage level)
          break;
          //
        //delay(1);
      }
    }
  }
}

void loop () {

}




