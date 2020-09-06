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

    cmd.toUpperCase();

    //"CAL_FASE_1 180" = CALIBRACION DE FASE 1, EL VALOR DE TENSION INGRESADO DE REFERENCIA ES 180 VOLTS.

    if (cmd.startsWith("CAL F1"))
    {
      CalibrateFase(FASE1_INPUT_PIN, 1, cmd.substring(6));
    }
    else if (cmd.startsWith("CAL F2"))
    {
      CalibrateFase(FASE2_INPUT_PIN, 2, cmd.substring(6));
    }
    else if (cmd.startsWith("CAL F3"))
    {
      CalibrateFase(FASE3_INPUT_PIN, 3, cmd.substring(6));
    }
    else if (cmd.startsWith("SET CICLOS"))
    {
      SetCiclos(cmd.substring(10));
      Serial.println(F("Ciclos modificados"));
    }
    else if (cmd.startsWith("CICLOS"))
    {
      Serial.print(F("Ciclos Maximo: "));
      Serial.println(automaticFSM.CiclosMax);
    }
    else if (cmd == "DEL FILLTIME")
    {
      //ELIMINA LOS VALORES DE TIEMPO DE LLENADO DEL TAMQUE
      CleanFillTimes();
      DoSaveStatistics(); //guardo los valores de llenada
      Serial.println(F("Tiempos de llenado borrados"));
    }
    else if (cmd == "ST")
    {
      DoPrintStatus();
    }
    else if (cmd == "STD")
    {
      PrintStatistics();
    }
    else if (cmd == "DEL STD")
    {
      //elimina las estadisticas y las guarda los valores
      CleanStatistics(true);
      Serial.println(F("Estadisticas borradas"));
    }
    else
    {
      Serial.println(F("Comando invalido."));
    }
  }
}


// *************************************************** //
//                    VIEW INFO BUTTON
// *************************************************** //

void ReadInfoViewButton()
{
  if (IsInfoViewButtonPressed())
  {
#ifdef LOG_ENABLED
    Serial.println(F("Show Info View "));
#endif
    ShowInfoView();
  }
}


// *************************************************** //
//                    SWAP BUTTON
// *************************************************** //

void ReadSwapButton()
{
  #ifdef BOMBA_SWAP_BTN_ENABLED
  if (IsBombaSwapButtonPressed())
  {
    //no puedo cambiar con el boton cuando estan encendidas
    if (IsBombaOn(&bomba1) || IsBombaOn(&bomba2))
      return;

    SwapAndActiveBomba();
  }
  #endif
}


// *************************************************** //
//                    RESET
// *************************************************** //

