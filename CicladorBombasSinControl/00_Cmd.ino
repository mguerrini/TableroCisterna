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
      PrintStatistics();
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
  Serial.print(F("Cisterna Nivel Minimo: "));
  PrintTrueOrFalse (sensores.IsCisternaSensorMinVal);

  Serial.print(F("Cisterna Empty Start Time: "));
  Serial.println(sensores.CisternaEmptyStartTime);

  Serial.print(F("Cisterna Empty Time (milisegundos): "));
  Serial.println(sensores.CisternaEmptyMillis);

  Serial.print(F("Tanque Nivel Minimo: "));
  PrintTrueOrFalse (sensores.IsTanqueSensorMinVal);

  Serial.print(F("Tanque Nivel Maximo: "));
  PrintTrueOrFalse (sensores.IsTanqueSensorMaxVal);

  Serial.println();

  //Bombas
  Serial.println(F("*** BOMBA 1 ***"));
  PrintBomba(&bomba1);
  Serial.println();

  Serial.println(F("*** BOMBA 2 ***"));
  PrintBomba(&bomba2);
  Serial.println();

  //Vista
  PrintView();
  Serial.println();

  PrintAlarm();
  Serial.println();
}

void PrintView()
{
  Serial.println(F("*** VIEW ****"));
  Serial.print(F("Main View Active: "));
  PrintTrueOrFalse (view.IsMainViewActive);

  Serial.print(F("Fase View Active: "));
  PrintTrueOrFalse (view.IsErrorFaseViewActive);

  Serial.print(F("Info View Active: "));
  PrintTrueOrFalse(view.IsInfoViewActive);

  Serial.print(F("Info View Active View Number: "));
  Serial.println(view.InfoViewNumberActive);
}

void PrintAlarm()
{
  Serial.println(F("*** Alarmas ***"));
  Serial.print(F("Manual: "));
  PrintOnOrOff (alarm.IsManualAlarmON);

  Serial.print(F("Bombas no disponibles: "));
  PrintOnOrOff (alarm.IsNotAvailableBombasAlarmON);

  Serial.print(F("Bomba 1: "));
  PrintOnOrOff (alarm.IsBomba1AlarmON);

  Serial.print(F("Bomba 2: "));
  PrintOnOrOff (alarm.IsBomba2AlarmON);

  Serial.print(F("Cisterna: "));
  PrintOnOrOff (alarm.IsCisternaAlarmON);

  Serial.println();
}

void PrintBomba(Bomba* bomba)
{
  Serial.print(F("IsEnabled: "));
  PrintTrueOrFalse (bomba->IsEnabled);

  Serial.print(F("IsActive: "));
  PrintTrueOrFalse (bomba->IsActive);

  Serial.print(F("State: "));
  PrintStateBomba(bomba, true);

  Serial.print(F("Machine Status: "));
  PrintStateBombaFSM(bomba->MachineState);
  Serial.println();

  Serial.print(F("Start Time: "));
  Serial.println(bomba->StartTime);

  Serial.print(F("Refresh Time: "));
  Serial.println(bomba->RefreshTime);

  Serial.print(F("Uses: "));
  Serial.println(bomba->Uses);

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

  Serial.print(F("IsContactorClosed: "));
  PrintTrueOrFalse (bomba->IsContactorClosed);

  Serial.print(F("IsTermicoOk: "));
  PrintTrueOrFalse (bomba->IsTermicoOk);

  Serial.print(F("RequestOn: "));
  PrintTrueOrFalse (bomba->RequestOn);

  Serial.print(F("RequestOff: "));
  PrintTrueOrFalse(bomba->RequestOff);

  Serial.print(F("RequestEnabled: "));
  PrintTrueOrFalse (bomba->RequestEnabled);

  Serial.print(F("RequestDisabled: "));
  PrintTrueOrFalse(bomba->RequestDisabled);

  Serial.print(F("Timer: "));
  Serial.println(bomba->Timer);

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


void PrintTrueOrFalse(boolean input)
{
  if (input)
    Serial.println(F("True"));
  else
    Serial.println(F("False"));
}


void PrintOnOrOff(boolean input)
{
  if (input)
    Serial.println(F("ON"));
  else
    Serial.println(F("OFF"));
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
