#include <Servo.h>//声明使用舵机库
#define ROTARY_ANGLE_SENSOR A0//定义旋钮电位计是A0引脚
#define ADC_REF 3 //ADC参考电压为3V
#define GROVE_VCC 3//VCC参考电压为3V
#define FULL_ANGLE 300 //旋钮电位计旋转的最大角度为300°
 
Servo myservo;  // 创建myservo实例以控制舵机
int pos = 0;    // 定义变量以存储舵机转动角度
 
void setup() {
  Serial.begin(9600);//初始化串口
  pinMode(ROTARY_ANGLE_SENSOR, INPUT);//设置旋钮电位计引脚为输入状态
  myservo.attach(5);  // 舵机信号myservo通过引脚5来传输
}
 
void loop() {
 
  float voltage;//将电压设置为浮点数型
  int sensor_value = analogRead(ROTARY_ANGLE_SENSOR);//读取在旋钮电位计引脚的模拟值
  voltage = (float)sensor_value * ADC_REF / 1023;//实时电压为读取到的模拟值乘以参考电压除以1023
  float degrees = (voltage * FULL_ANGLE) / GROVE_VCC;//旋钮转动的角度为实时电压乘以旋钮电位计最大角度值除以GROVE模块接口的电压值
  Serial.println("The angle between the mark and the starting position:");//串口打印字符
  Serial.println(degrees);//串口打印旋钮电位计转动角度值
  delay(50);
  myservo.write(degrees); //将旋钮电位计转动角度值写入舵机
}
