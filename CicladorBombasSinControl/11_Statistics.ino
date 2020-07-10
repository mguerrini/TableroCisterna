#include <EEPROM.h>

void SetupStatistics()
{
  CleanStatistics();
  ReadStatistics();
  statistics.LastTimeSaved = millis();
}


void SaveStatistics()
{
  if (statistics.Changed)
  {
    unsigned long curr = millis();
    unsigned long delta = deltaMillis(curr,  statistics.LastTimeSaved);

    if (delta > STATISTICS_TIME_TO_SAVE)
    {
      statistics.LastTimeSaved = curr;
      statistics.Changed = false;
      DoSaveStatistics();
    }
  }
}

void Statistics_BombaOn(Bomba* bomba)
{
  if (bomba->Number == BOMBA1)
  {
    if (bomba->StartTime == 0)
      return;

    statistics.Bomba1Uses = statistics.Bomba1Uses + 1;
  }
  else
  {
    if (bomba->StartTime == 0)
      return;

    statistics.Bomba2Uses = statistics.Bomba2Uses + 1;
  }

  statistics.Changed = true;
}


void Statistics_BombaOff(Bomba* bomba, boolean registerFillTime)
{
  if (bomba->Number == BOMBA1)
  {
    if (bomba->StartTime > 0)
    {
      unsigned long delta1 = deltaMillis(millis(), bomba->StartTime);
      unsigned int minutes1 = delta1 / 60000;
      if (registerFillTime)
        Stadistics_AddFillTime(bomba, minutes1);

      statistics.Bomba1TotalMinutes = statistics.Bomba1TotalMinutes + minutes1; //minutos
      //statistics.Bomba1OnTime = 0;
      statistics.Changed = true;
    }
  }
  else
  {
    if (bomba->StartTime > 0)
    {
      unsigned long delta2 = deltaMillis(millis(), bomba->StartTime);
      unsigned int minutes2 = delta2 / 60000;
      if (registerFillTime)
        Stadistics_AddFillTime(bomba, minutes2);

      statistics.Bomba2TotalMinutes = statistics.Bomba2TotalMinutes + minutes2; //minutos
      //statistics.Bomba2OnTime = 0;
      statistics.Changed = true;
    }
  }
}

void Stadistics_AddFillTime(Bomba* bomba, unsigned int fillTime)
{
  unsigned int total = fillTime;
  byte count = 1;

  for (int i = 8; i >= 0; i--)
  {
    bomba->FillTimeMinutes[i + 1] = bomba->FillTimeMinutes[i];

    if (bomba->FillTimeMinutes[i + 1] > 0)
    {
      count++;
      total += bomba->FillTimeMinutes[i + 1];
    }
  }

  bomba->FillTimeMinutes[0] = fillTime;
  bomba->FillTimeMinutesAverage = total / count;
}

void Statistics_BombaErrorTermico(Bomba* bomba)
{
  if (bomba->Number == BOMBA1)
  {
    //statistics.Bomba1OnTime = 0;
    statistics.Bomba1ErrorTermicoCount = statistics.Bomba1ErrorTermicoCount + 1;
  }
  else
  {
    //statistics.Bomba2OnTime = 0;
    statistics.Bomba2ErrorTermicoCount = statistics.Bomba2ErrorTermicoCount + 1;
  }

  statistics.Changed = true;
}

void Statistics_FaseError_Begin()
{
  if (statistics.FaseErrorBeginTime > 0)
    return;

  statistics.FaseErrorBeginTime = millis();
  statistics.ErrorFaseCount = statistics.ErrorFaseCount + 1;
  statistics.Changed = true;
}

void Statistics_FaseError_End()
{
  if (statistics.FaseErrorBeginTime <= 0)
    return;

  unsigned long delta = deltaMillis(millis(), statistics.FaseErrorBeginTime);
  statistics.FaseErrorBeginTime = 0;
  statistics.ErrorFaseTotalMinutes = statistics.ErrorFaseTotalMinutes + (delta / 60000); //minutos
  statistics.Changed = true;
}

void CleanStatistics()
{
  statistics.Bomba1Uses = 0;
  statistics.Bomba2Uses = 0;
  statistics.Bomba1TotalMinutes = 0;
  statistics.Bomba2TotalMinutes = 0;
  statistics.Bomba1ErrorTermicoCount = 0;
  statistics.Bomba2ErrorTermicoCount = 0;

  statistics.ErrorFaseTotalMinutes = 0;
  statistics.ErrorFaseCount = 0;

  statistics.Changed = false;

  //statistics.Bomba1OnTime = 0;
  //statistics.Bomba2OnTime = 0;
  statistics.FaseErrorBeginTime = 0;

  statistics.LastTimeSaved = millis();

  //guardo las estadisticas reiniciadas
  DoSaveStatistics();
}

