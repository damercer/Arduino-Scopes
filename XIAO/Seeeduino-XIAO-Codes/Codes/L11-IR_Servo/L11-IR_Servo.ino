#include <IRremote.h>
#include <Servo.h>
 
Servo myservo;  
int RECV_PIN = 7;
 
IRrecv irrecv(RECV_PIN);
 
decode_results results;
 
int pos = 90; 
void setup()
{
  Serial.begin(9600);
  Serial.println("Enabling IRin");  // remind enabling IR
  irrecv.enableIRIn(); // Start the receiver
  Serial.println("Enabled IRin");
  myservo.attach(5);  // attaches the servo on pin 2 to the servo object
 
}
 
// left  16712445 right  16761405
 
void loop() {
  if (irrecv.decode(&results)) { 
   if (results.value == 16761405) {    
      for (pos; pos <= 89; pos += 1) { // goes from 0 degrees to 90 degrees
        // in steps of 1 degree
        myservo.write(pos);              // tell servo to go to position in variable 'pos'
 
        delay(40);                       // waits 15ms for the servo to reach the position
        if (irrecv.decode(&results)) {
          irrecv.resume();
          if (results.value == 16712445)
            break;
        }
      }
    }
 
    if (results.value == 16712445) {    // fan swing to right
      for (pos; pos >= 1; pos -= 1) { // goes from 90 degrees to 0 degrees
        myservo.write(pos);              // tell servo to go to position in variable 'pos'
        delay(40);                       // waits 15ms for the servo to reach the position
 
        if (irrecv.decode(&results)) {
          irrecv.resume();
          if (results.value == 16761405)
            break;
        }
      }
    }
    Serial.println(pos);
    Serial.println(results.value, HEX);
    Serial.println(results.value);
    irrecv.resume();                    //recive next intrustion
 
  }
  delay(100);
}
