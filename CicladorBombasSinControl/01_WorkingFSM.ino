// *************************************************** //
//                  AUTO STATE MACHINE
// *************************************************** //

void CicladorLoop()
{
  Bomba* bomba = GetActiveBomba();
  automaticFSM.Message = 0;

  automaticFSM.FromState = automaticFSM.State;
  if (automaticFSM.NextState != AUTO_NULL)
    automaticFSM.State = automaticFSM.NextState;

  automaticFSM.NextState = AUTO_NULL;

  //ejecuto el estado correspondiente
  ExecuteAutoState(bomba);

  //ejecuto la maquina de estados de las bombas
  BombaStateMachine(&bomba1);
  BombaStateMachine(&bomba2);

#ifdef LOG_ENABLED
  if (CanPrintStateAutoFSM() || CanPrintStateBombaFSM(&bomba1) || CanPrintStateBombaFSM(&bomba2))
  {
    PrintStateAutoFSM();
    PrintStateBombaFSM(&bomba1);
    PrintStateBombaFSM(&bomba2);
    Serial.println();
  }
#endif
}


// ============== EXECUTION ============== //

void ExecuteAutoState(Bomba* bomba)
{
  byte state = automaticFSM.State;

  // **************************************** //
  //              AUTO_IDLE                    //
  // **************************************** //

  if (state == AUTO_IDLE)
  {
    //sino esta ok la fase....no arranca el sistema. Deberia volver a idle porque los motores se apagaron
    if (!automaticFSM.IsFaseOk)
    {
      return;
    }

    if (!IsBombaAvailable(bomba))
    {
      //Cambio de bomba.
      automaticFSM.NextState = AUTO_CHANGE_BOMBA_FROM_NOT_AVAILABLE;
      automaticFSM.Message = MSG_AUTO_BOMBA_ACTIVA_NO_DISPONIBLE;
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
        automaticFSM.Message = MSG_AUTO_ERROR_SENSORES;
      }

      automaticFSM.NextState = AUTO_STARTING;
      automaticFSM.Message = MSG_AUTO_BOMBA_ON;
      return;
    }

    //Esto pasa a encendido de la bomba porque puede que el contactor no se haya cerrado por un error
    if (CanTurnOnBomba())
    {
      //deberia encenderse la bomba
      automaticFSM.NextState = AUTO_STARTING;
      automaticFSM.Message = MSG_AUTO_TANQUE_VACIO;
      return;
    }

  }


  // **************************************** //
  //              AUTO_STARTING               //
  // **************************************** //

  else if (state == AUTO_STARTING)
  {
    //espero hasta que arranqe o que se ponga en error.
    bomba->RequestOn = false;

    if (IsBombaOn(bomba))
    {
      //listo...paso al estado working
      automaticFSM.NextState = AUTO_WORKING;
      automaticFSM.Message = MSG_AUTO_BOMBA_ON;
      return;
    }

    if (!IsBombaAvailable(bomba))
    {
      automaticFSM.NextState = AUTO_IDLE;
      automaticFSM.Message = MSG_AUTO_BOMBA_ACTIVA_NO_DISPONIBLE;
      return;
    }

    //indico que se encienda
    if (IsFirstTimeInAutoState())
    {
      bomba->RequestOn = true;
      automaticFSM.Message = MSG_AUTO_BOMBA_OFF_REQUEST_ON;
      return;
    }

    //intento encenderla...hasta que tire error por timeout o termico o arranqe
    bomba->RequestOn = true;
    //automaticFSM.Message = MSG_AUTO_BOMBA_OFF_REQUEST_ON;
    return;
  }


  // **************************************** //
  //              AUTO_WORKING                //
  // **************************************** //
  else if (state == AUTO_WORKING)
  {
    if (!IsBombaAvailable(bomba))
    {
      //Cambio de bomba.
      automaticFSM.NextState = AUTO_CHANGE_BOMBA_FROM_NOT_AVAILABLE;
      automaticFSM.Message = MSG_AUTO_BOMBA_ACTIVA_NO_DISPONIBLE;
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
        Serial.println(F("Error en la lectura de los sensores del tanque"));
#endif
      }

      //no se deberia dar nunca este caso a no ser que se pase justo a automatico o el circuito de lectura de los niveles este roto.
      //deberia registrar el error en el visor
      automaticFSM.NextState = AUTO_STOPPING;
      automaticFSM.Message = MSG_AUTO_BOMBA_OFF;
      return;
    }


    //detecta tanque lleno y o cisterna vacia....pero la bomba sigue encendida....el sistema no puede apagar una bomba
    //cuando se corta por primera vez la FSM de la bomba esta en estado ON, pero primero se ejecuta este estado.....por eso pareciera que fuera un error.
    //Habria que esperar un par de iteraciones antes de pensar que fue un error
    if (CanTurnOffBomba()) // == sensores.IsTanqueSensorMaxVal || sensores.IsCisternaSensorMinVal || IsBombaOff(bomba))
    {
#ifdef LOG_ENABLED
      /*
            if (sensores.IsTanqueSensorMaxVal)
            {
              if (sensores.IsCisternaSensorMinVal)
                //          automaticFSM.Message = MSG_AUTO_ERROR_SENSORES_TANQUE_LLENO_CISTERNA_VACIA;
                PrintAutoMessage(MSG_AUTO_ERROR_SENSORES_TANQUE_LLENO_CISTERNA_VACIA);
              else
                //          automaticFSM.Message = MSG_AUTO_ERROR_SENSORES_TANQUE_LLENO_CISTERNA_NORMAL;
                PrintAutoMessage(MSG_AUTO_ERROR_SENSORES_TANQUE_LLENO_CISTERNA_NORMAL);
            }
            else
            {
              PrintAutoMessage(MSG_AUTO_CISTERNA_EMPTY);

              //automaticFSM.Message = MSG_AUTO_CISTERNA_EMPTY;
            }

            Serial.println();
      */
#endif
      return;
    }

    //verifico el tiempo de bombeo...si supera el maximo....cambio de bomba
    unsigned long workingMax = GetBombaWorkingTimeMaximumSeconds(bomba);
    unsigned long workingTime = GetBombaWorkingTimeInSeconds(bomba);

    if (workingTime > workingMax)
    {
      //cambio de bomba.....no esta funcionando
      automaticFSM.NextState = AUTO_CHANGE_BOMBA_FROM_FILL_TIMEOUT;
      automaticFSM.Message = MSG_AUTO_BOMBA_ACTIVA_FILL_TIMEOUT;
    }

    return;
  }



  // **************************************** //
  //              AUTO_STOPPING               //
  // **************************************** //
  else if (state == AUTO_STOPPING)
  {
    //espero a que el contactor se abra
    bomba->RequestOff = false;

    if (IsBombaOff(bomba) || IsBombaError(bomba))
    {
      // BOMBA OFF => Tengo que actualizar los contadores
      bomba->Uses = bomba->Uses + 1;
      if (bomba->Uses >= automaticFSM.CiclosMax)
      {
        automaticFSM.NextState = AUTO_CHANGE_BOMBA;
        automaticFSM.Message = MSG_AUTO_CHANGE_BOMBA_USOS_MAXIMO_ALACANZADO;
      }
      else
      {
        automaticFSM.NextState = AUTO_IDLE;
      }

      return;
    }

    if (IsFirstTimeInAutoState())
    {
      automaticFSM.StoppingTimer = millis();
      bomba->RequestOff = true;
      automaticFSM.Message = MSG_AUTO_REQUEST_OFF;
      return;
    }

    if (IsBombaOn(bomba))
    {
      //espero un tiempo prudencial hasta que se detenga la bomba
      unsigned long wait = deltaMillis(millis(), automaticFSM.StoppingTimer);
      //espero a que inicie....la bomba activa deberia estar en ON.
      if (wait > (2 * BOMBA_TURNING_OFF_TIME))
      {
        //ERROR....NO SE DETUVO LA BOMBA Y SIGUE FUNCIONANDO...NUNCA DEBERIA OCURRIR, PORQUE LOS SENSORES APAGAN LAS BOMBAS
        automaticFSM.NextState = AUTO_ERROR_BOMBA_WORKING;
        automaticFSM.Message = MSG_AUTO_ERROR_STOPPING_BOMBA_TIMEOUT;
      }
      else
      {
        bomba->RequestOff = true;
        //PrintAutoMessage(MSG_AUTO_WAITING_APERTURA_CONTACTOR, true);
        //automaticFSM.Message = MSG_AUTO_WAITING_APERTURA_CONTACTOR;
      }
    }
    return;
  }


  // **************************************** //
  //            AUTO_CHANGE_BOMBA             //
  // **************************************** //
  else if (state == AUTO_CHANGE_BOMBA)
  {
    bomba = SwapAndActiveBomba();
    if (bomba == NULL)
    {
      automaticFSM.NextState = AUTO_NOT_AVAILABLES_BOMBAS;
      automaticFSM.Message = MSG_AUTO_ERROR_NOT_AVAILABLES_BOMBAS;
      return;
    }

    automaticFSM.NextState = AUTO_IDLE;

#ifdef LOG_ENABLED
    if (bomba->Number == BOMBA1)
      automaticFSM.Message = MSG_AUTO_B1_ACTIVA;
    else
      automaticFSM.Message = MSG_AUTO_B2_ACTIVA;
#endif
    return;
  }


  // **************************************** //
  //       AUTO_NOT_AVAILABLES_BOMBAS         //
  // **************************************** //
  else if (state == AUTO_NOT_AVAILABLES_BOMBAS)
  {

    //disparo la alarma
    if (IsFirstTimeInAutoState())
    {
      StartAlarmNotAvailablesBombas();
      automaticFSM.Message = MSG_AUTO_START_ALARMA;
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
        automaticFSM.Message = MSG_AUTO_B1_DISPONIBLE_STOP_ALARMA;
      else
        automaticFSM.Message = MSG_AUTO_B2_DISPONIBLE_STOP_ALARMA;
#endif

      StopAlarmNotAvailablesBombas();
    }


    return;
  }


  // *********************************************** //
  //       AUTO_CHANGE_BOMBA_FROM_NOT_AVAILABLE      //
  // *********************************************** //

  else if (state == AUTO_CHANGE_BOMBA_FROM_NOT_AVAILABLE)
  {
    if (IsBombaError(bomba) || !bomba->IsEnabled)
    {
      //Cambio de bomba.
      automaticFSM.NextState = AUTO_CHANGE_BOMBA;
#ifdef LOG_ENABLED
      if (!bomba->IsEnabled)
        automaticFSM.Message = MSG_AUTO_BOMBA_ACTIVA_DISABLED;
      else
        automaticFSM.Message = MSG_AUTO_BOMBA_ACTIVA_ERROR;
#endif
      return;
    }

    automaticFSM.NextState = AUTO_IDLE;
    automaticFSM.Message = MSG_AUTO_BOMBA_ACTIVA_OK;
    return;
  }



  // *********************************************** //
  //      AUTO_CHANGE_BOMBA_FROM_FILL_TIMEOUT        //
  // *********************************************** //

  else if (state == AUTO_CHANGE_BOMBA_FROM_FILL_TIMEOUT)
  {
    //no registro error....trato de switchear bomba

    //no registro el tiempo de funcionamiento
    Statistics_BombaOff(bomba, false);

    //deberia poner la bomba en estado de timeout de llenado
    automaticFSM.NextState = AUTO_CHANGE_BOMBA;
    automaticFSM.Message = MSG_AUTO_ERROR_FILL_TIMEOUT;
    return;
  }


  // *********************************************** //
  //             AUTO_ERROR_BOMBA_WORKING            //
  // *********************************************** //

  else if (state == AUTO_ERROR_BOMBA_WORKING)
  {

    //Si se apaga es por los niveles...los sensores quizas fallan
    if (IsBombaOff(bomba) || !IsBombaAvailable(bomba))
    {
      StopAlarmBomba(bomba);
      automaticFSM.NextState = AUTO_STOPPING;
#ifdef LOG_ENABLED
      if (IsBombaOff(bomba))
        automaticFSM.Message = MSG_AUTO_BOMBA_OFF;
      else
        automaticFSM.Message = MSG_AUTO_BOMBA_ACTIVA_NO_DISPONIBLE;
#endif
      return;
    }

    if (!CanTurnOffBomba())
    {
      //no deberia apagarse....quizas se paso a manual justo cuando midio el tanque lleno, vuelve a working
      StopAlarmBomba(bomba);
      automaticFSM.NextState = AUTO_WORKING;
      automaticFSM.Message = MSG_AUTO_TANQUE_VACIO_CISTERNA_NORMAL;
      return;
    }

    //a este punto no deberia llegar nunca....es porque esta prendida pero los niveles dicen que tiene que estar apagada.....
    //falla en la eletronica encargada de traducir los 24 volts de los tanques a la entrada del arduino
    //trato de recuperar el error antes de disparar la alarma
    if (IsFirstTimeInAutoState())
    {
      StartAlarmBombaNotStop(bomba);
      //TODO Resaltar que la bomba no se detiene
      automaticFSM.Message = MSG_AUTO_ERROR_BOMBA_ON_START_ALARMA;
      return;
    }

    return;
  }
}


