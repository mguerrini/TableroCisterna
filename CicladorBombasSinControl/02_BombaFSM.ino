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
        PrintExitStateBombaFSM(bomba, F("Bomba OFF"));
        break;
      }

      if (bomba->RequestDisabled)
      {
        bomba->NextMachineState = FSM_BOMBA_DISABLING;
        PrintExitStateBombaFSM(bomba, F("Request Disabled"));
        break;
      }

      if (bomba->RequestOn)
      {
        //bomba->RequestOn = false;
        bomba->Timer = millis();
        bomba->NextMachineState = FSM_BOMBA_TURNING_ON;
        PrintExitStateBombaFSM(bomba, F("Request On"));
        break;
      }

      if (bomba->IsContactorClosed)
      {
        bomba->NextMachineState = FSM_BOMBA_TURNING_ON;
        PrintExitStateBombaFSM(bomba, F("Contactor Cerrado"));
        break;
      }

      if (!bomba->IsTermicoOk)
      {
        bomba->NextMachineState = FSM_BOMBA_ERROR_TERMICO;
        PrintExitStateBombaFSM(bomba, F("Termico Abierto"));
        break;
      }

      PrintExitStateBombaFSM(bomba, NULL);
      break;

    case FSM_BOMBA_ON:

      if (IsFirstTimeInState(bomba))
      {
        bomba->State = BOMBA_STATE_ON;
        UpdateBombaDisplay(bomba);
        PrintExitStateBombaFSM(bomba, F("Bomba ON"));
        break;
      }

      if (bomba->RequestDisabled)
      {
        bomba->NextMachineState = FSM_BOMBA_DISABLING;
        PrintExitStateBombaFSM(bomba, F("Request: Disabled"));
        break;
      }

      if (bomba->RequestOff)
      {
        bomba->NextMachineState = FSM_BOMBA_TURNING_OFF;
        bomba->Timer = millis();
        PrintExitStateBombaFSM(bomba, F("Request: Off"));
        break;
      }

      if (!bomba->IsContactorClosed)
      {
        //se apago...
        bomba->NextMachineState = FSM_BOMBA_TURNING_OFF;
        bomba->Timer = millis();
        PrintExitStateBombaFSM(bomba, F("Contactor Abierto"));
        break;
      }

      if (!bomba->IsTermicoOk)
      {
        bomba->NextMachineState = FSM_BOMBA_ERROR_TERMICO;
        PrintExitStateBombaFSM(bomba, F("Termico abierto"));
        break;
      }

      PrintExitStateBombaFSM(bomba);
      break;

    case FSM_BOMBA_TURNING_ON:

      //espero a que el contactor este cerrado y que el termico este OK
      if (!bomba->IsTermicoOk)
      {
        bomba->NextMachineState = FSM_BOMBA_ERROR_TERMICO;
        PrintExitStateBombaFSM(bomba, F("Termico abierto"));
        break;
      }

      if (bomba->IsContactorClosed)
      {
        bomba->NextMachineState = FSM_BOMBA_ON;
        PrintExitStateBombaFSM(bomba, F("Contactor Cerrado"));
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
          PrintExitStateBombaFSM(bomba, F("Esperando Contactor Cerrado...Timeout"), wait);
        }
        else
          PrintExitStateBombaFSM(bomba, F("Esperando Contactor Cerrado..."), wait);
      }
      break;

    case FSM_BOMBA_TURNING_OFF:
      if (!bomba->IsTermicoOk)
      {
        bomba->NextMachineState = FSM_BOMBA_ERROR_TERMICO;
        PrintExitStateBombaFSM(bomba, F("Termico Abierto"));
        break;
      }

      if (!bomba->IsContactorClosed)
      {
        bomba->NextMachineState = FSM_BOMBA_OFF;
        PrintExitStateBombaFSM(bomba, F("Contactor Abierto"));
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
        PrintExitStateBombaFSM(bomba, F("Contactor Cerrado-Bomba continua encendida"), wait);
        break;
      }
      else
        PrintExitStateBombaFSM(bomba, F("Esperando contactor"), wait);
      break;

    case FSM_BOMBA_DISABLING:

      StopAlarm();
      bomba->NextMachineState = FSM_BOMBA_DISABLED;

      PrintExitStateBombaFSM(bomba, F("Disabling-Stop Alarm"));
      break;

    case FSM_BOMBA_ENABLING:
      StopAlarm();
      bomba->IsEnabled = true;

      if (!bomba->IsTermicoOk)
      {
        bomba->NextMachineState = FSM_BOMBA_ERROR_TERMICO;
        PrintExitStateBombaFSM(bomba, F("Enabled-StopAlarm-Termico Abierto"));
        break;
      }

      if (bomba->IsContactorClosed)
        bomba->NextMachineState = FSM_BOMBA_TURNING_ON;
      else
        bomba->NextMachineState = FSM_BOMBA_TURNING_OFF;

      PrintExitStateBombaFSM(bomba, F("Enabled-Stop Alarm"));
      break;

    case FSM_BOMBA_DISABLED:
      if (IsFirstTimeInState(bomba))
      {
        bomba->IsEnabled = false;
        UpdateBombaDisplay(bomba);
        PrintExitStateBombaFSM(bomba, F("Bomba Deshabilitada"));
        break;
      }

      if (bomba->RequestEnabled)
      {
        bomba->NextMachineState = FSM_BOMBA_ENABLING;
        PrintExitStateBombaFSM(bomba, F("Request Enabled"));
        break;
      }

      PrintExitStateBombaFSM(bomba);
      break;

    case FSM_BOMBA_ERROR_CONTACTOR_ABIERTO:
      if (IsFirstTimeInState(bomba))
      {
        bomba->State = BOMBA_STATE_ERROR_CONTACTOR_ABIERTO;
        UpdateBombaDisplay(bomba);
        PrintExitStateBombaFSM(bomba, F("Contactor abierto"));
        break;
      }

      if (bomba->RequestEnabled)
      {
        bomba->NextMachineState = FSM_BOMBA_ENABLING;
        PrintExitStateBombaFSM(bomba, F("Request Enabled"));
        break;
      }

      if (bomba->RequestDisabled)
      {
        bomba->NextMachineState = FSM_BOMBA_DISABLING;
        PrintExitStateBombaFSM(bomba, F("Request Disabled"));
        break;
      }

      if (bomba->IsContactorClosed)
      {
        bomba->NextMachineState = FSM_BOMBA_TURNING_ON;
        PrintExitStateBombaFSM(bomba, F("Contactor Cerrado"));
      }

      PrintExitStateBombaFSM(bomba);
      break;

    case FSM_BOMBA_ERROR_TERMICO:
      //por si las dos estan en este error y una se recupera, la otra tiene que seguir gritando
      StartAlarm();

      if (IsFirstTimeInState(bomba))
      {
        bomba->State = BOMBA_STATE_ERROR_TERMICO;
        UpdateBombaDisplay(bomba);
        PrintExitStateBombaFSM(bomba, F("Error Termico-Start Alarm"));
        break;
      }

      if (bomba->RequestDisabled)
      {
        bomba->NextMachineState = FSM_BOMBA_DISABLING;
        PrintExitStateBombaFSM(bomba, F("Request Disabled"));
        break;
      }

      //verifico si el termico se normalizo.
      if (bomba->IsTermicoOk)
      {
        StopAlarm();
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

        PrintExitStateBombaFSM(bomba, F("Termico OK-StopAlarm"));
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


void PrintExitStateBombaFSM(Bomba* bomba, const __FlashStringHelper* msg)
{
  PrintExitStateBombaFSM(bomba, msg, -1);
}

void PrintExitStateBombaFSM(Bomba* bomba, const __FlashStringHelper* msg, long wait)
{
  byte state = bomba->MachineState;
  if (bomba->NextMachineState != FSM_BOMBA_NULL)
    state = bomba->NextMachineState;

  if (msg == NULL)
    PrintStateBombaFSM(F(" -> "), state, true);
  else
  {
    PrintStateBombaFSM(F(" -> "), state, false);
    Serial.print(F(" ("));
    Serial.print(msg);
    if (wait > 0)
    {
      Serial.print(F(" wait: "));
      Serial.print(wait);
    }
    Serial.println(F(")"));
  }
}

void PrintStateBombaFSM(const __FlashStringHelper* prefijo, byte current, bool newline)
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
