//ItsyBitsy M4 Express Scope3 1/2/3 channel scope 1 internal awg 
// (3/10/2024)
#include <Adafruit_ZeroTimer.h>
#include "Adafruit_NeoPixel.h"

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN 47 // On Trinket or Gemma, suggest changing this to 1

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS 1 // Popular NeoPixel ring size
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// timer tester
Adafruit_ZeroTimer zerotimer = Adafruit_ZeroTimer(3);

uint8_t scopea[16384]; // 14336/8 ~ 1700 max samples
short awgouta[2048];
short awgoutb[2048];

short awgona;
short awgonb;
//short awgpwnon;
short awgres;
short addr;
short data;
//int awgdata;
short n;
short m;
short sync = 0;
short point = 0;
// Stefloatp;
unsigned int at, at2, st, st2, stReal;
short amax=2048; // Max length of AWG memory
short bmax=2048; // = 16384 / 8 Max number of capture memory per channel
uint16_t bs=1024;
uint16_t tbs=2048;
uint16_t rbs=3072;
uint16_t fbs=4096;
uint16_t vbs=5120;
uint16_t sbs=6144;
uint16_t ebs=7168;
uint16_t gbs=8192;
short dy=4;
uint8_t gn = 0xF; // default to gain of 1
uint8_t vn = 0x1;
uint16_t ns=1024;
uint16_t ms=1024;
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
  ADC0->CTRLA.bit.PRESCALER = ADC_CTRLA_PRESCALER_DIV8_Val; // ADC_CTRLA_PRESCALER_DIV16_Val;
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
  ADC0->SAMPCTRL.bit.SAMPLEN = dy;
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
// DAC output timer function fixed length of 1024 for now
void updatedac() {
  if (awgona) {
    if (n >= ns) n = 0;
    DAC->DATA[0].reg = awgouta[n]; // analogWrite(A0, awgouta[n]);
    n++;
  } else {
    DAC->DATA[0].reg = 0; // analogWrite(A0, 0);
    n = 0;
  }
  if (awgonb) {
    if (m >= ms) m = 0;
    DAC->DATA[1].reg = awgoutb[m]; // analogWrite(A1, awgoutb[m]); // pwm(A10, pwmf, awgoutb[m]);
      m++;
  } else {
    DAC->DATA[1].reg = awgoutb[m]; // analogWrite(A1, 0);
    m = 0;
  }
}

