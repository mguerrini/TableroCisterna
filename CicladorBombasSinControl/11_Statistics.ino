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
  if (bomba->StartTime > 0)
  {
    unsigned long delta1 = deltaMillis(millis(), bomba->StartTime);
    unsigned int seconds1 = delta1 / 1000;
    if (registerFillTime)
      Stadistics_AddFillTime(bomba, seconds1);

    if (bomba->Number == BOMBA1)
      statistics.Bomba1TotalSeconds = statistics.Bomba1TotalSeconds + seconds1;
    else
      statistics.Bomba2TotalSeconds = statistics.Bomba2TotalSeconds + seconds1;

    statistics.Changed = true;
  }
}

void Stadistics_AddFillTime(Bomba* bomba, unsigned long fillTime)
{
  unsigned long total = fillTime;
  byte count = 1;

  for (int i = BOMBA_FILLTIMES_READ_MAX - 2; i >= 0; i--)
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

  if (bomba->FillTimeSecondsMaximum < fillTime)
    bomba->FillTimeSecondsMaximum = fillTime;
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
  statistics.ErrorFaseTotalSeconds = statistics.ErrorFaseTotalSeconds + (delta / 1000); //minutos
  statistics.Changed = true;
}

void CleanStatistics(boolean save)
{
  statistics.Bomba1Uses = 0;
  statistics.Bomba2Uses = 0;
  statistics.Bomba1TotalSeconds = 0;
  statistics.Bomba2TotalSeconds = 0;
  statistics.Bomba1ErrorTermicoCount = 0;
  statistics.Bomba2ErrorTermicoCount = 0;

  statistics.ErrorFaseTotalSeconds = 0;
  statistics.ErrorFaseCount = 0;

  statistics.FaseErrorBeginTime = 0;
  statistics.LastTimeSaved = millis();

  //borro los tiempos de llenado
  CleanFillTimes();
  statistics.Changed = false;

  //guardo las estadisticas reiniciadas
  if (save)
    DoSaveStatistics();

  //hago sonar el buzzer
  Alarm_Pip();
}

void DoSaveStatistics()
{
#ifdef EEPROM_ENABLED
#ifndef LOG_ENABLED

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
  EEPROM.get(BOMBA1_TOTAL_SECONDS_ADDR, lAux);
  if (lAux != statistics.Bomba1TotalSeconds)
    EEPROM.put(BOMBA1_TOTAL_SECONDS_ADDR, statistics.Bomba1TotalSeconds);

  EEPROM.get(BOMBA2_TOTAL_SECONDS_ADDR, lAux);
  if (lAux != statistics.Bomba2TotalSeconds)
    EEPROM.put(BOMBA2_TOTAL_SECONDS_ADDR, statistics.Bomba2TotalSeconds);



  //Cantidad de errores de termico
  EEPROM.get(BOMBA1_ERROR_TERMICO_COUNT_ADDR, lAux);
  if (lAux != statistics.Bomba1ErrorTermicoCount)
    EEPROM.put(BOMBA1_ERROR_TERMICO_COUNT_ADDR, statistics.Bomba1ErrorTermicoCount);

  EEPROM.get(BOMBA2_ERROR_TERMICO_COUNT_ADDR, lAux);
  if (lAux != statistics.Bomba2ErrorTermicoCount)
    EEPROM.put(BOMBA2_ERROR_TERMICO_COUNT_ADDR, statistics.Bomba2ErrorTermicoCount);



  //tiempo de llenado
  EEPROM.get(BOMBA1_FILLING_TIME_SECONDS_AVG_ADDR, lAux);
  if (lAux != bomba1.FillTimeSecondsAverage)
    EEPROM.put(BOMBA1_FILLING_TIME_SECONDS_AVG_ADDR, bomba1.FillTimeSecondsAverage);

  EEPROM.get(BOMBA2_FILLING_TIME_SECONDS_AVG_ADDR, lAux);
  if (lAux != bomba2.FillTimeSecondsAverage)
    EEPROM.put(BOMBA2_FILLING_TIME_SECONDS_AVG_ADDR, bomba2.FillTimeSecondsAverage);

  EEPROM.get(BOMBA1_FILLING_TIME_SECONDS_MAX_ADDR, lAux);
  if (lAux != bomba1.FillTimeSecondsMaximum)
    EEPROM.put(BOMBA1_FILLING_TIME_SECONDS_MAX_ADDR, bomba1.FillTimeSecondsMaximum);

  EEPROM.get(BOMBA2_FILLING_TIME_SECONDS_MAX_ADDR, lAux);
  if (lAux != bomba2.FillTimeSecondsMaximum)
    EEPROM.put(BOMBA2_FILLING_TIME_SECONDS_MAX_ADDR, bomba2.FillTimeSecondsMaximum);




  //Cantidad de errores de fase
  EEPROM.get(ERROR_FASE_COUNT_ADDR, lAux);
  if (lAux != statistics.ErrorFaseCount)
    EEPROM.put(ERROR_FASE_COUNT_ADDR, statistics.ErrorFaseCount);

  //cantidad de minutos por error de fase
  EEPROM.get(ERROR_FASE_TOTAL_SECONDS_ADDR, lAux);
  if (lAux != statistics.ErrorFaseTotalSeconds)
    EEPROM.put(ERROR_FASE_TOTAL_SECONDS_ADDR, statistics.ErrorFaseTotalSeconds);

#endif
#endif
}

