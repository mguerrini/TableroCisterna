
void CicladorLoop()
{
  Bomba* bomba = GetActiveBomba();
  PrintEnterStateWorkingFSM();

  automaticFSM.NextState = AUTO_NULL;

  switch (automaticFSM.State)
  {
    case AUTO_IDLE:
      AUTO_Idle(bomba);
      break;

    case AUTO_STARTING:
      AUTO_Starting(bomba);
      break;

    case AUTO_WORKING:
      AUTO_Working(bomba);
      break;

    case AUTO_STOPPING:
      AUTO_Stopping(bomba);
      break;

    case AUTO_CHANGE_BOMBA:
      AUTO_ChangeBomba(bomba);
      break;

    case AUTO_NOT_AVAILABLES_BOMBAS:
      AUTO_NotAvailablesBombas(bomba);
      break;

    case AUTO_CHANGE_BOMBA_FROM_NOT_AVAILABLE:
      AUTO_ChangeBombaFromNotAvailable(bomba);
      break;

    case AUTO_CHANGE_BOMBA_FROM_FILL_TIMEOUT:
      //no registro error....trato de switchear bomba

      //deberia poner la bomba en estado de timeout de llenado
      automaticFSM.NextState = AUTO_CHANGE_BOMBA;
#ifdef LOG_ENABLED
      PrintWorkingFSMMessage(F("Bomba Activa Fill Timeout"));
#endif
      break;

    case AUTO_ERROR_BOMBA_WORKING:
      AUTO_ErrorBombaWorking(bomba);
      break;

  }

  PrintExitStateWorkingFSM();

  automaticFSM.FromState = automaticFSM.State;
  if (automaticFSM.NextState != AUTO_NULL)
    automaticFSM.State = automaticFSM.NextState;

  //ejecuto la maquina de estados de las bombas
  BombaStateMachine(&bomba1);
  BombaStateMachine(&bomba2);
}

// *************************************************** //
//                  STATES
// *************************************************** //

// ============== IDLE ============== //
void AUTO_Idle(Bomba* bomba)
{
  //sino esta ok la fase....no arranca el sistema. Deberia volver a idle porque los motores se apagaron
  if (!automaticFSM.IsFaseOk)
    return;

  if (!IsBombaAvailable(bomba))
  {
    //Cambio de bomba.
    automaticFSM.NextState = AUTO_CHANGE_BOMBA_FROM_NOT_AVAILABLE;
#ifdef LOG_ENABLED
    PrintWorkingFSMMessage(F("Bomba Activa no disponible"));
#endif
    return;
  }

  if (IsBombaOn(bomba))
  {
    //actualizo los sensores
    ReadTanqueSensors();
    ReadCisternaSensors();

    if (!CanTurnOnBomba())
    {
      //hay un error....no se deberia dar esta situacion a no ser que sea un error en el circuito de lectura de los sensores
      //registro el error en el display..
#ifdef LOG_ENABLED
      PrintWorkingFSMMessage(F("** Error en la lectura de los sensores del tanque **"));
#endif
    }

    automaticFSM.NextState = AUTO_STARTING;
#ifdef LOG_ENABLED
    PrintWorkingFSMMessage(F("Bomba Encendida"));
#endif
    return;
  }

  //Esto pasa a encendido de la bomba porque puede que el contactor no se haya cerrado por un error
  if (CanTurnOnBomba())
  {
    //deberia encenderse la bomba
    automaticFSM.NextState = AUTO_STARTING;
#ifdef LOG_ENABLED
    PrintWorkingFSMMessage(F("Tanque Vacío"));
#endif
    return;
  }
}


// ============== STARTING ============== //

void AUTO_Starting(Bomba* bomba)
{
  //espero hasta que arranqe o que se ponga en error.
  bomba->RequestOn = false;

  if (IsBombaOn(bomba))
  {
    //listo...paso al estado working
    automaticFSM.NextState = AUTO_WORKING;
#ifdef LOG_ENABLED
    PrintWorkingFSMMessage(F("Bomba Encendida"));
#endif
    return;
  }

  if (!IsBombaAvailable(bomba))
  {
    automaticFSM.NextState = AUTO_IDLE;
#ifdef LOG_ENABLED
    PrintWorkingFSMMessage(F("Bomba Activa no disponible"));
#endif
    return;
  }

  //indico que se encienda
  if (IsFirstTimeInAutoState())
  {
    bomba->RequestOn = true;
#ifdef LOG_ENABLED
    PrintWorkingFSMMessage(F("First Time - Request On"));
#endif
    return;
  }

  //intento encenderla...hasta que tire error por timeout o termico o arranqe
  bomba->RequestOn = true;
#ifdef LOG_ENABLED
  PrintWorkingFSMMessage(F("Bomba Off - Request On"));
#endif
}

