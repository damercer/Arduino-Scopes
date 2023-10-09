/*
  Blink
*/

// the setup function runs once when you press reset or power the board
void setup() {
  //  initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(LED_BUILTIN, HIGH);   // turn on the led
  delay(1000);                       // 
  digitalWrite(LED_BUILTIN, LOW);    // turn off the led
  delay(500);                       // 
}
