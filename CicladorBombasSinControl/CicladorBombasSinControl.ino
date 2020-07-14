
#define TEST

//DISPLAYS
#define DISPLAY_20x4_I2C

// --- DEBUG ---
#define DEBUG
#ifdef DEBUG
#define DEBUG_CONTINUE_PIN A3
#endif


//funciones con referencias
boolean IsButtonPressed(int pin, boolean &state, boolean &isPressed, unsigned long &startTime);
boolean IsButtonPressedWithTimeRange(int pin, boolean &state, boolean &isPressed, unsigned long &startTime, unsigned long minTime, unsigned long maxTime);

// ****************************************************************** //
//                        EEPROM MEMORY MAP
// ****************************************************************** //
//INT : 2 BYTES
//LONG: 4 BYTES
//ints
#define FASE1_TENSION_ENTRADA_ADDR 0 //int
#define FASE2_TENSION_ENTRADA_ADDR 2 //int
#define FASE3_TENSION_ENTRADA_ADDR 4 //int

#define FASE1_FACTOR_CONVERSION_ADDR 6  //int
#define FASE2_FACTOR_CONVERSION_ADDR 8  //int
#define FASE3_FACTOR_CONVERSION_ADDR 10 //int


#define BOMBA1_USES_ADDR 20 //unsigned long
#define BOMBA1_TOTAL_MINUTES_ADDR 24 //unsigned long
#define BOMBA1_ERROR_TERMICO_COUNT_ADDR 28 //unsigned long
#define BOMBA1_FILLING_TIME_MINUTES_ADDR 32 //unsigned int

#define BOMBA2_USES_ADDR 40 //unsigned long
#define BOMBA2_TOTAL_MINUTES_ADDR 44 //unsigned long
#define BOMBA2_ERROR_TERMICO_COUNT_ADDR 48 //unsigned long
#define BOMBA2_FILLING_TIME_MINUTES_ADDR 52 //unsigned int

#define ERROR_FASE_TOTAL_MINUTES_ADDR 60 //unsigned long
#define ERROR_FASE_COUNT_ADDR 64 //unsigned long


// ****************************************************************** //
//                        CONFIGURACIONES
// ****************************************************************** //

// ====================== BOTONES VARIOS ====================
#define BOMBA_SWAP_BTN_PIN 12  //Cambiar bomba seleccionada
#define RESET_BTN_PIN 11
#define VIEW_INFO_PIN A0 //Muestra información estadisticas y valores varios

#define INFO_VIEW_VISIBLE_TIME 10000 //10 SEGUNDOS
#define BTN_PRESSED_TIME 20 //20 milisegundos de boton presionado, para evitar rebote

// ====================== INVERSOR ======================
//salida al rele que activa el Rele de los contactores
#define BOMBA_SWAP_RELE_PIN 13
//valor para Bomba1 Activa
#define BOMBA1_ACTIVE  HIGH
//valor para Bomba2 Activa
#define BOMBA2_ACTIVE  LOW


// ====================== ALARMA ======================
//#define ALARM_BUZZER
#define ALARM_LED
#define ALARM_PIN 10


// ====================== FASES ======================
//#define FASE1_ENABLED
#define FASE1_INPUT_PIN A3

//#define FASE2_ENABLED
#define FASE2_INPUT_PIN A6

#define FASE3_ENABLED
#define FASE3_INPUT_PIN A7

#define FASE_OUTPUT_PIN A1
#define FASE_OUTPUT_CLOSE_RELE HIGH
#define FASE_OUTPUT_OPEN_RELE LOW

//tension de entrada
#define FASE_FROM_EEPROM_ENABLED // indica que se debe leer los valores almacenados en la EEPROM y no los valores constantes

#define TENSION_ENTRADA 5 //220 Volts, pero para testear lo pongo en 5
//valores de referencia para el valor de 220V
#define FASE1_220_VALUE 1023
#define FASE2_220_VALUE 1023
#define FASE3_220_VALUE 1023


