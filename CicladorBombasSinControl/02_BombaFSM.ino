void BombaStateMachine(Bomba* bomba)
{
  bomba->NextMachineState = FSM_BOMBA_NULL;

  PrintEnterStateBombaFSM(bomba);

  ExecuteBombaState(bomba);

  PrintExitStateBombaFSM(bomba);

  bomba->FromMachineState = bomba->MachineState;
  if (bomba->NextMachineState != FSM_BOMBA_NULL)
    bomba->MachineState = bomba->NextMachineState;
}

void ExecuteBombaState(Bomba* bomba)
{
  int bombaState = bomba->MachineState;

  //termico error

/*
  if (!bomba->IsTermicoOk)
  {
    bomba->NextMachineState = FSM_BOMBA_ERROR_TERMICO;
    PrintBombaMessage(F("Termico Abierto"));
    return;
  }
*/

/*
//fase error....no importa....total se apaga
  
*/

  switch (bombaState)
  {
    case FSM_BOMBA_OFF:
      FSM_Bomba_Off(bomba);
      break;

    case FSM_BOMBA_TURNING_ON:
      FSM_Bomba_TurningOn(bomba);
      break;

    case FSM_BOMBA_ON:
      FSM_Bomba_On(bomba);
      break;

    case FSM_BOMBA_TURNING_OFF:
      FSM_Bomba_TurningOff(bomba);
      break;

    case FSM_BOMBA_DISABLING:
      bomba->NextMachineState = FSM_BOMBA_DISABLED;
      PrintBombaMessage(F("Disabling-Stop Alarm"));
      break;

    case FSM_BOMBA_DISABLED:
      FSM_Bomba_Disabled(bomba);
      break;

    case FSM_BOMBA_ENABLING:
      FSM_Bomba_Enabling(bomba);
      break;

    case FSM_BOMBA_ERROR_CONTACTOR_ABIERTO:
      FSM_Bomba_ErrorContactorAbierto(bomba);
      break;

    case FSM_BOMBA_ERROR_TERMICO:
      FSM_Bomba_ErrorTermico(bomba);
      break;
  }
}

bool IsFirstTimeInState(Bomba* bomba)
{
  return bomba->FromMachineState != bomba->MachineState;
}

// ****************************************************************** //
//                          STATES
// ****************************************************************** //

void FSM_Bomba_Off(Bomba* bomba)
{
  //limpio cualquier pedido de apagado
  bomba->RequestOff = false;

  if (IsFirstTimeInState(bomba))
  {
    bomba->State = BOMBA_STATE_OFF;
    UpdateBombaDisplay(bomba);
    //registro si es modo automatico....sino puede funcionar sin llenado completo
    Statistics_BombaOff(bomba, IsAutomaticMode());
    bomba->StartTime = 0;
    PrintBombaMessage(F("Bomba OFF"));
    return;
  }

  if (!bomba->IsTermicoOk)
  {
    bomba->NextMachineState = FSM_BOMBA_ERROR_TERMICO;
    PrintBombaMessage(F("Termico Abierto"));
    return;
  }

  if (bomba->RequestDisabled)
  {
    bomba->NextMachineState = FSM_BOMBA_DISABLING;
    PrintBombaMessage(F("Request Disabled"));
    return;
  }

  if (bomba->RequestOn)
  {
    bomba->NextMachineState = FSM_BOMBA_TURNING_ON;
    PrintBombaMessage(F("Request On"));
    return;
  }

  if (bomba->IsContactorClosed)
  {
    bomba->NextMachineState = FSM_BOMBA_TURNING_ON;
    PrintBombaMessage(F("Contactor Cerrado"));
    return;
  }
}


