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
    if ((curr - statistics.LastTimeSaved) > STATISTICS_TIME_TO_SAVE)
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
    if (statistics.Bomba1OnTime > 0)
      return;

    statistics.Bomba1Uses = statistics.Bomba1Uses + 1;
    statistics.Bomba1OnTime = millis();
  }
  else
  {
    if (statistics.Bomba2OnTime > 0)
      return;

    statistics.Bomba2Uses = statistics.Bomba2Uses + 1;
    statistics.Bomba2OnTime = millis();
  }

  statistics.Changed = true;
}

unsigned long Statistics_BombaOff(Bomba* bomba)
{
  if (bomba->Number == BOMBA1)
  {
    if (statistics.Bomba1OnTime > 0)
    {
      unsigned long delta1 = millis() - statistics.Bomba2OnTime;
      unsigned int minutes1 = delta1 / 60000;
      Stadistics_AddFillTime(bomba, minutes1);

      statistics.Bomba1TotalMinutes = statistics.Bomba1TotalMinutes + minutes1; //minutos
      statistics.Bomba1OnTime = 0;
      statistics.Changed = true;
    }
  }
  else
  {
    if (statistics.Bomba2OnTime > 0)
    {
      unsigned long delta2 = millis() - statistics.Bomba2OnTime;
      unsigned int minutes2 = delta2 / 60000;
      Stadistics_AddFillTime(bomba, minutes2);

      statistics.Bomba2TotalMinutes = statistics.Bomba2TotalMinutes + minutes2; //minutos
      statistics.Bomba1OnTime = 0;
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
    bomba->FillTimes[i + 1] = bomba->FillTimes[i];

    if (bomba->FillTimes[i + 1] > 0)
    {
      count++;
      total += bomba->FillTimes[i + 1];
    }
  }

  bomba->FillTimes[0] = fillTime;
  bomba->FillTimeAverage = total / count;
}

void Statistics_BombaErrorTermico(Bomba* bomba)
{
  if (bomba->Number == BOMBA1)
  {
    statistics.Bomba1ErrorTermicoCount = statistics.Bomba1ErrorTermicoCount + 1;
  }
  else
  {
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

  unsigned long delta = millis() - statistics.FaseErrorBeginTime;
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

  statistics.Changed = true;

  statistics.Bomba1OnTime = 0;
  statistics.Bomba2OnTime = 0;
  statistics.FaseErrorBeginTime = 0;
}

void DoSaveStatistics()
{
#ifdef STATISTICS_SAVE_ENABLED

#endif
}

void ReadStatistics()
{
#ifdef STATISTICS_SAVE_ENABLED

#endif
}


long EEPROMReadlong(long address) {
  long four = EEPROM.read(address);
  long three = EEPROM.read(address + 1);
  long two = EEPROM.read(address + 2);
  long one = EEPROM.read(address + 3);

  return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
}

void EEPROMWritelong(int address, long value) {
  byte four = (value & 0xFF);
  byte three = ((value >> 8) & 0xFF);
  byte two = ((value >> 16) & 0xFF);
  byte one = ((value >> 24) & 0xFF);

  EEPROM.write(address, four);
  EEPROM.write(address + 1, three);
  EEPROM.write(address + 2, two);
  EEPROM.write(address + 3, one);
}
