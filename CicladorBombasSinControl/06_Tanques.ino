// ****************************************************************** //
//                          SETUP
// ****************************************************************** //

void SetupLevelSensors()
{
  //inicializo las variables de los sensores
  IsTanqueEmpty();
  IsCisternaEmpty();

  //espero a que se estabilice
  delay(BTN_PRESSED_TIME * 2);

  //leo el valor definitivo
  sensores.IsCisternaSensorMinVal = IsCisternaEmpty();
  sensores.IsTanqueSensorMinVal = IsTanqueEmpty();
  sensores.IsTanqueSensorMaxVal = IsTanqueFull();
}

// ****************************************************************** //
//                          READ
// ****************************************************************** //

void ReadCisternaSensors()
{
  bool state = IsCisternaEmpty();
  if (state != sensores.IsCisternaSensorMinVal)
  {
    sensores.IsCisternaSensorMinVal = state;
    UpdateCisternaDisplay();

    sensores.CisternaEmptyStartTime = millis();

#ifdef LOG_ENABLED
    if (sensores.IsCisternaSensorMinVal)
      Serial.println(F("Cisterna: Vacia."));
    else
      Serial.println(F("Cisterna: Normal."));
#endif
    
  }

  if (sensores.IsCisternaSensorMinVal)
  {
    sensores.CisternaEmptyMillis = deltaMillis(millis(), sensores.CisternaEmptyStartTime);

    if (sensores.CisternaEmptyMillis > CISTERNA_EMPTY_MAX_TIME)
    {
      StartCisternaEmptyAlarm();
    }
  }
  else
  {
    sensores.CisternaEmptyStartTime = 0;
    sensores.CisternaEmptyMillis = 0;

    StopCisternaEmptyAlarm();
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

#ifdef LOG_ENABLED
    if (full)
      Serial.println(F("Tanque: Lleno."));
    else
      Serial.println(F("Tanque: Vacío."));
#endif      
  }
}

// ****************************************************************** //
//                          PROPERTIES
// ****************************************************************** //

boolean IsCisternaFull()
{
  return !IsCisternaEmpty();
}

boolean IsCisternaEmpty()
{
  static unsigned long startTime = millis();
  static boolean state = digitalRead(CISTERNA_EMPTY_PIN);
  static boolean isPressed;

  IsButtonPressed(CISTERNA_EMPTY_PIN, state, isPressed, startTime);

  return isPressed;
}


boolean IsTanqueEmpty()
{
  static unsigned long startTime = millis();
  static boolean state = digitalRead(TANQUE_EMPTY_FULL_PIN);
  static boolean isPressed;

  IsButtonPressed(TANQUE_EMPTY_FULL_PIN, state, isPressed, startTime);

  return isPressed;
}

boolean IsTanqueFull()
{
  return !IsTanqueEmpty();
}
