/*
 *Button-SOS
 */
const int buttonPin = 1; // the number of the pushbutton pin
int pinBuzzer = 3;//the number of the buzzer pin
void setup() {
  // initialize the buzzer pin as an output:
  pinMode(pinBuzzer, OUTPUT);
  //  initialize the pushbutton pin as an input: 
  pinMode(buttonPin, INPUT_PULLUP);
}
 
void loop() {
  // variable for reading the pushbutton, read the state of the pushbutton value:
  int buttonState = digitalRead(buttonPin);
 
  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (buttonState == LOW) {
  //play a note on buzzer pin for 200 ms:
  tone(pinBuzzer, 200, 200);
  }
}
