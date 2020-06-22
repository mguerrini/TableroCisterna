
// --- MODO ---
void SetupCommands()
{
}

void ReadCommands()
{
  if (Serial.available() > 0) {
    // read the incoming byte:
    int incomingByte = Serial.read();
    char c = char(incomingByte);

    if (c == 'S' || c == 's')
    {
      DoPrintStatus();
    }
  }
}


void ReadResetButton()
{
  if (IsResetButtonPressed())
  {
    Serial.println(F("RESET"));
    automaticFSM.FromState = AUTO_IDLE;
    automaticFSM.State = AUTO_IDLE;
    automaticFSM.NextState = AUTO_NULL;

    //Read el estado enabled/disabled from EEPROM
    bomba1.State = BOMBA_STATE_OFF; //0=OFF 1=ON -1=ERROR CONTACTOR ABIERTO -2=ERROR CONTACTOR CERRADO -3=ERROR TERMICO -3=ERROR BOMBA (ESTE NO ESTA EN FUNCIONAMIENTO TODAVIA, FALTARIA UN SENSOR EN LA BOMBA QUE DETECTE FUNCIONAMIENTO)
    bomba1.Uses = 0;
    bomba1.MachineState = FSM_BOMBA_OFF;
    bomba1.FromMachineState = FSM_BOMBA_OFF;
    bomba1.IsTermicoOk = true;
    bomba2.IsContactorClosed = false;

    bomba2.State = BOMBA_STATE_OFF; //0=OFF 1=ON -1=ERROR CONTACTOR ABIERTO -2=ERROR CONTACTOR CERRADO -3=ERROR TERMICO -3=ERROR BOMBA (ESTE NO ESTA EN FUNCIONAMIENTO TODAVIA, FALTARIA UN SENSOR EN LA BOMBA QUE DETECTE FUNCIONAMIENTO)
    bomba2.Uses = 0;
    bomba2.MachineState = FSM_BOMBA_OFF;
    bomba2.FromMachineState = FSM_BOMBA_OFF;
    bomba2.IsTermicoOk = true;
    bomba2.IsContactorClosed = false;

    UpdateBomba1Display();
    UpdateBomba2Display();
    UpdateActiveBombaDisplay();
    UpdateCisternaDisplay();
    UpdateTanqueDisplay();
    
    StopAllAlarms();
  }
}

boolean IsResetButtonPressed()
{
  static unsigned long startTime = 0;
  static boolean state;
  static boolean isPressed;

  return IsButtonPressed(RESET_BTN_PIN, state, isPressed, startTime);
}

#ifdef GET_STATUS_BUTTON_ENABLED
boolean IsGetStatusButtonPressed()
{
  static unsigned long startTime = 0;
  static boolean state;
  static boolean isPressed;

  return IsButtonPressed(GET_STATUS_BTN_PIN, state, isPressed, startTime);
}
#endif

boolean IsContinueButtonPressed()
{
  static unsigned long startTime = 0;
  static boolean state;
  static boolean isPressed;

  return IsButtonPressed(DEBUG_CONTINUE_PIN, state, isPressed, startTime);
}
