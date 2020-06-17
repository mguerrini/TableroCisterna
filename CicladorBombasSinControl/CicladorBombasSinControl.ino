
#define TEST

//DISPLAYS
//#define DISPLAY_OLED_128x64
#define DISPLAY_20x4_I2C

//ALARMAS
//#define ALARM_BUZZER
#define ALARM_LED


const boolean DEBUG = true;

// ************ PINES **************

// --- DEBUG ---
const byte DEBUG_CONTINUE_PIN = 4;
#define GET_STATUS_AS_BUTTON
#ifdef GET_STATUS_AS_BUTTON
const byte GET_STATUS_BTN_PIN = 2;
#endif

// --- BUTTON ---

const byte BOMBA_SWAP_BTN_PIN = A2;
const byte CHANGE_MODE_BTN_PIN = A1;
const byte RESET_BTN_PIN = A3;
const byte VIEW_INFO_PIN = 2;

//--- INVERSOR ---
const byte BOMBA_SWAP_PIN = A0; //salida al rele que activa el Rele de los contactores
const byte BOMBA1_ACTIVE = HIGH; //valor para Bomba1 Activa
const byte BOMBA2_ACTIVE = LOW;  //valor para Bomba2 Activa

//--- ALARMA ---
const byte ALARM_PIN = 3;
//#define ALARM_AUX_ENBALED
#ifdef ALARM_AUX_ENBALED
const byte ALARM_PIN_AUX = 4;
#endif

// --- SENSOR FASE ---
const byte FASE_PIN = 3;

// --- SENSORES BOMBAS ---
const byte BOMBA1_ENABLE_PIN = 5;
const byte BOMBA2_ENABLE_PIN = 6;

const byte BOMBA1_CONTACTOR_RETORNO_PIN = 7;
const byte BOMBA2_CONTACTOR_RETORNO_PIN = 8;

const byte BOMBA1_TERMICO_RETORNO_PIN = 9;
const byte BOMBA2_TERMICO_RETORNO_PIN = 10;

// --- SENSOR DE NIVELES ---
const byte CISTERNA_EMPTY_PIN = 11;
const byte TANQUE_EMPTY_FULL_PIN = 12;

//--- MODO ---
const byte MODO_PIN = 13;

// --- TESTIGO BUTONES ---
#define TESTIGO_LED
#ifdef TESTIGO_LED
const byte LED_PIN = 2;
#endif


// ************ CONSTANTES **************

//--- MODO ---
const byte MANUAL = 0;
const byte AUTO = 1;

// --- BOMBAS ---
const byte BOMBA_USES_MAX = 1;
const long BOMBA_TURNING_ON_TIME = 5000; //tiempo en milisegundos que espera a que el contactor avise que se cerro.
const long BOMBA_TURNING_OFF_TIME = 5000; //tiempo que espera hasta que el contactor avise que se abrio.
const long BOMBA_CONTACTOR_ERROR_INTENTOS_MAX = 100; //Maxima cantidad de intentos
const long BOMBA_CONTACTOR_ERROR_INTERVAL = 10000; //Intervalo de tiempo entre intentos de recuperar el contactor

// --- CISTERNA ---
const long CISTERNA_EMPTY_MAX_TIME = 10000 * 1; //tiempo que espera antes de hacer sonar la alarma por cisterna vacia....no se esta llenando

const byte NONE = 0;
const byte BOMBA1 = 1;
const byte BOMBA2 = 2;

// --- TIPO DE BOTONES ---
const bool IS_CHANGE_MODE_PULSADOR = true;

// --- Statistics ---
const long STATISTICS_TIME_TO_SAVE = (1000 * 60) * 60; //una vez por hora...si cambia


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

  long ContactorErrorCounter;
  unsigned long StartError;
  unsigned long Timer;
} Bomba;

typedef struct {
  byte FromState;
  byte State;
  byte NextState;

  boolean IsFaseOk;
  unsigned long StoppingTimer;
} AutoFSM;

//--- SENSORES ---
typedef struct
{
  bool IsCisternaSensorMinVal;
  bool IsTanqueSensorMinVal;
  bool IsTanqueSensorMaxVal;
  unsigned long CisternaEmptyStartTime;
  unsigned long CisternaEmptyMillis;
} Sensor;

//--- ALARMA ---
typedef struct  {
  boolean IsBomba1AlarmON = false;
  boolean IsBomba2AlarmON = false;
  boolean IsManualAlarmON = false;
  boolean IsCisternaAlarmON = false;
  boolean IsNotAvailableBombasAlarmON = false;

  //alarma
  unsigned long ActiveTime;
  unsigned long InactiveTime;
  boolean IsActive;

} Alarm;

