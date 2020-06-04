void BombaStateMachine(Bomba* bomba)
{
  long wait = 0;
  bomba->NextMachineState = FSM_BOMBA_NULL;
  PrintEnterStateBombaFSM(bomba);

  switch (bomba->MachineState)
  {
    case FSM_BOMBA_OFF:
      if (IsFirstTimeInState(bomba))
      {
        bomba->State = BOMBA_STATE_OFF;
        UpdateBombaDisplay(bomba);
        break;
      }

      if (bomba->RequestDisabled)
      {
        bomba->NextMachineState = FSM_BOMBA_DISABLING;
        PrintExitStateBombaFSM(bomba, "Request Disabled");
        break;
      }

      if (bomba->RequestOn)
      {
        bomba->RequestOn = false;
        bomba->Timer = millis();
        bomba->NextMachineState = FSM_BOMBA_TURNING_ON;
        PrintExitStateBombaFSM(bomba, "Request On");
        break;
      }

      if (bomba->IsContactorClosed)
      {
        bomba->NextMachineState = FSM_BOMBA_TURNING_ON;
        PrintExitStateBombaFSM(bomba, "Contactor Cerrado");
        break;
      }

      if (!bomba->IsTermicoOk)
      {
        bomba->NextMachineState = FSM_BOMBA_ERROR_TERMICO;
        PrintExitStateBombaFSM(bomba, "Termico Abierto");
        break;
      }

      PrintExitStateBombaFSM(bomba, NULL);
      break;

    case FSM_BOMBA_ON:

      if (IsFirstTimeInState(bomba))
      {
        bomba->State = BOMBA_STATE_ON;
        UpdateBombaDisplay(bomba);
        break;
      }

      if (bomba->RequestDisabled)
      {
        bomba->NextMachineState = FSM_BOMBA_DISABLING;
        PrintExitStateBombaFSM(bomba, "Request: Disabled");
        break;
      }

      if (bomba->RequestOff)
      {
        bomba->NextMachineState = FSM_BOMBA_TURNING_OFF;
        bomba->Timer = millis();
        PrintExitStateBombaFSM(bomba, "Request: Off");
        break;
      }

      if (!bomba->IsContactorClosed)
      {
        //se apago...
        bomba->NextMachineState = FSM_BOMBA_TURNING_OFF;
        bomba->Timer = millis();
        PrintExitStateBombaFSM(bomba, "Contactor Abierto");
        break;
      }

      if (!bomba->IsTermicoOk)
      {
        bomba->NextMachineState = FSM_BOMBA_ERROR_TERMICO;
        PrintExitStateBombaFSM(bomba, "Termico abierto");
        break;
      }

      PrintExitStateBombaFSM(bomba, NULL);
      break;

    case FSM_BOMBA_TURNING_ON:

      //espero a que el contactor este cerrado y que el termico este OK
      if (!bomba->IsTermicoOk)
      {
        bomba->NextMachineState = FSM_BOMBA_ERROR_TERMICO;
        PrintExitStateBombaFSM(bomba, "Termico abierto");
        break;
      }

      if (bomba->IsContactorClosed)
      {
        bomba->NextMachineState = FSM_BOMBA_ON;
        PrintExitStateBombaFSM(bomba, "Contactor Cerrado");
        break;
      }
      else
      {
        //espero un tiempo prudencial hasta que arranque la bomba
        wait = millis() - bomba->Timer;
        //espero a que inicie la bomba activa que deberia estar en ON.
        //sino se inicia => el contactor fallo
        if (wait > BOMBA_TURNING_ON_TIME)
        {
          //cambio de bomba activa
          bomba->NextMachineState = FSM_BOMBA_ERROR_CONTACTOR_ABIERTO;
          PrintExitStateBombaFSM(bomba, "Esperando Contactor Cerrado...Timeout");
        }
        else
          PrintExitStateBombaFSM(bomba, "Esperando Contactor Cerrado...");
      }
      break;

    case FSM_BOMBA_TURNING_OFF:
      if (!bomba->IsTermicoOk)
      {
        bomba->NextMachineState = FSM_BOMBA_ERROR_TERMICO;
        PrintExitStateBombaFSM(bomba, "Termico Abierto");
        break;
      }

      if (!bomba->IsContactorClosed)
      {
        bomba->NextMachineState = FSM_BOMBA_OFF;
        PrintExitStateBombaFSM(bomba, "Contactor Abierto");
        break;
      }

      //espero un tiempo prudencial hasta que arranque la bomba
      wait = millis() - bomba->Timer;
      //espero a que inicie....la bomba activa deberia estar en ON.
      if (wait > BOMBA_TURNING_OFF_TIME)
      {
        //ERROR....NO SE DETUVO LA BOMBA Y SIGUE FUNCIONANDO...NUNCA DEBERIA OCURRIR, PORQUE LOS SENSORES APAGAN LAS BOMBAS
        //Serial.println("La bomba sigue andando con sensores que indican apagada. Error en los sensores.");
        //Vuelvo a ON
        bomba->State = BOMBA_STATE_ERROR_CONTACTOR_CERRADO;
        PrintExitStateBombaFSM(bomba, "Contactor Cerrado-Bomba continua encendida");
      }
      break;

    case FSM_BOMBA_DISABLING:

      StopAlarm();
      bomba->RequestDisabled = false;
      bomba->NextMachineState = FSM_BOMBA_DISABLED;

      PrintExitStateBombaFSM(bomba, "Disabling-Stop Alarm");
      break;

    case FSM_BOMBA_ENABLING:
      StopAlarm();
      bomba->RequestEnabled = false;
      bomba->IsEnabled = true;
      if (bomba->IsContactorClosed)
        bomba->NextMachineState = FSM_BOMBA_TURNING_ON;
      else
        bomba->NextMachineState = FSM_BOMBA_TURNING_OFF;

      PrintExitStateBombaFSM(bomba, "Enabling-Stop Alarm");
      break;

    case FSM_BOMBA_DISABLED:
      if (IsFirstTimeInState(bomba))
      {
        bomba->IsEnabled = false;
        UpdateBombaDisplay(bomba);
        PrintExitStateBombaFSM(bomba, "Bomba Deshabilitada");
        break;
      }

      if (bomba->RequestEnabled)
      {
        bomba->NextMachineState = FSM_BOMBA_ENABLING;
        PrintExitStateBombaFSM(bomba, "Request Enabled");
        break;
      }

      PrintExitStateBombaFSM(bomba);
      break;

    case FSM_BOMBA_ERROR_CONTACTOR_ABIERTO:
      if (IsFirstTimeInState(bomba))
      {
        bomba->State = BOMBA_STATE_ERROR_CONTACTOR_ABIERTO;
        UpdateBombaDisplay(bomba);
        PrintExitStateBombaFSM(bomba, "Bomba continua encendida");
        break;
      }

      if (bomba->RequestEnabled)
      {
        bomba->NextMachineState = FSM_BOMBA_ENABLING;
        PrintExitStateBombaFSM(bomba, "Request Enabled");
        break;
      }

      if (bomba->RequestDisabled)
      {
        bomba->NextMachineState = FSM_BOMBA_DISABLING;
        PrintExitStateBombaFSM(bomba, "Request Disabled");
        break;
      }

      break;
    /*
        case FSM_BOMBA_ERROR_CONTACTOR_CERRADO:
          PrintEnterStateBombaFSM(bomba);
          bomba->State = BOMBA_STATE_ON;
          UpdateBombaDisplay(bomba);
          PrintExitStateBombaFSM(bomba, "Bomba continua encendida");
          break;
    */
    case FSM_BOMBA_ERROR_TERMICO:
      //por si las dos estan en este error y una se recupera, la otra tiene que seguir gritando
      StartAlarm();

      if (IsFirstTimeInState(bomba))
      {
        bomba->State = BOMBA_STATE_ERROR_TERMICO;
        UpdateBombaDisplay(bomba);
        PrintExitStateBombaFSM(bomba, "Error Termico-Start Alarm");
        break;
      }

      if (bomba->RequestEnabled)
      {
        bomba->NextMachineState = FSM_BOMBA_ENABLING;
        PrintExitStateBombaFSM(bomba, "Request Enabled");
        break;
      }

      if (bomba->RequestDisabled)
      {
        bomba->NextMachineState = FSM_BOMBA_DISABLING;
        PrintExitStateBombaFSM(bomba, "Request Disabled");
        break;
      }

      //verifico si el termico se normalizo.
      if (bomba->IsTermicoOk)
      {
        StopAlarm();
        if (bomba->IsContactorClosed)
          bomba->NextMachineState = FSM_BOMBA_TURNING_ON;
        else
          bomba->NextMachineState = FSM_BOMBA_TURNING_OFF;

        PrintExitStateBombaFSM(bomba, "Termico OK-StopAlarm");
        break;
      }

      PrintExitStateBombaFSM(bomba);
      break;
  }

  bomba->FromMachineState = bomba->MachineState;
  if (bomba->NextMachineState != FSM_BOMBA_NULL)
    bomba->MachineState = bomba->NextMachineState;
}

bool IsFirstTimeInState(Bomba* bomba)
{
  return bomba->FromMachineState != bomba->MachineState;
}

void PrintEnterStateBombaFSM(Bomba* bomba)
{
  Serial.print(F("Bomba "));
  Serial.print(bomba->Number);
  Serial.print(F(": "));

  PrintStateBombaFSM(NULL, bomba->MachineState, false);
}

void PrintExitStateBombaFSM(Bomba* bomba)
{
  PrintExitStateBombaFSM(bomba, NULL);
}


void PrintExitStateBombaFSM(Bomba* bomba, const char* msg)
{
  BombaMachineStates state = bomba->MachineState;
  if (bomba->NextMachineState != FSM_BOMBA_NULL)
    state = bomba->NextMachineState;

  if (msg == NULL)
    PrintStateBombaFSM(" -> ", state, true);
  else
  {
    PrintStateBombaFSM(" -> ", state, false);
    Serial.print(F(" ("));
    Serial.print(msg);
    Serial.println(F(")"));
  }
}

void PrintStateBombaFSM(const char* prefijo, BombaMachineStates current, bool newline)
{
  if (prefijo != NULL)
    Serial.print(prefijo);

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

  if (newline)
    Serial.println();
}
