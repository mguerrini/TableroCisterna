
// ****************************************************************** //
//                          SETUP
// ****************************************************************** //

void SetupBombaSensors()
{
  //no hago nada..en principio
}

void SetupBombas()
{
  //Read el estado enabled/disabled from EEPROM
  bomba1.Number = BOMBA1;
  bomba1.IsActive = false;
  bomba1.FillTimeSecondsAverage = 0;
  ResetBomba(&bomba1);

  bomba2.Number = BOMBA2;
  bomba2.IsActive = false;
  bomba2.FillTimeSecondsAverage = 0;
  ResetBomba(&bomba2);

  //el tiempo promedio de llenado se carga cuando se cargan las estadisticas en el SetupStatistics

  //Inicializo los sensores
  IsBomba1ContactorClosed();
  IsBomba2ContactorClosed();
  IsBomba1EnabledButtonPressed();
  IsBomba2EnabledButtonPressed();
  IsBomba1TermicoOk();
  IsBomba2TermicoOk();

  //espero el tiempo de estabilizacion para volver a leerlos
  delay(BTN_PRESSED_TIME + BTN_PRESSED_TIME);

  //activo la bomba 1
  ActivateBomba(&bomba1, false);

  ReadBombaSensors();

  ReadEnabledBombas();
}


// ****************************************************************** //
//                              READ
// ****************************************************************** //

void ReadBombaSensors()
{
  ReadBombaSensors(&bomba1);
  ReadBombaSensors(&bomba2);
}

void ReadBombaSensors(Bomba* bomba)
{
  bool contactorClosed;
  bool termicoOk;
  bool contactorChanged;
  bool termicoChanged;
/*
    contactorClosed = IsBomba1ContactorClosed();
    Serial.print(F("Bomba 1"));
    if (contactorClosed)
      Serial.println(F(": Contactor Cerrado"));
    else
      Serial.println(F(": Contactor Abierto"));

    contactorClosed = IsBomba2ContactorClosed();
    Serial.print(F("Bomba 2"));
    if (contactorClosed)
      Serial.println(F(": Contactor Cerrado"));
    else
      Serial.println(F(": Contactor Abierto"));
*/
  if (bomba->Number == BOMBA1)
  {
    contactorClosed = IsBomba1ContactorClosed();
    termicoOk = IsBomba1TermicoOk();
  }
  else
  {
    contactorClosed = IsBomba2ContactorClosed();
    termicoOk = IsBomba2TermicoOk();
  }

  termicoChanged = termicoOk != bomba->IsTermicoOk;
  contactorChanged = contactorClosed != bomba->IsContactorClosed;
  /*
    #ifdef LOG_ENABLED
    if (termicoChanged)
    {
      Serial.print(F("Termico changed - "));
      Serial.print(bomba->IsTermicoOk);
      Serial.print(F(" -> "));
      Serial.println(termicoOk);
    }
    #endif
  */
  bomba->IsContactorClosed = contactorClosed;
  bomba->IsTermicoOk = termicoOk;

#ifdef LOG_ENABLED
  if (contactorChanged)
  {
    Serial.print(F("Bomba "));
    Serial.print(bomba->Number);
    if (bomba->IsContactorClosed)
      Serial.println(F(": Contactor Cerrado"));
    else
      Serial.println(F(": Contactor Abierto"));
  }

  if (termicoChanged)
  {
    Serial.print(F("Bomba "));
    Serial.print(bomba->Number);
    if (bomba->IsTermicoOk)
      Serial.println(F(": Termico Cerrado"));
    else
      Serial.println(F(": Termico Abierto"));
  }
#endif
}

