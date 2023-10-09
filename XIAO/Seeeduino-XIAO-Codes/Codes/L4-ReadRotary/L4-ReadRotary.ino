/*
 * Use the serial monitor to view the knob potentiometer
 */
#define ROTARY_ANGLE_SENSOR A0
#define ADC_REF 3 //reference voltage of ADC is 3v.
#define GROVE_VCC 3 //VCC of the grove interface is normally 3v
#define FULL_ANGLE 300 //full value of the rotary angle is 300 degrees
 
void setup()
{
    Serial.begin(9600);//start serial port at 9600 bps:
    pinMode(ROTARY_ANGLE_SENSOR, INPUT);//initialize the knob pin as an input
}
 
void loop()
{   
    float voltage;//the variable voltage is floating point type
    int sensorValue = analogRead(ROTARY_ANGLE_SENSOR);//read the analog value at the knob potentiometer pin
    voltage = (float)sensorValue*ADC_REF/1023;//calculate the real-time voltage
    float degrees = (voltage*FULL_ANGLE)/GROVE_VCC;//calculate the rotation angle of the knob potentiometer
    Serial.println("The angle between the mark and the starting position:");//serial port printing characters
    Serial.println(degrees);//serial printing Knob Potentiometer Rotation angle value
    delay(100);
}