// ============== WORKING ============== //

void AUTO_Working(Bomba* bomba)
{
  if (!IsBombaAvailable(bomba))
  {
    //Cambio de bomba.
    automaticFSM.NextState = AUTO_CHANGE_BOMBA_FROM_NOT_AVAILABLE;
#ifdef LOG_ENABLED
    PrintWorkingFSMMessage(F("Bomba Activa no disponible"));
#endif
    return;
  }

  //verifico que exista una bomba funcionando....sino, vuelvo a selecting bomba.
  if (IsBombaOff(bomba))
  {
    //actualizo los sensores por si cambiaron
    ReadTanqueSensors();
    ReadCisternaSensors();

    if (!CanTurnOffBomba())
    {
      //no se deberia dar nunca este caso a no ser que se pase justo a automatico o el circuito de lectura de los niveles este roto.
#ifdef LOG_ENABLED
      Serial.println(F("** Error en la lectura de los sensores del tanque **"));
#endif
    }

    //no se deberia dar nunca este caso a no ser que se pase justo a automatico o el circuito de lectura de los niveles este roto.
    //deberia registrar el error en el visor
    automaticFSM.NextState = AUTO_STOPPING;
#ifdef LOG_ENABLED
    PrintWorkingFSMMessage(F("Bomba Apagada"));
#endif
    return;
  }

  //detecta tanque lleno y o cisterna vacia....pero la bomba sigue encendida....el sistema no puede apagar una bomba
  if (CanTurnOffBomba()) //sensores.IsTanqueSensorMaxVal || sensores.IsCisternaSensorMinVal || IsBombaOff(bomba))
  {
#ifdef LOG_ENABLED
    PrintWorkingFSMMessage(F("*** Sensores de nivel en falla ***"));

    if (sensores.IsTanqueSensorMaxVal)
    {
      if (sensores.IsCisternaSensorMinVal)
        PrintWorkingFSMMessage(F("Tanque: Lleno, Cisterna: Vacia"));
      else
        PrintWorkingFSMMessage(F("Tanque: Lleno, Cisterna: Normal"));
    }
    else
    {
      PrintWorkingFSMMessage(F("Cisterna: Vacia"));
    }
#endif
    return;
  }

  //verifico el tiempo de bombeo...si supera el maximo....cambio de bomba
  unsigned long workingMax = GetBombaWorkingTimeMaximum(bomba);
  unsigned long workingTime = GetBombaWorkingTime(bomba);

  if (workingTime > workingMax)
  {
    //cambio de bomba.....no esta funcionando
    automaticFSM.NextState = AUTO_CHANGE_BOMBA_FROM_FILL_TIMEOUT;
#ifdef LOG_ENABLED
    PrintWorkingFSMMessage(F("Bomba Activa Fill Timeout"));
#endif
  }

}

// ============== NOT AVAILABLES BOMBAS ============== //

void AUTO_NotAvailablesBombas(Bomba * bomba) {
  //disparo la alarma
  if (IsFirstTimeInAutoState())
  {
    StartAlarmNotAvailablesBombas();
#ifdef LOG_ENABLED
    PrintWorkingFSMMessage(F("Start Alarma"));
#endif
    return;
  }

  bomba = GetAvailableBomba();

  //no lanzo error porque ya estarian en error las bombas
  if (bomba != NULL)
  {
    //Pongo activa la bomba seleccionada
    bomba = SwapAndActiveBomba();
    if (bomba == NULL)
      return;

    automaticFSM.NextState = AUTO_IDLE;

#ifdef LOG_ENABLED
    if (bomba->Number == BOMBA1)
      PrintWorkingFSMMessage(F("Bomba 1 disponible-Stop Alarma"));
    else
      PrintWorkingFSMMessage(F("Bomba 2 disponible-Stop Alarma"));
#endif

    StopAlarmNotAvailablesBombas();
  }
}


// ============== STOPPING ============== //