// *************************************************** //
//                FUNCIONES AUXLIARES
// *************************************************** //
void SetupAutoFSM()
{
  automaticFSM.IsFaseOk = true;
  automaticFSM.FromState = AUTO_IDLE;
  automaticFSM.State = AUTO_IDLE;
  automaticFSM.NextState = AUTO_NULL;

#ifdef EEPROM_ENABLED
  byte bAux;
  EEPROM.get(BOMBA_CICLOS_MAX_ADDR, bAux);
  automaticFSM.CiclosMax = bAux;
#else
  automaticFSM.CiclosMax = BOMBA_USES_MAX;
#endif

#ifdef LOG_ENABLED
  Serial.print(F("CiclosMax: "));
  Serial.println(automaticFSM.CiclosMax);
#endif
}

void SetCiclos(String ciclosStr)
{
  ciclosStr.trim();
  int iCiclos =  ciclosStr.toInt();

  if (iCiclos <= 0 || iCiclos > 50)
  {
    Serial.println(F("La cantidad de ciclos no puede ser menor a 0 ni mayor a 50."));
    return;
  }

  byte ciclos = (byte) iCiclos;
  EEPROM.put(BOMBA_CICLOS_MAX_ADDR, ciclos);

  automaticFSM.CiclosMax = ciclos;
}

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

