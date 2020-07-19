// *************************************************** //
//                  AUTO BOMBA MACHINE
// *************************************************** //

void BombaStateMachine(Bomba* bomba)
{
  bomba->NextMachineState = FSM_BOMBA_NULL;
  bomba->Message = 0;
  //  PrintEnterStateBombaFSM(bomba);

  ExecuteBombaState(bomba);

  //  PrintExitStateBombaFSM(bomba);
  PrintStateBombaFSM(bomba);

  bomba->FromMachineState = bomba->MachineState;
  if (bomba->NextMachineState != FSM_BOMBA_NULL)
    bomba->MachineState = bomba->NextMachineState;
}


void ExecuteBombaState(Bomba* bomba)
{
  int bombaState = bomba->MachineState;

  //si falla el termico vamos al estado de error por termico
  if (!bomba->IsTermicoOk && bomba->MachineState != FSM_BOMBA_ERROR_TERMICO)
  {
    bomba->NextMachineState = FSM_BOMBA_ERROR_TERMICO;

    bomba->Message = MSG_BOMBA_TERMICO_ABIERTO;
    return;
  }


  /*
    //fase error....no importa....total se apaga

  */
  // ********************************************** //
  //                   FSM_BOMBA_OFF
  // ********************************************** //

  if (bombaState == FSM_BOMBA_OFF)
  {
    //limpio cualquier pedido de apagado
    bomba->RequestOff = false;

    if (IsFirstTimeInState(bomba))
    {
      bomba->State = BOMBA_STATE_OFF;
      UpdateBombaDisplay(bomba);

      //actualizo el display del tanque, para que pase a normal
      UpdateTanqueDisplay();

      //registro si es modo automatico....sino puede funcionar sin llenado completo
      Statistics_BombaOff(bomba, IsAutomaticMode());
      bomba->StartTime = 0;

      bomba->Message = MSG_BOMBA_OFF;
      return;
    }
    /*
      if (!bomba->IsTermicoOk)
      {
        bomba->NextMachineState = FSM_BOMBA_ERROR_TERMICO;
        PrintBombaMessage(F("Termico Abierto"));
        return;
      }
    */
    if (bomba->RequestDisabled)
    {
      bomba->NextMachineState = FSM_BOMBA_DISABLING;

      bomba->Message = MSG_BOMBA_REQUEST_DISABLED;
      return;
    }

    if (bomba->RequestOn)
    {
      bomba->NextMachineState = FSM_BOMBA_TURNING_ON;

      bomba->Message = MSG_BOMBA_REQUEST_ON;
      return;
    }

    if (bomba->IsContactorClosed)
    {
      bomba->NextMachineState = FSM_BOMBA_TURNING_ON;

      bomba->Message = MSG_BOMBA_CONTACTOR_CERRADO;
      return;
    }

    return;
  }


  // ********************************************** //
  //              FSM_BOMBA_TURNING_ON
  // ********************************************** //

  if (bombaState == FSM_BOMBA_TURNING_ON)
  {
    bomba->RequestOn = false;

    if (bomba->RequestOff)
    {
      bomba->RequestOff = false;
      bomba->NextMachineState = FSM_BOMBA_TURNING_OFF;

      bomba->Message = MSG_BOMBA_REQUEST_OFF;
      return;
    }
    /*
      //espero a que el contactor este cerrado y que el termico este OK
      if (!bomba->IsTermicoOk)
      {
        bomba->NextMachineState = FSM_BOMBA_ERROR_TERMICO;
        PrintBombaMessage(F("Termico abierto"));
        return;
      }
    */
    if (bomba->IsContactorClosed)
    {
      bomba->NextMachineState = FSM_BOMBA_ON;

      bomba->Message = MSG_BOMBA_CONTACTOR_CERRADO;
      return;
    }

    if (IsFirstTimeInState(bomba))
    {
      bomba->Timer = millis();

      bomba->Message = MSG_BOMBA_START_TIMER;
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

      bomba->Message = MSG_BOMBA_CONTACTOR_CERRADO_TIMEOUT;
    }
    else
    {
      bomba->Message = MSG_BOMBA_WAITING_CONTACTOR_CERRADO;
    }

    return;
  }


  // ********************************************** //
  //                FSM_BOMBA_ON
  // ********************************************** //

  if (bombaState == FSM_BOMBA_ON)
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

      //actualizo el display del tanque, para que pase a cargando
      UpdateTanqueDisplay();

      //inicio la estadistica
      Statistics_BombaOn(bomba);

      bomba->Message = MSG_BOMBA_ON;
      return;
    }
    /*
      if (!bomba->IsTermicoOk)
      {
        bomba->NextMachineState = FSM_BOMBA_ERROR_TERMICO;
        PrintBombaMessage(F("Termico abierto"));
        return;
      }
    */
    if (bomba->RequestOff)
    {
      bomba->NextMachineState = FSM_BOMBA_TURNING_OFF;
  bomba->Message = MSG_BOMBA_REQUEST_OFF;
      return;
    }

    if (!bomba->IsContactorClosed)
    {
      //se apago...
      bomba->NextMachineState = FSM_BOMBA_TURNING_OFF;
     bomba->Message = MSG_BOMBA_CONTACTOR_ABIERTO;
      return;
    }

    if (bomba->RequestDisabled)
    {
      bomba->NextMachineState = FSM_BOMBA_DISABLING;
      bomba->Message = MSG_BOMBA_REQUEST_DISABLED;
      return;
    }

    //refresco el tiempo en pantalla
    unsigned long nowTime = millis();
    unsigned long delta = deltaMillis(nowTime, bomba->RefreshTime);
    if (delta > BOMBA_REFRESH_WORKING_TIME) {
      bomba->RefreshTime = nowTime;
      UpdateBombaWorkingTime(bomba);
    }

    return;
  }


  // ********************************************** //
  //            FSM_BOMBA_TURNING_OFF
  // ********************************************** //

  if (bombaState == FSM_BOMBA_TURNING_OFF)
  {
    unsigned long nowTime = millis();
    bomba->RequestOff = false;

    /*
      //la bomba esta en error
      if (!bomba->IsTermicoOk)
      {
        bomba->NextMachineState = FSM_BOMBA_ERROR_TERMICO;
        PrintBombaMessage(F("Termico Abierto"));
        return;
      }
    */

    //ya esta apagada
    if (!bomba->IsContactorClosed)
    {
      bomba->NextMachineState = FSM_BOMBA_OFF;

      bomba->Message = MSG_BOMBA_CONTACTOR_ABIERTO;
      return;
    }

    if (IsFirstTimeInState(bomba))
    {
      bomba->Timer = nowTime;

      bomba->Message = MSG_BOMBA_START_TIMER;
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

      bomba->Message = MSG_BOMBA_CONTACTOR_CERRADO_BOMBA_ON;
    }
    else
    {
      bomba->Message = MSG_BOMBA_WAITING_CONTACTOR;
    }

    return;
  }


  // ********************************************** //
  //             FSM_BOMBA_DISABLING
  // ********************************************** //

  if (bombaState == FSM_BOMBA_DISABLING)
  {
    bomba->NextMachineState = FSM_BOMBA_DISABLED;
    bomba->Message = MSG_BOMBA_DISABLING;

    return;
  }


  // ********************************************** //
  //             FSM_BOMBA_DISABLED
  // ********************************************** //

  if (bombaState == FSM_BOMBA_DISABLED)
  {
    if (IsFirstTimeInState(bomba))
    {
      //cancelo la estadistica de llenado
      Statistics_BombaOff(bomba, false);

      StopAlarmBomba(bomba);
      bomba->IsEnabled = false;
      UpdateBombaDisplay(bomba);

      //actualizo el display del tanque, para que pase a cargando
      UpdateTanqueDisplay();
      bomba->Message = MSG_BOMBA_DISABLED;
      return;
    }

    if (bomba->RequestEnabled)
    {
      bomba->NextMachineState = FSM_BOMBA_ENABLING;
      bomba->Message = MSG_BOMBA_REQUEST_ENABLED;
      return;
    }

    return;
  }


  // ********************************************** //
  //            FSM_BOMBA_ENABLING
  // ********************************************** //

  if (bombaState == FSM_BOMBA_ENABLING)
  {
    StopAlarmBomba(bomba);
    bomba->IsEnabled = true;
    /*
      if (!bomba->IsTermicoOk)
      {
        bomba->NextMachineState = FSM_BOMBA_ERROR_TERMICO;
        PrintBombaMessage(F("Enabled-StopAlarm-Termico Abierto"));
        return;
      }
    */
    //no verifico el estado del contactor....
    if (bomba->IsContactorClosed)
      bomba->NextMachineState = FSM_BOMBA_TURNING_ON;
    else
      bomba->NextMachineState = FSM_BOMBA_TURNING_OFF;

    bomba->Message = MSG_BOMBA_ENABLING;
    return;
  }


  // ********************************************** //
  //       FSM_BOMBA_ERROR_CONTACTOR_ABIERTO
  // ********************************************** //

  if (bombaState == FSM_BOMBA_ERROR_CONTACTOR_ABIERTO)
  {
    //deberia cerrarse para prenderse.

    //lo valido primero por si fue un falso contacto
    if (bomba->RequestOff)
    {
      StopAlarmBomba(bomba);
      bomba->NextMachineState = FSM_BOMBA_TURNING_OFF;
      bomba->Message = MSG_BOMBA_REQUEST_OFF;
      return;
    }

    if (bomba->RequestDisabled)
    {
      StopAlarmBomba(bomba);
      bomba->NextMachineState = FSM_BOMBA_DISABLING;
      bomba->Message = MSG_BOMBA_REQUEST_DISABLED;
      return;
    }

    if (IsFirstTimeInState(bomba))
    {
      StartAlarmBombaContactorAbierto(bomba);
      bomba->State = BOMBA_STATE_ERROR_CONTACTOR_ABIERTO;
      bomba->ContactorErrorCounter = bomba->ContactorErrorCounter + 1;
      bomba->StartError = millis();
      UpdateBombaDisplay(bomba);
      //actualizo el display del tanque, para que pase a cargando
      UpdateTanqueDisplay();

      bomba->Message = MSG_BOMBA_CONTACTOR_ABIERTO;
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

        bomba->Message = MSG_BOMBA_REQUEST_DISABLED;
        return;
      }
    }

    return;
  }


  // ********************************************** //
  //           FSM_BOMBA_ERROR_TERMICO
  // ********************************************** //

  if (bombaState == FSM_BOMBA_ERROR_TERMICO)
  {
    //limpio todo porque esta en error
    bomba->RequestOn = false;
    bomba->RequestOff = false;
    bomba->RequestEnabled = false;

    Statistics_BombaOff(bomba, false);

    //por si las dos estan en este error y una se recupera, la otra tiene que seguir gritando
    //verifico si el termico se normalizo.
    if (bomba->IsTermicoOk)
    {
      StopAlarmBomba(bomba);

      //si va a off....se actualiza solo

      if (!bomba->IsEnabled)
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

      bomba->Message = MSG_BOMBA_TERMICO_OK_STOP_ALARM;
      return;
    }

    StartAlarmBombaTermico(bomba);

    if (IsFirstTimeInState(bomba))
    {
      bomba->State = BOMBA_STATE_ERROR_TERMICO;
      bomba->StartError = millis();
      UpdateBombaDisplay(bomba);
      Statistics_BombaErrorTermico(bomba);

      bomba->Message = MSG_BOMBA_ERROR_TERMICO_START_ALARM;
      return;
    }

    if (bomba->RequestDisabled)
    {
      StopAlarmBomba(bomba);
      bomba->NextMachineState = FSM_BOMBA_DISABLING;

      bomba->Message = MSG_BOMBA_REQUEST_DISABLED;
      return;
    }

    return;
  }
}

