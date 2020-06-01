// ***** BOTONERA *****
const long BTN_PRESSED_TIME = 20; //20 milisegundos de boton presionado, para evitar rebote

//funciones con referencias
boolean IsButtonPressed(const char* buttonName, int pin, boolean &state, unsigned long &startTime);

// --- BOTONERA ---

boolean IsGetStatusButtonPressed()
{
  static unsigned long startTime = 0;
  static boolean state;
  static boolean isPressed;

  IsButtonPressed("Get Status", GET_STATUS_MODE_BTN_PIN, state, isPressed, startTime);

  return isPressed;
}

boolean IsContinueButtonPressed()
{
  static unsigned long startTime = 0;
  static boolean state;
  static boolean isPressed;

  IsButtonPressed("Debug Continue", DEBUG_CONTINUE_PIN, state, isPressed, startTime);

  return isPressed;
}


boolean IsChangeModeButtonPressed()
{
  static unsigned long startTime = 0;
  static boolean state;
  static boolean isPressed;

  IsButtonPressed("Change Mode", CHANGE_MODE_BTN_PIN, state, isPressed, startTime);

  return isPressed;
}

/*
boolean IsChangeActiveBombaButtonPressed()
{
  static unsigned long startTime = 0;
  static boolean state;
  static boolean isPressed;

  return IsButtonPressed("Change Active Bomba", CHANGE_ACTIVE_BTN_PIN, state, isPressed, startTime);
}
*/

bool IsBombaEnabledButtonPressed(byte number)
{
  if (number == BOMBA1)
    return IsBomba1EnabledButtonPressed();
  else
    return IsBomba2EnabledButtonPressed();
}

boolean IsBomba1EnabledButtonPressed()
{
  static unsigned long startTime = 0;
  static boolean state;
  static boolean isPressed;

  return IsButtonPressed("Bomba 1 Enabled/Disables", BOMBA1_ENABLE_PIN, state, isPressed, startTime);
}

boolean IsBomba2EnabledButtonPressed()
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
  }

  if (state == LOW && !isPressed) {
    boolean output = (millis() - startTime) > BTN_PRESSED_TIME;

    if (output)
    {
      isPressed = true;
      digitalWrite(LED_PIN, HIGH);
      delay(100);
      digitalWrite(LED_PIN, LOW);
    }

    return output;
  }
  else
    return false;
}
