
//--- Setup ---
void SetupBombaSensors()
{
  //no hago nada..en principio
}

void SetupBombas()
{
  //Read el estado enabled/disabled from EEPROM
  

  UpdateBomba1Display();
  UpdateBomba2Display();
}


//--- Control ---

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



void ReadEnabledBomba1()
{
  bool pressed = IsBomba1EnableButtonPressed();
  if (pressed)
  {
    bomba1.IsEnabled = !bomba1.IsEnabled;

    if (!IsBombaEnabled(BOMBA1))
    {
      if (IsBombaError(BOMBA1)) //estaba en error
      {
        //StopAlarm(); //freno la alarma porque ya atendi el error.

        //la pongo en OFF
        //Bomba1Off();
      }

      //detengo la bomba
      StopBomba1();
    }

    UpdateBomba1Display();
  }
}


void ReadEnabledBomba2()
{
  bool pressed = IsBomba2EnableButtonPressed();

  if (pressed)
  {
    bomba2.IsEnabled = !bomba2.IsEnabled;
    UpdateBomba2Display();

    if (!IsBombaEnabled(BOMBA2))
    {
      if (IsBombaError(BOMBA2)) //estaba en error
      {
        //StopAlarm(); //freno la alarma porque ya atendi el error.
      }

      //detengo la bomba
      StopBomba2();
    }
  }
}


bool IsBombaOff(byte bombaNumber)
{
  if (bombaNumber == BOMBA1)
    return bomba1.State == OFF;
  else
    return bomba2.State == OFF;
}

bool IsBombaOn(byte bombaNumber)
{
  if (bombaNumber == BOMBA1)
    return bomba1.State == ON;
  else
    return bomba2.State == ON;
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



bool IsBombaError(byte bombaNumber)
{
  if (bombaNumber == BOMBA1)
    return bomba1.State >= 0;
  else
    return bomba2.State >= 0;
}

bool IsBombaAvailable(byte bombaNumber)
{
  return IsBombaAvailable(bombaNumber) && !IsBombaError(bombaNumber);
}

void StartBomba1()
{

}

void StartBomba2()
{

}


void StopBombas()
{
  StopBomba1();
  StopBomba2();
}

void StopBomba1()
{

}

void StopBomba2()
{

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
