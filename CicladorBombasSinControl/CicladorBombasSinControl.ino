
#define TEST

//DISPLAYS
//#define DISPLAY_OLED_128x64
#define DISPLAY_20x4_I2C

//ALARMAS
//#define ALARM_BUZZER
#define ALARM_LED
//#define STATISTICS_SAVE_ENABLED


// ************ PINES **************

// --- DEBUG ---
#define DEBUG
#ifdef DEBUG
#define DEBUG_CONTINUE_PIN A3
#endif

//#define GET_STATUS_BUTTON_ENABLED
#ifdef GET_STATUS_BUTTON_ENABLED
#define GET_STATUS_BTN_PIN 2 //no se usa si no esta definido
#endif

// --- BUTTON ---
#define BOMBA_SWAP_BTN_PIN 12  //Cambiar bomba seleccionada
#define CHANGE_MODE_BTN_PIN 13 //selector Manual / Automatico
#define RESET_BTN_PIN 11
#define VIEW_INFO_PIN A0 //Muestra información estadisticas y valores varios

//--- INVERSOR ---
//salida al rele que activa el Rele de los contactores
#define BOMBA_SWAP_RELE_PIN A2
//valor para Bomba1 Activa
#define BOMBA1_ACTIVE  HIGH
//valor para Bomba2 Activa
#define BOMBA2_ACTIVE  LOW

//--- ALARMA ---
#define ALARM_PIN 10

//#define ALARM_AUX_ENBALED
#ifdef ALARM_AUX_ENBALED
#define ALARM_PIN_AUX 0
#endif

// --- SENSOR FASE ---
#define FASE_ERROR_PIN A1

//--- MODO ---
#ifndef DEBUG
#define MODO_OUTPUT_ENABLED
#define MODO_LED_PIN A3
#endif

// --- SENSORES BOMBAS ---
#define BOMBA1_ENABLE_PIN  2
#define BOMBA2_ENABLE_PIN  3

#define BOMBA1_CONTACTOR_RETORNO_PIN  4
#define BOMBA2_CONTACTOR_RETORNO_PIN  5

#define BOMBA1_TERMICO_RETORNO_PIN  6
#define BOMBA2_TERMICO_RETORNO_PIN  7

// --- SENSOR DE NIVELES ---
#define CISTERNA_EMPTY_PIN  8
#define TANQUE_EMPTY_FULL_PIN  9

// --- TESTIGO BUTONES ---
//#define TESTIGO_LED
#ifdef TESTIGO_LED
#define LED_PIN 2
#endif


// ************ CONSTANTES **************

#define INFO_VIEW_VISIBLE_TIME 10000 //10 SEGUNDOS

//--- MODO ---
#define MANUAL 0
#define AUTO 1

// --- BOMBAS ---
#define BOMBA_USES_MAX 1
#define BOMBA_TURNING_ON_TIME 5000 //tiempo en milisegundos que espera a que el contactor avise que se cerro.
#define BOMBA_TURNING_OFF_TIME 5000 //tiempo que espera hasta que el contactor avise que se abrio.
#define BOMBA_CONTACTOR_ERROR_INTENTOS_MAX 100 //Maxima cantidad de intentos
#define BOMBA_CONTACTOR_ERROR_INTERVAL 10000 //Intervalo de tiempo entre intentos de recuperar el contactor

// --- CISTERNA ---
#define CISTERNA_EMPTY_MAX_TIME 10000 //tiempo (milisegundos) que espera antes de hacer sonar la alarma por cisterna vacia....no se esta llenando

// --- TANQUE ---
#define TANQUE_TIME_TO_FULL 10000 //tiempo inicial para el llenado del tanque. Luego calcula el tiempo promedio por tanque y le suma u porcentage

#define NONE 0
#define BOMBA1 1
#define BOMBA2 2

// --- TIPO DE BOTONES ---
#define IS_CHANGE_MODE_PULSADOR true

// --- Statistics ---
const long STATISTICS_TIME_TO_SAVE = (1000 * 60) * 60; //una vez por hora...si cambia


// ***** MAQUINAS DE ESTADO *****

//--- BOMBA ---
#define BOMBA_STATE_ON 1
#define BOMBA_STATE_OFF 2
#define BOMBA_STATE_ERROR_CONTACTOR_ABIERTO -1
#define BOMBA_STATE_ERROR_CONTACTOR_CERRADO -2
#define BOMBA_STATE_ERROR_TERMICO -3


