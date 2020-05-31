// --- DISPLAY ---
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>


// --- DISPLAY ---
#define OLED_ADDR   0x3C

Adafruit_SSD1306 display(-1);

// --- BOTONERA ---
const byte LED_PIN = 13;

const long BTN_PRESSED_TIME = 20; //20 milisegundos de boton presionado, para evitar rebote

const byte CHANGE_MODE_BTN_PIN = 4;
const byte CHANGE_ACTIVE_BTN_PIN = 12;
const byte BOMBA1_ON_OFF_BTN_PIN = 11;
const byte BOMBA2_ON_OFF_BTN_PIN = 11;
const byte BOMBA1_ENABLE_PIN = 10;
const byte BOMBA2_ENABLE_PIN = 9;

//funciones con referencias
boolean IsButtonPressed(const char* buttonName, int pin, boolean &state, unsigned long &startTime);

// --- NIVELES ---
const byte CISTERNA_EMPTY_PIN = 5;
const byte TANQUE_EMPTY_PIN = 6;
const byte TANQUE_FULL_PIN = 7;

// --- ESTADOS ---
const byte MANUAL = 0;
const byte AUTO = 1;
const byte NONE = 0;
const byte BOMBA1 = 1;
const byte BOMBA2 = 2;
const bool ENABLED = true;
const bool DISABLED = false;
const int ON = 1;
const int OFF = 0;
const int ERROR_CONTACTOR = -1;
const int ERROR_TERMICO = -2;

byte _mode = 0; //0 = Manual, 1=Autmatico

// --- BOMBAS ---
const byte BOMBA_USES_MAX = 1;
byte _bombaUses = 0;
byte _bombaActiva = 1;

bool _bomba1Enabled = true; //0=DISABLED 1=ENABLED
bool _bomba2Enabled = true;

int _bomba1State = 1; //0=OFF 1=ON -1=ERROR CONTACTOR -2=ERROR TERMICO -3=ERROR BOMBA (ESTE NO ESTA EN FUNCIONAMIENTO TODAVIA, FALTARIA UN SENSOR EN LA BOMBA QUE DETECTE FUNCIONAMIENTO)
int _bomba2State = 1;

bool _isCisternaSensorMinVal = false;
bool _isTanqueSensorMinVal = false;
bool _isTanqueSensorMaxVal = false;

// --- SENSORES ---
const byte BOMBA1_CONTACTOR_RETORNO_PIN = A2;
const byte BOMBA2_CONTACTOR_RETORNO_PIN = A3;

const byte BOMBA1_TERMICO_RETORNO_PIN = A0;
const byte BOMBA2_TERMICO_RETORNO_PIN = A1;

// --- SALIDAS ---
const byte BOMBA1_CONTACTOR_PIN = A6;
const byte BOMBA2_CONTACTOR_PIN = A7;

//--- ALARMA ---
const byte ALARM_PIN = 0;

//--- MAQUINAS DE ESTADO ---

byte _modoMahcineState = AUTO;
//const byte AUTO
//const byte MANUAL
const byte AUTO_TO_MANUAL = 2;
const byte MANUAL_TO_AUTO = 3;

//--- MODO AUTOMATICO ESTADOS ---
byte _automaticMachineState = AUTO_IDLE;

const byte AUTO_IDLE = 0;
const byte AUTO_STARTING_BOMBA = 1;
const byte AUTO_SELECTING_BOMBA = 2;
const byte AUTO_CHANGING_BOMBA = 3;
const byte AUTO_NOT_AVAILABLES_BOMBAS = 4;

byte _manualMachineState;




//**************************************************//
//                     SETUP
//**************************************************//
void setup() {
  //start serial connection
  Serial.begin(9600);

  SetupPins();

  // put your setup code here, to run once:
  SetupDisplay();

  SetupLevels();

  SetupBombas();

  SetupSensors();

}

//************************************************//
//                     LOOP
//************************************************//

void loop() {
  //valido los niveles para visualizar en el display
  CheckTanque();

  CheckCisterna();

  CheckEnabledBombas();

  // put your main code here, to run repeatedly:
  if (IsAutomaticMode())
  {
    AutomaticLoop();
  }
  else
  {
    ManualLoop();
  }
}

