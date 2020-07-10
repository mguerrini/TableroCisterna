
// --- MODO ---
void SetupMode()
{
  automaticFSM.Mode = AUTO;
  ReadExecutionMode();
  
#ifdef MODO_OUTPUT_ENABLED
  digitalWrite(MODO_LED_PIN, HIGH);
#endif
}

void ReadExecutionMode()
{
  byte currMode = automaticFSM.Mode;

  if (IsChangeModeButtonPressed(IS_CHANGE_MODE_PULSADOR))
  {
    //si hay una bomba encendida...se apaga pero no se notifica si no se apago como un error porque puede ser que se haya prendido de manera externa.
    //paso a manual
    //pulsador...
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
      Serial.println(F("Change Mode -> AUTO"));
      StopManualAlarm();
      
#ifdef MODO_OUTPUT_VIEW_ENABLED
      UpdateDisplayToAutoMode();
#endif      
    }
    else
    {
      Serial.println(F("Change Mode -> MANUAL"));
      StartManualAlarm();

#ifdef MODO_OUTPUT_VIEW_ENABLED
      UpdateDisplayToManualMode();
#endif      
    }
  }
}


bool IsAutomaticMode()
{
  return automaticFSM.Mode == AUTO;
}

void ChangeToAutomaticMode()
{
  automaticFSM.Mode = AUTO;

  UpdateDisplayToAutoMode();
}

void ChangeToManualMode()
{
  automaticFSM.Mode = MANUAL;

  UpdateDisplayToManualMode();
}

boolean IsChangeModeButtonPressed(bool isPulsadorMode)
{
  static unsigned long startTime = 0;
  static boolean state;
  static boolean isPressed;

  boolean pulsador = IsButtonPressed(CHANGE_MODE_BTN_PIN, state, isPressed, startTime);

  if (isPulsadorMode)
    return pulsador;
  else
    return isPressed;
}
