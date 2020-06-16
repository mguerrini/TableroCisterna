
// --- MODO ---
void SetupMode()
{
  _mode = AUTO;
  ReadExecutionMode();
  digitalWrite(MODO_PIN, HIGH);
}

void ReadExecutionMode()
{
  byte currMode = _mode;

  if (IsChangeModeButtonPressed(IS_CHANGE_MODE_PULSADOR))
  {
    //si hay una bomba encendida...se apaga pero no se notifica si no se apago como un error porque puede ser que se haya prendido de manera externa.
    //paso a manual
    //pulsador...
    if (IsAutomaticMode())
    {
      _mode = MANUAL;
    }
    else
    {
      _mode = AUTO;
    }
     
  }


  if (currMode != _mode)
  {
    if (IsAutomaticMode())
    {
      Serial.println(F("Change Mode -> AUTO"));
      StopManualAlarm();

      digitalWrite(MODO_PIN, HIGH);
      UpdateDisplayToAutoMode();
    }
    else
    {
      Serial.println(F("Change Mode -> MANUAL"));
      StartManualAlarm();

      digitalWrite(MODO_PIN, LOW);
      UpdateDisplayToManualMode();
    }
  }
}



bool IsAutomaticMode()
{
  return _mode == AUTO;
}

void ChangeToAutomaticMode()
{
  _mode = AUTO;

  UpdateDisplayToAutoMode();
}

void ChangeToManualMode()
{
  _mode = MANUAL;

  UpdateDisplayToManualMode();
}

boolean IsChangeModeButtonPressed(bool isPulsadorMode)
{
  static unsigned long startTime = 0;
  static boolean state;
  static boolean isPressed;

  boolean pulsador = IsButtonPressed( CHANGE_MODE_BTN_PIN, state, isPressed, startTime);

  if (isPulsadorMode)
    return pulsador;
  else
    return isPressed;
}
