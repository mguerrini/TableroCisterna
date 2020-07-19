
// --- MODO ---
void SetupMode()
{
  automaticFSM.Mode = AUTO;
}

void ReadExecutionMode()
{
  byte currMode = automaticFSM.Mode;

  if (IsChangeModeButtonPressed(IS_CHANGE_MODE_PULSADOR))
  {
    //si hay una bomba encendida...se apaga pero no se notifica si no se apago como un error porque puede ser que se haya prendido de manera externa.
    //paso a manual
    //pulsador...
#ifdef LOG_ENABLED
    Serial.println(F("Change mode pressed."));
#endif
    if (IsAutomaticMode())
    {
      automaticFSM.Mode = MANUAL;
    }
    else
    {
      automaticFSM.Mode = AUTO;
    }
  }


  if (currMode != automaticFSM.Mode)
  {
    if (IsAutomaticMode())
    {
#ifdef LOG_ENABLED
      Serial.println(F("Change Mode -> AUTO"));
#endif
      StopManualAlarm();
    }
    else
    {
#ifdef LOG_ENABLED
      Serial.println(F("Change Mode -> MANUAL"));
#endif
      StartManualAlarm();

    }

    UpdateDisplayMode();
  }
}


bool IsAutomaticMode()
{
  return automaticFSM.Mode == AUTO;
}

boolean IsChangeModeButtonPressed(bool isPulsadorMode)
{
  static unsigned long startTime = 0;
  static boolean state;
  static boolean isPressed;

  boolean pulsador = IsButtonPressed(CHANGE_MODE_BTN_PIN, state, isPressed, startTime);

  if (isPulsadorMode)
  {
    return pulsador;
  }
  else
  {
    return isPressed;
  }
}
