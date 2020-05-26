/*
  Input Pull-up Serial

  This example demonstrates the use of pinMode(INPUT_PULLUP). It reads a digital
  input on pin 2 and prints the results to the Serial Monitor.

  The circuit:
  - momentary switch attached from pin 2 to ground
  - built-in LED on pin 13

  Unlike pinMode(INPUT), there is no pull-down resistor necessary. An internal
  20K-ohm resistor is pulled to 5V. This configuration causes the input to read
  HIGH when the switch is open, and LOW when it is closed.

  created 14 Mar 2012
  by Scott Fitzgerald

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/InputPullupSerial
*/
const long _debounceTime = 20; //20 milisegundos de boton presionado, para evitar rebote

const int _boton1 = 8;
const int _boton2 = 9;
const int _boton3 = 10;
const int _boton4 = 11;
const int _boton5 = 12;

const byte _ledPin = 13; 

char textBuffer[100];

//funciones con referencias
unsigned long IsButtonPressed(int pin, boolean &state, unsigned long &startTime);

void setup() {
  //start serial connection
  Serial.begin(9600);
  
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);

  //Niveles
  pinMode(5, INPUT_PULLUP);
  pinMode(6, INPUT_PULLUP);
  pinMode(7, INPUT_PULLUP);
  
  //Botonera
  pinMode(8, INPUT_PULLUP);
  pinMode(9, INPUT_PULLUP);
  pinMode(10, INPUT_PULLUP);
  pinMode(11, INPUT_PULLUP);
  pinMode(12, INPUT_PULLUP);

  //testigo de operacion: led
  pinMode(_ledPin, OUTPUT);
  //lo apago
  digitalWrite(_ledPin, LOW);
}

void loop() {
  if (IsButton1Pressed())
  {
      digitalWrite(_ledPin, HIGH);
      delay(100);
      digitalWrite(_ledPin, LOW);
  }
  
  if (IsButton2Pressed())
  {
      digitalWrite(_ledPin, HIGH);
      delay(100);
      digitalWrite(_ledPin, LOW);
  }

  if (IsButton3Pressed())
  {
      digitalWrite(_ledPin, HIGH);
      delay(100);
      digitalWrite(_ledPin, LOW);
  }

  if (IsButton4Pressed())
  {
      digitalWrite(_ledPin, HIGH);
      delay(100);
      digitalWrite(_ledPin, LOW);
  }

  if (IsButton5Pressed())
  {
      digitalWrite(_ledPin, HIGH);
      delay(100);
      digitalWrite(_ledPin, LOW);
  }

}


//botonera
boolean IsButton1Pressed()
{
  static unsigned long startTime = 0;
  static boolean state;

  return IsButtonPressed(_boton1, state, startTime);
}

boolean IsButton2Pressed()
{
  static unsigned long startTime = 0;
  static boolean state;

  return IsButtonPressed(_boton2, state, startTime);
}

boolean IsButton3Pressed()
{
  static unsigned long startTime = 0;
  static boolean state;

  return IsButtonPressed(_boton3, state, startTime);
}

boolean IsButton4Pressed()
{
  static unsigned long startTime = 0;
  static boolean state;

  return IsButtonPressed(_boton4, state, startTime);
}


boolean IsButton5Pressed()
{
  static unsigned long startTime = 0;
  static boolean state;

  return IsButtonPressed(_boton5, state, startTime);
}

unsigned long IsButtonPressed(int pin, boolean &state, unsigned long &startTime)
{
  if (digitalRead(pin) != state)
  {
    state = !state;
    startTime = millis();
  }

  if (state == LOW)
    return (millis() - startTime) > _debounceTime;
  else
    return false;
}