typedef struct {
  unsigned long Bomba1Uses;
  unsigned long Bomba2Uses;
  unsigned long Bomba1TotalMinutes;
  unsigned long Bomba2TotalMinutes;
  unsigned long Bomba1ErrorTermicoCount;
  unsigned long Bomba2ErrorTermicoCount;

  unsigned long ErrorFaseTotalMinutes;
  unsigned long ErrorFaseCount;
  unsigned long LastTimeSaved;

  unsigned long Bomba1OnTime;
  unsigned long Bomba2OnTime;
  unsigned long FaseErrorBeginTime;
  boolean Changed;
} Statistics;

// ----- VARIABLES -----
byte _mode = AUTO; //0 = Manual, 1=Automatico

Sensor sensores = {false, false, false};
Bomba bomba1 = {BOMBA1};
Bomba bomba2 = {BOMBA2};
Alarm alarm = {};
AutoFSM automaticFSM = {};
Statistics statistics = { };

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

  SetupFase();
  Serial.println(F("Fase Sensor - Ready"));

  SetupCommands();
  Serial.println(F("Commands - Ready"));

  SetupStatistics();
  Serial.println(F("Statistics - Ready"));

  automaticFSM.FromState = AUTO_IDLE;
  automaticFSM.State = AUTO_IDLE;
  automaticFSM.NextState = AUTO_NULL;

  Serial.println(F("Process - Ready"));
}


//************************************************//
//                     LOOP
//************************************************//

void loop() {
  ReadCommands();

  ReadResetButton();

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

  //ejecuta la alarma si corresponde
  ReadAlarm();

  //Las bombas se detienen asi que el circuito seguiria normal.......pero no deberia arrancar
  ReadFase();

  PrintStatus();

  if (IsBombaSwapButtonPressed())
  {
    SwapAndActiveBomba();
  }

  if (DEBUG)
  {
    if (!IsContinueButtonPressed())
      return;
    else
      Serial.println(F("Continue"));
  }

  // put your main code here, to run repeatedly:
  CicladorLoop();
}

//************************************************//
//                 AUXILIARES
//************************************************//

// --- PINS ---

void SetupPins()
{
  // --- DEBUG ---
  pinMode(DEBUG_CONTINUE_PIN, INPUT_PULLUP);
#ifdef GET_STATUS_AS_BUTTON
  pinMode(GET_STATUS_BTN_PIN, INPUT_PULLUP);
#endif

  // --- BUTTON ---
  pinMode(BOMBA_SWAP_BTN_PIN, INPUT_PULLUP);
  pinMode(CHANGE_MODE_BTN_PIN, INPUT_PULLUP);
  pinMode(RESET_BTN_PIN, INPUT_PULLUP);
  pinMode(VIEW_INFO_PIN, INPUT_PULLUP);

  // --- INVERSOR ---
  pinMode(BOMBA_SWAP_PIN, OUTPUT);

  // --- ALARMA ---
  pinMode(ALARM_PIN, OUTPUT);
#ifdef ALARM_AUX_ENBALED
  pinMode(ALARM_PIN_AUX, OUTPUT);
#endif

  // --- SENSOR FASE ---
  pinMode(FASE_PIN, INPUT_PULLUP);

  // --- SENSORES BOMBAS ---
  pinMode(BOMBA1_ENABLE_PIN, INPUT_PULLUP);
  pinMode(BOMBA2_ENABLE_PIN, INPUT_PULLUP);
  pinMode(BOMBA1_CONTACTOR_RETORNO_PIN, INPUT_PULLUP);
  pinMode(BOMBA2_CONTACTOR_RETORNO_PIN, INPUT_PULLUP);
  pinMode(BOMBA1_TERMICO_RETORNO_PIN, INPUT_PULLUP);
  pinMode(BOMBA2_TERMICO_RETORNO_PIN, INPUT_PULLUP);

  // --- SENSOR DE NIVELES ---
  pinMode(CISTERNA_EMPTY_PIN, INPUT_PULLUP);
  pinMode(TANQUE_EMPTY_FULL_PIN, INPUT_PULLUP);

  //--- MODO ---
  pinMode(MODO_PIN, OUTPUT);

  // --- TESTIGO BUTONES ---
#ifdef TESTIGO_LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
#endif
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

  PrintAlarm();

  //Bombas
  Serial.println(F("*** BOMBA 1 ***"));
  PrintBomba(&bomba1);
  Serial.println();

  Serial.println(F("*** BOMBA 2 ***"));
  PrintBomba(&bomba2);
  Serial.println();

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

  Serial.print(F("Timer: "));
  Serial.println(bomba->Timer);

  Serial.print(F("Contactor Error Counter: "));
  Serial.println(bomba->ContactorErrorCounter);


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
