
#ifdef ALARM_LED

void SetupAlarm()
{
  ResetAlarm();
  DeactivateAlarm();
}

void ActivateAlarm()
{
  digitalWrite(ALARM_PIN, HIGH);
}

void DeactivateAlarm()
{
  digitalWrite(ALARM_PIN, LOW);
}

#endif


#ifdef ALARM_BUZZER

void SetupAlarm()
{
  ResetAlarm();
  DeactivateAlarm();
}

void ActivateAlarm()
{
  tone(ALARM_PIN, 440);
}

void DeactivateAlarm()
{
  noTone(ALARM_PIN);
  digitalWrite(ALARM_PIN, HIGH);
}

#endif
