
void CicladorLoop()
{
  Bomba* bomba = GetActiveBomba();
  PrintEnterStateWorkingFSM();

  switch (automaticFSM.State)
  {
    case AUTO_IDLE:
      //verifico los niveles

      if (!IsBombaAvailable(bomba))
      {
        //Cambio de bomba.
        automaticFSM.State = AUTO_CHANGE_BOMBA_FROM_ERROR;
        PrintExitStateWorkingFSM("Bomba Activa No Disponible");
        break;
      }

      if (CanTurnOnBomba()) //sensores.IsTanqueSensorMinVal && !sensores.IsCisternaSensorMinVal)
      {
        //deberia encenderse la bomba
        bomba->RequestOn = true;
        automaticFSM.State = AUTO_STARTING;
        automaticFSM.Timer = millis();

        PrintExitStateWorkingFSM("Tanque Vacío");
        break;
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
          Serial.println(F("***** Error sensores del Tanque y Cisterna: No deberia estar encendida la bomba *****"));
        }

        bomba->RequestOn = true;
        automaticFSM.State = AUTO_STARTING;
        automaticFSM.Timer = millis();
        PrintExitStateWorkingFSM("Bomba Encendida");

        break;
      }

      PrintExitStateWorkingFSM();
      break;


    case AUTO_WORKING:
      if (!IsBombaAvailable(bomba))
      {
        //cambio de bomba...se detuvo
        automaticFSM.State = AUTO_CHANGE_BOMBA_FROM_ERROR;
        PrintExitStateWorkingFSM("Bomba Activa No Disponible");
        break;
      }

      //detecta tanque lleno y o cisterna vacia...espera a que se detenga la bomba
      if (CanTurnOffBomba()) //sensores.IsTanqueSensorMaxVal || sensores.IsCisternaSensorMinVal || IsBombaOff(bomba))
      {
        //deberia encenderse la bomba
        automaticFSM.State = AUTO_STOPPING;
        automaticFSM.Timer = millis();

        if (sensores.IsTanqueSensorMaxVal)
        {
          if (sensores.IsCisternaSensorMinVal)
            PrintExitStateWorkingFSM("Tanque: Lleno, Cisterna: Vacia");
          else
            PrintExitStateWorkingFSM("Tanque: Lleno, Cisterna: Normal");
        }
        else
        {
          PrintExitStateWorkingFSM("Cisterna: Vacial");
        }
        break;
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
          Serial.println(F("***** Error sensores del Tanque y Cisterna: Deberian estar la bomba apagada y los sensores abiertos  *****"));
        }

        //no se deberia dar nunca este caso a no ser que se pase justo a automatico o el circuito de lectura de los niveles este roto.
        //deberia registrar el error en el visor
        automaticFSM.State = AUTO_STOPPING;
        automaticFSM.Timer = millis();
        PrintExitStateWorkingFSM("Bomba Apagada");
        break;
      }

      PrintExitStateWorkingFSM();
      break;

    case AUTO_STARTING:
      //espero hasta que arranqe o que se ponga en error
      if (IsBombaOn(bomba))
      {
        //listo...paso al estado working
        automaticFSM.State = AUTO_WORKING;
        PrintExitStateWorkingFSM("Bomba Encendida");
        break;
      }

      if (!IsBombaAvailable(bomba))
      {
        automaticFSM.State = AUTO_CHANGE_BOMBA_FROM_ERROR;
        PrintExitStateWorkingFSM("Bomba enciendo no disponible.");
        break;
      }

      PrintExitStateWorkingFSM();
      break;

    case AUTO_STOPPING:
      //espero a que el contactor se abra
      if (IsBombaOn(bomba))
      {
        //espero un tiempo prudencial hasta que arranque la bomba
        long wait = millis() - automaticFSM.Timer;
        //espero a que inicie....la bomba activa deberia estar en ON.
        if (wait > (2 * BOMBA_TURNING_OFF_TIME))
        {
          //ERROR....NO SE DETUVO LA BOMBA Y SIGUE FUNCIONANDO...NUNCA DEBERIA OCURRIR, PORQUE LOS SENSORES APAGAN LAS BOMBAS

          //vuelvo al estado working
          automaticFSM.State = AUTO_WORKING;
          PrintExitStateWorkingFSM("Timeout Stopping - Bomba On");
          break;
        }

        PrintExitStateWorkingFSM();
        break;
      }

      // BOMBA OFF => Tengo que actualizar los contadores
      bomba->Uses = bomba->Uses + 1;
      if (bomba->Uses > BOMBA_USES_MAX)
      {
        automaticFSM.State = AUTO_CHANGE_BOMBA;
        PrintExitStateWorkingFSM("Usos máximo alcanzado->cambio de bomba");
      }
      else
      {
        automaticFSM.State = AUTO_IDLE;
        PrintExitStateWorkingFSM();
      }
      break;


    case AUTO_NOT_AVAILABLES_BOMBAS:
      bomba = GetAvailableBomba();
      //no lanzo error porque ya estarian en error las bombas
      if (bomba != NULL)
      {
        //Pongo activa la bomba seleccionada
        bomba = SwapAndActiveBomba();
        automaticFSM.State = AUTO_IDLE;
        PrintExitStateWorkingFSM("Bomba cambiada");
        break;
      }

      PrintExitStateWorkingFSM();
      break;


    case AUTO_CHANGE_BOMBA_FROM_ERROR:
      automaticFSM.State = AUTO_CHANGE_BOMBA;
      PrintExitStateWorkingFSM("Error en la bomba activa");
      break;

    case AUTO_CHANGE_BOMBA_FROM_TIMEOUT:
      //los sensores de los niveles indican que se debe encender la bomba y nunca se cerraron los contactores
      //bomba->RequestErrorContactorAbierto = true;
      automaticFSM.State = AUTO_CHANGE_BOMBA;
      PrintExitStateWorkingFSM();
      break;

    case AUTO_CHANGE_BOMBA:
      bomba = SwapAndActiveBomba();
      if (bomba == NULL)
        automaticFSM.State = AUTO_NOT_AVAILABLES_BOMBAS;
      else
        automaticFSM.State = AUTO_IDLE;

      PrintExitStateWorkingFSM();
      break;
  }

  //ejecuto la maquina de estados de las bombas
  BombaStateMachine(&bomba1);
  BombaStateMachine(&bomba2);
}