void FSM_Bomba_TurningOn(Bomba* bomba) {
  bomba->RequestOn = false;

  if (bomba->RequestOff)
  {
    bomba->RequestOff = false;
    bomba->NextMachineState = FSM_BOMBA_TURNING_OFF;
    PrintBombaMessage(F("Request Off"));
    return;
  }

  //espero a que el contactor este cerrado y que el termico este OK
  if (!bomba->IsTermicoOk)
  {
    bomba->NextMachineState = FSM_BOMBA_ERROR_TERMICO;
    PrintBombaMessage(F("Termico abierto"));
    return;
  }

  if (bomba->IsContactorClosed)
  {
    bomba->NextMachineState = FSM_BOMBA_ON;
    PrintBombaMessage(F("Contactor Cerrado"));
    return;
  }

  if (IsFirstTimeInState(bomba))
  {
    bomba->Timer = millis();
    PrintBombaMessage(F("Iniciando timer"));
    return;
  }

  //espero un tiempo prudencial hasta que arranque la bomba
  unsigned long wait = deltaMillis(millis(), bomba->Timer);
  //espero a que inicie la bomba activa que deberia estar en ON.
  //sino se inicia => el contactor fallo
  if (wait > BOMBA_TURNING_ON_TIME)
  {
    //cambio de bomba activa
    bomba->NextMachineState = FSM_BOMBA_ERROR_CONTACTOR_ABIERTO;
    PrintBombaMessage(F("Esperando Contactor Cerrado...Timeout"), wait, BOMBA_TURNING_ON_TIME);
  }
  else
    PrintBombaMessage(F("Esperando Contactor Cerrado..."), wait, BOMBA_TURNING_ON_TIME);
}

void FSM_Bomba_On(Bomba* bomba)
{
  //limpio cualquier pedido de encendido
  bomba->RequestOn = false;

  if (IsFirstTimeInState(bomba))
  {
    bomba->StartTime = millis(); //inicio de la bomba
    bomba->RefreshTime = bomba->StartTime; //refresco de la vista
    bomba->State = BOMBA_STATE_ON;
    bomba->ContactorErrorCounter = 0; //reseteo los errores del contactor ya que se encendio.
    UpdateBombaDisplay(bomba);

    //inicio la estadistica
    Statistics_BombaOn(bomba);
    PrintBombaMessage(F("Bomba ON"));
    return;
  }

  if (!bomba->IsTermicoOk)
  {
    bomba->NextMachineState = FSM_BOMBA_ERROR_TERMICO;
    PrintBombaMessage(F("Termico abierto"));
    return;
  }

  if (bomba->RequestOff)
  {
    bomba->NextMachineState = FSM_BOMBA_TURNING_OFF;
    PrintBombaMessage(F("Request: Off"));
    return;
  }

  if (!bomba->IsContactorClosed)
  {
    //se apago...
    bomba->NextMachineState = FSM_BOMBA_TURNING_OFF;
    PrintBombaMessage(F("Contactor Abierto"));
    return;
  }

  if (bomba->RequestDisabled)
  {
    bomba->NextMachineState = FSM_BOMBA_DISABLING;
    PrintBombaMessage(F("Request: Disabled"));
    return;
  }

  //muestro el tiempo en pantalla
  unsigned long nowTime = millis();
  unsigned long delta = deltaMillis(nowTime, bomba->RefreshTime);
  if (delta > BOMBA_REFRESH_WORKING_TIME) {
    bomba->RefreshTime = nowTime;
    UpdateBombaWorkingTime(bomba);
  }
}


void FSM_Bomba_TurningOff(Bomba* bomba)
{
  unsigned long nowTime = millis();
  bomba->RequestOff = false;

  //la bomba esta en error
  if (!bomba->IsTermicoOk)
  {
    bomba->NextMachineState = FSM_BOMBA_ERROR_TERMICO;
    PrintBombaMessage(F("Termico Abierto"));
    return;
  }

  //ya esta apagada
  if (!bomba->IsContactorClosed)
  {
    bomba->NextMachineState = FSM_BOMBA_OFF;
    PrintBombaMessage(F("Contactor Abierto"));
    return;
  }

  if (IsFirstTimeInState(bomba))
  {
    bomba->Timer = nowTime;
    PrintBombaMessage(F("Iniciando timer"));
    return;
  }

  //espero un tiempo prudencial hasta que arranque la bomba
  unsigned long wait = deltaMillis(nowTime, bomba->Timer);
  //espero a que inicie....la bomba activa deberia estar en ON.
  if (wait > BOMBA_TURNING_OFF_TIME)
  {
    //ERROR....NO SE DETUVO LA BOMBA Y SIGUE FUNCIONANDO...NUNCA DEBERIA OCURRIR, PORQUE LOS SENSORES APAGAN LAS BOMBAS
    //Serial.println("La bomba sigue andando con sensores que indican apagada. Error en los sensores.");
    //Vuelvo a ON
    bomba->NextMachineState = FSM_BOMBA_TURNING_ON; // FSM_BOMBA_ERROR_CONTACTOR_CERRADO;
    PrintBombaMessage(F("Contactor Cerrado-Bomba continua encendida"), wait, BOMBA_TURNING_OFF_TIME);
  }
  else
    PrintBombaMessage(F("Esperando contactor"), wait, BOMBA_TURNING_OFF_TIME);
}