//--- MAQUINAS DE ESTADO ---
#define FSM_BOMBA_OFF 1
#define FSM_BOMBA_TURNING_ON 2
#define FSM_BOMBA_ON 3
#define FSM_BOMBA_TURNING_OFF 4
#define FSM_BOMBA_ERROR_CONTACTOR_CERRADO 5
#define FSM_BOMBA_ERROR_CONTACTOR_ABIERTO 6
#define FSM_BOMBA_ERROR_TERMICO 7
#define FSM_BOMBA_DISABLING 8
#define FSM_BOMBA_DISABLED 9
#define FSM_BOMBA_ENABLING 10
#define FSM_BOMBA_NULL 0

//--- MODO AUTOMATICO ESTADOS ---
#define AUTO_IDLE 1
#define AUTO_SELECTING_BOMBA 2
#define AUTO_NOT_AVAILABLES_BOMBAS 3
#define AUTO_STARTING 4
#define AUTO_WORKING 5
#define AUTO_STOPPING 6
#define AUTO_TANQUE_FULL 7
#define AUTO_STOPPING_BOMBA 8
#define AUTO_CHANGE_BOMBA_FROM_NOT_AVAILABLE 9
#define AUTO_CHANGE_BOMBA 11
#define AUTO_ERROR_BOMBA_WORKING 12
#define AUTO_NULL 0

typedef struct  {
  byte Number;
  bool IsEnabled;
  byte State; //0=OFF 1=ON -1=ERROR CONTACTOR ABIERTO -2=ERROR CONTACTOR CERRADO -3=ERROR TERMICO -3=ERROR BOMBA (ESTE NO ESTA EN FUNCIONAMIENTO TODAVIA, FALTARIA UN SENSOR EN LA BOMBA QUE DETECTE FUNCIONAMIENTO)

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

  unsigned int FillTimes[10];
  unsigned int FillTimeAverage;

  long ContactorErrorCounter;
  unsigned long StartError;
  unsigned long Timer;
} Bomba;

typedef struct {
  byte FromState;
  byte State;
  byte NextState;

  byte Mode;

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
  unsigned long Bomba1FillingTime;
  unsigned long Bomba2FillingTime;

  unsigned long ErrorFaseTotalMinutes;
  unsigned long ErrorFaseCount;
  unsigned long LastTimeSaved;

  unsigned long Bomba1OnTime;
  unsigned long Bomba2OnTime;
  unsigned long FaseErrorBeginTime;
  boolean Changed;
} Statistics;

// ----- VARIABLES -----
Sensor sensores = {false, false, false};
Bomba bomba1 = {BOMBA1};
Bomba bomba2 = {BOMBA2};
Alarm alarm = {};
Statistics statistics = { };
AutoFSM automaticFSM = {};


//**************************************************//
//                     SETUP
//**************************************************//
void setup() {
  //start serial connection
  Serial.begin(115200);

  automaticFSM.Mode = AUTO;

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

  ReadSwapButton();

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

  ReadPrintStatus();

  //actualizo la vista si corresponde
  UpdateView();

#ifdef DEBUG
  if (!IsContinueButtonPressed())
    return;
  else
    Serial.println(F("Continue"));
#endif

  // put your main code here, to run repeatedly:
  CicladorLoop();

  SaveStatistics();
}


//************************************************//
//                 AUXILIARES
//************************************************//

// --- PINS ---

void SetupPins()
{
  // --- DEBUG ---
  pinMode(DEBUG_CONTINUE_PIN, INPUT_PULLUP);
#ifdef GET_STATUS_BUTTON_ENABLED
  pinMode(GET_STATUS_BTN_PIN, INPUT_PULLUP);
#endif

  // --- BUTTON ---
  pinMode(BOMBA_SWAP_BTN_PIN, INPUT_PULLUP);
  pinMode(CHANGE_MODE_BTN_PIN, INPUT_PULLUP);
  pinMode(RESET_BTN_PIN, INPUT_PULLUP);
  pinMode(VIEW_INFO_PIN, INPUT_PULLUP);

  // --- INVERSOR ---
  pinMode(BOMBA_SWAP_RELE_PIN, OUTPUT);

  // --- ALARMA ---
  pinMode(ALARM_PIN, OUTPUT);
#ifdef ALARM_AUX_ENBALED
  pinMode(ALARM_PIN_AUX, OUTPUT);
#endif

  // --- SENSOR FASE ---
  pinMode(FASE_ERROR_PIN, INPUT_PULLUP);

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
#ifdef MODO_OUTPUT_ENABLED
  pinMode(MODO_LED_PIN, OUTPUT);
#endif
  // --- TESTIGO BUTONES ---
#ifdef TESTIGO_LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
#endif
}
