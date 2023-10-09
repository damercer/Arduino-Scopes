#define ROTARY_ANGLE_SENSOR A0//定义旋转电位计接口A0
#define LEDPIN 13 //定义LED灯接口13
#define ADC_REF 3 //参考电压3V
#define GROVE_VCC 3 //GROVE参考电压3V
#define FULL_ANGLE 300 //旋转电位计最大旋转角度为300°
 
void setup()
{
    Serial.begin(9600);//串口初始化
    pinMode(ROTARY_ANGLE_SENSOR, INPUT);//设置旋转电位计引脚为输入状态
    pinMode(LEDPIN,OUTPUT); //设置LED灯引脚为输出状态 
}
 
void loop()
{   
    float voltage;//变量电压为浮点数型
    int sensor_value = analogRead(ROTARY_ANGLE_SENSOR);//读取在旋钮电位计引脚的模拟值
    voltage = (float)sensor_value*ADC_REF/1023;//计算实时电压
    float degrees = (voltage*FULL_ANGLE)/GROVE_VCC;//计算旋钮转动的角度
    Serial.println("The angle between the mark and the starting position:");//串口打印字符
    Serial.println(degrees);//串口打印旋钮电位计转动角度值
    delay(100);
    
    int brightness;//定义亮度变量
    brightness = map(degrees, 0, FULL_ANGLE, 0, 255);//将将旋钮电位计角度值映射到LED灯亮度值并存储到亮度变量
    analogWrite(LEDPIN,brightness);//LED灯输出变量值
    delay(500);
}