#define FASE_MIN_VOLTAGE 3 //Valor minimo antes de que se considere falla. 3 para testear
#define FASE_READ_COUNT_MAX 50 //100 milisegundos donde se promedia el valor leiado
#define FASE_WAIT_BETWEEN_READS 10 //10 milisegundos de espera entre las mediciones
#define FASE_REFRESH_TIME 500 //milisegundos que refresca las tensiones

// ====================== MODO ======================
#define MANUAL 0
#define AUTO 1

#define CHANGE_MODE_BTN_PIN A2 //selector Manual / Automatico
#define IS_CHANGE_MODE_PULSADOR true //tipo de boton, pulsador o llave

#define MODO_OUTPUT_VIEW_ENABLED //Muestra el modo en la pantalla

// ====================== BOMBAS ======================
// --- PINES ---
#define BOMBA1_ENABLE_PIN  2
#define BOMBA2_ENABLE_PIN  3

#define BOMBA1_CONTACTOR_RETORNO_PIN  4
#define BOMBA2_CONTACTOR_RETORNO_PIN  5

#define BOMBA1_TERMICO_RETORNO_PIN  6
#define BOMBA2_TERMICO_RETORNO_PIN  7

// --- CONSTANTES ---
#define NONE 0
#define BOMBA1 1
#define BOMBA2 2

#define BOMBA_USES_MAX 1
#define BOMBA_TURNING_ON_TIME 5000 //tiempo en milisegundos que espera a que el contactor avise que se cerro.
#define BOMBA_TURNING_OFF_TIME 5000 //tiempo que espera hasta que el contactor avise que se abrio.

#define BOMBA_CONTACTOR_ERROR_INTENTOS_MAX 0 //Maxima cantidad de intentos
#define BOMBA_CONTACTOR_ERROR_INTERVAL 10000 //Intervalo de tiempo entre intentos de recuperar el contactor

#define BOMBA_FILL_TIME_HOURS_MAX 6//Tiempo de llenado inicial del tanque en horas. Un numero grande para que no se corte antes de tiempo
#define BOMBA_REFRESH_WORKING_TIME 60000 //Tiempo entre refrescos de tiempo de trabajo de las bombas (milisegundos)

// ====================== CISTERNA/TANQUE ======================
// --- CISTERNA ---
#define CISTERNA_EMPTY_PIN  8
#define CISTERNA_EMPTY_MAX_TIME 10000 //tiempo (milisegundos) que espera antes de hacer sonar la alarma por cisterna vacia....no se esta llenando

// --- TANQUE ---
#define TANQUE_EMPTY_FULL_PIN  9
#define TANQUE_TIME_TO_FULL 10000 //tiempo (millisegundos) inicial para el llenado del tanque. Luego calcula el tiempo promedio por tanque
#define TANQUE_TIME_TO_FULL_AVERAGE_PERCENTAGE 2 //valor a multiplicar por el tiempo de llenado promedio para obtener el limite maximo de llenado


// ====================== ESTADISTICAS ======================
//#define STATISTICS_SAVE_ENABLED
const unsigned long STATISTICS_TIME_TO_SAVE = (24 / 3) * (60 * 60 * 1000); //3 veces por dia. Valor expresado en milisegundos
#define CLEAN_STADISTICS_PRESS_TIME 5000 //10 segundos - tiempo (milisegundos) que se tiene que tener presionado el boton reset para limpiar las estadisticas



// ****************************************************************** //
//                            ESTADOS
// ****************************************************************** //
// ====================== BOMBA ESTADOS ======================
#define BOMBA_STATE_ON 1
#define BOMBA_STATE_OFF 2
#define BOMBA_STATE_ERROR_CONTACTOR_ABIERTO -1
#define BOMBA_STATE_ERROR_CONTACTOR_CERRADO -2
#define BOMBA_STATE_ERROR_TERMICO -3
#define BOMBA_STATE_ERROR_FILL_TIMEOUT -4


