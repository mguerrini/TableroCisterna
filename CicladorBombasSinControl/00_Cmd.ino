// --- MODO ---
void SetupCommands()
{
}

void ReadCommands()
{
  if (Serial.available() > 0) {
    // read the incoming byte:
    String cmd = Serial.readString();

    Serial.print(F("Comando: "));
    Serial.println(cmd);

    cmd.trim();

    if (cmd.length() == 0)
      return;

    //"CAL_FASE_1 180" = CALIBRACION DE FASE 1, EL VALOR DE TENSION INGRESADO DE REFERENCIA ES 180 VOLTS.

    if (cmd.startsWith("CAL_FASE_1"))
    {
      calibrateFase(FASE1_INPUT_PIN, 1, cmd.substring(10));
    }
    else if (cmd.startsWith("CAL_FASE_2"))
    {
      calibrateFase(FASE2_INPUT_PIN, 2, cmd.substring(10));
    }
    else if (cmd.startsWith("CAL_FASE_3"))
    {
      calibrateFase(FASE3_INPUT_PIN, 3, cmd.substring(10));
    }
    else if (cmd == "DEL_FILL_DATA")
    {
      //ELIMINA LOS VALORES DE TIEMPO DE LLENADO DEL TAMQUE
      CleanFillTimes();
      //doSaveStatisctics(); //guarda el valor promedio de llenado
    }
    else if (cmd == "DEL_ST")
    {
      CleanStatistics(true);
    }
    else if (cmd == "S" || cmd == "s")
    {
      DoPrintStatus();
    }
    else if (cmd == "PRINT_STATISTICS")
    {
      printStatistics();
    }
    else
    {
      Serial.println(F("Comando invalido."));
    }
  }
}




// *************************************************** //
//                    SWAP BUTTON
// *************************************************** //


void ReadSwapButton()
{
  if (IsBombaSwapButtonPressed())
  {
    //no puedo cambiar con el boton cuando estan encendidas
    if (IsBombaOn(&bomba1) || IsBombaOn(&bomba2))
      return;

    SwapAndActiveBomba();
  }
}



// *************************************************** //
//                    RESET
// *************************************************** //


void ReadResetAndClearStatisticsButton()
{
  /*
    boolean resetBtn = IsResetButtonPressed();
    boolean clearBtn = IsCleanStatisticsButtonPressed();

    if (resetBtn || clearBtn)
    {
    Serial.print(F("Reset: "));
    if (resetBtn)
      Serial.print(F("TRUE "));
    else
      Serial.print(F("FALSE"));

    Serial.print(F(" - Clear: "));

    if (clearBtn)
      Serial.println(F("TRUE"));
    else
      Serial.println(F("FALSE"));
    }

    return;
  */
  if (IsResetButtonPressed())
  {
    Serial.println(F("RESET"));
    automaticFSM.FromState = AUTO_IDLE;
    automaticFSM.State = AUTO_IDLE;
    automaticFSM.NextState = AUTO_NULL;

    ResetBomba(&bomba1);
    ResetBomba(&bomba2);

    UpdateBomba1Display();
    UpdateBomba2Display();
    UpdateActiveBombaDisplay();
    UpdateCisternaDisplay();
    UpdateTanqueDisplay();
    UpdateDisplayMode();

    StopAllAlarms();
  }

  if (IsCleanStatisticsButtonPressed())
  {
    Serial.println(F("Clean Statistics"));
    CleanStatistics(true);
  }
}

boolean IsResetButtonPressed()
{
  static unsigned long startTime = millis();
  static boolean state = digitalRead(RESET_BTN_PIN);
  static boolean isPressed = false;

  return IsButtonPressedWithTimeRange(RESET_BTN_PIN, state, isPressed, startTime, 0, CLEAN_STADISTICS_PRESS_TIME);
}

boolean IsCleanStatisticsButtonPressed()
{
  static unsigned long startTime = millis();
  static boolean state = digitalRead(RESET_BTN_PIN);
  static boolean isPressed = false;

  return IsButtonPressedWithTimeRange(RESET_BTN_PIN, state, isPressed, startTime, CLEAN_STADISTICS_PRESS_TIME, 0);
}




// *************************************************** //
//                    DEBUG
// *************************************************** //

#ifdef DEBUG
boolean IsContinueButtonPressed()
{
  static unsigned long startTime = millis();
  static boolean state;
  static boolean isPressed;

  return IsButtonPressed(DEBUG_CONTINUE_PIN, state, isPressed, startTime);
}
#endif




// *************************************************** //
//                PRINT STATUS
// *************************************************** //