void AUTO_Stopping(Bomba* bomba)
{
  //espero a que el contactor se abra
  bomba->RequestOff = false;

  if (IsBombaOff(bomba) || IsBombaError(bomba))
  {
    // BOMBA OFF => Tengo que actualizar los contadores
    bomba->Uses = bomba->Uses + 1;
    if (bomba->Uses >= BOMBA_USES_MAX)
    {
      automaticFSM.NextState = AUTO_CHANGE_BOMBA;
#ifdef LOG_ENABLED
      PrintWorkingFSMMessage(F("Usos máximo alcanzado->cambio de bomba"));
#endif
    }
    else
    {
      automaticFSM.NextState = AUTO_IDLE;
#ifdef LOG_ENABLED
      PrintWorkingFSMMessage(F("Usos máximo no alcanzado"));
#endif
    }

    return;
  }

  if (IsFirstTimeInAutoState())
  {
    automaticFSM.StoppingTimer = millis();
    bomba->RequestOff = true;
#ifdef LOG_ENABLED
    PrintWorkingFSMMessage(F("Request Off"));
#endif
    return;
  }

  if (IsBombaOn(bomba))
  {
    //espero un tiempo prudencial hasta que arranque la bomba
    unsigned long wait = deltaMillis(millis(), automaticFSM.StoppingTimer);
    //espero a que inicie....la bomba activa deberia estar en ON.
    if (wait > (2 * BOMBA_TURNING_OFF_TIME))
    {
      //ERROR....NO SE DETUVO LA BOMBA Y SIGUE FUNCIONANDO...NUNCA DEBERIA OCURRIR, PORQUE LOS SENSORES APAGAN LAS BOMBAS
      automaticFSM.NextState = AUTO_ERROR_BOMBA_WORKING;
#ifdef LOG_ENABLED
      PrintWorkingFSMMessage(F("Timeout Stopping - Bomba On"), wait, 2 * BOMBA_TURNING_OFF_TIME);
#endif
    }
    else
    {
      bomba->RequestOff = true;
#ifdef LOG_ENABLED
      PrintWorkingFSMMessage(F("Esperando contactor abierto..."), wait, 2 * BOMBA_TURNING_OFF_TIME);
#endif
    }
  }
}

// ============== CHANGE BOMBA FROM NOT AVAILABLE ============== //

void AUTO_ChangeBombaFromNotAvailable(Bomba* bomba)
{
  if (IsBombaError(bomba))
  {
    //Cambio de bomba.
    automaticFSM.NextState = AUTO_CHANGE_BOMBA;
#ifdef LOG_ENABLED
    PrintWorkingFSMMessage(F("Bomba Activa en Error"));
#endif
    return;
  }

  if (!IsBombaEnabled(bomba))
  {
    //esta deshabilitada
    automaticFSM.NextState = AUTO_CHANGE_BOMBA;
#ifdef LOG_ENABLED
    PrintWorkingFSMMessage(F("Bomba Activa Deshabilitada"));
#endif
    return;
  }

  automaticFSM.NextState = AUTO_IDLE;
#ifdef LOG_ENABLED
  PrintWorkingFSMMessage(F("Bomba Activa Normalizada"));
#endif
}


// ============== CHANGE BOMBA ============== //

void AUTO_ChangeBomba(Bomba* bomba)
{
  bomba = SwapAndActiveBomba();
  if (bomba == NULL)
  {
    automaticFSM.NextState = AUTO_NOT_AVAILABLES_BOMBAS;
#ifdef LOG_ENABLED
    PrintWorkingFSMMessage(F("Sin Bombas disponibles"));
#endif
    return;
  }

  automaticFSM.NextState = AUTO_IDLE;

#ifdef LOG_ENABLED
  if (bomba->Number == BOMBA1)
    PrintWorkingFSMMessage(F("B1 activa"));
  else
    PrintWorkingFSMMessage(F("B2 activa"));
#endif
}

// ============== ERROR BOMBA WORKING ============== //

void AUTO_ErrorBombaWorking(Bomba* bomba)
{
  //Si se apaga es por los niveles...los sensores quizas fallan
  if (IsBombaOff(bomba))
  {
    StopAlarmBomba(bomba);
    automaticFSM.NextState = AUTO_STOPPING;
#ifdef LOG_ENABLED
    PrintWorkingFSMMessage(F("Bomba Off"));
#endif
    return;
  }

  //paso a error
  if (!IsBombaAvailable(bomba))
  {
    StopAlarmBomba(bomba);
    automaticFSM.NextState = AUTO_STOPPING;
#ifdef LOG_ENABLED
    PrintWorkingFSMMessage(F("Bomba No Disponible"));
#endif
    return;
  }

  if (!CanTurnOffBomba())
  {
    //no deberia apagarse....quizas se paso a manual justo cuando midio el tanque lleno, vuelve a working
    StopAlarmBomba(bomba);
    automaticFSM.NextState = AUTO_WORKING;
#ifdef LOG_ENABLED
    PrintWorkingFSMMessage(F("Tanque Empty - Cisterna Normal - Bomba On"));
#endif
    return;
  }

  //a este punto no deberia llegar nunca....es porque esta prendida pero los niveles dicen que tiene que estar apagada.....
  //falla en la eletronica encargada de traducir los 24 volts de los tanques a la entrada del arduino
  //trato de recuperar el error antes de disparar la alarma
  if (IsFirstTimeInAutoState())
  {
    StartAlarmBombaNotStop(bomba);
    //TODO Resaltar que la bomba no se detiene
#ifdef LOG_ENABLED
    PrintWorkingFSMMessage(F("Start Alarm - Error Bomba ON"));
#endif
    return;
  }

}

