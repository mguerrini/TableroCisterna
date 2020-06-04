// ***** BOTONERA *****
const long BTN_PRESSED_TIME = 20; //20 milisegundos de boton presionado, para evitar rebote

//funciones con referencias
boolean IsButtonPressed(const char* buttonName, int pin, boolean &state, unsigned long &startTime);

// --- BOTONERA ---

bool IsBombaSwapButtonPressed()
{
  static unsigned long startTime = 0;
  static boolean state;
  static boolean isPressed;

  return IsButtonPressed("Change Mode", BOMBA_SWAP_BTN_PIN, state, isPressed, startTime);
}

boolean IsGetStatusButtonPressed()
{
  static unsigned long startTime = 0;
  static boolean state;
  static boolean isPressed;

  return IsButtonPressed("Get Status", GET_STATUS_BTN_PIN, state, isPressed, startTime);
}

boolean IsContinueButtonPressed()
{
  static unsigned long startTime = 0;
  static boolean state;
  static boolean isPressed;

  return IsButtonPressed("Debug Continue", DEBUG_CONTINUE_PIN, state, isPressed, startTime);
}


boolean IsChangeModeButtonPressed()
{
  static unsigned long startTime = 0;
  static boolean state;
  static boolean isPressed;

  IsButtonPressed("Change Mode", CHANGE_MODE_BTN_PIN, state, isPressed, startTime);

  return isPressed;
}


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

  IsButtonPressed("Bomba 1 Enabled/Disables", BOMBA1_ENABLE_PIN, state, isPressed, startTime);

  return isPressed;
}

boolean IsBomba2EnabledButtonPressed()
{
  static unsigned long startTime = 0;
  static boolean state;
  static boolean isPressed;

  IsButtonPressed("Bomba 2 Enabled/Disables", BOMBA2_ENABLE_PIN, state, isPressed, startTime);

  return isPressed;
}

//Cada vez que se presiona el botón devuelve true y luego false. Mientras se mantenga presionado isPressed es true.

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