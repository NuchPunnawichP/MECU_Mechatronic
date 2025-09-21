// DC Motor
// This simulation is simulating how to control the direction
// of the motor by using 2 switches with maximum speed
/* Tinkercad:
https://www.tinkercad.com/things/aZtpcqlCekB-dc-motor-arduino?sharecode=qXWtjuIlKAXPR19X2jjAdmK-FMyE2SPdS1EPjKjhFY8
*/
int m1a = 7;
int m2a = 8;
int spd = 9; // PWM pin

int sw1 = A0;
int sw2 = A1;

int motorSpeed = 255; // full drive
bool Direc = true; // true = CW, false = CCW

void setup()
{
  pinMode(m1a, OUTPUT);
  pinMode(m2a, OUTPUT);
  pinMode(spd, OUTPUT);
  
  pinMode(sw1, INPUT); // press for CW
  pinMode(sw2, INPUT); // press for CCW
  
  digitalWrite(m1a, LOW);
  digitalWrite(m2a, LOW);
}

void loop()
{
  if(analogRead(sw1) > 100) // if press sw1
  {
    driveMotor(true, motorSpeed);
  }
  else if(analogRead(sw2) > 100) // if press sw2
  {
    driveMotor(false, motorSpeed);
  }
  else // if dont press any sw
  {
    driveMotor(false, 0); // stop the motor
  }
}

void driveMotor(bool direc, int PWM)
{
  if(direc) // CW
  {
    digitalWrite(m1a, HIGH);
    digitalWrite(m2a, LOW);
  }
  else // CCW
  {
    digitalWrite(m1a, LOW);
    digitalWrite(m2a, HIGH);
  }

  analogWrite(spd, PWM); // motor speed
}


