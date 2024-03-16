//XIAO Scope 4 exp 1/2/3/4 analog channel scope 1 internal awg 5 digital inputs (3/5/2024)
//
#include <TimerTC3.h>

uint8_t scopea[16384]; // 14336/8 ~ 1700 max samples
//uint8_t scopeb[4096];
//uint8_t scopec[4096]; 
//uint8_t digin[2048];
uint16_t awgouta[2048];
uint16_t awgoutb[2048];
short awgon;
short awgpwnon;
short addr;
short data;
short n;
short m;
short sync = 0;
short point = 0;
float Step;
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
short dy=1;
uint8_t gn = 0xF; // default to gain of 1
uint8_t vn = 0x1;
uint16_t ns=1024;
uint16_t ms=1024;
int pwmf = 500;
int pwid = 500;
//
static inline void syncADC() {
  // Taken from `hri_adc_d21.h`
  while (ADC->STATUS.bit.SYNCBUSY == 1) {}
}
void ADC_init() {
  /* Increase the ADC clock by setting the PRESCALER from default DIV128 to a
  smaller divisor. This is needed for DAQ rates larger than ~20 kHz on SAMD51
  and DAQ rates larger than ~10 kHz on SAMD21. Setting too small divisors will
  result in ADC errors. Keep as large as possible to increase ADC accuracy.

  SAMD21:
    ADC input clock is default at 48 MHz (Generic Clock Generator 0).
    \.platformio\packages\framework-arduino-samd\cores\arduino\wiring.c

    Max ADC input clock frequency    :  48 MHz    (datasheet table 37-7)
    Max ADC internal clock frequency : 2.1 MHz    (datasheet table 37-24)

    Handy calculator:
    https://blog.thea.codes/getting-the-most-out-of-the-samd21-adc/

  */

  // Turn off ADC
  ADC->CTRLA.reg = 0;
  syncADC();

  // Load the factory calibration
  uint32_t bias =
      (*((uint32_t *)ADC_FUSES_BIASCAL_ADDR) & ADC_FUSES_BIASCAL_Msk) >>
      ADC_FUSES_BIASCAL_Pos;
  uint32_t linearity =
      (*((uint32_t *)ADC_FUSES_LINEARITY_0_ADDR) & ADC_FUSES_LINEARITY_0_Msk) >>
      ADC_FUSES_LINEARITY_0_Pos;
  linearity |= ((*((uint32_t *)ADC_FUSES_LINEARITY_1_ADDR) &
                 ADC_FUSES_LINEARITY_1_Msk) >>
                ADC_FUSES_LINEARITY_1_Pos)
               << 5;

  ADC->CALIB.reg =
      ADC_CALIB_BIAS_CAL(bias) | ADC_CALIB_LINEARITY_CAL(linearity);
  // No sync needed according to `hri_adc_d21.h`

  // The ADC clock must remain below 2.1 MHz, see SAMD21 datasheet table 37-24.
  // Hence, don't go below DIV32 @ 48 MHz.
  ADC->CTRLB.bit.PRESCALER = ADC_CTRLB_PRESCALER_DIV16_Val;
  syncADC();

  // AnalogRead resolution
  ADC->CTRLB.bit.RESSEL = ADC_CTRLB_RESSEL_16BIT_Val;
  syncADC();

  // Sample averaging
  ADC->AVGCTRL.bit.SAMPLENUM = ADC_AVGCTRL_SAMPLENUM_1_Val;
  // No sync needed according to `hri_adc_d21.h`
  ADC->AVGCTRL.bit.ADJRES = 1; // 2^N, must match `ADC0->AVGCTRL.bit.SAMPLENUM`
  // No sync needed according to `hri_adc_d21.h`

  // Sampling length, larger means increased max input impedance
  // default 63, stable 32 @ DIV32 & SAMPLENUM_4
  ADC->SAMPCTRL.bit.SAMPLEN = dy;
  // No sync needed according to `hri_adc_d21.h`

  ADC->CTRLB.bit.DIFFMODE = 0;
  syncADC();
  ADC->INPUTCTRL.bit.MUXPOS = g_APinDescription[A1].ulADCChannelNumber;
  syncADC();
  ADC->INPUTCTRL.bit.MUXNEG = ADC_INPUTCTRL_MUXNEG_GND_Val;
  syncADC();
  //ADC->REFCTRL.bit.REFSEL = ADC_REFCTRL_REFSEL_INT1V_Val; // 1.0V voltage reference
  //ADC->REFCTRL.bit.REFSEL = ADC_REFCTRL_REFSEL_INTVCC0_Val; // 1/1.48 VDDANA = 1/1.48* 3V3 = 2.2297
  ADC->REFCTRL.bit.REFSEL = ADC_REFCTRL_REFSEL_INTVCC1_Val; // 1/2 VDDANA
  //ADC->REFCTRL.bit.REFSEL = ADC_REFCTRL_REFSEL_AREFA_Val; // Use external Ref voltage 
  // No sync needed according to `hri_adc_d21.h`
  ADC->REFCTRL.bit.REFCOMP = 0;
  // No sync needed according to `hri_adc_d21.h`

  ADC->INPUTCTRL.bit.GAIN = ADC_INPUTCTRL_GAIN_DIV2_Val;
  syncADC();
  // Turn on ADC
  ADC->CTRLA.bit.ENABLE = 1;
  syncADC();
}
//
int16_t ADC_read_signal(int pin) {
  ADC->INPUTCTRL.bit.MUXPOS = pin; // Selection new conversion pin
  ADC->SWTRIG.bit.START = 1; // Request start conversion
  syncADC();                 // Make sure conversion has been initiated
  __asm__("nop\n\t");        // Tiny delay before we can poll RESRDY
  __asm__("nop\n\t");
  __asm__("nop\n\t");
  while (!ADC->INTFLAG.bit.RESRDY) {} // Wait for result ready to be read
  return ADC->RESULT.reg;             // Read result
}

