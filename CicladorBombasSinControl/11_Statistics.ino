#include <EEPROM.h>

void SetupStatistics()
{
  CleanStatistics(false);
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
  if (bomba->StartTime == 0)
    return;

  if (bomba->Number == BOMBA1)
    statistics.Bomba1Uses = statistics.Bomba1Uses + 1;
  else
    statistics.Bomba2Uses = statistics.Bomba2Uses + 1;

  statistics.Changed = true;
}


void Statistics_BombaOff(Bomba* bomba, boolean registerFillTime)
{
  /*
    if (bomba->Number == BOMBA1)
    {
      if (bomba->StartTime > 0)
      {
        unsigned long delta1 = deltaMillis(millis(), bomba->StartTime);
        unsigned int seconds1 = delta1 / 1000;
        if (registerFillTime)
          Stadistics_AddFillTime(bomba, seconds1);

        statistics.Bomba1TotalMinutes = statistics.Bomba1TotalMinutes + (seconds1 / 60); //minutos
        statistics.Changed = true;
      }
    }
    else
    {
      if (bomba->StartTime > 0)
      {
        unsigned long delta2 = deltaMillis(millis(), bomba->StartTime);
        unsigned int seconds2 = delta2 / 1000;
        if (registerFillTime)
          Stadistics_AddFillTime(bomba, seconds2);

        statistics.Bomba2TotalMinutes = statistics.Bomba2TotalMinutes + (seconds2 / 60); //minutos
        statistics.Changed = true;
      }
    }
  */
  if (bomba->StartTime > 0)
  {
    unsigned long delta1 = deltaMillis(millis(), bomba->StartTime);
    unsigned int seconds1 = delta1 / 1000;
    if (registerFillTime)
      Stadistics_AddFillTime(bomba, seconds1);

    if (bomba->Number == BOMBA1)
      statistics.Bomba1TotalMinutes = statistics.Bomba1TotalMinutes + (seconds1 / 60); //minutos
    else
      statistics.Bomba2TotalMinutes = statistics.Bomba2TotalMinutes + (seconds1 / 60); //minutos

    statistics.Changed = true;
  }
}

void Stadistics_AddFillTime(Bomba* bomba, unsigned long fillTime)
{
  unsigned long total = fillTime;
  byte count = 1;

  for (int i = 8; i >= 0; i--)
  {
    bomba->FillTimeSeconds[i + 1] = bomba->FillTimeSeconds[i];

    if (bomba->FillTimeSeconds[i + 1] > 0)
    {
      count++;
      total += bomba->FillTimeSeconds[i + 1];
    }
  }

  bomba->FillTimeSeconds[0] = fillTime;
  bomba->FillTimeSecondsAverage = total / count;
}

void Statistics_BombaErrorTermico(Bomba* bomba)
{
  if (bomba->Number == BOMBA1)
    statistics.Bomba1ErrorTermicoCount = statistics.Bomba1ErrorTermicoCount + 1;
  else
    statistics.Bomba2ErrorTermicoCount = statistics.Bomba2ErrorTermicoCount + 1;

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

void CleanStatistics(boolean save)
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

  statistics.FaseErrorBeginTime = 0;

  statistics.LastTimeSaved = millis();

  //guardo las estadisticas reiniciadas
  if (save)
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
    EEPROM.put(BOMBA1_FILLING_TIME_MINUTES_ADDR, bomba1.FillTimeMinutesAverage);

  EEPROM.get(BOMBA2_FILLING_TIME_MINUTES_ADDR, iAux);
  if (iAux != bomba2.FillTimeMinutesAverage)
    EEPROM.put(BOMBA2_FILLING_TIME_MINUTES_ADDR, bomba2.FillTimeMinutesAverage);



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

void PrintStatistics()
{
  Serial.println(F("*** ESTADISTICAS ***"));
  PrintStatisticsBomba(statistics.Bomba1Uses, statistics.Bomba1TotalMinutes, statistics.Bomba1ErrorTermicoCount, &bomba1);
  Serial.println();
  PrintStatisticsBomba(statistics.Bomba2Uses, statistics.Bomba2TotalMinutes, statistics.Bomba2ErrorTermicoCount, &bomba2);
  Serial.println();

  Serial.println();
  Serial.println(F("*** ERR DE FASE ***"));

  Serial.print(F("# Err: "));
  Serial.println(statistics.ErrorFaseCount);

  Serial.print(F("Tiempo (minutos): "));
  Serial.println(statistics.ErrorFaseTotalMinutes);
}

void PrintStatisticsBomba(unsigned long uses, unsigned long min, unsigned long errTerm, Bomba* bomba)
{
  Serial.print(F("*** BOMBA "));
  Serial.print(bomba->Number);
  Serial.println(F(" ***"));

  Serial.print(F("# Usos: "));
  Serial.println(uses);

  Serial.print(F("Usos min: "));
  Serial.println(min);

  Serial.print(F("# ERR termico: "));
  Serial.println(errTerm);

  Serial.print(F("Tiempo llenado seg.: "));
  Serial.println(bomba->FillTimeSecondsAverage);
}