// ====================== FSM BOMBA ESTADOS ======================
#define FSM_BOMBA_NULL 100
#define FSM_BOMBA_OFF 101
#define FSM_BOMBA_TURNING_ON 102
#define FSM_BOMBA_ON 103
#define FSM_BOMBA_TURNING_OFF 104
#define FSM_BOMBA_ERROR_CONTACTOR_CERRADO 105
#define FSM_BOMBA_ERROR_CONTACTOR_ABIERTO 106
#define FSM_BOMBA_ERROR_TERMICO 107
#define FSM_BOMBA_DISABLING 108
#define FSM_BOMBA_DISABLED 109
#define FSM_BOMBA_ENABLING 110

// ====================== FSM AUTO ESTADOS ======================
#define AUTO_NULL 200
#define AUTO_IDLE 201
#define AUTO_NOT_AVAILABLES_BOMBAS 203
#define AUTO_STARTING 204
#define AUTO_WORKING 205
#define AUTO_STOPPING 206
#define AUTO_CHANGE_BOMBA_FROM_NOT_AVAILABLE 209
#define AUTO_CHANGE_BOMBA_FROM_FILL_TIMEOUT 210
#define AUTO_CHANGE_BOMBA 211
#define AUTO_ERROR_BOMBA_WORKING 212


// ****************************************************************** //
//                        ESTRUCTURAS DE DATOS
// ****************************************************************** //

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

  unsigned int FillTimeMinutes[10];
  unsigned int FillTimeMinutesAverage;

  unsigned long StartTime;
  unsigned long RefreshTime;

  long ContactorErrorCounter;
  unsigned long StartError;
  unsigned long Timer; //se usa para controlar el tiempo de arranque o de paro
} Bomba;


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


//--- ESTADISTICAS ---
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

  unsigned long FaseErrorBeginTime;
  boolean Changed;
} Statistics;


//--- FASES ---
typedef struct {
  int Voltage;
  boolean IsOk;

  int InputVoltsReference;
  int ConversionFactor;

  unsigned long ReadTotal;
  byte ReadCount;
  unsigned long LastRead;
} Fase;


//--- FSM AUTO ---
typedef struct {
  byte FromState;
  byte State;
  byte NextState;

  byte Mode;

  boolean IsFaseOk;
  unsigned long StoppingTimer;
} AutoFSM;


//--- VIEW ---
typedef struct {
  boolean IsMainViewActive = true;
  boolean IsErrorFaseViewActive = false;
  boolean IsInfoViewActive = false;

  unsigned long FaseViewLastRefreshTime = 0;

  byte InfoViewNumberActive = 0;
  unsigned long InfoViewNumberActiveTime = 0;
} View;



// ****************************************************************** //
//                       VARIABLES DE SISTEMA
// ****************************************************************** //

Sensor sensores = {false, false, false};
Bomba bomba1 = {BOMBA1};
Bomba bomba2 = {BOMBA2};
Alarm alarm = { };
Statistics statistics = { };
AutoFSM automaticFSM = { };
Fase fase1 = { };
Fase fase2 = { };
Fase fase3 = { };
View view = {};



// ****************************************************************** //
//                               SETUP
// ****************************************************************** //
void setup() {
  //start serial connection
  Serial.begin(115200);

  automaticFSM.Mode = AUTO;

  SetupPins();
  Serial.println(F("Pins - Ready"));

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
  Serial.println(F("Fases Sensor - Ready"));

  SetupCommands();
  Serial.println(F("Commands - Ready"));

  SetupStatistics();
  Serial.println(F("Statistics - Ready"));

  automaticFSM.IsFaseOk = true;
  automaticFSM.FromState = AUTO_IDLE;
  automaticFSM.State = AUTO_IDLE;
  automaticFSM.NextState = AUTO_NULL;
  Serial.println(F("Main FSM  - Ready"));

  // put your setup code here, to run once:
  SetupDisplay();
  Serial.println(F("Display - Ready"));

  Serial.println(F("Process - Ready"));

}


//************************************************//
//                     LOOP
//************************************************//

