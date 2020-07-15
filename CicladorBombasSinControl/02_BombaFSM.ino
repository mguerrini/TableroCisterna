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

  //si falla el termico vamos al estado de error por termico
  if (!bomba->IsTermicoOk && bomba->MachineState != FSM_BOMBA_ERROR_TERMICO)
  {
    bomba->NextMachineState = FSM_BOMBA_ERROR_TERMICO;
#ifdef LOG_ENABLED
    PrintBombaMessage(F("Termico Abierto"));
    #endif
    return;
  }


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
#ifdef LOG_ENABLED
      PrintBombaMessage(F("Disabling-Stop Alarm"));
#endif      
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

// ============== OFF ============== //

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
#ifdef LOG_ENABLED
    PrintBombaMessage(F("Bomba OFF"));
#endif    
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
#ifdef LOG_ENABLED
    PrintBombaMessage(F("Request Disabled"));
#endif    
    return;
  }

  if (bomba->RequestOn)
  {
    bomba->NextMachineState = FSM_BOMBA_TURNING_ON;
#ifdef LOG_ENABLED
    PrintBombaMessage(F("Request On"));
#endif    
    return;
  }

  if (bomba->IsContactorClosed)
  {
    bomba->NextMachineState = FSM_BOMBA_TURNING_ON;
#ifdef LOG_ENABLED
    PrintBombaMessage(F("Contactor Cerrado"));
#endif    
    return;
  }
}

// ============== TURNING ON ============== //

void FSM_Bomba_TurningOn(Bomba* bomba) {
  bomba->RequestOn = false;

  if (bomba->RequestOff)
  {
    bomba->RequestOff = false;
    bomba->NextMachineState = FSM_BOMBA_TURNING_OFF;
#ifdef LOG_ENABLED
    PrintBombaMessage(F("Request Off"));
#endif    
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
#ifdef LOG_ENABLED
    PrintBombaMessage(F("Contactor Cerrado"));
#endif    
    return;
  }

  if (IsFirstTimeInState(bomba))
  {
    bomba->Timer = millis();
#ifdef LOG_ENABLED
    PrintBombaMessage(F("Iniciando timer"));
#endif    
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
#ifdef LOG_ENABLED
    PrintBombaMessage(F("Esperando Contactor Cerrado...Timeout"), wait, BOMBA_TURNING_ON_TIME);
#endif    
  }
  else
  {
#ifdef LOG_ENABLED
    PrintBombaMessage(F("Esperando Contactor Cerrado..."), wait, BOMBA_TURNING_ON_TIME);
#endif    
  }
}

// ============== ON ============== //

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
#ifdef LOG_ENABLED
    PrintBombaMessage(F("Bomba ON"));
#endif    
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
#ifdef LOG_ENABLED
    PrintBombaMessage(F("Request: Off"));
#endif    
    return;
  }

  if (!bomba->IsContactorClosed)
  {
    //se apago...
    bomba->NextMachineState = FSM_BOMBA_TURNING_OFF;
#ifdef LOG_ENABLED
    PrintBombaMessage(F("Contactor Abierto"));
#endif    
    return;
  }

  if (bomba->RequestDisabled)
  {
    bomba->NextMachineState = FSM_BOMBA_DISABLING;
#ifdef LOG_ENABLED
    PrintBombaMessage(F("Request: Disabled"));
#endif    
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

// ============== TURNING OFF ============== //

void FSM_Bomba_TurningOff(Bomba* bomba)
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
#ifdef LOG_ENABLED
    PrintBombaMessage(F("Contactor Abierto"));
#endif    
    return;
  }

  if (IsFirstTimeInState(bomba))
  {
    bomba->Timer = nowTime;
#ifdef LOG_ENABLED
    PrintBombaMessage(F("Iniciando timer"));
#endif    
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
#ifdef LOG_ENABLED
    PrintBombaMessage(F("Contactor Cerrado-Bomba continua encendida"), wait, BOMBA_TURNING_OFF_TIME);
#endif    
  }
  else
  {
#ifdef LOG_ENABLED
    PrintBombaMessage(F("Esperando contactor"), wait, BOMBA_TURNING_OFF_TIME);
#endif    
  }
}