// *************************************************** //
//                FUNCIONES AUXLIARES
// *************************************************** //

bool IsFirstTimeInAutoState()
{
  return automaticFSM.FromState != automaticFSM.State;
}


bool CanTurnOnBomba()
{
  if (IsAutomaticMode())
    return sensores.IsTanqueSensorMinVal && !sensores.IsCisternaSensorMinVal;
  else
    return !sensores.IsCisternaSensorMinVal;
}


bool CanTurnOffBomba()
{
  if (IsAutomaticMode())
    return sensores.IsTanqueSensorMaxVal || sensores.IsCisternaSensorMinVal;
  else
    return sensores.IsCisternaSensorMinVal;
}


Bomba* GetAvailableBomba()
{
  if (!IsBombaAvailable(&bomba1) && !IsBombaAvailable(&bomba2))
    return NULL;

  if (IsBombaAvailable(&bomba1) && IsBombaAvailable(&bomba2))
  {
    if (bomba1.IsActive)
      return &bomba1;
    else
      return &bomba2;
  }

  if (IsBombaAvailable(&bomba1))
    return &bomba1;

  if (IsBombaAvailable(&bomba2))
    return &bomba2;

  return NULL;
}

Bomba* GetActiveBomba()
{
  if (bomba1.IsActive)
    return &bomba1;

  if (bomba2.IsActive)
    return &bomba2;

  return NULL;
}


// *************************************************** //
//                       PRINT
// *************************************************** //

void PrintEnterStateWorkingFSM()
{
#ifdef LOG_ENABLED
  Serial.print(F("Process: "));
  PrintStateWorkingFSM(automaticFSM.State);
#endif
}


void PrintExitStateWorkingFSM()
{
#ifdef LOG_ENABLED
  byte state = automaticFSM.State;

  if (automaticFSM.NextState != AUTO_NULL)
    state = automaticFSM.NextState;

  Serial.print(F(" -> "));
  PrintStateWorkingFSM(state);
  Serial.println();
#endif
}

void PrintWorkingFSMMessage(const __FlashStringHelper* msg)
{
#ifdef LOG_ENABLED
  Serial.print(F(" ("));
  Serial.print(msg);
  Serial.print(F(")"));
#endif

}

void PrintWorkingFSMMessage(const __FlashStringHelper* msg, long wait, long maximo)
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

void PrintStateWorkingFSM(byte current)
{
  switch (current)
  {
    case AUTO_IDLE:
      Serial.print(F("AUTO_IDLE"));
      break;
    case AUTO_NOT_AVAILABLES_BOMBAS:
      Serial.print(F("AUTO_NOT_AVAILABLES_BOMBAS"));
      break;
    case AUTO_STARTING:
      Serial.print(F("AUTO_STARTING"));
      break;
    case AUTO_WORKING:
      Serial.print(F("AUTO_WORKING"));
      break;
    case AUTO_STOPPING:
      Serial.print(F("AUTO_STOPPING"));
      break;
    case AUTO_CHANGE_BOMBA_FROM_NOT_AVAILABLE:
      Serial.print(F("AUTO_CHANGE_BOMBA_FROM_NOT_AVAILABLE"));
      break;
    case AUTO_CHANGE_BOMBA:
      Serial.print(F("AUTO_CHANGE_BOMBA"));
      break;
    case AUTO_ERROR_BOMBA_WORKING:
      Serial.print(F("AUTO_ERROR_BOMBA_WORKING"));
      break;
    case AUTO_CHANGE_BOMBA_FROM_FILL_TIMEOUT:
      Serial.print(F("AUTO_CHANGE_BOMBA_FROM_FILL_TIMEOUT"));
      break;
  }
}
