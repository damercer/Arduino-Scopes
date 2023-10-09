/*
 读取遥控器编码
 */

#include <IRremote.h>

int RECV_PIN = 7;
IRrecv irrecv(RECV_PIN);
decode_results results;
void setup() {
    Serial.begin(9600);
    irrecv.enableIRIn(); 
    Serial.println(RECV_PIN);
}

void loop() {
    if (irrecv.decode(&results)) {
        Serial.println(results.value, HEX);
        Serial.println(results.value);
        irrecv.resume(); 
    }
    delay(100);
}