void DoPrintStatus()
{
  Serial.println();

  //Modo
  if (IsAutomaticMode())
    Serial.println(F("*** MODE: Automatic ***"));
  else
    Serial.println(F("*** MODE: Manual ***"));

  //Estado del proceso
  Serial.print(F("Automatic FSM Status: "));
  PrintStateWorkingFSM(automaticFSM.State);
  Serial.println();

  //Timer
  Serial.print(F("Stopping Timer: "));
  Serial.println(automaticFSM.StoppingTimer);
  Serial.println();

  //Niveles
  Serial.println(F("*** Niveles ***"));
  if (sensores.IsCisternaSensorMinVal)
    Serial.println(F("Cisterna Nivel Minimo: true"));
  else
    Serial.println(F("Cisterna Nivel Minimo: false"));

  Serial.print(F("Cisterna Empty Start Time: "));
  Serial.println(sensores.CisternaEmptyStartTime);
  Serial.print(F("Cisterna Empty Time (milisegundos): "));
  Serial.println(sensores.CisternaEmptyMillis);

  if (sensores.IsTanqueSensorMinVal)
    Serial.println(F("Tanque Nivel Minimo: true"));
  else
    Serial.println(F("Tanque Nivel Minimo: false"));

  if (sensores.IsTanqueSensorMaxVal)
    Serial.println(F("Tanque Nivel Maximo: true"));
  else
    Serial.println(F("Tanque Nivel Maximo: false"));

  Serial.println();

  PrintAlarm();

  //Bombas
  Serial.println(F("*** BOMBA 1 ***"));
  PrintBomba(&bomba1);
  Serial.println();

  Serial.println(F("*** BOMBA 2 ***"));
  PrintBomba(&bomba2);
  Serial.println();

  //Vista
  PrintView();

}

void PrintView()
{
  Serial.println(F("*** VIEW ****"));
  if (view.IsMainViewActive)
    Serial.println(F("Main View Active: True"));
  else
    Serial.println(F("Main View Active: False"));
  if (view.IsErrorFaseViewActive)
    Serial.println(F("Fase View Active: True"));
  else
    Serial.println(F("Fase View Active: False"));

  if (view.IsInfoViewActive)
    Serial.println(F("Info View Active: True"));
  else
    Serial.println(F("Info View Active: False"));

  Serial.print(F("Info View Active View Number: "));
  Serial.println(view.InfoViewNumberActive);
}

void PrintAlarm()
{
  Serial.println(F("*** Alarmas ***"));
  if (alarm.IsManualAlarmON)
    Serial.println(F("Manual: ON"));
  else
    Serial.println(F("Manual: OFF"));


  if (alarm.IsNotAvailableBombasAlarmON)
    Serial.println(F("Bombas no disponibles: ON"));
  else
    Serial.println(F("Bombas no disponibles: OFF"));

  if (alarm.IsBomba1AlarmON)
    Serial.println(F("Bomba 1: ON"));
  else
    Serial.println(F("Bomba 1: OFF"));

  if (alarm.IsBomba2AlarmON)
    Serial.println(F("Bomba 2: ON"));
  else
    Serial.println(F("Bomba 2: OFF"));

  if (alarm.IsCisternaAlarmON)
    Serial.println(F("Cisterna: ON"));
  else
    Serial.println(F("Cisterna: OFF"));

  Serial.println();
}

void PrintBomba(Bomba* bomba)
{
  if (bomba->IsEnabled)
    Serial.println(F("IsEnabled: true"));
  else
    Serial.println(F("IsEnabled: false"));

  if (bomba->IsActive)
    Serial.println(F("IsActive: true"));
  else
    Serial.println(F("IsActive: false"));

  Serial.print(F("State: "));
  PrintStateBomba(bomba, true);

  Serial.print(F("Machine Status: "));
  PrintStateBombaFSM(bomba->MachineState);
  Serial.println();

  Serial.print(F("Uses: "));
  Serial.println(bomba->Uses);

  //  Serial.print(F("Timer: "));
  //  Serial.println(bomba->Timer);

  Serial.print(F("Contactor Error Counter: "));
  Serial.println(bomba->ContactorErrorCounter);

  Serial.print(F("Tiempo de llenado: "));
  Serial.println(bomba->FillTimeMinutesAverage);

  Serial.print(F("Tiempos de llenado: "));
  for (int i = 0; i < 9; i++)
  {
    Serial.print(bomba->FillTimeMinutes[i]);
    Serial.print(F(", "));
  }
  Serial.println(bomba->FillTimeMinutes[9]);

  if (bomba->IsContactorClosed)
    Serial.println(F("IsContactorClosed: true"));
  else
    Serial.println(F("IsContactorClosed: false"));

  if (bomba->IsTermicoOk)
    Serial.println(F("IsTermicoOk: true"));
  else
    Serial.println(F("IsTermicoOk: false"));

  if (bomba->RequestOn)
    Serial.println(F("RequestOn: true"));
  else
    Serial.println(F("RequestOn: false"));

  if (bomba->RequestOff)
    Serial.println(F("RequestOff: true"));
  else
    Serial.println(F("RequestOff: false"));

  if (bomba->RequestEnabled)
    Serial.println(F("RequestEnabled: true"));
  else
    Serial.println(F("RequestEnabled: false"));

  if (bomba->RequestDisabled)
    Serial.println(F("RequestDisabled: true"));
  else
    Serial.println(F("RequestDisabled: false"));

}

