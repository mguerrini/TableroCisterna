
//--- Setup ---
void SetupBombaSensors()
{
  //no hago nada..en principio
}

void SetupBombas()
{
  //Read el estado enabled/disabled from EEPROM
  bomba1.Number = BOMBA1;
  bomba1.IsEnabled = true;
  bomba1.State = BOMBA_STATE_OFF; //0=OFF 1=ON -1=ERROR CONTACTOR ABIERTO -2=ERROR CONTACTOR CERRADO -3=ERROR TERMICO -3=ERROR BOMBA (ESTE NO ESTA EN FUNCIONAMIENTO TODAVIA, FALTARIA UN SENSOR EN LA BOMBA QUE DETECTE FUNCIONAMIENTO)
  bomba1.IsActive = true;
  bomba1.Uses = 0;
  bomba1.MachineState = FSM_BOMBA_OFF;

  bomba2.Number = BOMBA2;
  bomba2.IsEnabled = true;
  bomba2.State = BOMBA_STATE_OFF; //0=OFF 1=ON -1=ERROR CONTACTOR ABIERTO -2=ERROR CONTACTOR CERRADO -3=ERROR TERMICO -3=ERROR BOMBA (ESTE NO ESTA EN FUNCIONAMIENTO TODAVIA, FALTARIA UN SENSOR EN LA BOMBA QUE DETECTE FUNCIONAMIENTO)
  bomba2.IsActive = false;
  bomba2.Uses = 0;
  bomba2.MachineState = FSM_BOMBA_OFF;

  UpdateBomba1Display();
  UpdateBomba2Display();
}


//--- Control ---

void ReadBombaSensors()
{
  ReadBombaSensors(&bomba1);
  ReadBombaSensors(&bomba2);
}

void ReadBombaSensors(Bomba* bomba)
{
  if (bomba->Number == BOMBA1)
  {
    bomba->IsContactorClosed = IsBomba1ContactorClosed();
    bomba->IsTermicoOk = IsBomba1TermicoOk();
  }
  else
  {
    bomba->IsContactorClosed = IsBomba2ContactorClosed();
    bomba->IsTermicoOk = IsBomba1TermicoOk();
  }
}

void ReadEnabledBombas()
{
  bomba1.RequestEnabled = false;
  bomba1.RequestDisabled = false;
  bomba2.RequestEnabled = false;
  bomba2.RequestDisabled = false;

  bool pressed1 = IsBomba1EnabledButtonPressed();
  bool pressed2 = IsBomba1EnabledButtonPressed();

  if (!pressed1 && !pressed2)
  {
    //estan habilitadas las dos
    if (!bomba1.IsEnabled)
    {
      bomba1.RequestEnabled = true;
    }

    if (!bomba2.IsEnabled)
    {
      bomba2.RequestEnabled = true;
    }
  }
  else if (pressed1)
  {
    if (!bomba1.IsEnabled)
    {
      bomba1.RequestEnabled = true;
      bomba2.RequestDisabled = true;
    }
  }
  else if (pressed2)
  {
    if (!bomba2.IsEnabled)
    {
      bomba2.RequestEnabled = true;
      bomba1.RequestDisabled = false;
    }
  }
}

bool IsBombaOff(Bomba* bomba)
{
  return bomba->State == BOMBA_STATE_OFF;
}

bool IsBombaOff(byte bombaNumber)
{
  if (bombaNumber == BOMBA1)
    return bomba1.State == BOMBA_STATE_OFF;
  else
    return bomba2.State == BOMBA_STATE_OFF;
}

bool IsBombaOn(Bomba* bomba)
{
  return bomba->State == BOMBA_STATE_ON;
}

bool IsBombaOn(byte bombaNumber)
{
  if (bombaNumber == BOMBA1)
    return bomba1.State == BOMBA_STATE_ON;
  else
    return bomba2.State == BOMBA_STATE_ON;
}

bool IsBombaEnabled(Bomba* bomba)
{
  if (bomba->Number == BOMBA1)
    return bomba1.IsEnabled;
  else
    return bomba2.IsEnabled;
}

bool IsBombaEnabled(byte bombaNumber)
{
  if (bombaNumber == BOMBA1)
    return bomba1.IsEnabled;
  else
    return bomba2.IsEnabled;
}

bool IsBombaError(Bomba* bomba)
{
  return bomba->State >= 0;
}

bool IsBombaError(byte bombaNumber)
{
  if (bombaNumber == BOMBA1)
    return bomba1.State >= 0;
  else
    return bomba2.State >= 0;
}


bool IsBombaAvailable(byte bombaNumber)
{
  return IsBombaEnabled(bombaNumber) && !IsBombaError(bombaNumber);
}


bool IsBombaAvailable(Bomba* bomba)
{
  return IsBombaEnabled(bomba) && !IsBombaError(bomba);
}


boolean IsBomba1ContactorClosed()
{
  static unsigned long startTime = 0;
  static boolean state;
  static boolean isPressed;

  return IsButtonPressed("Bomba 1 Contactor", BOMBA1_CONTACTOR_RETORNO_PIN, state, isPressed, startTime);
}

boolean IsBomba2ContactorClosed()
{
  static unsigned long startTime = 0;
  static boolean state;
  static boolean isPressed;

  return IsButtonPressed("Bomba 2 Contactor", BOMBA2_CONTACTOR_RETORNO_PIN, state, isPressed, startTime);
}


boolean IsBomba1TermicoOk()
{
  static unsigned long startTime = 0;
  static boolean state;
  static boolean isPressed;

  return IsButtonPressed("Bomba 1 Contactor", BOMBA1_TERMICO_RETORNO_PIN, state, isPressed, startTime);
}

boolean IsBomba2TermicoOk()
{
  static unsigned long startTime = 0;
  static boolean state;
  static boolean isPressed;

  return IsButtonPressed("Bomba 2 Contactor", BOMBA2_TERMICO_RETORNO_PIN, state, isPressed, startTime);
}