// DAC output timer function fixed length of 1024 for now
void updatedac() {
  if (awgon) {
    if (n >= ns) n = 0;
    DAC->DATA.reg = awgouta[n]; // analogWrite(A0, awgouta[n]);
    n++;
    if (awgpwnon) {
      if (m >= ms) m = 0;
      TCC1->CC[0].reg = awgoutb[m]; // Set Width Reg
      //pwm(D10, pwmf, awgoutb[m]);
      m++;
    }
  } else {
    DAC->DATA.reg = 0; // analogWrite(A0, 0);
    n = 0;
    m = 0;
    if (awgpwnon) {
      TCC1->CC[0].reg = 0; // pwm(D10, pwmf, 0);
    }
  }
}

void setup() {
  Serial.begin(2000000);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  awgon = 0; // default AWG off   
  short measa;
  short measb;
  short measc;
  short measd;
  short digmeas;
  //int VDD;
  char c, c2;
  uint32_t ta, TotalReal, StartReal;
  st=11;
  at=20;
  short Ain1;
  Ain1 = 4; // pin number for A1
  short Ain2;
  Ain2 = 18; // pin number for A2
  short Ain3;
  Ain3 = 19; // pin number for A3
  short Ain4;
  Ain4 = 16; // pin number for A4
  //
  // start AWG timer
  TimerTc3.initialize(at);
  TimerTc3.attachInterrupt(updatedac);
  //
  //m0tweak::cpuFrequency(64);
  //m0tweak::adcPrecision(12);
  analogWriteResolution(10);
  analogWrite(A0, 0); // DAC_init();
  //analogReadResolution(12);
  ADC_init();
  //ADC->CTRLB.reg = ADC_CTRLB_PRESCALER_DIV32;
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
  pinMode(A4, INPUT);
  // pinMode(D10, OUTPUT);
  pinMode(D5, INPUT);
  pinMode(D6, INPUT);
  pinMode(D7, INPUT);
  pinMode(D8, INPUT);
  pinMode(D9, INPUT);
  while (true) { 
    if (Serial.available()){
      c=Serial.read();
      switch (c){
        case 'I': // read back FW Rev
          Serial.println ("XIAO Scope 4.0");
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
        case 'N': // change number of AWG A samples 
          ns = Serial.parseInt();
          if(ns>amax){
            ns=amax;
          }
          break;
        case 'M': // change number of AWG B samples 
          ms = Serial.parseInt();
          if(ms>amax){
            ms=amax;
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
            if(data > 1023){
              data=1023;
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
          ADC->SAMPCTRL.bit.SAMPLEN = dy;
          break;
        case 'g': // set ADC gain setting
          // 
          gn = Serial.parseInt();
          ADC->INPUTCTRL.bit.GAIN = gn & 0xF; // ADC_INPUTCTRL_GAIN_DIV2_Val;
          syncADC();
          break;
        case 'v': // set ADC Reference setting
          // 
          vn = Serial.parseInt();
          if(vn == 0){
            ADC->REFCTRL.bit.REFSEL = ADC_REFCTRL_REFSEL_INT1V_Val; // 1.0V voltage reference
          }
          if(vn == 1){
            ADC->REFCTRL.bit.REFSEL = ADC_REFCTRL_REFSEL_INTVCC0_Val; // 1/1.48 VDDANA = 1/1.48* 3V3 = 2.2297
          }
          if(vn == 2){
            ADC->REFCTRL.bit.REFSEL = ADC_REFCTRL_REFSEL_INTVCC1_Val; // 1/2 VDDANA
          }
          if(vn == 3){
            ADC->REFCTRL.bit.REFSEL = ADC_REFCTRL_REFSEL_AREFA_Val; // Use external Ref voltage 
          }
          break;
        case 'G': // enable - disable AWG output
          c2 = Serial.read();
          if(c2=='o'){
            awgon = 1;
            n = 0;
            m = 0;
            TimerTc3.attachInterrupt(updatedac);
          }else{
            awgon = 0;
            TimerTc3.detachInterrupt();
          }
          break;
        case 'R': // Reset AWG start point at start of aquire if > 0 and set delay in uSec
          sync = Serial.parseInt();
          break;
        case 'r': // Set AWG start address when case R > 0
          point = Serial.parseInt();
          break;
        case 'S': // enable - disable AWG PWM output 
          c2 = Serial.read();
          if(c2=='o'){
            awgpwnon = 1;
            TCC1->PER.reg = 512; // Set Frequency divider
            // pwm(D10, 96000, 500);
          }else{
            awgpwnon = 0;
            TCC1->PER.reg = 512; // pwm(D10, 500, 0);
            TCC1->CC[0].reg = 0;
          }
          break;
        case 's': // enable - disable PWM output
          c2 = Serial.read();
          if(c2=='o'){
            // set digital pin to high current mode
            //TCC1->PER.reg = pwmf; 
            //pwm(D10, 500, 0);
            //TCC1->CC[0].reg = pwid; 
            pwm(D10, pwmf, pwid);
            PORT->Group[g_APinDescription[D10].ulPort].PINCFG[g_APinDescription[D10].ulPin].bit.DRVSTR = 1;
          }else{
            //TCC1->CC[0].reg = 0;
            pwm(D10, pwmf, 0);
            PORT->Group[g_APinDescription[D10].ulPort].PINCFG[g_APinDescription[D10].ulPin].bit.DRVSTR = 0;
          }
          break;
        case 'p': // change analog write pwm frequency value
          pwmf = Serial.parseInt();
          //TCC1->PER.reg = 8192;
          pwm(D10, pwmf, pwid);
          PORT->Group[g_APinDescription[D10].ulPort].PINCFG[g_APinDescription[D10].ulPin].bit.DRVSTR = 1;
          break;
        case 'm': // change pwm duty cycle % 
          pwid = Serial.parseInt(); // range from 0 (0%) to 1000 (100%)
          //TCC1->CC[0].reg = pwid;
          pwm(D10, pwmf, pwid);
          PORT->Group[g_APinDescription[D10].ulPort].PINCFG[g_APinDescription[D10].ulPin].bit.DRVSTR = 1;
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
        case '0': // capture just the 5 digital pins
        // if sync is on reset start of awg buffer pointer
          if (sync > 0 ) {
            n = 0;
            m = 0;
            delayMicroseconds(sync);
          }
          ta = micros();
          StartReal = ta;
          for (int i = 0; i < bs; i++){ // Fill Buffer
            //digmeas = multipleDigitalRead(inputPins, numberOfInputPins); // returns corresponding decimal number reading fom input pins
            //digmeas = digitalRead(D5);
            //digmeas += digitalRead(D6) << 1;
            //digmeas += digitalRead(D7) << 2;
            //digmeas += digitalRead(D8) << 3;
            //digmeas += digitalRead(D9) << 4;
            digmeas = (REG_PORT_IN0 & PORT_PA09) >> 9;
            digmeas += (REG_PORT_IN1 & PORT_PB08) >> 7;
            digmeas += (REG_PORT_IN1 & PORT_PB09) >> 7;
            digmeas += (REG_PORT_IN0 & PORT_PA07) >> 4;
            digmeas += (REG_PORT_IN0 & PORT_PA05 >> 1);
            scopea[i] = digmeas & 0x1F;
            ta+=st;
            while (ta>micros());
          }
          TotalReal=micros()-StartReal;
          //stReal=TotalReal/bs; // calculate the average time for each reading
          digitalWrite(LED_BUILTIN, LOW);  // turn the LED on (HIGH is the voltage level)
          Serial.print("stReal= ");
          Serial.println(TotalReal);
          Serial.write(scopea, bs);
          Serial.println("");
          //
          digitalWrite(LED_BUILTIN, HIGH);  // turn the LED off (HIGH is the voltage level)
          break;
        case '1': // do scope ch a single capture
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
          digitalWrite(LED_BUILTIN, LOW);  // turn the LED on (HIGH is the voltage level)
          Serial.print("stReal= ");
          Serial.println(TotalReal); // report total time for bs samples
          // dump buffer over serial
          Serial.write(scopea, tbs);
          Serial.println("");
          //
          digitalWrite(LED_BUILTIN, HIGH);  // turn the LED off (HIGH is the voltage level)
          break;
          //
        case '2': // do scope ch a and b single capture
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
          digitalWrite(LED_BUILTIN, LOW);  // turn the LED on (HIGH is the voltage level)
          Serial.print("stReal= ");
          Serial.println(TotalReal);
          // Dump Buffer over serial
          Serial.write(scopea, fbs);
          Serial.println("");
          //
          digitalWrite(LED_BUILTIN, HIGH);  // turn the LED off (HIGH is the voltage level)
          break;
          //
        case '3': // do scope ch a b and c single capture
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
          digitalWrite(LED_BUILTIN, LOW);  // turn the LED on (HIGH is the voltage level)
          Serial.print("stReal= ");
          Serial.println(TotalReal);
          // Dump Buffer over serial
          Serial.write(scopea, sbs);
          Serial.println("");
          //
          digitalWrite(LED_BUILTIN, HIGH);  // turn the LED off (HIGH is the voltage level)
          break;
        case '4': // do scope ch a b c and d single capture
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
          digitalWrite(LED_BUILTIN, LOW);  // turn the LED on (HIGH is the voltage level)
          Serial.print("stReal= ");
          Serial.println(TotalReal);
          Serial.write(scopea, gbs);
          Serial.println("");
          //
          digitalWrite(LED_BUILTIN, HIGH);  // turn the LED off (HIGH is the voltage level)
          break;
        case '5': // do scope single analog + digital capture
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
            //digmeas = multipleDigitalRead(inputPins, numberOfInputPins); // returns corresponding decimal number reading fom input pins
            //digmeas = digitalRead(D5);
            //digmeas += digitalRead(D6) << 1;
            //digmeas += digitalRead(D7) << 2;
            //digmeas += digitalRead(D8) << 3;
            //digmeas += digitalRead(D9) << 4;
            digmeas = (REG_PORT_IN0 & PORT_PA09) >> 9;
            digmeas += (REG_PORT_IN1 & PORT_PB08) >> 7;
            digmeas += (REG_PORT_IN1 & PORT_PB09) >> 7;
            digmeas += (REG_PORT_IN0 & PORT_PA07) >> 4;
            digmeas += (REG_PORT_IN0 & PORT_PA05) >> 1;
            scopea[i] = (measa & 0xFF00) >> 8;
            scopea[i+bs] = measa & 0xFF;
            scopea[i+tbs] = digmeas & 0x1F;
            ta+=st;
            while (ta>micros());
          }
          TotalReal=micros()-StartReal;
          //stReal=TotalReal/bs; // calculate the average time for each reading
          digitalWrite(LED_BUILTIN, LOW);  // turn the LED on (HIGH is the voltage level)
          Serial.print("stReal= ");
          Serial.println(TotalReal); // report total time for bs samples
          // dump buffer over serial
          Serial.write(scopea, rbs);
          Serial.println("");
          //
          digitalWrite(LED_BUILTIN, HIGH);  // turn the LED off (HIGH is the voltage level)
          break;
          //
        case '6': // do scope ch a and b single capture
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
            //digmeas = multipleDigitalRead(inputPins, numberOfInputPins); // returns corresponding decimal number reading fom input pins
            //digmeas = digitalRead(D5);
            //digmeas += digitalRead(D6) << 1;
            //digmeas += digitalRead(D7) << 2;
            //digmeas += digitalRead(D8) << 3;
            //digmeas += digitalRead(D9) << 4;
            digmeas = (REG_PORT_IN0 & PORT_PA09) >> 9;
            digmeas += (REG_PORT_IN1 & PORT_PB08) >> 7;
            digmeas += (REG_PORT_IN1 & PORT_PB09) >> 7;
            digmeas += (REG_PORT_IN0 & PORT_PA07) >> 4;
            digmeas += (REG_PORT_IN0 & PORT_PA05 >> 1);
            scopea[i] = (measa & 0xFF00) >> 8;
            scopea[i+bs] = measa & 0xFF;
            scopea[i+tbs] = (measb & 0xFF00) >> 8;
            scopea[i+rbs] = measb & 0xFF;
            scopea[i+fbs] = digmeas & 0x1F;
            ta+=st;
            while (ta>micros());
          }
          TotalReal=micros()-StartReal;
          //stReal=TotalReal/bs; // calculate the average time for each reading
          digitalWrite(LED_BUILTIN, LOW);  // turn the LED on (HIGH is the voltage level)
          Serial.print("stReal= ");
          Serial.println(TotalReal);
          // Dump Buffer over serial
          Serial.write(scopea, vbs);
          Serial.println("");
          //
          digitalWrite(LED_BUILTIN, HIGH);  // turn the LED off (HIGH is the voltage level)
          break;
          //
        case '7': // do scope ch a b and c single capture
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
            //digmeas = multipleDigitalRead(inputPins, numberOfInputPins); // returns corresponding decimal number reading fom input pins
            //digmeas = digitalRead(D5);
            //digmeas += digitalRead(D6) << 1;
            //digmeas += digitalRead(D7) << 2;
            //digmeas += digitalRead(D8) << 3;
            //digmeas += digitalRead(D9) << 4;
            digmeas = (REG_PORT_IN0 & PORT_PA09) >> 9;
            digmeas += (REG_PORT_IN1 & PORT_PB08) >> 7;
            digmeas += (REG_PORT_IN1 & PORT_PB09) >> 7;
            digmeas += (REG_PORT_IN0 & PORT_PA07) >> 4;
            digmeas += (REG_PORT_IN0 & PORT_PA05) >> 1;
            scopea[i] = (measa & 0xFF00) >> 8;
            scopea[i+bs] = measa & 0xFF;
            scopea[i+tbs] = (measb & 0xFF00) >> 8;
            scopea[i+rbs] = measb & 0xFF;
            scopea[i+fbs] = (measc & 0xFF00) >> 8;
            scopea[i+vbs] = measc & 0xFF;
            scopea[i+sbs] = digmeas & 0x1F;
            ta+=st;
            while (ta>micros());
          }
          TotalReal=micros()-StartReal;
          //stReal=TotalReal/bs; // calculate the average time for each reading
          digitalWrite(LED_BUILTIN, LOW);  // turn the LED on (HIGH is the voltage level)
          Serial.print("stReal= ");
          Serial.println(TotalReal);
          // Dump Buffer over serial
          Serial.write(scopea, ebs);
          Serial.println("");
          //
          digitalWrite(LED_BUILTIN, HIGH);  // turn the LED off (HIGH is the voltage level)
          break;
        //delay(1);
      }
    }
  }
}

void loop () {

}