void PrintStateBomba(Bomba* bomba, bool newLine)
{
  switch (bomba->State)
  {
    case BOMBA_STATE_ON:
      Serial.print(F("BOMBA_STATE_ON"));
      break;
    case BOMBA_STATE_OFF:
      Serial.print(F("BOMBA_STATE_OFF"));
      break;
    case BOMBA_STATE_ERROR_CONTACTOR_ABIERTO:
      Serial.print(F("BOMBA_STATE_ERROR_CONTACTOR_ABIERTO"));
      break;
    case BOMBA_STATE_ERROR_CONTACTOR_CERRADO:
      Serial.print(F("BOMBA_STATE_ERROR_CONTACTOR_CERRADO"));
      break;
    case BOMBA_STATE_ERROR_TERMICO:
      Serial.print(F("BOMBA_STATE_ERROR_TERMICO"));
      break;
  }

  if (newLine)
    Serial.println();
}


long mapLocal(float value, float in_min, float in_max, float out_min, float out_max)
{
  float v = (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
  float output = round(v);

  /*
    Serial.print("Map Value: ");
    Serial.print(v);
    Serial.print(" ");
    Serial.print(output);
  */
  return output;
}

// *************************************************** //
//                    READ EEPROM
// *************************************************** //

/*

  long EEPROMReadlong(long address) {
  long four = EEPROM.read(address);
  long three = EEPROM.read(address + 1);
  long two = EEPROM.read(address + 2);
  long one = EEPROM.read(address + 3);

  return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
  }

  void EEPROMWritelong(int address, long value) {
  byte four = (value & 0xFF);
  byte three = ((value >> 8) & 0xFF);
  byte two = ((value >> 16) & 0xFF);
  byte one = ((value >> 24) & 0xFF);

  EEPROM.write(address, four);
  EEPROM.write(address + 1, three);
  EEPROM.write(address + 2, two);
  EEPROM.write(address + 3, one);
  }

*/

// *************************************************** //
//                    ANALOG BUTTONS
// *************************************************** //
/*
  #define BTN_ANALOG_PRESSED_TIME 20
  #define ANALOG_BUTTONS_COUNT 7
  #define ANALOG_BUTTONS_PIN A3


  int currentValue;

  int buttonStep = 1023 / ANALOG_BUTTONS_COUNT;

  unsigned long startTime = 0;

  boolean AnalogButtons_State[ANALOG_BUTTONS_COUNT];
  boolean AnalogButtons_IsPressed[ANALOG_BUTTONS_COUNT];
  unsigned long AnalogButtons_StartTime[ANALOG_BUTTONS_COUNT];

  void SetupAnalogButtons()
  {
  Serial.println("Analog buttons Setup");
  for (int i = 0; i < ANALOG_BUTTONS_COUNT; i++)
  {
    AnalogButtons_State[i] = false;
    AnalogButtons_IsPressed[i] = false;
    AnalogButtons_StartTime[i] = 0;
  }
  }


  void ReadAnalogButtons()
  {
  currentValue = analogRead(ANALOG_BUTTONS_PIN);
  Serial.print(F("Analog Buttons Readed Value: "));
  Serial.println(currentValue);
  delay(500);

  for (int i=0; i<ANALOG_BUTTONS_COUNT; i++)
  {
    boolean b = IsAnalogButtonPressed(i);
  }
  }

  boolean IsAnalogButtonPressed(int number)
  {
  return IsAnalogButtonPressed(number, currentValue);
  }

  //Cada vez que se presiona el botón devuelve true y luego false. Mientras se mantenga presionado isPressed es true.
  boolean IsAnalogButtonPressed(int buttonNumber, int currentValue)
  {
  boolean state = AnalogButtons_State[buttonNumber];
  boolean isPressed = AnalogButtons_IsPressed[buttonNumber];
  unsigned long startTime = AnalogButtons_StartTime[buttonNumber];

  int middleValue = buttonStep * buttonNumber;
  int minValue = middleValue + 10;
  int maxValue = middleValue - 10;

  //estado del boton presionado
  boolean currState = minValue <= currentValue && currentValue <= maxValue;

  //verifico si cambio el estado...y registro el inicio del cambios
  if (currState != state)
  {
    state = !state;
    startTime = millis();
    isPressed = false;

    AnalogButtons_State[buttonNumber] = state;
    AnalogButtons_IsPressed[buttonNumber] = isPressed;
    AnalogButtons_StartTime[buttonNumber] = startTime;
  }

  //state == HIGH -> Boton presionado
  if (state == HIGH && !isPressed) {
    unsigned long delta = deltaMillis(millis(), startTime);
    boolean output = delta > BTN_ANALOG_PRESSED_TIME;

    if (output)
    {
      isPressed = true;
      AnalogButtons_IsPressed[buttonNumber] = isPressed;
    }

    return output;
  }
  else
  {
    return false;
  }
  }
*/
