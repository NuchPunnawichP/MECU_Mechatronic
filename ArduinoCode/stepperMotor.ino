// Stepper motor
// stepper motor will rotate in CW or CCW if you press sw1 or sw2
// with rotational step with at your input keyboard
/*
Dont have stepper motor on Tinkercad
*/
int sw1 = A0;
int sw2 = A1;

int m1a = 8;
int m2a = 9;
int m1b = 10;
int m2b = 11;

int motorSpeed = 4;

char val = '0';

int CW[8][4] = {
  {1, 0, 0, 0},
  {1, 1, 0, 0},
  {0, 1, 0, 0},
  {0, 1, 1, 0},
  {0, 0, 1, 0},
  {0, 0, 1, 1},
  {0, 0, 0, 1},
  {1, 0, 0, 1}
};

int CCW[8][4] = {
  {1, 0, 0, 1},
  {0, 0, 0, 1},
  {0, 0, 1, 1},
  {0, 0, 1, 0},
  {0, 1, 1, 0},
  {0, 1, 0, 0},
  {1, 1, 0, 0},
  {1, 0, 0, 0}
};

void setup()
{
  pinMode(m1a, OUTPUT);
  pinMode(m2a, OUTPUT);
  pinMode(m1b, OUTPUT);
  pinMode(m2b, OUTPUT);
  
  Serial.begin(9600);
}

void loop()
{
  if (Serial.available() > 0)
  {
    val = Serial.parseInt();

    int steps = val;
    
    Serial.print("Set stepper steps: ");
    Serial.println(val);

    if(analogRead(sw1) > 100)
    {
      stepMotor(true, steps);
    }
    else if(analogRead(sw2) > 100)
    {
      stepMotor(false, steps);
    }
  }
}

void stepMotor(bool direc, int steps)
{
  for(int i=0 ; i<steps ; i++)
  {
    for(int j=0 ; j<8 ; j++)
    {
      if(direc) // CW
      {
        digitalWrite(m1a, CW[j][0]);
        digitalWrite(m2a, CW[j][1]);
        digitalWrite(m1b, CW[j][2]);
        digitalWrite(m2b, CW[j][3]);
        
        delay(motorSpeed);
      }
      else
      {
        digitalWrite(m1a, CCW[j][0]);
        digitalWrite(m2a, CCW[j][1]);
        digitalWrite(m1b, CCW[j][2]);
        digitalWrite(m2b, CCW[j][3]);
        
        delay(motorSpeed);
      }
    }
  }
  stopMotor();
}

void stopMotor()
{
  digitalWrite(m1a, LOW);
  digitalWrite(m2a, LOW);
  digitalWrite(m1b, LOW);
  digitalWrite(m2b, LOW);
}
