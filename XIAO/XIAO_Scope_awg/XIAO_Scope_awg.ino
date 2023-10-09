//XIAO Scope2 1/2/3 channel scope 1 internal awg (10/1/2023)
//
//#include "arduino_m0_tweak.hpp"
#include <TimerTC3.h>

#ifndef MULTIPLE_PIN_OPS_H
#define MULTIPLE_PIN_OPS_H
//

void multiplePinMode(const int *pins, const int numberOfPins, uint8_t mode);
void multipleDigitalWrite(const int *pins, const int numberOfPins, uint16_t number);
uint64_t multipleDigitalRead(const int *pins, const int numberOfPins);
void multipleAnalogRead(const int *pins, const int numberOfPins, int *values);
void multipleAnalogWrite(const int *pins, const int numberOfPins, uint8_t* values);

#endif // MULTIPLE_PIN_OPS_H
unsigned int scopea[1024]; 
unsigned int scopeb[1024];
unsigned int scopec[1024];
unsigned short digin[1024];
unsigned int awgouta[2048];

int wavea;
int cyclea;
int ampla;
int offseta;
int awgon;
int addr;
int data;
int awgdata;
int n;
int sync = 0;
float Step;
unsigned int at, at2, st, st2, stReal;
int bmax=2048;
int bs=512;
int ws=1024;
int pwmf = 500;
int pwid = 500;