void FSM_Bomba_Disabled(Bomba* bomba)
{
  if (IsFirstTimeInState(bomba))
  {
    //cancelo la estadistica de llenado
    Statistics_BombaOff(bomba, false);

    StopAlarmBomba(bomba);
    bomba->IsEnabled = false;
    UpdateBombaDisplay(bomba);
    PrintBombaMessage(F("Bomba Deshabilitada"));
    return;
  }

  if (bomba->RequestEnabled)
  {
    bomba->NextMachineState = FSM_BOMBA_ENABLING;
    PrintBombaMessage(F("Request Enabled"));
    return;
  }
}


void FSM_Bomba_Enabling(Bomba* bomba) {
  StopAlarmBomba(bomba);
  bomba->IsEnabled = true;

  if (!bomba->IsTermicoOk)
  {
    bomba->NextMachineState = FSM_BOMBA_ERROR_TERMICO;
    PrintBombaMessage(F("Enabled-StopAlarm-Termico Abierto"));
    return;
  }

  //no verifico el estado del contactor....
  if (bomba->IsContactorClosed)
    bomba->NextMachineState = FSM_BOMBA_TURNING_ON;
  else
    bomba->NextMachineState = FSM_BOMBA_TURNING_OFF;

  PrintBombaMessage(F("Enabled-Stop Alarm"));
}


void FSM_Bomba_ErrorContactorAbierto(Bomba* bomba) {
  //deberia cerrarse para prenderse.

  //lo valido primero por si fue un falso contacto
  if (bomba->RequestOff)
  {
    StopAlarmBomba(bomba);
    bomba->NextMachineState = FSM_BOMBA_TURNING_OFF;
    PrintBombaMessage(F("Request Off"));
    return;
  }

  if (bomba->RequestDisabled)
  {
    StopAlarmBomba(bomba);
    bomba->NextMachineState = FSM_BOMBA_DISABLING;
    PrintBombaMessage(F("Request Disabled"));
    return;
  }

  if (IsFirstTimeInState(bomba))
  {
    StartAlarmBombaContactorAbierto(bomba);
    bomba->State = BOMBA_STATE_ERROR_CONTACTOR_ABIERTO;
    bomba->ContactorErrorCounter = bomba->ContactorErrorCounter + 1;
    bomba->StartError = millis();
    UpdateBombaDisplay(bomba);
    PrintBombaMessage(F("Contactor abierto"));
    return;
  }

  //despues de un tiempo deberia salir del error y probar de nuevo
  if (bomba->ContactorErrorCounter < BOMBA_CONTACTOR_ERROR_INTENTOS_MAX)
  {
    unsigned long delta = deltaMillis(millis(), bomba->StartError);
    if (delta > BOMBA_CONTACTOR_ERROR_INTERVAL)
    {
      StopAlarmBomba(bomba);
      bomba->NextMachineState = FSM_BOMBA_TURNING_OFF;
      PrintBombaMessage(F("Request Disabled"));
      return;
    }
  }

}

