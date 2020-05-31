
// ************ PINES **************
// --- BOTONERA ---
const byte LED_PIN = 13;

const byte CHANGE_MODE_BTN_PIN = 12;

// --- SENSORES ---
const byte BOMBA1_ENABLE_PIN = 10;
const byte BOMBA2_ENABLE_PIN = 9;

const byte BOMBA1_CONTACTOR_RETORNO_PIN = A2;
const byte BOMBA2_CONTACTOR_RETORNO_PIN = A3;

const byte BOMBA1_TERMICO_RETORNO_PIN = A0;
const byte BOMBA2_TERMICO_RETORNO_PIN = A1;

// --- NIVELES ---
const byte CISTERNA_EMPTY_PIN = 5;
const byte TANQUE_EMPTY_FULL_PIN = 7;

// --- SALIDAS ---
const byte BOMBA_SWAP_PIN = A6;
const byte BOMBA1_ACTIVE = HIGH;
const byte BOMBA2_ACTIVE = LOW;

//--- ALARMA ---
const byte ALARM_PIN = 0;


// ************ CONSTANTES **************


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
  FSM_BOMBA_ENABLING
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
  BombaMachineStates MachineState;
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


// ----- VARIABLES -----
byte _mode = 0; //0 = Manual, 1=Automatico

Sensor sensores = {false, false, false};
Bomba bomba1 = {BOMBA1}; //, true, OFF, true, 0, false, false, false, false, false, true, BOMBA_OFF, 0};
Bomba bomba2 = {BOMBA2};//, true, OFF, false, 0, false, false, false, false, false, true, BOMBA_OFF, 0};
AutoFSM automaticFSM = {AUTO_IDLE, 0};
AutoFSM manualFSM = {AUTO_IDLE, 0};

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

  //leo los sensores de las bombas
  ReadBombaSensors();

  //leo la habilitación de las bombas
  ReadEnabledBombas();

  // put your main code here, to run repeatedly:
  AutomaticLoop();
}


// --- PINS ---

void SetupPins()
{
  //Salidas contactores
  pinMode(BOMBA_SWAP_PIN, OUTPUT);
  //  pinMode(BOMBA1_CONTACTOR_PIN, OUTPUT);
  //  pinMode(BOMBA2_CONTACTOR_PIN, OUTPUT);

  //sensores: contatores, termicos
  pinMode(BOMBA1_CONTACTOR_RETORNO_PIN, INPUT_PULLUP);
  pinMode(BOMBA2_CONTACTOR_RETORNO_PIN, INPUT_PULLUP);
  pinMode(BOMBA1_TERMICO_RETORNO_PIN, INPUT_PULLUP);
  pinMode(BOMBA2_TERMICO_RETORNO_PIN, INPUT_PULLUP);

  //Niveles
  pinMode(CISTERNA_EMPTY_PIN, INPUT_PULLUP);
  pinMode(TANQUE_EMPTY_FULL_PIN, INPUT_PULLUP);

  //Botonera
  pinMode(CHANGE_MODE_BTN_PIN, INPUT_PULLUP);
  //pinMode(CHANGE_ACTIVE_BTN_PIN, INPUT_PULLUP);
  //pinMode(BOMBA1_ON_OFF_BTN_PIN, INPUT_PULLUP);
  //pinMode(BOMBA2_ON_OFF_BTN_PIN, INPUT_PULLUP);
  pinMode(BOMBA1_ENABLE_PIN, INPUT_PULLUP);
  pinMode(BOMBA2_ENABLE_PIN, INPUT_PULLUP);

  //testigo de operacion: led
  pinMode(LED_PIN, OUTPUT);
  //lo apago
  digitalWrite(LED_PIN, LOW);
}
