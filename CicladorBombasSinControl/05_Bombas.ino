
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
  bomba1.FillTimeMinutesAverage = 0;
  ResetBomba(&bomba1);

  bomba2.Number = BOMBA2;
  bomba2.IsActive = false;
  bomba2.FillTimeMinutesAverage = 0;
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
  
  if (termicoChanged)
  {
    Serial.print(F("Termico changed - "));
    Serial.print(bomba->IsTermicoOk);
    Serial.print(F(" -> "));
    Serial.println(termicoOk);
  }

  bomba->IsContactorClosed = contactorClosed;
  bomba->IsTermicoOk = termicoOk;

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
      Serial.println(F("Bomba 1: Request Enabled"));
    }

    if (!bomba2.IsEnabled && !bomba2.RequestEnabled)
    {
      bomba2.RequestEnabled = true;
      Serial.println(F("Bomba 2: Request Enabled"));
    }
  }
  else if (pressed1 && pressed2)
  {
    bomba1.RequestEnabled = false;
    bomba2.RequestEnabled = false;

    if (bomba1.IsEnabled && !bomba1.RequestDisabled)
    {
      bomba1.RequestDisabled = true;
      Serial.println(F("Bomba 1: Request Disabled"));
    }

    if (bomba2.IsEnabled && !bomba2.RequestDisabled)
    {
      bomba2.RequestDisabled = true;
      Serial.println(F("Bomba 2: Request Disabled"));
    }
  }
  else if (pressed1)
  {
    bomba1.RequestDisabled = false;
    bomba2.RequestEnabled = false;

    if (!bomba1.IsEnabled && !bomba1.RequestEnabled)
    {
      bomba1.RequestEnabled = true;
      Serial.println(F("Bomba 1: Request Enabled"));
    }

    if (bomba2.IsEnabled && !bomba2.RequestDisabled)
    {
      bomba2.RequestDisabled = true;
      Serial.println(F("Bomba 2: Request Disabled"));
    }
  }
  else if (pressed2)
  {
    bomba1.RequestEnabled = false;
    bomba2.RequestDisabled = false;

    if (bomba1.IsEnabled && !bomba1.RequestDisabled)
    {
      bomba1.RequestDisabled = true;
      Serial.println(F("Bomba 1: Request Disabled"));
    }
    if (!bomba2.IsEnabled && !bomba2.RequestEnabled)
    {
      bomba2.RequestEnabled = true;
      Serial.println(F("Bomba 2: Request Enabled"));
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
  bomba->MachineState = FSM_BOMBA_OFF;
  bomba->FromMachineState = FSM_BOMBA_OFF;
  bomba->IsTermicoOk = true;
  bomba->IsContactorClosed = false;
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
  for (int i = 0; i < 10; i++)
  {
    bomba1.FillTimeMinutes[i] = 0;
    bomba2.FillTimeMinutes[i] = 0;

    bomba1.FillTimeMinutesAverage = 0;
    bomba1.FillTimeMinutesAverage = 0;
  }
}


// ****************************************************************** //
//                        PROPIEDADES
// ****************************************************************** //

byte GetActiveBombaNumber()
{
  if (bomba1.IsActive)
    return BOMBA1;
  else
    return BOMBA2;
}

byte GetActiveBombaUses()
{
  if (bomba1.IsActive)
    return bomba1.Uses;
  else
    return bomba2.Uses;
}

bool IsBombaOff(Bomba* bomba)
{
  return bomba->State == BOMBA_STATE_OFF;
}

bool IsBombaOn(Bomba* bomba)
{
  return bomba->State == BOMBA_STATE_ON;
}

bool IsBombaEnabled(Bomba* bomba)
{
  if (bomba->Number == BOMBA1)
    return bomba1.IsEnabled;
  else
    return bomba2.IsEnabled;
}

bool IsBombaError(Bomba* bomba, byte error)
{
  return bomba->State == error;
}

bool IsBombaError(Bomba* bomba)
{
  return bomba->State < 0;
}

bool IsBombaAvailable(Bomba* bomba)
{
  return IsBombaEnabled(bomba) && !IsBombaError(bomba);
}

byte GetBombaState(Bomba* bomba)
{
  return bomba->State;
}


//Devuelve el tiempo de encendido de la bomba en minutos
unsigned long GetBombaWorkingTime(Bomba* bomba)
{
  if (!IsBombaOn(bomba))
    return 0;

  unsigned long delta = 0;
  unsigned long minutes = 0;

  delta = deltaMillis(millis(), bomba->StartTime);
  minutes = delta / 60000;

  return minutes;
}

//Devuelve el tiempo maximo de llenado antes de que se considere que la bomba no funciona.
unsigned long GetBombaWorkingTimeMaximum(Bomba* bomba)
{
  unsigned int avg = bomba->FillTimeMinutesAverage;
  if (avg > 0)
    return TANQUE_TIME_TO_FULL_AVERAGE_PERCENTAGE * avg;
  else
    return TANQUE_TIME_TO_FULL;
}


// ****************************************************************** //
//                        SENSORES
// ****************************************************************** //

bool IsBombaSwapButtonPressed()
{
  static unsigned long startTime = millis();
  static boolean state = digitalRead(BOMBA_SWAP_BTN_PIN);
  static boolean isPressed;

  return IsButtonPressed(BOMBA_SWAP_BTN_PIN, state, isPressed, startTime);
}


boolean IsBomba1ContactorClosed()
{
  static unsigned long startTime = millis();
  static boolean state = digitalRead(BOMBA1_CONTACTOR_RETORNO_PIN);
  static boolean isPressed;

  IsButtonPressed(BOMBA1_CONTACTOR_RETORNO_PIN, state, isPressed, startTime);

  return isPressed;
}

boolean IsBomba2ContactorClosed()
{
  static unsigned long startTime = millis();
  static boolean state = digitalRead(BOMBA2_CONTACTOR_RETORNO_PIN);
  static boolean isPressed;

  IsButtonPressed(BOMBA2_CONTACTOR_RETORNO_PIN, state, isPressed, startTime);

  return isPressed;
}


boolean IsBomba1TermicoOk()
{
  static unsigned long startTime = millis();
  static boolean state = digitalRead(BOMBA1_TERMICO_RETORNO_PIN);
  static boolean isPressed;

  IsButtonPressed(BOMBA1_TERMICO_RETORNO_PIN, state, isPressed, startTime);

  return isPressed;
}

boolean IsBomba2TermicoOk()
{
  static unsigned long startTime = millis();
  static boolean state = digitalRead(BOMBA2_TERMICO_RETORNO_PIN);
  static boolean isPressed;

  IsButtonPressed(BOMBA2_TERMICO_RETORNO_PIN, state, isPressed, startTime);

  return isPressed;
}

boolean IsBomba1EnabledButtonPressed()
{
  static unsigned long startTime = millis();
  static boolean state = digitalRead(BOMBA1_ENABLE_PIN);
  static boolean isPressed;

  IsButtonPressed(BOMBA1_ENABLE_PIN, state, isPressed, startTime);

  return isPressed;
}

boolean IsBomba2EnabledButtonPressed()
{
  static unsigned long startTime = millis();
  static boolean state = digitalRead(BOMBA2_ENABLE_PIN);
  static boolean isPressed;

  IsButtonPressed(BOMBA2_ENABLE_PIN, state, isPressed, startTime);

  return isPressed;
}