void FSM_Bomba_ErrorTermico(Bomba* bomba) {
  Statistics_BombaOff(bomba, false);

  //por si las dos estan en este error y una se recupera, la otra tiene que seguir gritando
  //verifico si el termico se normalizo.
  if (bomba->IsTermicoOk)
  {
    StopAlarmBomba(bomba);
    if (!IsBombaEnabled(bomba))
    {
      bomba->NextMachineState = FSM_BOMBA_DISABLING;
    }
    else
    {
      if (bomba->IsContactorClosed)
        bomba->NextMachineState = FSM_BOMBA_TURNING_ON;
      else
        bomba->NextMachineState = FSM_BOMBA_TURNING_OFF;
    }

    PrintBombaMessage(F("Termico OK-StopAlarm"));
    return;
  }

  StartAlarmBombaTermico(bomba);

  if (IsFirstTimeInState(bomba))
  {
    bomba->State = BOMBA_STATE_ERROR_TERMICO;
    bomba->StartError = millis();
    UpdateBombaDisplay(bomba);
    Statistics_BombaErrorTermico(bomba);
    PrintBombaMessage(F("Error Termico-Start Alarm"));
    return;
  }

  if (bomba->RequestDisabled)
  {
    StopAlarmBomba(bomba);
    bomba->NextMachineState = FSM_BOMBA_DISABLING;
    PrintBombaMessage(F("Request Disabled"));
    return;
  }
}

// ****************************************************************** //
//                          PRINT
// ****************************************************************** //

void PrintBombaMessage(const __FlashStringHelper* msg)
{
#ifdef TEST
  Serial.print(F(" ("));
  Serial.print(msg);
  Serial.print(F(")"));
#endif
}

void PrintBombaMessage(const __FlashStringHelper* msg, long wait, long maximo)
{
#ifdef TEST
  Serial.print(F(" ("));
  Serial.print(msg);
  Serial.print(F(" wait: "));
  Serial.print(wait);
  Serial.print(F(" Max: "));
  Serial.print(maximo);
  Serial.print(F(")"));
#endif

}

void PrintEnterStateBombaFSM(Bomba* bomba)
{
#ifdef TEST
  Serial.print(F("Bomba "));
  Serial.print(bomba->Number);
  Serial.print(F(": "));

  PrintStateBombaFSM(bomba->MachineState);
#endif

}

void PrintExitStateBombaFSM(Bomba* bomba)
{
#ifdef TEST
  byte state = bomba->MachineState;
  if (bomba->NextMachineState != FSM_BOMBA_NULL)
    state = bomba->NextMachineState;

  Serial.print(F(" -> "));
  PrintStateBombaFSM(state);
  Serial.println();
#endif

}

void PrintStateBombaFSMWithNewLine(byte current)
{
  PrintStateBombaFSM(current);
  Serial.println();
}

void PrintStateBombaFSM(byte current)
{

  switch (current)
  {
    case FSM_BOMBA_OFF:
      Serial.print(F("FSM_BOMBA_OFF"));
      break;
    case FSM_BOMBA_TURNING_ON:
      Serial.print(F("FSM_BOMBA_TURNING_ON"));
      break;
    case FSM_BOMBA_ON:
      Serial.print(F("FSM_BOMBA_ON"));
      break;
    case FSM_BOMBA_TURNING_OFF:
      Serial.print(F("FSM_BOMBA_TURNING_OFF"));
      break;
    case FSM_BOMBA_ERROR_CONTACTOR_CERRADO:
      Serial.print(F("FSM_BOMBA_ERROR_CONTACTOR_CERRADO"));
      break;
    case FSM_BOMBA_ERROR_CONTACTOR_ABIERTO:
      Serial.print(F("FSM_BOMBA_ERROR_CONTACTOR_ABIERTO"));
      break;
    case FSM_BOMBA_ERROR_TERMICO:
      Serial.print(F("FSM_BOMBA_ERROR_TERMICO"));
      break;
    case FSM_BOMBA_DISABLING:
      Serial.print(F("FSM_BOMBA_DISABLING"));
      break;
    case FSM_BOMBA_DISABLED:
      Serial.print(F("FSM_BOMBA_DISABLED"));
      break;
    case FSM_BOMBA_ENABLING:
      Serial.print(F("FSM_BOMBA_ENABLING"));
      break;
  }
}