// ============== DISABLED ============== //

void FSM_Bomba_Disabled(Bomba* bomba)
{
  if (IsFirstTimeInState(bomba))
  {
    //cancelo la estadistica de llenado
    Statistics_BombaOff(bomba, false);

    StopAlarmBomba(bomba);
    bomba->IsEnabled = false;
    UpdateBombaDisplay(bomba);
#ifdef LOG_ENABLED
    PrintBombaMessage(F("Bomba Deshabilitada"));
#endif    
    return;
  }

  if (bomba->RequestEnabled)
  {
    bomba->NextMachineState = FSM_BOMBA_ENABLING;
#ifdef LOG_ENABLED
    PrintBombaMessage(F("Request Enabled"));
#endif    
    return;
  }
}


// ============== ENABLING ============== //

void FSM_Bomba_Enabling(Bomba* bomba) {
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

#ifdef LOG_ENABLED
  PrintBombaMessage(F("Enabled-Stop Alarm"));
#endif  
}


// ============== ERROR: CONTACTOR ABIERTO ============== //

void FSM_Bomba_ErrorContactorAbierto(Bomba* bomba) {
  //deberia cerrarse para prenderse.

  //lo valido primero por si fue un falso contacto
  if (bomba->RequestOff)
  {
    StopAlarmBomba(bomba);
    bomba->NextMachineState = FSM_BOMBA_TURNING_OFF;
#ifdef LOG_ENABLED
    PrintBombaMessage(F("Request Off"));
#endif    
    return;
  }

  if (bomba->RequestDisabled)
  {
    StopAlarmBomba(bomba);
    bomba->NextMachineState = FSM_BOMBA_DISABLING;
#ifdef LOG_ENABLED
    PrintBombaMessage(F("Request Disabled"));
#endif    
    return;
  }

  if (IsFirstTimeInState(bomba))
  {
    StartAlarmBombaContactorAbierto(bomba);
    bomba->State = BOMBA_STATE_ERROR_CONTACTOR_ABIERTO;
    bomba->ContactorErrorCounter = bomba->ContactorErrorCounter + 1;
    bomba->StartError = millis();
    UpdateBombaDisplay(bomba);
#ifdef LOG_ENABLED
    PrintBombaMessage(F("Contactor abierto"));
#endif    
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
#ifdef LOG_ENABLED
      PrintBombaMessage(F("Request Disabled"));
#endif      
      return;
    }
  }

}

// ============== ERROR: TERMICO SALTADO ============== //

void FSM_Bomba_ErrorTermico(Bomba* bomba) {
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

#ifdef LOG_ENABLED
    PrintBombaMessage(F("Termico OK-StopAlarm"));
#endif    
    return;
  }

  StartAlarmBombaTermico(bomba);

  if (IsFirstTimeInState(bomba))
  {
    bomba->State = BOMBA_STATE_ERROR_TERMICO;
    bomba->StartError = millis();
    UpdateBombaDisplay(bomba);
    Statistics_BombaErrorTermico(bomba);
#ifdef LOG_ENABLED
    PrintBombaMessage(F("Error Termico-Start Alarm"));
#endif    
    return;
  }

  if (bomba->RequestDisabled)
  {
    StopAlarmBomba(bomba);
    bomba->NextMachineState = FSM_BOMBA_DISABLING;
#ifdef LOG_ENABLED
    PrintBombaMessage(F("Request Disabled"));
#endif    
    return;
  }
}

// ****************************************************************** //
//                          PRINT
// ****************************************************************** //

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
  Serial.print(F("Bomba "));
  Serial.print(bomba->Number);
  Serial.print(F(": "));

  PrintStateBombaFSM(bomba->MachineState);
#endif

}

void PrintExitStateBombaFSM(Bomba* bomba)
{
#ifdef LOG_ENABLED
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
