void BombaStateMachine(Bomba* bomba)
{
  long wait = 0;

  switch (bomba->MachineState)
  {
    case FSM_BOMBA_OFF:
      PrintEnterStateBombaFSM(bomba);

      if (bomba->RequestDisabled)
      {
        bomba->MachineState = FSM_BOMBA_DISABLING;
        PrintExitStateBombaFSM(bomba, "Request Disabled");
        return;
      }

      if (bomba->RequestOn)
      {
        bomba->RequestOn = false;
        bomba->Timer = millis();
        bomba->MachineState = FSM_BOMBA_TURNING_ON;
        PrintExitStateBombaFSM(bomba, "Request On");
        return;
      }

      if (bomba->IsContactorClosed)
      {
        bomba->MachineState = FSM_BOMBA_TURNING_ON;
        PrintExitStateBombaFSM(bomba, "Contactor Cerrado");
        return;
      }

      if (!bomba->IsTermicoOk)
      {
        bomba->MachineState = FSM_BOMBA_ERROR_TERMICO;
        PrintExitStateBombaFSM(bomba, "Termico Abierto");
        return;
      }

      PrintExitStateBombaFSM(bomba, NULL);
      break;

    case FSM_BOMBA_ON:
      PrintEnterStateBombaFSM(bomba);

      if (bomba->RequestDisabled)
      {
        bomba->MachineState = FSM_BOMBA_DISABLING;
        PrintExitStateBombaFSM(bomba, "Request: Disabled");
        return;
      }

      if (bomba->RequestOff)
      {
        bomba->MachineState = FSM_BOMBA_TURNING_OFF;
        bomba->Timer = millis();
        PrintExitStateBombaFSM(bomba, "Request: Off");
        return;
      }

      if (!bomba->IsContactorClosed)
      {
        //se apago...
        bomba->MachineState = FSM_BOMBA_TURNING_OFF;
        bomba->Timer = millis();
        PrintExitStateBombaFSM(bomba, "Contactor Abierto");
        return;
      }

      if (!bomba->IsTermicoOk)
      {
        bomba->MachineState = FSM_BOMBA_ERROR_TERMICO;
        PrintExitStateBombaFSM(bomba, "Termico abierto");
        return;
      }

      PrintExitStateBombaFSM(bomba, NULL);
      break;

    case FSM_BOMBA_TURNING_ON:
      PrintEnterStateBombaFSM(bomba);

      //espero a que el contactor este cerrado y que el termico este OK
      if (!bomba->IsTermicoOk)
      {
        bomba->MachineState = FSM_BOMBA_ERROR_TERMICO;
        PrintExitStateBombaFSM(bomba, "Termico abierto");
        return;
      }

      if (bomba->IsContactorClosed)
      {
        bomba->State = BOMBA_STATE_ON;
        UpdateBombaDisplay(bomba);
        bomba->MachineState = FSM_BOMBA_ON;
        PrintExitStateBombaFSM(bomba, "Contactor Cerrado");
        return;
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
          bomba->MachineState = FSM_BOMBA_ERROR_CONTACTOR_ABIERTO;
          PrintExitStateBombaFSM(bomba, "Esperando Contactor Cerrado...Timeout");
        }
        else
          PrintExitStateBombaFSM(bomba, "Esperando Contactor Cerrado...");
      }
      break;

    case FSM_BOMBA_TURNING_OFF:
      PrintEnterStateBombaFSM(bomba);

      if (!bomba->IsTermicoOk)
      {
        bomba->MachineState = FSM_BOMBA_ERROR_TERMICO;
        PrintExitStateBombaFSM(bomba, "Termico Abierto");
        return;
      }

      if (!bomba->IsContactorClosed)
      {
        bomba->State = BOMBA_STATE_OFF;
        UpdateBombaDisplay(bomba);
        bomba->MachineState = FSM_BOMBA_OFF;
        PrintExitStateBombaFSM(bomba, "Bomba Apagada");
        return;
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
        PrintExitStateBombaFSM(bomba, "Bomba continua encendida");
      }
      break;

    case FSM_BOMBA_DISABLING:
      PrintEnterStateBombaFSM(bomba);

      StopAlarm();
      bomba->IsEnabled = false;
      UpdateBombaDisplay(bomba);
      bomba->MachineState = FSM_BOMBA_DISABLED;

      PrintExitStateBombaFSM(bomba, "Bomba Deshabilitada-Stop Alarma");
      break;

    case FSM_BOMBA_ENABLING:
      PrintEnterStateBombaFSM(bomba);

      StopAlarm();
      bomba->IsEnabled = true;
      if (bomba->IsContactorClosed)
        bomba->MachineState = FSM_BOMBA_TURNING_ON;
      else
        bomba->MachineState = FSM_BOMBA_TURNING_OFF;

      PrintExitStateBombaFSM(bomba, "Enabled");
      break;

    case FSM_BOMBA_DISABLED:
      PrintEnterStateBombaFSM(bomba);

      if (bomba->RequestEnabled)
      {
        bomba->MachineState = FSM_BOMBA_ENABLING;
        PrintExitStateWorkingFSM("Request Enabled");
        return;
      }

      PrintExitStateBombaFSM(bomba);
      break;

    case FSM_BOMBA_ERROR_CONTACTOR_ABIERTO:
      if (bomba->State != BOMBA_STATE_ERROR_CONTACTOR_ABIERTO)
      {
        PrintEnterStateBombaFSM(bomba);

        bomba->State = BOMBA_STATE_ERROR_CONTACTOR_ABIERTO;
        UpdateBombaDisplay(bomba);
        PrintExitStateBombaFSM(bomba, "Bomba continua encendida");
        return;
      }

      if (bomba->RequestEnabled)
      {
        bomba->MachineState = FSM_BOMBA_ENABLING;
        PrintExitStateBombaFSM(bomba, "Request Enabled");
        return;
      }

      if (bomba->RequestDisabled)
      {
        bomba->MachineState = FSM_BOMBA_DISABLING;
        PrintExitStateBombaFSM(bomba, "Request Disabled");
        return;
      }
      
      break;

    case FSM_BOMBA_ERROR_CONTACTOR_CERRADO:
      PrintEnterStateBombaFSM(bomba);
      bomba->State = BOMBA_STATE_ON;
      UpdateBombaDisplay(bomba);
      PrintExitStateBombaFSM(bomba, "Bomba continua encendida");
      break;

    case FSM_BOMBA_ERROR_TERMICO:
      PrintEnterStateBombaFSM(bomba);

      //por si las dos estan en este error y una se recupera, la otra tiene que seguir gritando
      StartAlarm();

      if (bomba->State != BOMBA_STATE_ERROR_TERMICO)
      {
        bomba->State = BOMBA_STATE_ERROR_TERMICO;
        UpdateBombaDisplay(bomba);

        PrintExitStateBombaFSM(bomba, "Error Termico-Start Alarm");
        return;
      }

      if (bomba->RequestEnabled)
      {
        bomba->MachineState = FSM_BOMBA_ENABLING;
        PrintExitStateBombaFSM(bomba, "Request Enabled");
        return;
      }

      if (bomba->RequestDisabled)
      {
        bomba->MachineState = FSM_BOMBA_DISABLING;
        PrintExitStateBombaFSM(bomba, "Request Disabled");
        return;
      }

      //verifico si el termico se normalizo.
      if (bomba->IsTermicoOk)
      {
        StopAlarm();
        if (bomba->IsContactorClosed)
          bomba->MachineState = FSM_BOMBA_TURNING_ON;
        else
          bomba->MachineState = FSM_BOMBA_TURNING_OFF;

        PrintExitStateBombaFSM(bomba, "Termico OK");
        return;
      }

      PrintExitStateBombaFSM(bomba);
      break;
  }
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
  if (msg == NULL)
    PrintStateBombaFSM(" -> ", bomba->MachineState, true);
  else
  {
    PrintStateBombaFSM(" -> ", bomba->MachineState, false);
    Serial.print(F(" ("));
    Serial.print(msg);
    Serial.println(F(")"));
  }
}

void PrintStateBombaFSM(const char* prefijo, BombaMachineStates current, bool newline)
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

  if (newline)
    Serial.println();
}
