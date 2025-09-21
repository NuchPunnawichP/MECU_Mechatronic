// how to wire for analogRead
// Potentiometer + LED
/* Tinkercad
https://www.tinkercad.com/things/aAVQqxauZE8-ledpotentiometer
*/
int LED = 9;

int potentiometer = A0;

void setup()
{
  pinMode(LED, OUTPUT);
  pinMode(potentiometer, INPUT);

  Serial.begin(9600); // check the value
}

void loop()
{
  // potentiometer is 10 bits but LED is only 8 bits
  int poten = analogRead(potentiometer);
  int brightness = map(poten, 0, 1023, 0, 255);

  analogWrite(LED, brightness);
  Serial.print("brightness: ");
  Serial.println(brightness);
}