void setup() {
  Serial.begin(2000000);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  RGB_LED_off();
  awgona = 0; // default AWG a off
  awgonb = 0; // default AWG b off
  awgres = 4095;  
  short measa;
  short measb;
  short measc;
  short measd;
  short digmeas;
  //int VDD;
  char c, c2;
  uint32_t ta, TotalReal, StartReal;
  st=10;
  at=10;
  short Ain1;
  Ain1 = 2; // pin number for A2
  short Ain2;
  Ain2 = 3; // pin number for A3
  short Ain3;
  Ain3 = 4; // pin number for A4
  short Ain4;
  Ain4 = 6; // pin number for A5
  
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
  pinMode(2, INPUT);
  pinMode(3, INPUT);
  pinMode(4, INPUT);
  //pinMode(5, INPUT); 
  pinMode(6, INPUT);
  analogWriteResolution(12);
  analogWrite(A0, 0); // DAC 0 out
  analogWrite(A1, 0); // DAC 1 out
  //analogReadResolution(12);
  ADC_init();
  //
  while (true) { 
    if (Serial.available()){
      c=Serial.read();
      switch (c){
        case 'I': // read back FW Rev
          Serial.println ("Bitsy M4 Scope 4.0");
          break;
        case 'T': // change the AWG value of at in uSec both channels must be same rate
          at2 = Serial.parseInt();
          if(at2>0){
            at=at2;
            //zerotimer.enable(false);
            compare = 48*at;
            zerotimer.setCompare(0, compare);
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
          tbs = bs * 2;
          }
          tbs = bs * 2;
          rbs = bs * 3;
          fbs = bs * 4;
          vbs = bs * 5;
          sbs = bs * 6;
          ebs = bs * 7;
          gbs = bs * 8;
          break;
        case 'd': // set ADC settling delay
          // Sampling length, larger means increased max input impedance
          // default 63, stable 32 @ DIV32 & SAMPLENUM_4
          dy = Serial.parseInt();
          ADC0->SAMPCTRL.bit.SAMPLEN = dy;
          break;
        case 'h': // set ADC gain setting
          // 
          gn = Serial.parseInt();
          //ADC0->INPUTCTRL.bit.GAIN = gn & 0xF; // ADC_INPUTCTRL_GAIN_DIV2_Val;
          syncADC(ADC0, ADC_SYNCBUSY_MASK);
          break;
        case 'v': // set ADC Reference setting
          // 
          vn = Serial.parseInt();
          if(vn == 0){
            ADC0->REFCTRL.bit.REFSEL = 0x1; //ADC_REFCTRL_REFSEL_INT1V_Val; // 1.0V voltage reference
          }
          if(vn == 1){
            ADC0->REFCTRL.bit.REFSEL = ADC_REFCTRL_REFSEL_INTVCC0_Val; // 1/1.48 VDDANA = 1/1.48* 3V3 = 2.2297
          }
          if(vn == 2){
            ADC0->REFCTRL.bit.REFSEL = ADC_REFCTRL_REFSEL_INTVCC1_Val; // 1/2 VDDANA
          }
          if(vn == 3){
            ADC0->REFCTRL.bit.REFSEL = ADC_REFCTRL_REFSEL_AREFA_Val; // Use external Ref voltage 
          }
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
        case 'R': // Reset AWG start point at start of aquire if > 0 and set delay in uSec
          sync = Serial.parseInt();
          break;
        case 'r': // Set AWG start address when case R > 0
          point = Serial.parseInt();
          break;
        case 's': // enable - disable PWM output awgpwnon
          c2 = Serial.read();
          if(c2=='o'){
            analogWrite(A6, pwid); //pwm(A10, pwmf, pwid);
          }else{
            analogWrite(A6, 0); //pwm(A10, pwmf, 0);
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
        // Read ADC channels
        case '1': // do scope ch a and b single capture
        // if sync is on reset start of awg buffer pointer
          if (sync > 0 ) {
            n = point;
            m = point;
            delayMicroseconds(sync);
          }
          ta = micros();
          StartReal = ta;
          for (int i = 0; i < bs; i++){ // Fill Buffer
            measa = ADC_read_signal(Ain1); // analogRead(A1);
            scopea[i] = (measa & 0xFF00) >> 8;
            scopea[i+bs] = measa & 0xFF;
            ta+=st;
            while (ta>micros());
          }
          TotalReal=micros()-StartReal;
          //stReal=TotalReal/bs; // calculate the average time for each reading
          digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
          Serial.print("stReal= ");
          Serial.println(TotalReal);
          // Dump Buffer over serial
          Serial.write(scopea, tbs);
          Serial.println("");
          //
          digitalWrite(LED_BUILTIN, LOW);  // turn the LED off (HIGH is the voltage level)
          break;
          //
        case '2': // do scope ch a and c single capture
        // if sync is on reset start of awg buffer pointer
          if (sync > 0 ) {
            n = point;
            m = point;
            delayMicroseconds(sync);
          }
          ta = micros();
          StartReal = ta;
          for (int i = 0; i < bs; i++){ // Fill Buffer
            measa = ADC_read_signal(Ain1); // analogRead(A1);
            measb = ADC_read_signal(Ain2); // analogRead(A2);
            scopea[i] = (measa & 0xFF00) >> 8;
            scopea[i+bs] = measa & 0xFF;
            scopea[i+tbs] = (measb & 0xFF00) >> 8;
            scopea[i+rbs] = measb & 0xFF;
            ta+=st;
            while (ta>micros());
          }
          TotalReal=micros()-StartReal;
          //stReal=TotalReal/bs; // calculate the average time for each reading
          digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
          Serial.print("stReal= ");
          Serial.println(TotalReal);
          // Dump Buffer over serial
          Serial.write(scopea, fbs);
          Serial.println("");
          //
          digitalWrite(LED_BUILTIN, LOW);  // turn the LED off (HIGH is the voltage level)
          break;
          //
        case '3': // do scope ch b and c single capture
        // if sync is on reset start of awg buffer pointer
          if (sync > 0 ) {
            n = point;
            m = point;
            delayMicroseconds(sync);
          }
          ta = micros();
          StartReal = ta;
          for (int i = 0; i < bs; i++){ // Fill Buffer
            measa = ADC_read_signal(Ain1); // analogRead(A1);
            measb = ADC_read_signal(Ain2); // analogRead(A2);
            measc = ADC_read_signal(Ain3); // analogRead(A3);
            scopea[i] = (measa & 0xFF00) >> 8;
            scopea[i+bs] = measa & 0xFF;
            scopea[i+tbs] = (measb & 0xFF00) >> 8;
            scopea[i+rbs] = measb & 0xFF;
            scopea[i+fbs] = (measc & 0xFF00) >> 8;
            scopea[i+vbs] = measc & 0xFF;
            ta+=st;
            while (ta>micros());
          }
          TotalReal=micros()-StartReal;
          //stReal=TotalReal/bs; // calculate the average time for each reading
          digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
          Serial.print("stReal= ");
          Serial.println(TotalReal);
          // Dump Buffer over serial
          Serial.write(scopea, sbs);
          Serial.println("");
          //
          digitalWrite(LED_BUILTIN, LOW);  // turn the LED off (HIGH is the voltage level)
          break;
          //
        case '4': // do scope ch a b and c single capture
        // if sync is on reset start of awg buffer pointer
          if (sync > 0 ) {
            n = point;
            m = point;
            delayMicroseconds(sync);
          }
          ta = micros();
          StartReal = ta;
          for (int i = 0; i < bs; i++){ // Fill Buffer
            measa = ADC_read_signal(Ain1); // analogRead(A1);
            measb = ADC_read_signal(Ain2); // analogRead(A2);
            measc = ADC_read_signal(Ain3); // analogRead(A3);
            measd = ADC_read_signal(Ain4); // analogRead(A4);
            scopea[i] = (measa & 0xFF00) >> 8;
            scopea[i+bs] = measa & 0xFF;
            scopea[i+tbs] = (measb & 0xFF00) >> 8;
            scopea[i+rbs] = measb & 0xFF;
            scopea[i+fbs] = (measc & 0xFF00) >> 8;
            scopea[i+vbs] = measc & 0xFF;
            scopea[i+sbs] = (measd & 0xFF00) >> 8;
            scopea[i+ebs] = measd & 0xFF;
            ta+=st;
            while (ta>micros());
          }
          TotalReal=micros()-StartReal;
          //stReal=TotalReal/bs; // calculate the average time for each reading
          digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
          Serial.print("stReal= ");
          Serial.println(TotalReal);
          // Dump Buffer over serial
          Serial.write(scopea, gbs);
          Serial.println("");
          //
          digitalWrite(LED_BUILTIN, LOW);  // turn the LED off (HIGH is the voltage level)
          break;
        
        //delay(1);
      }
    }
  }
}

void loop () {

}




