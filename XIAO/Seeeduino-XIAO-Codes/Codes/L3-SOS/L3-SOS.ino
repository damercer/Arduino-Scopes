/*
 * SOS
 */
int pinBuzzer = 3;
//the number of the buzzer pin
void setup() {
  // initialize the buzzer pin as an output:
  pinMode(pinBuzzer, OUTPUT);
}
 
void loop() {
 //transmit a short tone three times
  tone(pinBuzzer, 200);// the frequency of the tone in hertz.
  delay(100);
  noTone(pinBuzzer); //stop the tone playing:
  delay(100);
  
  tone(pinBuzzer, 200);
  delay(100);
  noTone(pinBuzzer);
  delay(100);

  tone(pinBuzzer, 200);
  delay(100);
  noTone(pinBuzzer);
  delay(400);
//transmit a long tone three times
  tone(pinBuzzer, 200);
  delay(300);
  noTone(pinBuzzer);
  delay(300);

  tone(pinBuzzer, 200);
  delay(300);
  noTone(pinBuzzer);
  delay(300);
  
  tone(pinBuzzer, 200);
  delay(300);
  noTone(pinBuzzer);
  delay(400);
//transmit a short tone three times
  tone(pinBuzzer, 200);
  delay(100);
  noTone(pinBuzzer);
  delay(100);

  tone(pinBuzzer, 200);
  delay(100);
  noTone(pinBuzzer);
  delay(100);

  tone(pinBuzzer, 200);
  delay(100);
  noTone(pinBuzzer);
  delay(800);
}