void ReadEnabledBombas()
{
  bool pressed1 = IsBomba1EnabledButtonPressed();
  bool pressed2 = IsBomba2EnabledButtonPressed();

  if (!pressed1 && !pressed2)
  {
    //estan habilitadas las dos
    bomba1.RequestDisabled = false;
    bomba2.RequestDisabled = false;

    if (!bomba1.IsEnabled && !bomba1.RequestEnabled)
    {
      bomba1.RequestEnabled = true;
#ifdef LOG_ENABLED
      Serial.println(F("Bomba 1: Request Enabled"));
#endif
    }

    if (!bomba2.IsEnabled && !bomba2.RequestEnabled)
    {
      bomba2.RequestEnabled = true;
#ifdef LOG_ENABLED
      Serial.println(F("Bomba 2: Request Enabled"));
#endif
    }
  }
  else if (pressed1 && pressed2)
  {
    bomba1.RequestEnabled = false;
    bomba2.RequestEnabled = false;

    if (bomba1.IsEnabled && !bomba1.RequestDisabled)
    {
      bomba1.RequestDisabled = true;
#ifdef LOG_ENABLED
      Serial.println(F("Bomba 1: Request Disabled"));
#endif
    }

    if (bomba2.IsEnabled && !bomba2.RequestDisabled)
    {
      bomba2.RequestDisabled = true;
#ifdef LOG_ENABLED
      Serial.println(F("Bomba 2: Request Disabled"));
#endif
    }
  }
  else if (pressed1)
  {
    bomba1.RequestDisabled = false;
    bomba2.RequestEnabled = false;

    if (!bomba1.IsEnabled && !bomba1.RequestEnabled)
    {
      bomba1.RequestEnabled = true;
#ifdef LOG_ENABLED
      Serial.println(F("Bomba 1: Request Enabled"));
#endif
    }

    if (bomba2.IsEnabled && !bomba2.RequestDisabled)
    {
      bomba2.RequestDisabled = true;
#ifdef LOG_ENABLED
      Serial.println(F("Bomba 2: Request Disabled"));
#endif
    }
  }
  else if (pressed2)
  {
    bomba1.RequestEnabled = false;
    bomba2.RequestDisabled = false;

    if (bomba1.IsEnabled && !bomba1.RequestDisabled)
    {
      bomba1.RequestDisabled = true;
#ifdef LOG_ENABLED
      Serial.println(F("Bomba 1: Request Disabled"));
#endif
    }
    if (!bomba2.IsEnabled && !bomba2.RequestEnabled)
    {
      bomba2.RequestEnabled = true;
#ifdef LOG_ENABLED
      Serial.println(F("Bomba 2: Request Enabled"));
#endif
    }
  }
}

// ****************************************************************** //
//                          METODOS
// ****************************************************************** //

//reseteo todos los valores...en la siguiente vuelta se va a poner todo normal
void ResetBomba(Bomba* bomba)
{
  bomba->IsEnabled = true;
  bomba->State = BOMBA_STATE_OFF; //0=OFF 1=ON -1=ERROR CONTACTOR ABIERTO -2=ERROR CONTACTOR CERRADO -3=ERROR TERMICO -3=ERROR BOMBA (ESTE NO ESTA EN FUNCIONAMIENTO TODAVIA, FALTARIA UN SENSOR EN LA BOMBA QUE DETECTE FUNCIONAMIENTO)
  bomba->Uses = 0;
  bomba->FromMachineState = FSM_BOMBA_OFF;
  bomba->MachineState = FSM_BOMBA_OFF;
  bomba->NextMachineState = FSM_BOMBA_NULL;

  bomba->IsTermicoOk = true;
  bomba->IsContactorClosed = false;
  bomba->RequestOn = false;
  bomba->RequestOff = false;
  bomba->RequestEnabled = false;
  bomba->RequestDisabled = false;
}

void ActivateBomba(Bomba* bomba, boolean updateView)
{
  if (bomba->IsActive)
  {
    return;
  }

  if (bomba->Number == BOMBA1)
  {
    bomba2.IsActive = false;
    bomba1.IsActive = true;
    bomba1.Uses = 0;

    digitalWrite(BOMBA_SWAP_RELE_PIN, BOMBA1_ACTIVE);
  }
  else
  {
    bomba1.IsActive = false;
    bomba2.IsActive = true;
    bomba2.Uses = 0;

    digitalWrite(BOMBA_SWAP_RELE_PIN, BOMBA2_ACTIVE);
  }

  if (updateView)
    UpdateActiveBombaDisplay();
}


Bomba* SwapAndActiveBomba()
{
  bool isAvailable1 = IsBombaAvailable(&bomba1);
  bool isAvailable2 = IsBombaAvailable(&bomba2);

  if (isAvailable1 && isAvailable2)
  {
    if (bomba1.IsActive)
    {
      ActivateBomba(&bomba2, true);
      return &bomba2;
    }
    else
    {
      ActivateBomba(&bomba1, true);
      return &bomba1;
    }
  }

  if (IsBombaAvailable(&bomba1))
  {
    ActivateBomba(&bomba1, true);
    return &bomba1;
  }

  if (IsBombaAvailable(&bomba2))
  {
    ActivateBomba(&bomba2, true);
    return &bomba2;
  }

  return NULL;
}


