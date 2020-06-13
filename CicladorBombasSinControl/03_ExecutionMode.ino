
// --- MODO ---
void SetupMode()
{
  _mode = AUTO;
  ReadExecutionMode();
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
      UpdateDisplayToAutoMode();
    }
    else
    {
      Serial.println(F("Change Mode -> MANUAL"));
      StartManualAlarm();
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