void DoSaveStatistics()
{

#ifdef STATISTICS_SAVE_ENABLED
  unsigned long lAux = 0;
  unsigned int iAux = 0;

  //cantidad de usos
  EEPROM.get(BOMBA1_USES_ADDR, lAux);
  if (lAux != statistics.Bomba1Uses)
    EEPROM.put(BOMBA1_USES_ADDR, statistics.Bomba1Uses);

  EEPROM.get(BOMBA2_USES_ADDR, lAux);
  if (lAux != statistics.Bomba2Uses)
    EEPROM.put(BOMBA2_USES_ADDR, statistics.Bomba2Uses);



  //Minutos de uso
  EEPROM.get(BOMBA1_TOTAL_MINUTES_ADDR, lAux);
  if (lAux != statistics.Bomba1TotalMinutes)
    EEPROM.put(BOMBA1_TOTAL_MINUTES_ADDR, statistics.Bomba1TotalMinutes);

  EEPROM.get(BOMBA2_TOTAL_MINUTES_ADDR, lAux);
  if (lAux != statistics.Bomba2TotalMinutes)
    EEPROM.put(BOMBA2_TOTAL_MINUTES_ADDR, statistics.Bomba2TotalMinutes);



  //Cantidad de errores de termico
  EEPROM.get(BOMBA1_ERROR_TERMICO_COUNT_ADDR, lAux);
  if (lAux != statistics.Bomba1ErrorTermicoCount)
    EEPROM.put(BOMBA1_ERROR_TERMICO_COUNT_ADDR, statistics.Bomba1ErrorTermicoCount);

  EEPROM.get(BOMBA2_ERROR_TERMICO_COUNT_ADDR, lAux);
  if (lAux != statistics.Bomba2ErrorTermicoCount)
    EEPROM.put(BOMBA2_ERROR_TERMICO_COUNT_ADDR, statistics.Bomba2ErrorTermicoCount);



  //tiempo de llenado
  EEPROM.get(BOMBA1_FILLING_TIME_MINUTES_ADDR, iAux);
  if (iAux != bomba1.FillTimeMinutesAverage)
    EEPROM.put(BOMBA1_FILLING_TIME_MINUTES_ADDR, bomba1.Bomba1ErrorTermicoCount);

  EEPROM.get(BOMBA2_FILLING_TIME_MINUTES_ADDR, iAux);
  if (iAux != bomba2.FillTimeMinutesAverage)
    EEPROM.put(BOMBA2_FILLING_TIME_MINUTES_ADDR, bomba2.Bomba2ErrorTermicoCount);



  //Cantidad de errores de fase
  EEPROM.get(ERROR_FASE_COUNT_ADDR, lAux);
  if (lAux != statistics.ErrorFaseCount)
    EEPROM.put(ERROR_FASE_COUNT_ADDR, statistics.ErrorFaseCount);


  //cantidad de minutos por error de fase
  EEPROM.get(ERROR_FASE_TOTAL_MINUTES_ADDR, lAux);
  if (lAux != statistics.ErrorFaseTotalMinutes)
    EEPROM.put(ERROR_FASE_TOTAL_MINUTES_ADDR, statistics.ErrorFaseTotalMinutes);

#endif
}

void ReadStatistics()
{
#ifdef STATISTICS_SAVE_ENABLED
  unsigned long lAux = 0;
  unsigned int iAux = 0;

  //cantidad de usos
  EEPROM.get(BOMBA1_USES_ADDR, lAux);
  statistics.Bomba1Uses = lAux;

  EEPROM.get(BOMBA2_USES_ADDR, lAux);
  statistics.Bomba2Uses = lAux;


  //Minutos de uso
  EEPROM.get(BOMBA1_TOTAL_MINUTES_ADDR, lAux);
  statistics.Bomba1TotalMinutes = lAux;

  EEPROM.get(BOMBA2_TOTAL_MINUTES_ADDR, lAux);
  statistics.Bomba2TotalMinutes = lAux;


  //Cantidad de errores de termico
  EEPROM.get(BOMBA1_ERROR_TERMICO_COUNT_ADDR, lAux);
  statistics.Bomba1ErrorTermicoCount = lAux;

  EEPROM.get(BOMBA2_ERROR_TERMICO_COUNT_ADDR, lAux);
  statistics.Bomba2ErrorTermicoCount = lAux;


  //tiempo de llenado
  EEPROM.get(BOMBA1_FILLING_TIME_MINUTES_ADDR, iAux);
  bomba1.FillTimeMinutesAverage = iAux;

  EEPROM.get(BOMBA2_FILLING_TIME_MINUTES_ADDR, iAux);
  bomba2.FillTimeMinutesAverage = iAux;


  //Cantidad de errores de fase
  EEPROM.get(ERROR_FASE_COUNT_ADDR, lAux);
  statistics.ErrorFaseCount = lAux;


  //cantidad de minutos por error de fase
  EEPROM.get(ERROR_FASE_TOTAL_MINUTES_ADDR, lAux);
  statistics.ErrorFaseTotalMinutes = lAux;

#endif
}

void printStatistics()
{
  Serial.println(F("Valores Estadisticos"));
  Serial.println(F("BOMBA 1"));

  Serial.print(F("Cantidad de Usos: "));
  Serial.println(statistics.Bomba1Uses);

  Serial.print(F("Minutos totales de uso: "));
  Serial.println(statistics.Bomba1TotalMinutes);

  Serial.print(F("Cantidad de errores de termico: "));
  Serial.println(statistics.Bomba1ErrorTermicoCount);

  Serial.print(F("Tiempo promedio de llenado de tanque (minutos): "));
  Serial.println(bomba1.FillTimeMinutesAverage);

  Serial.println();
  Serial.println(F("BOMBA 2"));

  Serial.print(F("Cantidad de Usos: "));
  Serial.println(statistics.Bomba2Uses);

  Serial.print(F("Minutos totales de uso: "));
  Serial.println(statistics.Bomba2TotalMinutes);

  Serial.print(F("Cantidad de errores de termico: "));
  Serial.println(statistics.Bomba2ErrorTermicoCount);

  Serial.print(F("Tiempo promedio de llenado de tanque (minutos): "));
  Serial.println(bomba2.FillTimeMinutesAverage);

  Serial.println();
  Serial.println(F("ERRORES DE FASE"));

  Serial.print(F("Cantidad de errores de fase: "));
  Serial.println(statistics.ErrorFaseCount);

  Serial.print(F("Tiempo total de perdida de fase (minutos): "));
  Serial.println(statistics.ErrorFaseTotalMinutes);
}
