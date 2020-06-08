
//#define TEST
const byte DEBUG = false;

// ************ PINES **************
// --- BOTONERA ---
const byte LED_PIN = A2;

const byte CHANGE_MODE_BTN_PIN = 11;

// --- SENSORES ---
const byte BOMBA1_ENABLE_PIN = 2;
const byte BOMBA2_ENABLE_PIN = 3;

const byte BOMBA1_CONTACTOR_RETORNO_PIN = 4;
const byte BOMBA2_CONTACTOR_RETORNO_PIN = 5;

const byte BOMBA1_TERMICO_RETORNO_PIN = 6;
const byte BOMBA2_TERMICO_RETORNO_PIN = 7;

// --- NIVELES ---
const byte CISTERNA_EMPTY_PIN = 8;
const byte TANQUE_EMPTY_FULL_PIN = 9;

// --- SALIDAS ---
const byte BOMBA_SWAP_BTN_PIN = 10;
const byte BOMBA_SWAP_PIN = 12; //salida al rele que activa el Rele de los contactores
const byte BOMBA1_ACTIVE = HIGH; //valor para Bomba1 Activa
const byte BOMBA2_ACTIVE = LOW;  //valor para Bomba2 Activa

//--- ALARMA ---
const byte ALARM_PIN = 13;


// *************** DEBUG ***************
const byte DEBUG_CONTINUE_PIN = A1;
const byte GET_STATUS_BTN_PIN = A0;


// ************ CONSTANTES **************

//--- MODO ---
const byte MANUAL = 0;
const byte AUTO = 1;

// --- BOMBAS ---
const byte BOMBA_USES_MAX = 1;
const long BOMBA_TURNING_ON_TIME = 30000; //tiempo en milisegundos que espera a que el contactor avise que se cerro.
const long BOMBA_TURNING_OFF_TIME = 60000; //tiempo que espera hasta que el contactor avise que se abrio.

// --- CISTERNA ---
const long CISTERNA_EMPTY_MAX_TIME = 60000 * 1; //tiempo que espera antes de hacer sonar la alarma por cisterna vacia....no se esta llenando

const byte NONE = 0;
const byte BOMBA1 = 1;
const byte BOMBA2 = 2;


// ***** MAQUINAS DE ESTADO *****

//--- BOMBA ---
enum BombaStates
{
  BOMBA_STATE_ON = 1,
  BOMBA_STATE_OFF = 2,
  BOMBA_STATE_ERROR_CONTACTOR_ABIERTO = -1,
  BOMBA_STATE_ERROR_CONTACTOR_CERRADO = -2,
  BOMBA_STATE_ERROR_TERMICO = -3
};


//--- MAQUINAS DE ESTADO ---
//enum BombaMachineStates {
const byte FSM_BOMBA_OFF = 1;
const byte FSM_BOMBA_TURNING_ON = 2;
const byte FSM_BOMBA_ON = 3;
const byte FSM_BOMBA_TURNING_OFF = 4;
const byte FSM_BOMBA_ERROR_CONTACTOR_CERRADO = 5;
const byte FSM_BOMBA_ERROR_CONTACTOR_ABIERTO = 6;
const byte FSM_BOMBA_ERROR_TERMICO = 7;
const byte FSM_BOMBA_DISABLING = 8;
const byte FSM_BOMBA_DISABLED = 9;
const byte FSM_BOMBA_ENABLING = 10;
const byte FSM_BOMBA_NULL = 0;

//};

//--- MODO AUTOMATICO ESTADOS ---
//enum AutomaticModeStates {
const byte AUTO_IDLE = 1;
const byte AUTO_SELECTING_BOMBA = 2;
const byte AUTO_NOT_AVAILABLES_BOMBAS = 3;
const byte AUTO_STARTING = 4;
const byte AUTO_WORKING = 5;
const byte AUTO_STOPPING = 6;
const byte AUTO_TANQUE_FULL = 7;
const byte AUTO_STOPPING_BOMBA = 8;
const byte AUTO_CHANGE_BOMBA_FROM_NOT_AVAILABLE = 9;
const byte AUTO_CHANGE_BOMBA_FROM_TIMEOUT = 10;
const byte AUTO_CHANGE_BOMBA = 11;
const byte AUTO_ERROR_BOMBA_WORKING = 12;
const byte AUTO_NULL = 0;
//};

typedef struct  {
  byte Number;
  bool IsEnabled;
  BombaStates State; //0=OFF 1=ON -1=ERROR CONTACTOR ABIERTO -2=ERROR CONTACTOR CERRADO -3=ERROR TERMICO -3=ERROR BOMBA (ESTE NO ESTA EN FUNCIONAMIENTO TODAVIA, FALTARIA UN SENSOR EN LA BOMBA QUE DETECTE FUNCIONAMIENTO)

  bool IsActive;
  int Uses;

  bool RequestOn;
  bool RequestOff;

  bool RequestEnabled;
  bool RequestDisabled;
  bool IsContactorClosed;
  bool IsTermicoOk;
  byte FromMachineState;
  byte MachineState;
  byte NextMachineState;
  long Timer;
} Bomba;

