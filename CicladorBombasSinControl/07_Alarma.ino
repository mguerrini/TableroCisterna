
// --- ALARM ---
void SetupAlarm()
{
  noTone(ALARM_PIN); 
}



void StartAlarmNotAvailablesBombas()
{
  tone(ALARM_PIN, 440);
}

void StopAlarmNotAvailablesBombas()
{
  noTone(ALARM_PIN); 
}


void StartAlarmBombaContactorAbierto()
{
  tone(ALARM_PIN, 440);
}


void StartAlarmBombaNotStop()
{
  tone(ALARM_PIN, 440);
}

void StartAlarmBombaTermico()
{
  tone(ALARM_PIN, 440);
}

void StopBombaAlarm()
{
  noTone(ALARM_PIN);
}



void StartCisternaEmptyAlarm()
{
  tone(ALARM_PIN, 440);
}

void StopCisternaEmptyAlarm()
{
  noTone(ALARM_PIN);
}



void StopAllAlarms()
{
  noTone(ALARM_PIN);
}