void ReadResetAndClearStatisticsButton()
{
  if (IsResetButtonPressed())
  {
#ifdef LOG_ENABLED
    Serial.println(F("RESET"));
#endif    
    automaticFSM.FromState = AUTO_IDLE;
    automaticFSM.State = AUTO_IDLE;
    automaticFSM.NextState = AUTO_NULL;

    ResetBomba(&bomba1);
    ResetBomba(&bomba2);

    UpdateBombaDisplay(&bomba1);
    UpdateBombaDisplay(&bomba2);

    UpdateActiveBombaDisplay();
    UpdateCisternaDisplay();
    UpdateTanqueDisplay();
    UpdateDisplayMode();

    StopAllAlarms();
  }

  if (IsCleanStatisticsButtonPressed())
  {
#ifdef LOG_ENABLED
    Serial.println(F("Clean Statistics"));
#endif
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

boolean IsInfoViewButtonPressed()
{
  static unsigned long startTime = millis();
  static boolean state = digitalRead(VIEW_INFO_PIN);
  static boolean isPressed = false;

  return IsButtonPressed(BTN_PRESSED_TIME, VIEW_INFO_PIN, state, isPressed, startTime);
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

  return IsButtonPressed(BTN_PRESSED_TIME, DEBUG_CONTINUE_PIN, state, isPressed, startTime);
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
    Serial.println(F("** MODO: Automatic **"));
  else
    Serial.println(F("** MODO: Manual **"));

  //Estado del proceso
  Serial.print(F("FSM Status: "));
  PrintStateWorkingFSM(automaticFSM.State);
  Serial.println();

  //Timer
  Serial.print(F("Stopping Timer: "));
  Serial.println(automaticFSM.StoppingTimer);
  Serial.println();

  //Niveles
  Serial.println(F("** Niveles **"));
  Serial.print(F("Cisterna Minimo: "));
  PrintTrueOrFalse (sensores.IsCisternaSensorMinVal);

  Serial.print(F("Cisterna Empty Start Time: "));
  Serial.println(sensores.CisternaEmptyStartTime);

  Serial.print(F("Cisterna Empty Time: "));
  Serial.println(sensores.CisternaEmptyMillis);

  Serial.print(F("Tanque Minimo: "));
  PrintTrueOrFalse (sensores.IsTanqueSensorMinVal);

  Serial.print(F("Tanque Maximo: "));
  PrintTrueOrFalse (sensores.IsTanqueSensorMaxVal);

  Serial.println();

  //Bombas
  Serial.println(F("** B1 **"));
  PrintBomba(&bomba1);
  Serial.println();

  Serial.println(F("** B2 **"));
  PrintBomba(&bomba2);
  Serial.println();

  //Vista
  //PrintView();
  //Serial.println();

  PrintAlarm();
  Serial.println();

  PrintFases();
  Serial.println();

}
/*
void PrintView()
{
  Serial.println(F("*** VIEW ****"));
  Serial.print(F("Main View: "));
  PrintTrueOrFalse (view.IsMainViewActive);

  Serial.print(F("Fase View: "));
  PrintTrueOrFalse (view.IsErrorFaseViewActive);

  Serial.print(F("Info View: "));
  PrintTrueOrFalse(view.IsInfoViewActive);

  Serial.print(F("Info View Number: "));
  Serial.println(view.InfoViewNumberActive);
}
*/
void PrintAlarm()
{
  Serial.println(F("*** Alarmas ***"));
  Serial.print(F("Manual: "));
  PrintOnOrOff (alarm.IsManualAlarmON);

  Serial.print(F("Bombas no disponibles: "));
  PrintOnOrOff (alarm.IsNotAvailableBombasAlarmON);

  Serial.print(F("B1: "));
  PrintOnOrOff (alarm.IsBomba1AlarmON);

  Serial.print(F("B2: "));
  PrintOnOrOff (alarm.IsBomba2AlarmON);

  Serial.print(F("Cisterna: "));
  PrintOnOrOff (alarm.IsCisternaAlarmON);

  Serial.println();
}

void PrintBomba(Bomba* bomba)
{
  Serial.print(F("Enabled: "));
  PrintTrueOrFalse (bomba->IsEnabled);

  Serial.print(F("Active: "));
  PrintTrueOrFalse (bomba->IsActive);

  Serial.print(F("State: "));
  PrintStateBomba(bomba, true);

  Serial.print(F("MachineState: "));
  PrintStateBombaFSM(bomba->MachineState);
  Serial.println();

  Serial.print(F("StartTime: "));
  Serial.println(bomba->StartTime);

  Serial.print(F("RefreshTime: "));
  Serial.println(bomba->RefreshTime);

  Serial.print(F("Uses: "));
  Serial.println(bomba->Uses);

  Serial.print(F("#ContactorErr: "));
  Serial.println(bomba->ContactorErrorCounter);

  Serial.print(F("Fill Time: "));
  Serial.println(bomba->FillTimeSecondsAverage);

  Serial.print(F("Fill Times: "));
  for (int i = 0; i < BOMBA_FILLTIMES_READ_MAX-1; i++)
  {
    Serial.print(bomba->FillTimeSeconds[i]);
    Serial.print(F(", "));
  }
  Serial.println(bomba->FillTimeSeconds[BOMBA_FILLTIMES_READ_MAX-1]);

  Serial.print(F("ContactorClosed: "));
  PrintTrueOrFalse (bomba->IsContactorClosed);

  Serial.print(F("TermicoOk: "));
  PrintTrueOrFalse (bomba->IsTermicoOk);

  Serial.print(F("ReqOn: "));
  PrintTrueOrFalse (bomba->RequestOn);

  Serial.print(F("ReqOff: "));
  PrintTrueOrFalse(bomba->RequestOff);

  Serial.print(F("ReqEnabled: "));
  PrintTrueOrFalse (bomba->RequestEnabled);

  Serial.print(F("ReqDisabled: "));
  PrintTrueOrFalse(bomba->RequestDisabled);

  Serial.print(F("Timer: "));
  Serial.println(bomba->Timer);

}

void PrintStateBomba(Bomba* bomba, bool newLine)
{
  switch (bomba->State)
  {
    case BOMBA_STATE_ON:
      Serial.print(F("ON"));
      break;
    case BOMBA_STATE_OFF:
      Serial.print(F("OFF"));
      break;
    case BOMBA_STATE_ERROR_CONTACTOR_ABIERTO:
      Serial.print(F("ERR_CONTACTOR_ABIERTO"));
      break;
    case BOMBA_STATE_ERROR_CONTACTOR_CERRADO:
      Serial.print(F("ERR_CONTACTOR_CERRADO"));
      break;
    case BOMBA_STATE_ERROR_TERMICO:
      Serial.print(F("ERR_TERMICO"));
      break;
  }

  if (newLine)
    Serial.println();
}

void PrintFases()
{
  Serial.print(F("F1: "));
  Serial.println(fase1.IsOk);
  Serial.print(F("F2: "));
  Serial.println(fase2.IsOk);
  Serial.print(F("F3: "));
  Serial.println(fase3.IsOk);
}


// *************************************************** //
//                AUXILIARES
// *************************************************** //

void convertSeconds2HMS(unsigned long totalSec, int &h, int &m, int &s)
{
  s = totalSec % 60;

  totalSec = (totalSec - s) / 60;
  m = totalSec % 60;

  totalSec = (totalSec - m) / 60;
  h = totalSec;
}

float mapLocal(float value, float in_min, float in_max, float out_min, float out_max)
{
  float v = (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
  //  float output = round(v);

  /*
    Serial.print("Map Value: ");
    Serial.print(v);
    Serial.print(" ");
    Serial.print(output);
  */
  return v;
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


unsigned long deltaMillis(unsigned long currRead, unsigned long prevRead)
{
  if (currRead >= prevRead)
    return currRead - prevRead; // prevRead ----- currRead ----- maxValue
  else
    return (4294967295 - prevRead) + currRead; //volvio a cero: prevRead -------- maxValue - 0 ------- currRead
}

byte GetLen(unsigned long value)
{
  if (value < 10)
    return 1;
  else if (value < 100)
    return 2;
  else if (value < 1000)
    return 3;
  else if (value < 10000)
    return 4;
  else if (value < 100000)
    return 5;
  else if (value < 1000000)
    return 6;
  else if (value < 10000000)
    return 7;
  else if (value < 100000000)
    return 8;
  else if (value < 1000000000)
    return 9;
  else if (value < 10000000000)
    return 10;
  else
    return 11;
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
