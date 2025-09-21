// RC Servo
// This servo is 0-180 degree
// Type the angle to the serial monitor
// then the RC servo will moving the that
// specific angle
/* Tinkercad:
https://www.tinkercad.com/things/aBtVABPGSNu-rc-servo-motor-arduino?sharecode=undefined
*/
int PWM = 9;  // PWM pin for servo control
int pwmFreq = 50; // 50Hz for servo control
int T = 1000000 / pwmFreq; // period of pwm: 20ms

void setup()
{
  Serial.begin(9600);
  pinMode(PWM, OUTPUT);
  servo(500);
  
  Serial.println("500us = 0, 1500us = 90, 2500us = 180"); // PWM interval
}

void loop()
{
  if (Serial.available() > 0)
  {
    int angle = Serial.parseInt();
    //int pwmValue = Serial.parseInt();
    
    if (angle >= 0 && angle <= 180) // only for 0-180 degree
    {
      int pwmValue = map(angle, 0, 180, 500, 2500); // mapping range to pwmValue
      
      Serial.print("The angle: ");
      Serial.println(angle);
      
      servo(pwmValue); // moving to specific angle
    }
  }
}

void servo(int PW)
{
  for (int i = 0; i < 50; i++) // In the servo mechanical need the continue pulse for win the inner friction
  {
    digitalWrite(PWM, HIGH);
    delayMicroseconds(PW); // HIGH for t
    
    digitalWrite(PWM, LOW);
    delayMicroseconds(T - PW); // LOW for T-t --> then the duty circle is t/T
  }
}
