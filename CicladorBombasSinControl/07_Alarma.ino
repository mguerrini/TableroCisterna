//************ VARIABLES ******************

const int TimeOn_Alarma = 500;
const int TimeOff_Alarma = 500;

const int TimeOn_AlarmaManual = 500;
const int TimeOff_AlarmaManual = 500;

const int TimeOn_AlarmaBomba = 500;
const int TimeOff_AlarmaBomba = 500;

typedef struct  {
  boolean IsBomba1AlarmON = false;
  boolean IsBomba2AlarmON = false;
  boolean IsManualAlarmON = false;
  boolean IsCisternaAlarmON = false;
  boolean IsNotAvailableBombasAlarmON = false;

  long ActiveTime;
  long InactiveTime;
  boolean IsActive;

  long StartTimeBomba1;
  long StartTimeBomba2;
  long StartCisternaAlarm;
  long StartTimeIsNotAvailableBombas;
} Alarm;

Alarm alarm = {};

//********************************************


void ReadAlarm()
{
  if (!alarm.IsManualAlarmON && !alarm.IsCisternaAlarmON && !alarm.IsBomba1AlarmON && !alarm.IsBomba2AlarmON && !alarm.IsNotAvailableBombasAlarmON)
    return;

  if (alarm.ActiveTime == 0)
  {
    //primera vez que se prende.
    alarm.ActiveTime = millis();
    alarm.IsActive=true;
    ActivateAlarm();
    return;
  }

  if (alarm.IsManualAlarmON)
  {
    //tonos de la alarma manual
    ProcessAlarm(TimeOn_AlarmaManual, TimeOff_AlarmaManual);
  }
  else if (alarm.IsCisternaAlarmON)
  {
    ProcessAlarm(TimeOn_Alarma, TimeOff_Alarma);
  }
  else if (alarm.IsNotAvailableBombasAlarmON)
  {
    ProcessAlarm(TimeOn_Alarma, TimeOff_Alarma);
  }
  else if (alarm.IsBomba1AlarmON || alarm.IsBomba2AlarmON)
  {
    ProcessAlarm(TimeOn_AlarmaBomba, TimeOff_AlarmaBomba);
  }
}

void ProcessAlarm(int activeTime, int inactiveTime)
{
  long t = millis();
  long delta;
  if (alarm.IsActive)
  {
    delta = t-alarm.ActiveTime;
    if (delta > activeTime)
    {
      alarm.IsActive = false;
      alarm.InactiveTime = t;
      DeactivateAlarm();
    }
  }
  else
  {
    delta = t-alarm.InactiveTime;
    if (delta > inactiveTime)
    {
      alarm.IsActive = true;
      alarm.ActiveTime = t;
      ActivateAlarm();
    }
  }
}

void ResetAlarm()
{
  alarm.IsBomba1AlarmON = false;
  alarm.IsBomba2AlarmON = false;
  alarm.IsManualAlarmON = false;
  alarm.IsCisternaAlarmON = false;
  alarm.IsNotAvailableBombasAlarmON = false;

  alarm.ActiveTime = 0;
  alarm.InactiveTime = 0;
  alarm.IsActive = false;
}

void ValidateActive()
{
  if (!alarm.IsManualAlarmON && !alarm.IsCisternaAlarmON && !alarm.IsBomba1AlarmON && !alarm.IsBomba2AlarmON && !alarm.IsNotAvailableBombasAlarmON)
  {
    alarm.ActiveTime = 0;
    alarm.IsActive = false;
  }
}

void StartAlarmNotAvailablesBombas()
{
  alarm.IsNotAvailableBombasAlarmON = true;
}

void StopAlarmNotAvailablesBombas()
{
  alarm.IsNotAvailableBombasAlarmON = false;
  ValidateActive();
}


void StartAlarmBombaContactorAbierto(Bomba* bomba)
{
  if (bomba->Number == BOMBA1)
    alarm.IsBomba1AlarmON = true;
  else
    alarm.IsBomba2AlarmON = true;
}


void StartAlarmBombaNotStop(Bomba* bomba)
{
  if (bomba->Number == BOMBA1)
    alarm.IsBomba1AlarmON = true;
  else
    alarm.IsBomba2AlarmON = true;
}

void StartAlarmBombaTermico(Bomba* bomba)
{
  if (bomba->Number == BOMBA1)
    alarm.IsBomba1AlarmON = true;
  else
    alarm.IsBomba2AlarmON = true;
}

void StopAlarmBomba(Bomba* bomba)
{
  if (bomba->Number == BOMBA1)
    alarm.IsBomba1AlarmON = false;
  else
    alarm.IsBomba2AlarmON = false;

  ValidateActive();
}



void StartCisternaEmptyAlarm()
{
  alarm.IsCisternaAlarmON = true;
}

void StopCisternaEmptyAlarm()
{
  alarm.IsCisternaAlarmON = false;
  ValidateActive();
}


void StopAllAlarms()
{
  ResetAlarm();
}

void StartManualAlarm()
{
  alarm.IsManualAlarmON = true;
}

void StopManualAlarm()
{
  alarm.IsManualAlarmON = true;
}