boolean CanPrintStateAutoFSM()
{
#ifdef LOG_ENABLED

#ifdef LOG_MIN_ENABLED
  return automaticFSM.FromState != automaticFSM.State ||
         (automaticFSM.State != automaticFSM.NextState && automaticFSM.NextState != AUTO_NULL) ||
         automaticFSM.Message != 0;
#else
  return true;
#endif

#else
  return false;
#endif
}

void PrintStateAutoFSM()
{
#ifdef LOG_ENABLED
  Serial.print(F("Process: "));
  //Estado actual
  PrintStateWorkingFSM(automaticFSM.State);

  //mensaje
  PrintAutoMessage(automaticFSM.Message);

  byte state = automaticFSM.State;

  if (automaticFSM.NextState != AUTO_NULL)
    state = automaticFSM.NextState;

  Serial.print(F(" -> "));
  PrintStateWorkingFSM(state);
  Serial.println();
#endif
}

void PrintStateWorkingFSM(byte current)
{
  switch (current)
  {
    case AUTO_IDLE:
      Serial.print(F("IDLE"));
      break;
    case AUTO_NOT_AVAILABLES_BOMBAS:
      Serial.print(F("NOT_AVAILABLES_BOMBAS"));
      break;
    case AUTO_STARTING:
      Serial.print(F("STARTING"));
      break;
    case AUTO_WORKING:
      Serial.print(F("WORKING"));
      break;
    case AUTO_STOPPING:
      Serial.print(F("STOPPING"));
      break;
    case AUTO_CHANGE_BOMBA_FROM_NOT_AVAILABLE:
      Serial.print(F("CHANGE_BOMBA_FROM_NOT_AVAILABLE"));
      break;
    case AUTO_CHANGE_BOMBA:
      Serial.print(F("CHANGE_BOMBA"));
      break;
    case AUTO_ERROR_BOMBA_WORKING:
      Serial.print(F("ERROR_BOMBA_WORKING"));
      break;
    case AUTO_CHANGE_BOMBA_FROM_FILL_TIMEOUT:
      Serial.print(F("CHANGE_BOMBA_FROM_FILL_TIMEOUT"));
      break;
  }
}

