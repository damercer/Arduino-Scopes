/*Button
*/

const int buttonPin = 1;     // the number of the pushbutton pin
int buttonState = 0;    // variable for reading the pushbutton status

void setup() {
  //initialize the LED pin as an output:
  pinMode(LED_BUILTIN, OUTPUT);
  //initialize the pushbutton pin as an input:
  pinMode(buttonPin, INPUT_PULLUP);
}

void loop() {
  // read the state of the pushbutton value:
  buttonState = digitalRead(buttonPin);
  // check if the pushbutton is pressed. If it is, the buttonState 
  if (buttonState == HIGH) {
    //  turn LED on:
    digitalWrite(LED_BUILTIN, HIGH);
  }
  else {
    //  turn LED off:
    digitalWrite(LED_BUILTIN, LOW);
  }
}
