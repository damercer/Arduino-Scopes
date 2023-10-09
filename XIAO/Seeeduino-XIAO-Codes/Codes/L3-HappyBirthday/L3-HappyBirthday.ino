#include <Servo.h>
#define ROTARY_ANGLE_SENSOR A0
#define ADC_REF 3 //reference voltage of ADC is 3v.If the Vcc switch on the seeeduino
#define GROVE_VCC 3 //VCC of the grove interface is normally 3v
#define FULL_ANGLE 300 //full value of the rotary angle is 300 degrees
 
Servo myservo;  // create servo object to control a servo
// twelve servo objects can be created on most boards
 
int pos = 0;    // variable to store the servo position
 
void setup() {
  Serial.begin(9600);
  pinMode(ROTARY_ANGLE_SENSOR, INPUT);
  myservo.attach(5);  // attaches the servo on pin 9 to the servo object
}
 
void loop() {
 
  float voltage;
  int sensor_value = analogRead(ROTARY_ANGLE_SENSOR);
  voltage = (float)sensor_value * ADC_REF / 1023;
  float degrees = (voltage * FULL_ANGLE) / GROVE_VCC;
  Serial.println("The angle between the mark and the starting position:");
  Serial.println(degrees);
  delay(50);
  myservo.write(degrees); 
}