void loop() {
  ReadCommands();

  //leo el modo de ejecución (MANUAL o AUTOMATICO)
  ReadExecutionMode();

  //Veo el reset
  ReadResetAndClearStatisticsButton();

  //Leo el boton Swap
  ReadSwapButton();

  //valido los niveles para visualizar en el display
  ReadTanqueSensors();

  //valido la cisterna. Actualizo el estado de la variable.
  ReadCisternaSensors();

  //leo los sensores de las bombas
  ReadBombaSensors();

  //leo la habilitación de las bombas
  ReadEnabledBombas();

  //leo el estado de las fases
  //Las bombas se detienen asi que el circuito seguiria normal.......pero no deberia arrancar
  ReadFases();

  //ejecuta la alarma si corresponde
  ReadAlarm();

  //actualizo la vista si corresponde
  RefreshViews();

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
#ifdef DEBUG
  pinMode(DEBUG_CONTINUE_PIN, INPUT_PULLUP);
  //digitalWrite(DEBUG_CONTINUE_PIN, HIGH);
#endif

  // --- BUTTON ---
  pinMode(CHANGE_MODE_BTN_PIN, INPUT_PULLUP);
  //digitalWrite(CHANGE_MODE_BTN_PIN, HIGH);

  pinMode(BOMBA_SWAP_BTN_PIN, INPUT_PULLUP);
  //digitalWrite(BOMBA_SWAP_BTN_PIN, HIGH);

  pinMode(RESET_BTN_PIN, INPUT_PULLUP);
  //digitalWrite(RESET_BTN_PIN, HIGH);

  pinMode(VIEW_INFO_PIN, INPUT_PULLUP);
  //digitalWrite(VIEW_INFO_PIN, HIGH);

  // --- INVERSOR ---
  pinMode(BOMBA_SWAP_RELE_PIN, OUTPUT);

  // --- ALARMA ---
  pinMode(ALARM_PIN, OUTPUT);

  // --- SENSOR FASE ---
#ifdef FASE1_ENABLED
  pinMode(FASE1_INPUT_PIN, INPUT);
#endif
#ifdef FASE1_ENABLED
  pinMode(FASE2_INPUT_PIN, INPUT);
#endif
#ifdef FASE1_ENABLED
  pinMode(FASE3_INPUT_PIN, INPUT);
#endif
  pinMode(FASE_OUTPUT_PIN, OUTPUT);

  // --- SENSORES BOMBAS ---
  pinMode(BOMBA1_ENABLE_PIN, INPUT_PULLUP);
  //digitalWrite(BOMBA1_ENABLE_PIN, HIGH);

  pinMode(BOMBA2_ENABLE_PIN, INPUT_PULLUP);
  //digitalWrite(BOMBA2_ENABLE_PIN, HIGH);

  pinMode(BOMBA1_CONTACTOR_RETORNO_PIN, INPUT_PULLUP);
  //digitalWrite(BOMBA1_CONTACTOR_RETORNO_PIN, HIGH);

  pinMode(BOMBA2_CONTACTOR_RETORNO_PIN, INPUT_PULLUP);
  //digitalWrite(BOMBA2_CONTACTOR_RETORNO_PIN, HIGH);

  pinMode(BOMBA1_TERMICO_RETORNO_PIN, INPUT_PULLUP);
  //digitalWrite(BOMBA1_TERMICO_RETORNO_PIN, HIGH);

  pinMode(BOMBA2_TERMICO_RETORNO_PIN, INPUT_PULLUP);
  //digitalWrite(BOMBA2_TERMICO_RETORNO_PIN, HIGH);

  // --- SENSOR DE NIVELES ---
  pinMode(CISTERNA_EMPTY_PIN, INPUT_PULLUP);
  //digitalWrite(CISTERNA_EMPTY_PIN, HIGH);
  pinMode(TANQUE_EMPTY_FULL_PIN, INPUT_PULLUP);
  //digitalWrite(TANQUE_EMPTY_FULL_PIN, HIGH);
}

inline unsigned long deltaMillis(unsigned long currRead, unsigned long prevRead)
{
  if (currRead >= prevRead)
    return currRead - prevRead;
  else
    return currRead; //volvio a cero...uso este valor como referencia....pierdo prevRead hasta el maximo..pero pasa 1 vez cada 47 dias
}
