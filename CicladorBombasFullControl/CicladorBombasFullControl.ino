
// ************ PINES **************
// --- BOTONERA ---
const byte LED_PIN = 13;

const byte CHANGE_MODE_BTN_PIN = 12;
const byte CHANGE_ACTIVE_BTN_PIN = 11;
const byte BOMBA1_ENABLE_PIN = 10;
const byte BOMBA2_ENABLE_PIN = 9;

// --- SENSORES ---
const byte BOMBA1_CONTACTOR_RETORNO_PIN = A2;
const byte BOMBA2_CONTACTOR_RETORNO_PIN = A3;

const byte BOMBA1_TERMICO_RETORNO_PIN = A0;
const byte BOMBA2_TERMICO_RETORNO_PIN = A1;

// --- NIVELES ---
const byte CISTERNA_EMPTY_PIN = 5;
const byte TANQUE_EMPTY_PIN = 6;
const byte TANQUE_FULL_PIN = 7;

// --- SALIDAS ---
const byte BOMBA1_CONTACTOR_PIN = A6;
const byte BOMBA2_CONTACTOR_PIN = A7;

//--- ALARMA ---
const byte ALARM_PIN = 0;

// ************ END - PINES **************

//--- MODO ---
const byte MANUAL = 0;
const byte AUTO = 1;

// --- BOMBAS ---
const byte BOMBA_USES_MAX = 1;
const long BOMBA_TURNING_ON_TIME = 5000; //tiempo en milisegundos que espera a que el contactor avise que se cerro.
const long BOMBA_TURNING_OFF_TIME = 5000; //tiempo que espera hasta que el contactor avise que se abrio.

const byte NONE = 0;
const byte BOMBA1 = 1;
const byte BOMBA2 = 2;


// ***** MAQUINAS DE ESTADO *****


//--- Cambio bomba activa ---
enum ChangeActiveMachineStates {
  CHANGE_ACTIVE_IDLE,
  CHANGE_ACTIVE_CHANGING_REQUEST,
  CHANGE_ACTIVE_CHANGING_FROM_ON,
  CHANGE_ACTIVE_CHANGING_FROM_OFF,
  CHANGE_ACTIVE_STOPPING
};


//--- BOMBA ---
enum BombaStates
{
  ON = 1,
  OFF = 2,
  ERROR_CONTACTOR_ABIERTO = -1,
  ERROR_CONTACTOR_CERRADO = -2,
  ERROR_TERMICO = -3
};

enum BombaMachineStates {
  BOMBA_OFF,
  BOMBA_TURNING_ON,
  BOMBA_ON,
  BOMBA_TURNING_OFF,
  BOMBA_ERROR_CONTACTOR,
  BOMBA_ERROR_TERMICO,
  BOMBA_DISABLING_FROM_ON,
  BOMBA_DISABLING_FROM_OFF,
  BOMBA_DISABLING_FROM_ERROR,
  BOMBA_DISABLED,
  BOMBA_ENABLING

};

//--- MODO AUTOMATICO ESTADOS ---
enum AutomaticModeStates {
  AUTO_IDLE,
  AUTO_SELECTING_BOMBA,
  AUTO_NOT_AVAILABLES_BOMBAS,
  AUTO_WORKING,
  AUTO_TANQUE_FULL,
  AUTO_STOPPING_BOMBA
} ;

typedef struct  {
  byte Number;
  bool IsEnabled;
  BombaStates State; //0=OFF 1=ON -1=ERROR CONTACTOR ABIERTO -2=ERROR CONTACTOR CERRADO -3=ERROR TERMICO -3=ERROR BOMBA (ESTE NO ESTA EN FUNCIONAMIENTO TODAVIA, FALTARIA UN SENSOR EN LA BOMBA QUE DETECTE FUNCIONAMIENTO)
  bool IsActive;
  int Uses;
  bool RequestDisable;
  bool RequestEnable;
  bool RequestOn;
  bool RequestOff;
  bool IsContactorClosed;
  bool IsTermicoOk;
  BombaMachineStates MachineState;
  long Timer;
} Bomba;

//--- SENSORES ---
typedef struct
{
  bool IsCisternaSensorMinVal;
  bool IsTanqueSensorMinVal;
  bool IsTanqueSensorMaxVal;
} Sensor;


// ----- VARIABLES -----
byte _mode = 0; //0 = Manual, 1=Automatico
ChangeActiveMachineStates _changeActiveMachineState = CHANGE_ACTIVE_IDLE;
AutomaticModeStates _automaticModeState = AUTO_IDLE;

Sensor sensores = {false, false, false};
Bomba bomba1 = {BOMBA1, true, OFF, true, 0, false, false, false, false, false, true, BOMBA_OFF, 0};
Bomba bomba2 = {BOMBA2, true, OFF, false, 0, false, false, false, false, false, true, BOMBA_OFF, 0};


//**************************************************//
//                     SETUP
//**************************************************//
void setup() {
  //start serial connection
  Serial.begin(9600);

  SetupPins();

  // put your setup code here, to run once:
  SetupDisplay();

  SetupLevelSensors();

  SetupBombaSensors();

  SetupBombas();
}

//************************************************//
//                     LOOP
//************************************************//

void loop() {

  //valido los niveles para visualizar en el display
  ReadTanqueSensors();

  //valido la cisterna. Actualizo el estado de la variable.
  ReadCisternaSensors();

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
  //pinMode(BOMBA1_ON_OFF_BTN_PIN, INPUT_PULLUP);
  //pinMode(BOMBA2_ON_OFF_BTN_PIN, INPUT_PULLUP);
  pinMode(BOMBA1_ENABLE_PIN, INPUT_PULLUP);
  pinMode(BOMBA2_ENABLE_PIN, INPUT_PULLUP);

  //testigo de operacion: led
  pinMode(LED_PIN, OUTPUT);
  //lo apago
  digitalWrite(LED_PIN, LOW);
}






void ManualLoop()
{
  if (IsChangeModeButtonPressed())
  {
    ChangeToAutomaticMode();
    return;
  }
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

  UpdateDisplayToAuto();
}

void ChangeToManualMode()
{
  _mode = MANUAL;

  //Detengo las bombas
  StopBombas();

  UpdateDisplayToManual();
}