typedef struct {
  byte FromState;
  byte State;
  byte NextState;

  long StoppingTimer;
} AutoFSM;

//--- SENSORES ---
typedef struct
{
  bool IsCisternaSensorMinVal;
  bool IsTanqueSensorMinVal;
  bool IsTanqueSensorMaxVal;
  long CisternaEmptyStartTime;
  long CisternaEmptyMillis;
} Sensor;

// ----- TIPO DE BOTONES -----
const bool IS_CHANGE_MODE_PULSADOR = true;


// ----- VARIABLES -----
byte _mode = AUTO; //0 = Manual, 1=Automatico

Sensor sensores = {false, false, false};
Bomba bomba1 = {BOMBA1};
Bomba bomba2 = {BOMBA2};
AutoFSM automaticFSM = {};

//**************************************************//
//                     SETUP
//**************************************************//
void setup() {
  //start serial connection
  Serial.begin(115200);

  _mode = AUTO;

  SetupPins();
  Serial.println(F("Pins - Ready"));

  // put your setup code here, to run once:
  SetupDisplay();
  Serial.println(F("Display - Ready"));

  SetupLevelSensors();
  Serial.println(F("Level Sensors - Ready"));

  SetupBombaSensors();
  Serial.println(F("Bombas Sensors - Ready"));

  SetupBombas();
  Serial.println(F("Bombas - Ready"));

  SetupAlarm();
  Serial.println(F("Alarm - Ready"));

  SetupMode();
  Serial.println(F("Mode - Ready"));

  automaticFSM.FromState = AUTO_IDLE;
  automaticFSM.State = AUTO_IDLE;
  automaticFSM.NextState = AUTO_NULL;
  //automaticFSM.Timer = 0;

  Serial.println(F("Process - Ready"));
}

//************************************************//
//                     LOOP
//************************************************//

void loop() {
  //valido los niveles para visualizar en el display
  ReadTanqueSensors();

  //valido la cisterna. Actualizo el estado de la variable.
  ReadCisternaSensors();

  //leo los sensores de las bombas
  ReadBombaSensors();

  //leo la habilitación de las bombas
  ReadEnabledBombas();

  //leo el modo de ejecución (MANUAL o AUTOMATICO)
  ReadExecutionMode();

  if (IsBombaSwapButtonPressed())
  {
    SwapAndActiveBomba();
  }

  PrintStatus();

  if (DEBUG)
  {
    if (!IsContinueButtonPressed())
      return;
  }

  Serial.println(F("Continue"));

  // put your main code here, to run repeatedly:
  CicladorLoop();
}


// --- PINS ---

void SetupPins()
{
  //testigo de operacion: led
  pinMode(LED_PIN, OUTPUT);

  pinMode(CHANGE_MODE_BTN_PIN, INPUT_PULLUP);

  //sensores: contatores, termicos
  pinMode(BOMBA1_ENABLE_PIN, INPUT_PULLUP);
  pinMode(BOMBA2_ENABLE_PIN, INPUT_PULLUP);
  pinMode(BOMBA1_CONTACTOR_RETORNO_PIN, INPUT_PULLUP);
  pinMode(BOMBA2_CONTACTOR_RETORNO_PIN, INPUT_PULLUP);
  pinMode(BOMBA1_TERMICO_RETORNO_PIN, INPUT_PULLUP);
  pinMode(BOMBA2_TERMICO_RETORNO_PIN, INPUT_PULLUP);

  //Niveles
  pinMode(CISTERNA_EMPTY_PIN, INPUT_PULLUP);
  pinMode(TANQUE_EMPTY_FULL_PIN, INPUT_PULLUP);

  //Swap
  pinMode(BOMBA_SWAP_BTN_PIN, INPUT_PULLUP);

  //Salidas contactores
  pinMode(BOMBA_SWAP_PIN, OUTPUT);

  //Debug
  pinMode(DEBUG_CONTINUE_PIN, INPUT_PULLUP);
  pinMode(GET_STATUS_BTN_PIN, INPUT_PULLUP);

  //lo apago
  digitalWrite(LED_PIN, LOW);
}

void PrintStatus()
{
  if (!IsGetStatusButtonPressed())
    return;

  DoPrintStatus();
}

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


  //Bombas
  Serial.println(F("*** BOMBA 1 ***"));
  PrintBomba(&bomba1);
  Serial.println();

  Serial.println(F("*** BOMBA 2 ***"));
  PrintBomba(&bomba2);
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

  Serial.print(F("Timer: "));
  Serial.println(bomba->Timer);

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

  if (bomba->IsContactorClosed)
    Serial.println(F("IsContactorClosed: true"));
  else
    Serial.println(F("IsContactorClosed: false"));

  if (bomba->IsTermicoOk)
    Serial.println(F("IsTermicoOk: true"));
  else
    Serial.println(F("IsTermicoOk: false"));
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