bool IsFirstTimeInState(Bomba* bomba)
{
  return bomba->FromMachineState != bomba->MachineState;
}

// ****************************************************************** //
//                          PRINT
// ****************************************************************** //
/*
  void PrintBombaMessage(const __FlashStringHelper* msg)
  {
  #ifdef LOG_ENABLED
  Serial.print(F(" ("));
  Serial.print(msg);
  Serial.print(F(")"));
  #endif
  }

  void PrintBombaMessage(const __FlashStringHelper* msg, long wait, long maximo)
  {
  #ifdef LOG_ENABLED
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
  #ifdef LOG_ENABLED
  #ifdef LOG_MIN_ENABLED
  if (bomba->FromMachineState != bomba->MachineState)
  {
    Serial.print(F("Bomba "));
    Serial.print(bomba->Number);
    Serial.print(F(": "));

    PrintStateBombaFSM(bomba->MachineState);
  }
  #else
  Serial.print(F("Bomba "));
  Serial.print(bomba->Number);
  Serial.print(F(": "));

  PrintStateBombaFSM(bomba->MachineState);
  #endif
  #endif

  }

  void PrintExitStateBombaFSM(Bomba* bomba)
  {
  #ifdef LOG_ENABLED
  #ifdef LOG_MIN_ENABLED

  if (bomba->NextMachineState != FSM_BOMBA_NULL && bomba->MachineState != bomba->NextMachineState)
  {
    byte state = bomba->MachineState;
    if (bomba->NextMachineState != FSM_BOMBA_NULL)
      state = bomba->NextMachineState;

    Serial.print(F(" -> "));
    PrintStateBombaFSM(state);
    Serial.println();
  }

  #else

  byte state = bomba->MachineState;
  if (bomba->NextMachineState != FSM_BOMBA_NULL)
    state = bomba->NextMachineState;

  Serial.print(F(" -> "));
  PrintStateBombaFSM(state);
  Serial.println();
  #endif
  #endif
  }

  void PrintStateBombaFSMWithNewLine(byte current)
  {
  PrintStateBombaFSM(current);
  Serial.println();
  }
*/



