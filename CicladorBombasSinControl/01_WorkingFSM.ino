
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
      PrintWorkingFSMMessage(F("Bomba Activa Fill Timeout"));
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
    PrintWorkingFSMMessage(F("Bomba Activa no disponible"));
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
      PrintWorkingFSMMessage(F("** Error en la lectura de los sensores del tanque **"));
    }

    automaticFSM.NextState = AUTO_STARTING;
    PrintWorkingFSMMessage(F("Bomba Encendida"));
    return;
  }

  //Esto pasa a encendido de la bomba porque puede que el contactor no se haya cerrado por un error
  if (CanTurnOnBomba())
  {
    //deberia encenderse la bomba
    automaticFSM.NextState = AUTO_STARTING;
    PrintWorkingFSMMessage(F("Tanque Vacío"));
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
    PrintWorkingFSMMessage(F("Bomba Encendida"));
    return;
  }

  if (!IsBombaAvailable(bomba))
  {
    automaticFSM.NextState = AUTO_IDLE;
    PrintWorkingFSMMessage(F("Bomba Activa no disponible"));
    return;
  }

  //indico que se encienda
  if (IsFirstTimeInAutoState())
  {
    bomba->RequestOn = true;
    PrintWorkingFSMMessage(F("First Time - Request On"));
    return;
  }

  //intento encenderla...hasta que tire error por timeout o termico o arranqe
  bomba->RequestOn = true;
  PrintWorkingFSMMessage(F("Bomba Off - Request On"));
}

// ============== WORKING ============== //

void AUTO_Working(Bomba* bomba)
{
  if (!IsBombaAvailable(bomba))
  {
    //Cambio de bomba.
    automaticFSM.NextState = AUTO_CHANGE_BOMBA_FROM_NOT_AVAILABLE;
    PrintWorkingFSMMessage(F("Bomba Activa no disponible"));
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
      Serial.println(F("** Error en la lectura de los sensores del tanque **"));
    }

    //no se deberia dar nunca este caso a no ser que se pase justo a automatico o el circuito de lectura de los niveles este roto.
    //deberia registrar el error en el visor
    automaticFSM.NextState = AUTO_STOPPING;
    PrintWorkingFSMMessage(F("Bomba Apagada"));
    return;
  }

  //detecta tanque lleno y o cisterna vacia....pero la bomba sigue encendida....el sistema no puede apagar una bomba
  if (CanTurnOffBomba()) //sensores.IsTanqueSensorMaxVal || sensores.IsCisternaSensorMinVal || IsBombaOff(bomba))
  {
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
    return;
  }

  //verifico el tiempo de bombeo...si supera el maximo....cambio de bomba
  unsigned long workingMax = GetBombaWorkingTimeMaximum(bomba);
  unsigned long workingTime = GetBombaWorkingTime(bomba);

  if (workingTime > workingMax)
  {
    //cambio de bomba.....no esta funcionando
    automaticFSM.NextState = AUTO_CHANGE_BOMBA_FROM_FILL_TIMEOUT;
    PrintWorkingFSMMessage(F("Bomba Activa Fill Timeout"));
  }

}

// ============== NOT AVAILABLES BOMBAS ============== //

void AUTO_NotAvailablesBombas(Bomba * bomba) {
  //disparo la alarma
  if (IsFirstTimeInAutoState())
  {
    StartAlarmNotAvailablesBombas();
    PrintWorkingFSMMessage(F("Start Alarma"));
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
    if (bomba->Number == BOMBA1)
      PrintWorkingFSMMessage(F("Bomba 1 disponible-Stop Alarma"));
    else
      PrintWorkingFSMMessage(F("Bomba 2 disponible-Stop Alarma"));

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
      PrintWorkingFSMMessage(F("Usos máximo alcanzado->cambio de bomba"));
    }
    else
    {
      automaticFSM.NextState = AUTO_IDLE;
      PrintWorkingFSMMessage(F("Usos máximo no alcanzado"));
    }

    return;
  }

  if (IsFirstTimeInAutoState())
  {
    automaticFSM.StoppingTimer = millis();
    bomba->RequestOff = true;
    PrintWorkingFSMMessage(F("Request Off"));
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
      PrintWorkingFSMMessage(F("Timeout Stopping - Bomba On"), wait, 2 * BOMBA_TURNING_OFF_TIME);
    }
    else
    {
      bomba->RequestOff = true;
      PrintWorkingFSMMessage(F("Esperando contactor abierto..."), wait, 2 * BOMBA_TURNING_OFF_TIME);
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
    PrintWorkingFSMMessage(F("Bomba Activa en Error"));
    return;
  }

  if (!IsBombaEnabled(bomba))
  {
    //esta deshabilitada
    automaticFSM.NextState = AUTO_CHANGE_BOMBA;
    PrintWorkingFSMMessage(F("Bomba Activa Deshabilitada"));
    return;
  }

  automaticFSM.NextState = AUTO_IDLE;
  PrintWorkingFSMMessage(F("Bomba Activa Normalizada"));
}


// ============== CHANGE BOMBA ============== //

void AUTO_ChangeBomba(Bomba* bomba)
{
  bomba = SwapAndActiveBomba();
  if (bomba == NULL)
  {
    automaticFSM.NextState = AUTO_NOT_AVAILABLES_BOMBAS;
    PrintWorkingFSMMessage(F("Sin Bombas disponibles"));
    return;
  }

  automaticFSM.NextState = AUTO_IDLE;

  if (bomba->Number == BOMBA1)
    PrintWorkingFSMMessage(F("Bomba 1 activa"));
  else
    PrintWorkingFSMMessage(F("Bomba 2 activa"));

}

// ============== ERROR BOMBA WORKING ============== //

void AUTO_ErrorBombaWorking(Bomba* bomba)
{
  //Si se apaga es por los niveles...los sensores quizas fallan
  if (IsBombaOff(bomba))
  {
    StopAlarmBomba(bomba);
    automaticFSM.NextState = AUTO_STOPPING;
    PrintWorkingFSMMessage(F("Bomba Off"));
    return;
  }

  //paso a error
  if (!IsBombaAvailable(bomba))
  {
    StopAlarmBomba(bomba);
    automaticFSM.NextState = AUTO_STOPPING;
    PrintWorkingFSMMessage(F("Bomba No Disponible"));
    return;
  }

  if (!CanTurnOffBomba())
  {
    //no deberia apagarse....quizas se paso a manual justo cuando midio el tanque lleno, vuelve a working
    StopAlarmBomba(bomba);
    automaticFSM.NextState = AUTO_WORKING;
    PrintWorkingFSMMessage(F("Tanque Empty - Cisterna Normal - Bomba On"));
    return;
  }

  //a este punto no deberia llegar nunca....es porque esta prendida pero los niveles dicen que tiene que estar apagada.....
  //falla en la eletronica encargada de traducir los 24 volts de los tanques a la entrada del arduino
  //trato de recuperar el error antes de disparar la alarma
  if (IsFirstTimeInAutoState())
  {
    StartAlarmBombaNotStop(bomba);
    //TODO Resaltar que la bomba no se detiene
    PrintWorkingFSMMessage(F("Start Alarm - Error Bomba ON"));
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
#ifdef TEST
  Serial.print(F("Process: "));
  PrintStateWorkingFSM(automaticFSM.State);
#endif
}


void PrintExitStateWorkingFSM()
{
#ifdef TEST
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
#ifdef TEST
  Serial.print(F(" ("));
  Serial.print(msg);
  Serial.print(F(")"));
#endif

}

void PrintWorkingFSMMessage(const __FlashStringHelper* msg, long wait, long maximo)
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
