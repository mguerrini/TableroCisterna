//--- Sensores Tanque y Cisterna ---

void SetupLevelSensors()
{
  sensores.IsCisternaSensorMinVal = IsCisternaEmpty();
  sensores.IsTanqueSensorMinVal = IsTanqueEmpty();
  sensores.IsTanqueSensorMaxVal = IsTanqueFull();

  UpdateTanqueDisplay();
  UpdateCisternaDisplay();
}

void ReadCisternaSensors()
{
  bool state = IsCisternaEmpty();
  if (state != sensores.IsCisternaSensorMinVal)
  {
    sensores.IsCisternaSensorMinVal = state;
    UpdateCisternaDisplay();
  }
}

void ReadTanqueSensors()
{
  bool empty = IsTanqueEmpty();
  bool full = IsTanqueFull();

  if (empty != sensores.IsTanqueSensorMinVal || full != sensores.IsTanqueSensorMaxVal)
  {
    sensores.IsTanqueSensorMinVal = empty;
    sensores.IsTanqueSensorMaxVal = full;

    UpdateTanqueDisplay();
  }
}

boolean IsCisternaFull()
{
  return !IsCisternaEmpty();
}

boolean IsCisternaEmpty()
{
  static unsigned long startTime = 0;
  static boolean state;
  static boolean isPressed;

  IsButtonPressed("Cisterna Vacia", CISTERNA_EMPTY_PIN, state, isPressed, startTime);

  return isPressed;
}


boolean IsTanqueEmpty()
{
  static unsigned long startTime = 0;
  static boolean state;
  static boolean isPressed;

  IsButtonPressed("Tanque Vacio", TANQUE_EMPTY_FULL_PIN, state, isPressed, startTime);

  return isPressed;
}

boolean IsTanqueFull()
{
  return !IsTanqueEmpty();
}