void CleanFillTimes()
{
  for (int i = 0; i < BOMBA_FILLTIMES_READ_MAX; i++)
  {
    bomba1.FillTimeSeconds[i] = 0;
    bomba2.FillTimeSeconds[i] = 0;
  }

  bomba1.FillTimeSecondsAverage = 0;
  bomba2.FillTimeSecondsAverage = 0;
  bomba1.FillTimeSecondsMaximum = 0;
  bomba2.FillTimeSecondsMaximum = 0;
}


// ****************************************************************** //
//                        PROPIEDADES
// ****************************************************************** //
bool IsBombaOff(Bomba* bomba)
{
  return bomba->State == BOMBA_STATE_OFF;
}

bool IsBombaOn(Bomba* bomba)
{
  return bomba->State == BOMBA_STATE_ON;
}

bool IsBombaError(Bomba* bomba)
{
  return bomba->State < 0;
}

bool IsBombaAvailable(Bomba* bomba)
{
  return bomba->IsEnabled && !IsBombaError(bomba);
}


//Devuelve el tiempo de encendido de la bomba en segundos
unsigned long GetBombaWorkingTimeInSeconds(Bomba* bomba)
{
  if (!IsBombaOn(bomba))
    return 0;

  unsigned long delta = 0;
  unsigned long minutes = 0;

  delta = deltaMillis(millis(), bomba->StartTime);
  minutes = delta / 1000;

  return minutes;
}

//Devuelve el tiempo maximo de llenado antes de que se considere que la bomba no funciona.
unsigned long GetBombaWorkingTimeMaximumSeconds(Bomba* bomba)
{
  unsigned int maxTime = bomba->FillTimeSecondsMaximum;
  if (maxTime > 0)
    return TANQUE_TIME_TO_FULL_FACTOR * maxTime;
  else
    return TANQUE_TIME_TO_FULL_INITIAL;
}


// ****************************************************************** //
//                        SENSORES
// ****************************************************************** //

bool IsBombaSwapButtonPressed()
{
  static unsigned long startTime = millis();
  static boolean state = digitalRead(BOMBA_SWAP_BTN_PIN);
  static boolean isPressed;

  return IsButtonPressed(BTN_PRESSED_TIME, BOMBA_SWAP_BTN_PIN, state, isPressed, startTime);
}


boolean IsBomba1ContactorClosed()
{
  static unsigned long startTime = millis();
  static boolean state = digitalRead(BOMBA1_CONTACTOR_RETORNO_PIN);
  static boolean isPressed;

  IsButtonPressed(CONTACTOR_CLOSED_TIME, BOMBA1_CONTACTOR_RETORNO_PIN, state, isPressed, startTime);

  return isPressed;
}

boolean IsBomba2ContactorClosed()
{
  static unsigned long startTime = millis();
  static boolean state = digitalRead(BOMBA2_CONTACTOR_RETORNO_PIN);
  static boolean isPressed;

  IsButtonPressed(CONTACTOR_CLOSED_TIME, BOMBA2_CONTACTOR_RETORNO_PIN, state, isPressed, startTime);

  return isPressed;
}


boolean IsBomba1TermicoOk()
{
  static unsigned long startTime = millis();
  static boolean state = digitalRead(BOMBA1_TERMICO_RETORNO_PIN);
  static boolean isPressed;

  IsButtonPressed(BTN_PRESSED_TIME, BOMBA1_TERMICO_RETORNO_PIN, state, isPressed, startTime);

  if (BOMBA1_TERMICO_OK_NA)
    return !isPressed;
  else
    return isPressed;
}

boolean IsBomba2TermicoOk()
{
  static unsigned long startTime = millis();
  static boolean state = digitalRead(BOMBA2_TERMICO_RETORNO_PIN);
  static boolean isPressed;

  IsButtonPressed(BTN_PRESSED_TIME, BOMBA2_TERMICO_RETORNO_PIN, state, isPressed, startTime);

  if (BOMBA2_TERMICO_OK_NA)
    return !isPressed;
  else
    return isPressed;
}

boolean IsBomba1EnabledButtonPressed()
{
  static unsigned long startTime = millis();
  static boolean state = digitalRead(BOMBA1_ENABLE_PIN);
  static boolean isPressed;

  IsButtonPressed(BTN_PRESSED_TIME, BOMBA1_ENABLE_PIN, state, isPressed, startTime);

  return isPressed;
}

boolean IsBomba2EnabledButtonPressed()
{
  static unsigned long startTime = millis();
  static boolean state = digitalRead(BOMBA2_ENABLE_PIN);
  static boolean isPressed;

  IsButtonPressed(BTN_PRESSED_TIME, BOMBA2_ENABLE_PIN, state, isPressed, startTime);

  return isPressed;
}