void PrintStateBombaFSM(Bomba* bomba)
{
#ifdef LOG_ENABLED
#ifdef LOG_MIN_ENABLED
  if (bomba->FromMachineState != bomba->MachineState ||
      (bomba->NextMachineState != FSM_BOMBA_NULL && bomba->MachineState != bomba->NextMachineState) ||
      bomba->Message != 0)

  {
#endif
    Serial.print(F("Bomba "));
    Serial.print(bomba->Number);
    Serial.print(F(": "));

    PrintStateBombaFSM(bomba->MachineState);

    //mensaje
    PrintBombaMessage(bomba);

    byte state = bomba->MachineState;
    if (bomba->NextMachineState != FSM_BOMBA_NULL)
      state = bomba->NextMachineState;

    Serial.print(F(" -> "));
    PrintStateBombaFSM(state);
    Serial.println();

#ifdef LOG_MIN_ENABLED
  }
#endif
#endif
}


void PrintStateBombaFSM(byte current)
{
#ifdef LOG_ENABLED
  byte number = automaticFSM.Message;

  if (number == 0)
    return;

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

void PrintBombaMessage(Bomba* bomba)
{
  byte number = bomba->Message;

  switch (number)
  {
    case MSG_BOMBA_ON:
      Serial.print(F("Bomba ON"));
      break;
    case MSG_BOMBA_OFF:
      Serial.print(F("Bomba OFF"));
      break;
    case MSG_BOMBA_REQUEST_DISABLED:
      Serial.print(F("Request Disabled"));
      break;
    case MSG_BOMBA_REQUEST_ENABLED:
      Serial.print(F("Request Enabled"));
      break;
    case MSG_BOMBA_REQUEST_ON:
      Serial.print(F("Request On"));
      break;
    case MSG_BOMBA_REQUEST_OFF:
      Serial.print(F("Request: Off"));
      break;
    case MSG_BOMBA_TERMICO_ABIERTO:
      Serial.print(F("Termico Abierto"));
      break;
    case MSG_BOMBA_TERMICO_OK_STOP_ALARM:
      Serial.print(F("Termico OK-StopAlarm"));
      break;
    case MSG_BOMBA_CONTACTOR_CERRADO:
      Serial.print(F("Contactor Cerrado"));
      break;
    case MSG_BOMBA_CONTACTOR_ABIERTO:
      Serial.print(F("Contactor Abierto"));
      break;
    case MSG_BOMBA_START_TIMER:
      Serial.print(F("Iniciando timer"));
      break;
    case MSG_BOMBA_CONTACTOR_CERRADO_TIMEOUT:
      Serial.print(F("Esperando Contactor Cerrado...Timeout"));
      break;
    case MSG_BOMBA_WAITING_CONTACTOR_CERRADO:
      Serial.print(F("Esperando Contactor Cerrado..."));
      break;
    case MSG_BOMBA_CONTACTOR_CERRADO_BOMBA_ON:
      Serial.print(F("Contactor Cerrado-Bomba continua encendida"));
      break;
    case MSG_BOMBA_WAITING_CONTACTOR:
      Serial.print(F("Esperando contactor"));
      break;
    case MSG_BOMBA_DISABLING:
      Serial.print(F("Disabling-Stop Alarm"));
      break;
    case MSG_BOMBA_DISABLED:
      Serial.print(F("Bomba Deshabilitada"));
      break;
    case MSG_BOMBA_ENABLING:
      Serial.print(F("Enabling-Stop Alarm"));
      break;
    case MSG_BOMBA_ERROR_TERMICO_START_ALARM:
      Serial.print(F("Error Termico-Start Alarm"));
      break;
  }

  Serial.print(F(")"));
#endif
}