void setup() {
  Serial.begin(2000000);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  wavea = 1; // default to sine shape (for now)
  cyclea = 8; // default is 8 cycles in 1024 samples
  ampla = 500; // default is 127 or full 0 to 255 peak to prak
  offseta = 512; // default is 128 or mid range 0 to 255
  awgon = 0; // default AWG off
  makewavea();   
  int measa;
  int measb;
  int measc;
  int digmeas;
  //int VDD;
  char c, c2;
  uint32_t ta, TotalReal, StartReal;
  st=11;
  at=20;
  //
  const int numberOfInputPins = 6;
  const int inputPins[numberOfInputPins] = {D4, D5, D6, D7, D8, D9}; // pins to use as inputs
  multiplePinMode(inputPins, numberOfInputPins, INPUT);
  // start AWG timer
  TimerTc3.initialize(at);
  TimerTc3.attachInterrupt(updatedac);
  //
  //m0tweak::cpuFrequency(64);
  //m0tweak::adcPrecision(12);
  analogReadResolution(12);
  //ADC->CTRLB.reg = ADC_CTRLB_PRESCALER_DIV32;
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
  //pinMode(A3, INPUT); 
  while (true) { 
    if (Serial.available()){
      c=Serial.read();
      switch (c){
        case 'I': // read back FW Rev
          Serial.println ("XIAO Scope 3.0");
          break;
        case 'T': // change the AWG value of at in uSec both channels must be same rate
          at2 = Serial.parseInt();
          if(at2>0){
            at=at2;
            TimerTc3.detachInterrupt();
            TimerTc3.initialize(at);
            TimerTc3.attachInterrupt(updatedac);
          }
          break;
        case 'B': // change number of AWG samples both channels must be the same length
          ws = Serial.parseInt();
          if(ws>bmax){
            ws=bmax;
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
            if(data > 1023){
              data=1023;
            }
            if(data < 0){
              data=0;
            }
          } else {
            data = 0;
          }
          awgouta[addr] = data;
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
        case 'N': // change number of waveform cycles ch a
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
            awgon = 1;
            TimerTc3.attachInterrupt(updatedac);
          }else{
            awgon = 0;
            TimerTc3.detachInterrupt();
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
            pwm(D10, pwmf, pwid);
          }else{
            pwm(D10, pwmf, 0);
          }
          break;
        case 'p': // change analog write pwm frequency value
          pwmf = Serial.parseInt();
          pwm(D10, pwmf, pwid);
          break;
        case 'm': // change pwm duty cycle % 
          pwid = Serial.parseInt(); // range from 0 (0%) to 1000 (100%)
          pwm(D10, pwmf, pwid);
          break;
        case '1': // do scope ch a single capture
        // if sync is on reset start of awg buffer pointer
          if (sync > 0 ) {
            n = 0;
            delayMicroseconds(10);
          }
          ta = micros();
          StartReal = ta;
          for (int n = 0; n < bs; n++){ // Fill Buffer
            scopea[n]=analogRead(A1);
            digin[n]=digitalRead(D4);
            ta+=st;
            while (ta>micros());
          }
          TotalReal=micros()-StartReal;
          stReal=TotalReal/bs; // calculate the average time for each reading
          digitalWrite(LED_BUILTIN, LOW);  // turn the LED on (HIGH is the voltage level)
          Serial.print("stReal= ");
          Serial.println(stReal);
          for (int n = 0; n < bs; n++){ // Dunp Buffer over serial
            measa = scopea[n];
            measa = measa;
            digmeas = digin[n];
            Serial.write((byte) 0x00);
            Serial.write((measa & 0xFF00) >> 8);
            Serial.write(measa & 0xFF);
            Serial.write(digmeas & 0xFF);
          }
          digitalWrite(LED_BUILTIN, HIGH);  // turn the LED off (HIGH is the voltage level)
          break;
          //
        case '2': // do scope ch a and b single capture
        // if sync is on reset start of awg buffer pointer
          if (sync > 0 ) {
            n = 0;
            delayMicroseconds(10);
          }
          ta = micros();
          StartReal = ta;
          for (int n = 0; n < bs; n++){ // Fill Buffer
            scopea[n]=analogRead(A1);
            scopeb[n]=analogRead(A2);
            digin[n]=digitalRead(D4)+128;
            ta+=st;
            while (ta>micros());
          }
          TotalReal=micros()-StartReal;
          stReal=TotalReal/bs; // calculate the average time for each reading
          digitalWrite(LED_BUILTIN, LOW);  // turn the LED on (HIGH is the voltage level)
          Serial.print("stReal= ");
          Serial.println(stReal);
          for (int n = 0; n < bs; n++){ // Dunp Buffer over serial
            measa = scopea[n];
            measa = measa;
            measb = scopeb[n];
            measb = measb;
            digmeas = digin[n];
            Serial.write((byte) 0x00);
            Serial.write((measa & 0xFF00) >> 8);
            Serial.write(measa & 0xFF);
            Serial.write((measb & 0xFF00) >> 8);
            Serial.write(measb & 0xFF);
            Serial.write(digmeas & 0xFF);
          }
          digitalWrite(LED_BUILTIN, HIGH);  // turn the LED off (HIGH is the voltage level)
          break;
          //
        case '3': // do scope ch a b and c single capture
        // if sync is on reset start of awg buffer pointer
          if (sync > 0 ) {
            n = 0;
            delayMicroseconds(10);
          }
          ta = micros();
          StartReal = ta;
          for (int n = 0; n < bs; n++){ // Fill Buffer
            scopea[n]=analogRead(A1);
            scopeb[n]=analogRead(A2);
            scopec[n]=analogRead(A3);
            digin[n]=digitalRead(D4);
            ta+=st;
            while (ta>micros());
          }
          TotalReal=micros()-StartReal;
          stReal=TotalReal/bs; // calculate the average time for each reading
          digitalWrite(LED_BUILTIN, LOW);  // turn the LED on (HIGH is the voltage level)
          Serial.print("stReal= ");
          Serial.println(stReal);
          for (int n = 0; n < bs; n++){ // Dunp Buffer over serial
            measa = scopea[n] << 1;
            measa = measa + 257;
            measb = scopeb[n] << 1;
            measb = measb + 257;
            measc = scopec[n] << 1;
            measc = measc + 257;
            digmeas = digin[n];
            Serial.write((byte) 0x00);
            Serial.write((measa & 0xFF00) >> 8);
            Serial.write(measa & 0xFF);
            Serial.write((measb & 0xFF00) >> 8);
            Serial.write(measb & 0xFF);
            Serial.write((measc & 0xFF00) >> 8);
            Serial.write(measc & 0xFF);
            Serial.write(digmeas & 0xFF);
          }
          digitalWrite(LED_BUILTIN, HIGH);  // turn the LED off (HIGH is the voltage level)
          break;
        case '4': // do scope ch a single capture
        // if sync is on reset start of awg buffer pointer
          if (sync > 0 ) {
            n = 0;
            delayMicroseconds(10);
          }
          ta = micros();
          StartReal = ta;
          for (int n = 0; n < bs; n++){ // Fill Buffer
            scopea[n]=analogRead(A1);
            digin[n]=multipleDigitalRead(inputPins, numberOfInputPins); // returns corresponding decimal number reading fom input pins
            ta+=st;
            while (ta>micros());
          }
          TotalReal=micros()-StartReal;
          stReal=TotalReal/bs; // calculate the average time for each reading
          digitalWrite(LED_BUILTIN, LOW);  // turn the LED on (HIGH is the voltage level)
          Serial.print("stReal= ");
          Serial.println(stReal);
          for (int n = 0; n < bs; n++){ // Dunp Buffer over serial
            measa = scopea[n] << 1;
            measa = measa + 257;
            digmeas = digin[n];
            Serial.write((byte) 0x00);
            Serial.write((measa & 0xFF00) >> 8);
            Serial.write(measa & 0xFF);
            Serial.write(digmeas & 0xFF);
          }
          digitalWrite(LED_BUILTIN, HIGH);  // turn the LED off (HIGH is the voltage level)
          break;
        case '5': // do scope ch a and b single capture
        // if sync is on reset start of awg buffer pointer
          if (sync > 0 ) {
            n = 0;
            delayMicroseconds(10);
          }
          ta = micros();
          StartReal = ta;
          for (int n = 0; n < bs; n++){ // Fill Buffer
            scopea[n]=analogRead(A1);
            scopeb[n]=analogRead(A2);
            digin[n]=multipleDigitalRead(inputPins, numberOfInputPins); // returns corresponding decimal number reading fom input pins
            ta+=st;
            while (ta>micros());
          }
          TotalReal=micros()-StartReal;
          stReal=TotalReal/bs; // calculate the average time for each reading
          digitalWrite(LED_BUILTIN, LOW);  // turn the LED on (HIGH is the voltage level)
          Serial.print("stReal= ");
          Serial.println(stReal);
          for (int n = 0; n < bs; n++){ // Dunp Buffer over serial
            measa = scopea[n] << 1;
            measa = measa + 257;
            measb = scopeb[n] << 1;
            measb = measb + 257;
            digmeas = digin[n];
            Serial.write((byte) 0x00);
            Serial.write((measa & 0xFF00) >> 8);
            Serial.write(measa & 0xFF);
            Serial.write((measb & 0xFF00) >> 8);
            Serial.write(measb & 0xFF);
            Serial.write(digmeas & 0xFF);
          }
          digitalWrite(LED_BUILTIN, HIGH);  // turn the LED off (HIGH is the voltage level)
          break;
        case '6': // do scope ch a b and c single capture
        // if sync is on reset start of awg buffer pointer
          if (sync > 0 ) {
            n = 0;
            delayMicroseconds(10);
          }
          ta = micros();
          StartReal = ta;
          for (int n = 0; n < bs; n++){ // Fill Buffer
            scopea[n]=analogRead(A1);
            scopeb[n]=analogRead(A2);
            scopec[n]=analogRead(A3);
            digin[n]=multipleDigitalRead(inputPins, numberOfInputPins); // returns corresponding decimal number reading fom input pins
            ta+=st;
            while (ta>micros());
          }
          TotalReal=micros()-StartReal;
          stReal=TotalReal/bs; // calculate the average time for each reading
          digitalWrite(LED_BUILTIN, LOW);  // turn the LED on (HIGH is the voltage level)
          Serial.print("stReal= ");
          Serial.println(stReal);
          for (int n = 0; n < bs; n++){ // Dunp Buffer over serial
            measa = scopea[n] << 1;
            measa = measa + 257;
            measb = scopeb[n] << 1;
            measb = measb + 257;
            measc = scopec[n] << 1;
            measc = measc + 257;
            digmeas = digin[n];
            Serial.write((byte) 0x00);
            Serial.write((measa & 0xFF00) >> 8);
            Serial.write(measa & 0xFF);
            Serial.write((measb & 0xFF00) >> 8);
            Serial.write(measb & 0xFF);
            Serial.write((measc & 0xFF00) >> 8);
            Serial.write(measc & 0xFF);
            Serial.write(digmeas & 0xFF);
          }
          digitalWrite(LED_BUILTIN, HIGH);  // turn the LED off (HIGH is the voltage level)
          break;
          //
        //delay(1);
      }
    }
  }
}

void loop () {

}

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
  if (awgon) {
    analogWrite(A0, awgouta[n]);
    n++;
    if (n > ws) n = 0;
  } else {
    analogWrite(A0, 0);
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
    for (int j = 0; j < ws; j++){
      int temp = offseta + (ampla * sin(2*cyclea*PI*j/ws));
      if(temp > 1023){
        temp=1023;
      }
      if(temp < 0){
        temp=0;
      }
      awgouta[j] = temp;
    }
  } else if(wavea == 2){ // make a triangle wave shape
    for (int j = 0; j < ws; j++){
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
    for (int j = 0; j < ws; j++){
      awgouta[j] = offseta;
    }
  }
}