void AutomaticLoop()
{
  //valido si cambio el modo
  if (IsChangeModeButtonPressed())
  {
    ChangeToManualMode();
    return;
  }

  //const byte AUTO_IDLE = 0;
  //const byte AUTO_SELECTING_BOMBA = 2;
  //const byte AUTO_STARTING_BOMBA = 1;
  //const byte AUTO_CHANGING_BOMBA = 3;
  //const byte AUTO_NOT_AVAILABLES_BOMBAS = 3;

  //Valido si cambio la bomba activa.
  CheckActiveBomba();

  //al estar en automatico el boton lo unico que hace es limpiar el error y ponerla bomba en off
  CheckControlManualBombas(true);

  switch (_automaticState)
  {
    case AUTO_IDLE:
      //verifico los niveles
      if (IsTanqueEmpty() && IsCisternaFull())
        ChangeAutomaticState(AUTO_SELECTING_BOMBA);
      break;

    case AUTO_SELECTING_BOMBA:
      Automatic_SelectBomba();
      break;

    case AUTO_NOT_AVAILABLES_BOMBAS:
      break;

    case AUTO_STARTING_BOMBA:
      break;

    case AUTO_CHANGING_BOMBA:
      //detengo todas las bombas
      StopBombas();

      ChangeAutomaticState(AUTO_IDLE);
      break;

  }
}

void ManualLoop()
{
  if (IsChangeModeButtonPressed())
  {
    ChangeToAutomaticMode();
    return;
  }
}

void ChangeAutomaticState(byte state)
{
  _automaticState = state;
}

void Automatic_SelectBomba()
{
  //valido si existe una bomba disponible
  if (!IsBombaAvailable(BOMBA1) && !IsBombaAvailable(BOMBA2))
  {
    UpdateActiveBombaDisplay(NONE);
    ChangeAutomaticState(AUTO_NOT_AVAILABLES_BOMBAS);
    return;
  }

  //me fijo si estan las dos en estado de error
  bool updateActiveBomba = false;

  if (_bombaUses >= BOMBA_USES_MAX)
  {
    //cambio de bomba
    if (_bombaActiva == BOMBA1)
      _bombaActiva = BOMBA2;
    else
      _bombaActiva = BOMBA1;

    updateActiveBomba = true;
  }
  else
    _bombaUses++;

}

//************************************************//
//                   AUXILIARES
//************************************************//

// --- PINS ---

void SetupPins()
{
  //Salidas contactores
  pinMode(BOMBA1_CONTACTOR_PIN, OUTPUT);
  pinMode(BOMBA2_CONTACTOR_PIN, OUTPUT);

  //sensores: contatores, termicos
  pinMode(BOMBA1_CONTACTOR_RETORNO_PIN, INPUT_PULLUP);
  pinMode(BOMBA2_CONTACTOR_RETORNO_PIN, INPUT_PULLUP);
  pinMode(BOMBA1_TERMICO_RETORNO_PIN, INPUT_PULLUP);
  pinMode(BOMBA2_TERMICO_RETORNO_PIN, INPUT_PULLUP);

  //Niveles
  pinMode(CISTERNA_EMPTY_PIN, INPUT_PULLUP);
  pinMode(TANQUE_EMPTY_PIN, INPUT_PULLUP);
  pinMode(TANQUE_FULL_PIN, INPUT_PULLUP);

  //Botonera
  pinMode(CHANGE_MODE_BTN_PIN, INPUT_PULLUP);
  pinMode(CHANGE_ACTIVE_BTN_PIN, INPUT_PULLUP);
  pinMode(BOMBA1_ON_OFF_BTN_PIN, INPUT_PULLUP);
  pinMode(BOMBA2_ON_OFF_BTN_PIN, INPUT_PULLUP);
  pinMode(BOMBA1_ENABLE_PIN, INPUT_PULLUP);
  pinMode(BOMBA2_ENABLE_PIN, INPUT_PULLUP);

  //testigo de operacion: led
  pinMode(LED_PIN, OUTPUT);
  //lo apago
  digitalWrite(LED_PIN, LOW);
}

//--- Contactores / Termicos ---
void SetupSensors()
{
  //no hago nada..en principio
}

//--- Bombas ---

void SetupBombas()
{
  //Read el estado enabled/disabled from EEPROM
  _bomba1Enabled = ENABLED;
  _bomba2Enabled = ENABLED;

  _bomba1State = OFF;
  _bomba2State = OFF;

  UpdateBomba1Display();
  UpdateBomba2Display();
}


void CheckEnabledBombas()
{
  bool pressed = IsBomba1EnableButtonPressed();
  if (pressed)
  {
    _bomba1Enabled = !_bomba1Enabled;
    UpdateBomba1Display();

    if (!IsBombaEnabled(BOMBA1))
    {
      if (IsBombaError(BOMBA1)) //estaba en error
      {
        StopAlarm(); //freno la alarma porque ya atendi el error.
      }

      //detengo la bomba
      StopBomba1();
    }
  }

  pressed = IsBomba2EnableButtonPressed();
  if (pressed)
  {
    _bomba2Enabled = !_bomba1Enabled;
    UpdateBomba2Display();

    if (!IsBombaEnabled(BOMBA2))
    {
      if (IsBombaError(BOMBA2)) //estaba en error
      {
        StopAlarm(); //freno la alarma porque ya atendi el error.
      }

      //detengo la bomba
      StopBomba2();
    }
  }
}