void PrintAutoMessage(byte number)
{
#ifdef LOG_ENABLED
  if (number == 0)
    return;

  Serial.print(F(" ("));
  switch (number)
  {
    case MSG_AUTO_ERROR_SENSORES:
      Serial.print(F("Error sensores tanque"));
      break;

    case MSG_AUTO_ERROR_SENSORES_TANQUE_LLENO_CISTERNA_VACIA:
      Serial.print(F("Error Sensores - Tanque: Lleno, Cisterna: Vacia"));
      break;

    case MSG_AUTO_ERROR_SENSORES_TANQUE_LLENO_CISTERNA_NORMAL:
      Serial.print(F("Error Sensores - Tanque: Lleno, Cisterna: Normal"));
      break;

    case MSG_AUTO_ERROR_STOPPING_BOMBA_TIMEOUT:
      Serial.print(F("Stop Timeout - Bomba On"));
      break;

    case MSG_AUTO_ERROR_NOT_AVAILABLES_BOMBAS:
      Serial.print(F("No Bombas disponibles"));
      break;

    case MSG_AUTO_ERROR_FILL_TIMEOUT:
      Serial.print(F("Fill Timeout"));
      break;

    case MSG_AUTO_ERROR_BOMBA_ON_START_ALARMA:
      Serial.print(F("Start Alarm - Error Bomba ON"));
      break;

    case MSG_AUTO_BOMBA_ON:
      Serial.print(F("Bomba ON"));
      break;

    case MSG_AUTO_BOMBA_OFF_REQUEST_ON:
      Serial.print(F("Bomba OFF - Req On"));
      break;

    case MSG_AUTO_BOMBA_OFF:
      Serial.print(F("Bomba OFF"));
      break;

    case MSG_AUTO_TANQUE_VACIO:
      Serial.print(F("Tanque Vacío"));
      break;

    case MSG_AUTO_CISTERNA_EMPTY:
      Serial.print(F("Cisterna Vacia"));
      break;

    case MSG_AUTO_TANQUE_VACIO_CISTERNA_NORMAL:
      Serial.print(F("Tanque Vacio - Cisterna Normal - Bomba ON"));
      break;

    case MSG_AUTO_CHANGE_BOMBA_USOS_MAXIMO_ALACANZADO:
      Serial.print(F("Usos máx alcanzado->cambio de bomba"));
      break;

    case MSG_AUTO_BOMBA_ACTIVA_NO_DISPONIBLE:
      Serial.print(F("B Activa no disponible"));
      break;
    case MSG_AUTO_BOMBA_ACTIVA_FILL_TIMEOUT:
      Serial.print(F("B Activa Fill Timeout"));
      break;

    case MSG_AUTO_REQUEST_OFF:
      Serial.print(F("Req Off"));
      break;

    case MSG_AUTO_WAITING_APERTURA_CONTACTOR:
      Serial.print(F("Esperando contactor abierto..."));
      break;

    case MSG_AUTO_START_ALARMA:
      Serial.print(F("Start Alarma"));
      break;
    case MSG_AUTO_B1_DISPONIBLE_STOP_ALARMA:
      Serial.print(F("B1 disponible-Stop Alarma"));
      break;

    case MSG_AUTO_B2_DISPONIBLE_STOP_ALARMA:
      Serial.print(F("B2 disponible-Stop Alarma"));
      break;

    case MSG_AUTO_B1_ACTIVA:
      Serial.print(F("B1 activa"));
      break;

    case MSG_AUTO_B2_ACTIVA:
      Serial.print(F("B2 activa"));
      break;

    case MSG_AUTO_BOMBA_ACTIVA_DISABLED:
      Serial.print(F("B Activa Deshabilitada"));
      break;

    case MSG_AUTO_BOMBA_ACTIVA_ERROR:
      Serial.print(F("B Activa en Error"));
      break;

    case MSG_AUTO_BOMBA_ACTIVA_OK:
      Serial.print(F("B Activa Normalizada"));
      break;
  }

  Serial.print(F(")"));
#endif

}
