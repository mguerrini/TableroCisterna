void BombaStateMachine(Bomba* bomba)
{
  long wait = 0;
  bomba->NextMachineState = FSM_BOMBA_NULL;
  PrintEnterStateBombaFSM(bomba);

  switch (bomba->MachineState)
  {
    case FSM_BOMBA_OFF:
      //limpio cualquier pedido de apagado
      bomba->RequestOff = false;

      if (IsFirstTimeInState(bomba))
      {
        bomba->State = BOMBA_STATE_OFF;
        UpdateBombaDisplay(bomba);
        PrintBombaMessage(F("Bomba OFF"));
        break;
      }

      if (!bomba->IsTermicoOk)
      {
        bomba->NextMachineState = FSM_BOMBA_ERROR_TERMICO;
        PrintBombaMessage(F("Termico Abierto"));
        break;
      }

      if (bomba->RequestDisabled)
      {
        bomba->NextMachineState = FSM_BOMBA_DISABLING;
        PrintBombaMessage(F("Request Disabled"));
        break;
      }

      if (bomba->RequestOn)
      {
        bomba->NextMachineState = FSM_BOMBA_TURNING_ON;
        PrintBombaMessage(F("Request On"));
        break;
      }

      if (bomba->IsContactorClosed)
      {
        bomba->NextMachineState = FSM_BOMBA_TURNING_ON;
        PrintBombaMessage(F("Contactor Cerrado"));
        break;
      }

      break;

    case FSM_BOMBA_TURNING_ON:
      bomba->RequestOn = false;

      if (bomba->RequestOff)
      {
        bomba->RequestOff = false;
        bomba->NextMachineState = FSM_BOMBA_TURNING_OFF;
        PrintBombaMessage(F("Request Off"));
        break;
      }

      //espero a que el contactor este cerrado y que el termico este OK
      if (!bomba->IsTermicoOk)
      {
        bomba->NextMachineState = FSM_BOMBA_ERROR_TERMICO;
        PrintBombaMessage(F("Termico abierto"));
        break;
      }

      if (bomba->IsContactorClosed)
      {
        bomba->NextMachineState = FSM_BOMBA_ON;
        PrintBombaMessage(F("Contactor Cerrado"));
        break;
      }

      if (IsFirstTimeInState(bomba))
      {
        bomba->Timer = millis();
        PrintBombaMessage(F("Iniciando timer"));
        break;
      }

      //espero un tiempo prudencial hasta que arranque la bomba
      wait = millis() - bomba->Timer;
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

      break;


    case FSM_BOMBA_ON:
      //limpio cualquier pedido de encendido
      bomba->RequestOn = false;

      if (IsFirstTimeInState(bomba))
      {
        bomba->State = BOMBA_STATE_ON;
        UpdateBombaDisplay(bomba);
        PrintBombaMessage(F("Bomba ON"));
        break;
      }

      if (!bomba->IsTermicoOk)
      {
        bomba->NextMachineState = FSM_BOMBA_ERROR_TERMICO;
        PrintBombaMessage(F("Termico abierto"));
        break;
      }

      if (bomba->RequestDisabled)
      {
        bomba->NextMachineState = FSM_BOMBA_DISABLING;
        PrintBombaMessage(F("Request: Disabled"));
        break;
      }

      if (bomba->RequestOff)
      {
        bomba->NextMachineState = FSM_BOMBA_TURNING_OFF;
        PrintBombaMessage(F("Request: Off"));
        break;
      }

      if (!bomba->IsContactorClosed)
      {
        //se apago...
        bomba->NextMachineState = FSM_BOMBA_TURNING_OFF;
        PrintBombaMessage(F("Contactor Abierto"));
        break;
      }

      break;


    case FSM_BOMBA_TURNING_OFF:
      bomba->RequestOff = false;

      //la bomba esta en error
      if (!bomba->IsTermicoOk)
      {
        bomba->NextMachineState = FSM_BOMBA_ERROR_TERMICO;
        PrintBombaMessage(F("Termico Abierto"));
        break;
      }

      //ya esta apagada
      if (!bomba->IsContactorClosed)
      {
        bomba->NextMachineState = FSM_BOMBA_OFF;
        PrintBombaMessage(F("Contactor Abierto"));
        break;
      }

      if (IsFirstTimeInState(bomba))
      {
        bomba->Timer = millis();
        PrintBombaMessage(F("Iniciando timer"));
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
        bomba->NextMachineState = FSM_BOMBA_TURNING_ON; // FSM_BOMBA_ERROR_CONTACTOR_CERRADO;
        PrintBombaMessage(F("Contactor Cerrado-Bomba continua encendida"), wait, BOMBA_TURNING_OFF_TIME);
      }
      else
        PrintBombaMessage(F("Esperando contactor"), wait, BOMBA_TURNING_OFF_TIME);

      break;


    case FSM_BOMBA_DISABLING:
      bomba->NextMachineState = FSM_BOMBA_DISABLED;
      PrintBombaMessage(F("Disabling-Stop Alarm"));
      break;

    case FSM_BOMBA_DISABLED:
      if (IsFirstTimeInState(bomba))
      {
        StopBombaAlarm();
        bomba->IsEnabled = false;
        UpdateBombaDisplay(bomba);
        PrintBombaMessage(F("Bomba Deshabilitada"));
        break;
      }

      if (bomba->RequestEnabled)
      {
        bomba->NextMachineState = FSM_BOMBA_ENABLING;
        PrintBombaMessage(F("Request Enabled"));
        break;
      }

      break;

    case FSM_BOMBA_ENABLING:
      StopBombaAlarm();
      bomba->IsEnabled = true;

      if (!bomba->IsTermicoOk)
      {
        bomba->NextMachineState = FSM_BOMBA_ERROR_TERMICO;
        PrintBombaMessage(F("Enabled-StopAlarm-Termico Abierto"));
        break;
      }

      if (bomba->IsContactorClosed)
        bomba->NextMachineState = FSM_BOMBA_TURNING_ON;
      else
        bomba->NextMachineState = FSM_BOMBA_TURNING_OFF;

      PrintBombaMessage(F("Enabled-Stop Alarm"));
      break;

/*
    case FSM_BOMBA_ERROR_CONTACTOR_CERRADO:
      //se intento apagar pero el contactor sigue cerrado....o los sensores estan mal o puede seguir funcionando

      //se arreglo....salgo
      if (!bomba->IsContactorClosed)
      {
        StopBombaAlarm();
        bomba->NextMachineState = FSM_BOMBA_TURNING_OFF;
        PrintBombaMessage(F("Contactor Abierto"));
        break;
      }

      //pedido de encendido...lo atiendo
      if (bomba->RequestOn)
      {
        StopBombaAlarm();
        bomba->NextMachineState = FSM_BOMBA_TURNING_ON;
        PrintBombaMessage(F("Request On"));
        break;
      }

      //si esta el pedido de deshabilitar...salgo
      if (bomba->RequestDisabled)
      {
        StopBombaAlarm();
        bomba->NextMachineState = FSM_BOMBA_DISABLING;
        PrintBombaMessage(F("Request Disabled"));
        break;
      }

      if (IsFirstTimeInState(bomba))
      {
        StartAlarmBombaContactorCerrado();
        bomba->State = BOMBA_STATE_ERROR_CONTACTOR_CERRADO;
        UpdateBombaDisplay(bomba);
        PrintBombaMessage(F("Contactor abierto"));
        break;
      }

      break;
*/
    case FSM_BOMBA_ERROR_CONTACTOR_ABIERTO:
      //deberia cerrarse para prenderse.

      //lo valido primero por si fue un falso contacto
      if (bomba->RequestOff)
      {
        bomba->NextMachineState = FSM_BOMBA_TURNING_OFF;
        PrintBombaMessage(F("Request Disabled"));
        break;
      }

      if (bomba->RequestDisabled)
      {
        bomba->NextMachineState = FSM_BOMBA_DISABLING;
        PrintBombaMessage(F("Request Disabled"));
        break;
      }

      if (IsFirstTimeInState(bomba))
      {
        StartAlarmBombaContactorAbierto();
        bomba->State = BOMBA_STATE_ERROR_CONTACTOR_ABIERTO;
        UpdateBombaDisplay(bomba);
        PrintBombaMessage(F("Contactor abierto"));
        break;
      }

      break;

    case FSM_BOMBA_ERROR_TERMICO:
      //por si las dos estan en este error y una se recupera, la otra tiene que seguir gritando
      //verifico si el termico se normalizo.
      if (bomba->IsTermicoOk)
      {
        StopBombaAlarm();
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
        break;
      }

      StartAlarmBombaTermico();

      if (IsFirstTimeInState(bomba))
      {
        bomba->State = BOMBA_STATE_ERROR_TERMICO;
        UpdateBombaDisplay(bomba);
        PrintBombaMessage(F("Error Termico-Start Alarm"));
        break;
      }

      if (bomba->RequestDisabled)
      {
        bomba->NextMachineState = FSM_BOMBA_DISABLING;
        PrintBombaMessage(F("Request Disabled"));
        break;
      }

      break;
  }

  PrintExitStateBombaFSM(bomba);

  bomba->FromMachineState = bomba->MachineState;
  if (bomba->NextMachineState != FSM_BOMBA_NULL)
    bomba->MachineState = bomba->NextMachineState;
}

bool IsFirstTimeInState(Bomba* bomba)
{
  return bomba->FromMachineState != bomba->MachineState;
}

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
