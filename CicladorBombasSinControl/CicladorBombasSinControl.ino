
#define TEST

//DISPLAYS
#define DISPLAY_20x4_I2C

// --- DEBUG ---
#define DEBUG
#ifdef DEBUG
#define DEBUG_CONTINUE_PIN A3
#endif

//#define GET_STATUS_BUTTON_ENABLED
#ifdef GET_STATUS_BUTTON_ENABLED
#define GET_STATUS_BTN_PIN 2 //no se usa si no esta definido
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
#define FASE1_TENSION_ENTRADA_ADDR 0
#define FASE2_TENSION_ENTRADA_ADDR 2
#define FASE3_TENSION_ENTRADA_ADDR 4

#define FASE1_FACTOR_CONVERSION_ADDR 6
#define FASE2_FACTOR_CONVERSION_ADDR 8
#define FASE3_FACTOR_CONVERSION_ADDR 10

//longs
#define BOMBA1_USES_ADDR 20
#define BOMBA1_TOTAL_MINUTES_ADDR 24
#define BOMBA1_ERROR_TERMICO_COUNT_ADDR 28
#define BOMBA1_FILLING_TIME_MINUTES_ADDR 32

#define BOMBA2_USES_ADDR 40
#define BOMBA2_TOTAL_MINUTES_ADDR 44
#define BOMBA2_ERROR_TERMICO_COUNT_ADDR 48
#define BOMBA2_FILLING_TIME_MINUTES_ADDR 52

#define ERROR_FASE_TOTAL_MINUTES_ADDR 60
#define ERROR_FASE_COUNT_ADDR 64


// ****************************************************************** //
//                        CONFIGURACIONES
// ****************************************************************** //

// ====================== BOTONES VARIOS ====================
#define BOMBA_SWAP_BTN_PIN 12  //Cambiar bomba seleccionada
#define RESET_BTN_PIN 11
#define VIEW_INFO_PIN A0 //Muestra información estadisticas y valores varios

#define INFO_VIEW_VISIBLE_TIME 10000 //10 SEGUNDOS
#define IS_CHANGE_MODE_PULSADOR true //tipo de boton, pulsador o llave


// ====================== INVERSOR ======================
//salida al rele que activa el Rele de los contactores
#define BOMBA_SWAP_RELE_PIN A2
//valor para Bomba1 Activa
#define BOMBA1_ACTIVE  HIGH
//valor para Bomba2 Activa
#define BOMBA2_ACTIVE  LOW


// ====================== ALARMA ======================
//#define ALARM_BUZZER
#define ALARM_LED
#define ALARM_PIN 10

//#define ALARM_AUX_ENBALED
#ifdef ALARM_AUX_ENBALED
#define ALARM_PIN_AUX 0
#endif


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

#ifndef FASE_FROM_EEPROM_ENABLED
  #define TENSION_ENTRADA 5 //220 Volts, pero para testear lo pongo en 5
  //valores de referencia para el valor de 220V
  #define FASE1_220_VALUE 1023
  #define FASE2_220_VALUE 1023
  #define FASE3_220_VALUE 1023
#endif


#define FASE_MIN_VOLTAGE 3 //Valor minimo antes de que se considere falla. 3 para testear
#define FASE_READ_COUNT_MAX 50 //100 milisegundos donde se promedia el valor leiado
#define FASE_WAIT_BETWEEN_READS 10 //10 milisegundos de espera entre las mediciones
#define FASE_REFRESH_TIME 500 //milisegundos que refresca las tensiones

// ====================== MODO ======================
#define MANUAL 0
#define AUTO 1
#define CHANGE_MODE_BTN_PIN 13 //selector Manual / Automatico

#ifndef DEBUG
#define MODO_OUTPUT_VIEW_ENABLED
#endif

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
#define BOMBA_CONTACTOR_ERROR_INTENTOS_MAX 100 //Maxima cantidad de intentos
#define BOMBA_CONTACTOR_ERROR_INTERVAL 10000 //Intervalo de tiempo entre intentos de recuperar el contactor


// ====================== CISTERNA/TANQUE ======================
// --- CISTERNA ---
#define CISTERNA_EMPTY_PIN  8
#define CISTERNA_EMPTY_MAX_TIME 10000 //tiempo (milisegundos) que espera antes de hacer sonar la alarma por cisterna vacia....no se esta llenando

// --- TANQUE ---
#define TANQUE_EMPTY_FULL_PIN  9
#define TANQUE_TIME_TO_FULL 10000 //tiempo (millisegundos) inicial para el llenado del tanque. Luego calcula el tiempo promedio por tanque y le suma u porcentage


// ====================== ESTADISTICAS ======================
//#define STATISTICS_SAVE_ENABLED
const long STATISTICS_TIME_TO_SAVE = (1000 * 60) * 60; //una vez por hora...si cambia
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


// ====================== FSM BOMBA ESTADOS ======================
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

// ====================== FSM AUTO ESTADOS ======================
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

  long ContactorErrorCounter;
  unsigned long StartError;
  unsigned long Timer;
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

  unsigned long Bomba1OnTime;
  unsigned long Bomba2OnTime;
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


// ****************************************************************** //
//                               SETUP
// ****************************************************************** //
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
  /*
    ReadCommands();
  */
  //Veo el reset
  ReadResetAndClearStatisticsButton();
  /*
    ReadSwapButton();

    //valido los niveles para visualizar en el display
    ReadTanqueSensors();

    //valido la cisterna. Actualizo el estado de la variable.
    ReadCisternaSensors();

    //leo los sensores de las bombas
    ReadBombaSensors();

    //leo la habilitación de las bombas
    ReadEnabledBombas();
  */
  //leo el estado de las fases
  //Las bombas se detienen asi que el circuito seguiria normal.......pero no deberia arrancar
  ReadFases();
  /*
    //leo el modo de ejecución (MANUAL o AUTOMATICO)
    ReadExecutionMode();

    //ejecuta la alarma si corresponde
    ReadAlarm();

    ReadPrintStatus();
  */
  //actualizo la vista si corresponde
  UpdateView();
  /*
    #ifdef DEBUG
    if (!IsContinueButtonPressed())
      return;
    else
      Serial.println(F("Continue"));
    #endif

    // put your main code here, to run repeatedly:
    CicladorLoop();

    SaveStatistics();
  */
}


//************************************************//
//                 AUXILIARES
//************************************************//

// --- PINS ---

void SetupPins()
{
  // --- DEBUG ---
  //  pinMode(DEBUG_CONTINUE_PIN, INPUT_PULLUP);
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
}

inline unsigned long deltaMillis(unsigned long currRead, unsigned long prevRead)
{
  if (currRead >= prevRead)
    return currRead - prevRead;
  else
    return currRead; //volvio a cero...uso este valor como referencia....pierdo prevRead hasta el maximo..pero pasa 1 vez cada 47 dias
}