void CheckControlManualBombas(bool automaticMode)
{
  if (automaticMode)
  {
    //si la bomba esta en error la pone en OFF. Sino no hace nada.
  }
  else
  {
    //Si la bomba esta en OFF la prende...sino la apaga
  }
}

bool IsBombaOff(byte bombaNumber)
{
  if (bombaNumber == BOMBA1)
    return _bomba1State == OFF;
  else
    return _bomba2State == OFF;
}

bool IsBombaOn(byte bombaNumber)
{
  if (bombaNumber == BOMBA1)
    return _bomba1State == ON;
  else
    return _bomba2State == ON;
}

bool IsBombaEnabled(byte bombaNumber)
{
  if (bombaNumber == BOMBA1)
    return _bomba1Enabled;
  else
    return _bomba2Enabled;
}

bool IsBombaError(byte bombaNumber)
{
  if (bombaNumber == BOMBA1)
    return _bomba1State >= 0;
  else
    return _bomba2State >= 0;
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


void UpdateBomba1Display()
{
  UpdateBombaDisplay(_bomba1Enabled, _bomba1State, 0);
}

void UpdateBomba2Display()
{
  UpdateBombaDisplay(_bomba2Enabled, _bomba2State, 8);
}

void UpdateBombaDisplay(bool enabled, int state, int row)
{
  display.setTextColor(WHITE, BLACK);
  display.setTextSize(1);
  display.setCursor(42, row);

  if (enabled == DISABLED)
    display.print("Deshabilitada");
  else
  {
    if (state == ON)
      display.print("ON");
    else if (state == OFF)
      display.print("OFF");
    else if (state == ERROR_CONTACTOR)
      display.print("Error contactor");
    else if (state == ERROR_TERMICO)
      display.print("Error termico");
  }

  display.display();
}

//--- Sensores Tanque y Cisterna ---

void SetupLevels()
{
  _isCisternaSensorMinVal = IsCisternaEmpty();
  _isTanqueSensorMinVal = IsTanqueEmpty();
  _isTanqueSensorMaxVal = IsTanqueFull();

  UpdateTanqueDisplay();
  UpdateCisternaDisplay();
}

void CheckCisterna()
{
  bool state = IsCisternaEmpty();
  if (state != _isCisternaSensorMinVal)
  {
    _isCisternaSensorMinVal = state;
    UpdateCisternaDisplay();
  }
}

void CheckTanque()
{
  bool empty = IsTanqueEmpty();
  bool full = IsTanqueFull();

  if (empty != _isTanqueSensorMinVal || full != _isTanqueSensorMaxVal)
  {
    _isTanqueSensorMinVal = empty;
    _isTanqueSensorMaxVal = full;

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

  IsButtonPressed("Tanque Vacio", TANQUE_EMPTY_PIN, state, isPressed, startTime);

  return isPressed;
}

boolean IsTanqueFull()
{
  static unsigned long startTime = 0;
  static boolean state;
  static boolean isPressed;

  IsButtonPressed("Tanque Full", TANQUE_FULL_PIN, state, isPressed, startTime);

  return isPressed;
}

void UpdateCisternaDisplay()
{
  display.setTextColor(WHITE, BLACK);
  display.setTextSize(1);

  display.setCursor(54, 24);
  if (_isCisternaSensorMinVal)
    display.print("Minimo");
  else
    display.print("Normal");

  display.display();
}

void UpdateTanqueDisplay()
{
  display.setTextColor(WHITE, BLACK);
  display.setTextSize(1);

  display.setCursor(54, 16);
  if (_isTanqueSensorMinVal)
    display.print("Minimo");
  else
    display.print("Normal");

  display.display();
}


// --- MODO ---

void SetupMode()
{
  _mode = AUTO;
}


bool IsAutomaticMode()
{
  return _mode == AUTO;
}

void ChangeToAutomaticMode()
{
  _mode = AUTO;

  //Detengo las bombas
  StopBombas();

  UpdateDisplayMode("A");
}

void ChangeToManualMode()
{
  _mode = MANUAL;

  //Detengo las bombas
  StopBombas();

  UpdateDisplayMode("M");
}

void UpdateDisplayMode(const char* mode)
{
  display.setTextColor(WHITE, BLACK);
  display.setTextSize(2);
  display.setCursor(116, 0);
  display.print(mode);
  display.display();
}


//--- Change Active Bombas ---

void CheckActiveBomba()
{
  if (IsChangeActiveBombaButtonPressed())
  {
    Serial.println("Cambiar bomba");
    if (_bombaActiva == BOMBA1)
    {
      if (IsBombaAvailable(BOMBA2))
      {
        _bombaActiva = BOMBA2;
        _bombaUses = 0;
        UpdateActiveBombaDisplay(BOMBA2);

        ChangeAutomaticState(AUTO_CHANGING_BOMBA);
      }
    }
    else
    {
      if (IsBombaAvailable(BOMBA1))
      {
        _bombaActiva = BOMBA1;
        _bombaUses = 0;
        UpdateActiveBombaDisplay(BOMBA1);

        ChangeAutomaticState(AUTO_CHANGING_BOMBA);
      }
    }
  }
}


void UpdateActiveBombaDisplay(byte active)
{
  display.setTextColor(WHITE, BLACK);
  display.setTextSize(1);
  if (active == BOMBA1)
  {
    display.setCursor(24, 0);
    display.print("*");
    display.setCursor(24, 8);
    display.print(" ");
  }
  else if (active == BOMBA2)

  {
    display.setCursor(24, 0);
    display.print(" ");
    display.setCursor(24, 8);
    display.print("*");
  }
  else
  {
    display.setCursor(24, 0);
    display.print(" ");
    display.setCursor(24, 8);
    display.print(" ");
  }

  display.display();
}

// --- DISPLAY ---
void SetupDisplay()
{
  // initialize and clear display
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.clearDisplay();
  display.display();

  // display a line of text
  display.setFont(NULL);
  display.setTextColor(WHITE);

  PrintInitialText();
}

void PrintInitialText()
{
  display.setTextSize(2);
  display.setCursor(116, 0);
  if (_mode == AUTO)
    display.print("A");
  else
    display.print("M");

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("B1 ( ):");

  display.setCursor(0, 8);
  display.print("B2 ( ):");

  display.setCursor(0, 16);
  display.print("Tanque:");

  display.setCursor(0, 24);
  display.print("Cisterna:");

  // update display with all of the above graphics
  display.display();
}

// --- ALARM ---
void StartAlarm()
{
  digitalWrite(LED_PIN, HIGH);
}

void StopAlarm()
{
  digitalWrite(LED_PIN, LOW);
}


// --- BOTONERA ---

boolean IsChangeModeButtonPressed()
{
  static unsigned long startTime = 0;
  static boolean state;
  static boolean isPressed;

  IsButtonPressed("Change Mode", CHANGE_MODE_BTN_PIN, state, isPressed, startTime);

  return isPressed;
}

boolean IsChangeActiveBombaButtonPressed()
{
  static unsigned long startTime = 0;
  static boolean state;
  static boolean isPressed;

  return IsButtonPressed("Change Active Bomba", CHANGE_ACTIVE_BTN_PIN, state, isPressed, startTime);
}

boolean IsBomba1OnOffButtonPressed()
{
  static unsigned long startTime = 0;
  static boolean state;
  static boolean isPressed;

  return IsButtonPressed("Bomba 1 ON/OFF", BOMBA1_ON_OFF_BTN_PIN, state, isPressed, startTime);
}

boolean IsBomba2OnOffButtonPressed()
{
  static unsigned long startTime = 0;
  static boolean state;
  static boolean isPressed;

  return IsButtonPressed("Bomba 2 ON/OFF", BOMBA2_ON_OFF_BTN_PIN, state, isPressed, startTime);
}


boolean IsBomba1EnableButtonPressed()
{
  static unsigned long startTime = 0;
  static boolean state;
  static boolean isPressed;

  return IsButtonPressed("Bomba 1 Enabled/Disables", BOMBA1_ENABLE_PIN, state, isPressed, startTime);
}

boolean IsBomba2EnableButtonPressed()
{
  static unsigned long startTime = 0;
  static boolean state;
  static boolean isPressed;

  IsButtonPressed("Bomba 2 Enabled/Disables", BOMBA2_ENABLE_PIN, state, isPressed, startTime);

  return isPressed;
}

boolean IsButtonPressed(const char* buttonName, int pin, boolean &state, boolean &isPressed, unsigned long &startTime)
{
  if (digitalRead(pin) != state)
  {
    state = !state;
    startTime = millis();
    isPressed = false;

    //Serial.print("Button ");
    //Serial.print(buttonName);
    //Serial.print(" changed - state ");
    //Serial.println(state);
  }

  if (state == LOW && !isPressed) {
    boolean output = (millis() - startTime) > BTN_PRESSED_TIME;

    if (output)
    {
      isPressed = true;
      //Serial.print("Button ");
      //Serial.print(buttonName);
      //Serial.println(" pressed");
      digitalWrite(LED_PIN, HIGH);
      delay(100);
      digitalWrite(LED_PIN, LOW);
    }

    return output;
  }
  else
    return false;
}