void ReadStatistics()
{
#ifdef EEPROM_ENABLED
#ifndef LOG_ENABLED
  unsigned long lAux = 0;
  unsigned int iAux = 0;

  //cantidad de usos
  EEPROM.get(BOMBA1_USES_ADDR, lAux);
  statistics.Bomba1Uses = lAux;

  EEPROM.get(BOMBA2_USES_ADDR, lAux);
  statistics.Bomba2Uses = lAux;


  //Minutos de uso
  EEPROM.get(BOMBA1_TOTAL_SECONDS_ADDR, lAux);
  statistics.Bomba1TotalSeconds = lAux;

  EEPROM.get(BOMBA2_TOTAL_SECONDS_ADDR, lAux);
  statistics.Bomba2TotalSeconds = lAux;


  //Cantidad de errores de termico
  EEPROM.get(BOMBA1_ERROR_TERMICO_COUNT_ADDR, lAux);
  statistics.Bomba1ErrorTermicoCount = lAux;

  EEPROM.get(BOMBA2_ERROR_TERMICO_COUNT_ADDR, lAux);
  statistics.Bomba2ErrorTermicoCount = lAux;


  //tiempo de llenado
  EEPROM.get(BOMBA1_FILLING_TIME_SECONDS_AVG_ADDR, lAux);
  bomba1.FillTimeSecondsAverage = lAux;

  EEPROM.get(BOMBA2_FILLING_TIME_SECONDS_AVG_ADDR, lAux);
  bomba2.FillTimeSecondsAverage = lAux;

  EEPROM.get(BOMBA1_FILLING_TIME_SECONDS_MAX_ADDR, lAux);
  bomba1.FillTimeSecondsMaximum = lAux;

  EEPROM.get(BOMBA2_FILLING_TIME_SECONDS_MAX_ADDR, lAux);
  bomba2.FillTimeSecondsMaximum = iAux;

  //Cantidad de errores de fase
  EEPROM.get(ERROR_FASE_COUNT_ADDR, lAux);
  statistics.ErrorFaseCount = lAux;

  //cantidad de minutos por error de fase
  EEPROM.get(ERROR_FASE_TOTAL_SECONDS_ADDR, lAux);
  statistics.ErrorFaseTotalSeconds = lAux;
#endif
#endif
}

void PrintStatistics()
{
#ifndef LOG_ENABLED
  PrintStatisticsBomba(statistics.Bomba1Uses, statistics.Bomba1TotalSeconds, statistics.Bomba1ErrorTermicoCount, &bomba1);
  Serial.println();
  PrintStatisticsBomba(statistics.Bomba2Uses, statistics.Bomba2TotalSeconds, statistics.Bomba2ErrorTermicoCount, &bomba2);
  Serial.println();

  Serial.println(F("** ERR DE FASE **"));

  Serial.print(F("# Err: "));
  Serial.println(statistics.ErrorFaseCount);

  Serial.print(F("T seg: "));
  Serial.println(statistics.ErrorFaseTotalSeconds);

#endif
}

void PrintStatisticsBomba(unsigned long uses, unsigned long sec, unsigned long errTerm, Bomba* bomba)
{
#ifndef LOG_ENABLED

  Serial.print(F("** BOMBA "));
  Serial.print(bomba->Number);
  Serial.println(F(" **"));

  Serial.print(F("# Usos: "));
  Serial.println(uses);

  Serial.print(F("ON seg: "));
  Serial.println(sec);

  Serial.print(F("# ERR termico: "));
  Serial.println(errTerm);

  Serial.print(F("Llenado seg: "));
  Serial.println(bomba->FillTimeSecondsAverage);

#endif
}