void PrintEnterStateWorkingFSM()
{
  PrintStateWorkingFSM("Process: ", automaticFSM.State, false);
}

void PrintExitStateWorkingFSM()
{
  PrintExitStateWorkingFSM(NULL);
}

void PrintExitStateWorkingFSM(const char* msg)
{
  if (msg == NULL)
    PrintStateWorkingFSM(" -> ", automaticFSM.State, true);
  else
  {
    PrintStateWorkingFSM(" -> ", automaticFSM.State, false);
    Serial.print(F(" ("));
    Serial.print(msg);
    Serial.println(F(")"));
  }
}

void PrintStateWorkingFSM(const char* prefijo, AutomaticModeStates current, bool newline)
{
  if (prefijo != NULL)
    Serial.print(prefijo);

  switch (current)
  {
    case AUTO_IDLE:
      Serial.print(F("AUTO_IDLE"));
      break;
    case AUTO_SELECTING_BOMBA:
      Serial.print(F("AUTO_SELECTING_BOMBA"));
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
    case AUTO_TANQUE_FULL:
      Serial.print(F("AUTO_TANQUE_FULL"));
      break;
    case AUTO_STOPPING_BOMBA:
      Serial.print(F("AUTO_STOPPING_BOMBA"));
      break;
    case AUTO_CHANGE_BOMBA_FROM_ERROR:
      Serial.print(F("AUTO_CHANGE_BOMBA_FROM_ERROR"));
      break;
    case AUTO_CHANGE_BOMBA_FROM_TIMEOUT:
      Serial.print(F("AUTO_CHANGE_BOMBA_FROM_TIMEOUT"));
      break;
    case AUTO_CHANGE_BOMBA:
      Serial.print(F("AUTO_CHANGE_BOMBA"));
      break;

  }

  if (newline)
    Serial.println();
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

/*
  Bomba* GetActiveBomba()
  {
  if (!IsBombaAvailable(&bomba1) && !IsBombaAvailable(&bomba2))
    return NULL;

  if (bomba1.IsActive && IsBombaAvailable(&bomba1))
    return &bomba1;

  if (bomba2.IsActive && IsBombaAvailable(&bomba2))
    return &bomba2;

  return NULL;
  }
*/
