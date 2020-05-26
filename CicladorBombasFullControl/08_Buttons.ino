// ***** BOTONERA *****
const long BTN_PRESSED_TIME = 20; //20 milisegundos de boton presionado, para evitar rebote

//funciones con referencias
boolean IsButtonPressed(const char* buttonName, int pin, boolean &state, unsigned long &startTime);

// --- BOTONERA ---

boolean IsChangeModeButtonPressed()
{
  static unsigned long startTime = 0;
  static boolean state;
  static boolean isPressed;

  IsButtonPressed("Change Mode", CHANGE_MODE_BTN_PIN, state, isPressed, startTime);

  return isPressed;
}

boolean IsChangeActiveBombaButtonPressed()
{
  static unsigned long startTime = 0;
  static boolean state;
  static boolean isPressed;

  return IsButtonPressed("Change Active Bomba", CHANGE_ACTIVE_BTN_PIN, state, isPressed, startTime);
}

bool IsBombaEnableButtonPressed(byte number)
{
  if (number == BOMBA1)
    return IsBomba1EnableButtonPressed();
  else
    return IsBomba2EnableButtonPressed();
}

boolean IsBomba1EnableButtonPressed()
{
  static unsigned long startTime = 0;
  static boolean state;
  static boolean isPressed;

  return IsButtonPressed("Bomba 1 Enabled/Disables", BOMBA1_ENABLE_PIN, state, isPressed, startTime);
}

boolean IsBomba2EnableButtonPressed()
{
  static unsigned long startTime = 0;
  static boolean state;
  static boolean isPressed;

  IsButtonPressed("Bomba 2 Enabled/Disables", BOMBA2_ENABLE_PIN, state, isPressed, startTime);

  return isPressed;
}

boolean IsButtonPressed(const char* buttonName, int pin, boolean &state, boolean &isPressed, unsigned long &startTime)
{
  if (digitalRead(pin) != state)
  {
    state = !state;
    startTime = millis();
    isPressed = false;

    //Serial.print("Button ");
    //Serial.print(buttonName);
    //Serial.print(" changed - state ");
    //Serial.println(state);
  }

  if (state == LOW && !isPressed) {
    boolean output = (millis() - startTime) > BTN_PRESSED_TIME;

    if (output)
    {
      isPressed = true;
      //Serial.print("Button ");
      //Serial.print(buttonName);
      //Serial.println(" pressed");
      digitalWrite(LED_PIN, HIGH);
      delay(100);
      digitalWrite(LED_PIN, LOW);
    }

    return output;
  }
  else
    return false;
}
