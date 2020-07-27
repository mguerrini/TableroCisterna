//************ VARIABLES ******************

#define TimeOn_AlarmaManual 100
#define TimeOff_AlarmaManual 3000

#define TimeOn_AlarmaHigh 200
#define TimeOff_AlarmaHigh 200

#define TimeOn_AlarmaBomba 100
#define TimeOff_AlarmaBomba 300

//********************************************


void ReadAlarm()
{
  if (!alarm.IsManualAlarmON && !alarm.IsCisternaAlarmON && !alarm.IsBomba1AlarmON && !alarm.IsBomba2AlarmON && !alarm.IsNotAvailableBombasAlarmON)
  {
    if (alarm.IsActive)
    {
      alarm.ActiveTime = 0;
      alarm.IsActive = false;
      DeactivateAlarm();
    }
    return;
  }

  if (alarm.ActiveTime == 0)
  {
    //primera vez que se prende.
    alarm.ActiveTime = millis();
    alarm.IsActive = true;
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
    ProcessAlarm(TimeOn_AlarmaHigh, TimeOff_AlarmaHigh);
  }
  else if (alarm.IsNotAvailableBombasAlarmON)
  {
    ProcessAlarm(TimeOn_AlarmaHigh, TimeOff_AlarmaHigh);
  }
  else if (alarm.IsBomba1AlarmON || alarm.IsBomba2AlarmON)
  {
    ProcessAlarm(TimeOn_AlarmaBomba, TimeOff_AlarmaBomba);
  }
}



void ProcessAlarm(int activeTime, int inactiveTime)
{
  unsigned long t = millis();
  long delta;
  if (alarm.IsActive)
  {
    delta = deltaMillis(t, alarm.ActiveTime);
    if (delta > activeTime)
    {
      alarm.IsActive = false;
      alarm.InactiveTime = t;
      DeactivateAlarm();
    }
  }
  else
  {
    delta = t - alarm.InactiveTime;
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
  StopAllAlarms();
  alarm.ActiveTime = 0;
  alarm.InactiveTime = 0;
  alarm.IsActive = false;

  DeactivateAlarm();
}


void StopAllAlarms()
{
  alarm.IsBomba1AlarmON = false;
  alarm.IsBomba2AlarmON = false;
  alarm.IsManualAlarmON = false;
  alarm.IsCisternaAlarmON = false;
  alarm.IsNotAvailableBombasAlarmON = false;
}

void StartAlarmNotAvailablesBombas()
{
  alarm.IsNotAvailableBombasAlarmON = true;
}

void StopAlarmNotAvailablesBombas()
{
  alarm.IsNotAvailableBombasAlarmON = false;
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
}



void StartCisternaEmptyAlarm()
{
  alarm.IsCisternaAlarmON = true;
}

void StopCisternaEmptyAlarm()
{
  alarm.IsCisternaAlarmON = false;
}

void StartManualAlarm()
{
  alarm.IsManualAlarmON = true;
}

void StopManualAlarm()
{
  alarm.IsManualAlarmON = false;
}
