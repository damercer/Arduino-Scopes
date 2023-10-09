/*
 * Check if the button is pressed
 */
const int buttonPin = 1;//the number of the pushbutton pin
int buttonState = 0;//variable for reading the pushbutton status
void setup() {
  //initialize the pushbutton pin as an input
pinMode(buttonPin, INPUT_PULLUP);
///start serial port at 9600 bps
Serial.println(9600);
}

void loop() {
  // read the state of the pushbutton value
buttonState = digitalRead(buttonPin);
//Send button status data to serial port
Serial.println(buttonState);
delay(500);
}
