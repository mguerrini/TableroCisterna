
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

void ReadSwapButton()
{
  if (IsBombaSwapButtonPressed())
  {
    //no puedo cambiar con el boton cuando estan encendidas
    if (IsBombaOn(&bomba1) || IsBombaOn(&bomba2))
      return;

    SwapAndActiveBomba();
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

void ReadPrintStatus()
{
#ifdef GET_STATUS_BUTTON_ENABLED
  if (!IsGetStatusButtonPressed())
    return;

  DoPrintStatus();
#endif
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





void DoPrintStatus()
{
  Serial.println();
  //Modo
  if (IsAutomaticMode())
    Serial.println(F("*** MODE: Automatic ***"));
  else
    Serial.println(F("*** MODE: Manual ***"));

  //Estado del proceso
  Serial.print(F("Automatic FSM Status: "));
  PrintStateWorkingFSM(automaticFSM.State);
  Serial.println();

  //Timer
  Serial.print(F("Stopping Timer: "));
  Serial.println(automaticFSM.StoppingTimer);
  Serial.println();

  //Niveles
  Serial.println(F("*** Niveles ***"));
  if (sensores.IsCisternaSensorMinVal)
    Serial.println(F("Cisterna Nivel Minimo: true"));
  else
    Serial.println(F("Cisterna Nivel Minimo: false"));

  Serial.print(F("Cisterna Empty Start Time: "));
  Serial.println(sensores.CisternaEmptyStartTime);
  Serial.print(F("Cisterna Empty Time (milisegundos): "));
  Serial.println(sensores.CisternaEmptyMillis);

  if (sensores.IsTanqueSensorMinVal)
    Serial.println(F("Tanque Nivel Minimo: true"));
  else
    Serial.println(F("Tanque Nivel Minimo: false"));

  if (sensores.IsTanqueSensorMaxVal)
    Serial.println(F("Tanque Nivel Maximo: true"));
  else
    Serial.println(F("Tanque Nivel Maximo: false"));

  Serial.println();

  PrintAlarm();

  //Bombas
  Serial.println(F("*** BOMBA 1 ***"));
  PrintBomba(&bomba1);
  Serial.println();

  Serial.println(F("*** BOMBA 2 ***"));
  PrintBomba(&bomba2);
  Serial.println();

}

void PrintAlarm()
{
  Serial.println(F("*** Alarmas ***"));
  if (alarm.IsManualAlarmON)
    Serial.println(F("Manual: ON"));
  else
    Serial.println(F("Manual: OFF"));


  if (alarm.IsNotAvailableBombasAlarmON)
    Serial.println(F("Bombas no disponibles: ON"));
  else
    Serial.println(F("Bombas no disponibles: OFF"));

  if (alarm.IsBomba1AlarmON)
    Serial.println(F("Bomba 1: ON"));
  else
    Serial.println(F("Bomba 1: OFF"));

  if (alarm.IsBomba2AlarmON)
    Serial.println(F("Bomba 2: ON"));
  else
    Serial.println(F("Bomba 2: OFF"));

  if (alarm.IsCisternaAlarmON)
    Serial.println(F("Cisterna: ON"));
  else
    Serial.println(F("Cisterna: OFF"));

  Serial.println();
}

void PrintBomba(Bomba* bomba)
{
  if (bomba->IsEnabled)
    Serial.println(F("IsEnabled: true"));
  else
    Serial.println(F("IsEnabled: false"));

  if (bomba->IsActive)
    Serial.println(F("IsActive: true"));
  else
    Serial.println(F("IsActive: false"));

  Serial.print(F("State: "));
  PrintStateBomba(bomba, true);

  Serial.print(F("Machine Status: "));
  PrintStateBombaFSM(bomba->MachineState);
  Serial.println();

  Serial.print(F("Uses: "));
  Serial.println(bomba->Uses);

//  Serial.print(F("Timer: "));
//  Serial.println(bomba->Timer);

  Serial.print(F("Contactor Error Counter: "));
  Serial.println(bomba->ContactorErrorCounter);

  Serial.print(F("Tiempo de llenado: "));
  Serial.println(bomba->FillTimeAverage);

  Serial.print(F("Tiempos de llenado: "));
  for (int i=0; i<9; i++)
  {
    Serial.print(bomba->FillTimes[i]);
    Serial.print(F(", "));
  }
  Serial.println(bomba->FillTimes[9]);

  if (bomba->IsContactorClosed)
    Serial.println(F("IsContactorClosed: true"));
  else
    Serial.println(F("IsContactorClosed: false"));

  if (bomba->IsTermicoOk)
    Serial.println(F("IsTermicoOk: true"));
  else
    Serial.println(F("IsTermicoOk: false"));

  if (bomba->RequestOn)
    Serial.println(F("RequestOn: true"));
  else
    Serial.println(F("RequestOn: false"));

  if (bomba->RequestOff)
    Serial.println(F("RequestOff: true"));
  else
    Serial.println(F("RequestOff: false"));

  if (bomba->RequestEnabled)
    Serial.println(F("RequestEnabled: true"));
  else
    Serial.println(F("RequestEnabled: false"));

  if (bomba->RequestDisabled)
    Serial.println(F("RequestDisabled: true"));
  else
    Serial.println(F("RequestDisabled: false"));

}

void PrintStateBomba(Bomba* bomba, bool newLine)
{
  switch (bomba->State)
  {
    case BOMBA_STATE_ON:
      Serial.print(F("BOMBA_STATE_ON"));
      break;
    case BOMBA_STATE_OFF:
      Serial.print(F("BOMBA_STATE_OFF"));
      break;
    case BOMBA_STATE_ERROR_CONTACTOR_ABIERTO:
      Serial.print(F("BOMBA_STATE_ERROR_CONTACTOR_ABIERTO"));
      break;
    case BOMBA_STATE_ERROR_CONTACTOR_CERRADO:
      Serial.print(F("BOMBA_STATE_ERROR_CONTACTOR_CERRADO"));
      break;
    case BOMBA_STATE_ERROR_TERMICO:
      Serial.print(F("BOMBA_STATE_ERROR_TERMICO"));
      break;
  }

  if (newLine)
    Serial.println();
}
