
// ************ PINES **************
// --- BOTONERA ---
const byte LED_PIN = 13;

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
const byte ALARM_PIN = A1;


// *************** DEBUG ***************
const byte DEBUG = true;
const byte DEBUG_CONTINUE_PIN = A1;
const byte GET_STATUS_BTN_PIN = A0;


// ************ CONSTANTES **************

//--- MODO ---
const byte MANUAL = 0;
const byte AUTO = 1;

// --- BOMBAS ---
const byte BOMBA_USES_MAX = 1;
const long BOMBA_TURNING_ON_TIME = 60000; //tiempo en milisegundos que espera a que el contactor avise que se cerro.
const long BOMBA_TURNING_OFF_TIME = 60000; //tiempo que espera hasta que el contactor avise que se abrio.


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
enum BombaMachineStates {
  FSM_BOMBA_OFF,
  FSM_BOMBA_TURNING_ON,
  FSM_BOMBA_ON,
  FSM_BOMBA_TURNING_OFF,
  FSM_BOMBA_ERROR_CONTACTOR_CERRADO,
  FSM_BOMBA_ERROR_CONTACTOR_ABIERTO,
  FSM_BOMBA_ERROR_TERMICO,
  FSM_BOMBA_DISABLING,
  FSM_BOMBA_DISABLED,
  FSM_BOMBA_ENABLING,
  FSM_BOMBA_NULL
};

//--- MODO AUTOMATICO ESTADOS ---
enum AutomaticModeStates {
  AUTO_IDLE,
  AUTO_SELECTING_BOMBA,
  AUTO_NOT_AVAILABLES_BOMBAS,
  AUTO_STARTING,
  AUTO_WORKING,
  AUTO_STOPPING,
  AUTO_TANQUE_FULL,
  AUTO_STOPPING_BOMBA,
  AUTO_CHANGE_BOMBA_FROM_ERROR,
  AUTO_CHANGE_BOMBA_FROM_TIMEOUT,
  AUTO_CHANGE_BOMBA
} ;

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
  BombaMachineStates FromMachineState;
  BombaMachineStates MachineState;
  BombaMachineStates NextMachineState;
  long Timer;
} Bomba;

typedef struct {
  AutomaticModeStates State;
  long Timer;
} AutoFSM;

//--- SENSORES ---
typedef struct
{
  bool IsCisternaSensorMinVal;
  bool IsTanqueSensorMinVal;
  bool IsTanqueSensorMaxVal;
} Sensor;

// ----- TIPO DE BOTONES -----
const bool IS_CHANGE_MODE_PULSADOR = true;


// ----- VARIABLES -----
byte _mode = 0; //0 = Manual, 1=Automatico

Sensor sensores = {false, false, false};
Bomba bomba1 = {BOMBA1}; //, true, OFF, true, 0, false, false, false, false, false, true, BOMBA_OFF, 0};
Bomba bomba2 = {BOMBA2};//, true, OFF, false, 0, false, false, false, false, false, true, BOMBA_OFF, 0};
AutoFSM automaticFSM = {AUTO_IDLE, 0};

//**************************************************//
//                     SETUP
//**************************************************//
void setup() {
  //start serial connection
  Serial.begin(115200);

  SetupPins();

  // put your setup code here, to run once:
  SetupDisplay();

  SetupLevelSensors();

  SetupBombaSensors();

  SetupBombas();

  SetupMode();
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
  //Modo
  if (IsAutomaticMode())
    Serial.println(F("*** MODE: Automatic ***"));
  else
    Serial.println(F("*** MODE: Manual ***"));

  //Estado del proceso
  Serial.print(F("Automatic FSM Status: "));
  PrintStateWorkingFSM(NULL, automaticFSM.State, true);
  Serial.println();

  //Niveles
  Serial.println(F("*** Niveles ***"));
  if (sensores.IsCisternaSensorMinVal)
    Serial.println(F("Cisterna Nivel Minimo: true"));
  else
    Serial.println(F("Cisterna Nivel Minimo: false"));

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
  PrintStateBombaFSM(NULL, bomba->MachineState, true);

  Serial.print(F("Uses: "));
  Serial.println(bomba->Uses);

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
