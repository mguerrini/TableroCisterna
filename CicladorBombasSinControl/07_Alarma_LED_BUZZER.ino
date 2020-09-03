void Alarm_Pip()
{
  ActivateAlarm();
  delay(100);
  DeactivateAlarm();
}

#ifdef ALARM_LED

void SetupAlarm()
{
  ResetAlarm();
  DeactivateAlarm();
}

void ActivateAlarm()
{
#ifdef ALARM_ENABLED
  digitalWrite(ALARM_PIN, HIGH);
#endif
}

void DeactivateAlarm()
{
#ifdef ALARM_ENABLED
  digitalWrite(ALARM_PIN, LOW);
#endif
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
#ifdef ALARM_ENABLED
  tone(ALARM_PIN, 440);
#endif
}

void DeactivateAlarm()
{
#ifdef ALARM_ENABLED
  noTone(ALARM_PIN);
  digitalWrite(ALARM_PIN, HIGH);
#endif

}

#endif
