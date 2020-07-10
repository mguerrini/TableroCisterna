// --- MODO ---
void SetupFase()
{
  fase1.Voltage = 220;
  fase2.Voltage = 220;
  fase3.Voltage = 220;

  fase1.IsOk = true;
  fase2.IsOk = true;
  fase3.IsOk = true;

  #ifdef FASE_FROM_EEPROM_ENABLED

  #else
  fase1.InputVoltsReference = TENSION_ENTRADA;
  fase2.InputVoltsReference = TENSION_ENTRADA;
  fase3.InputVoltsReference = TENSION_ENTRADA;

  fase1.ConversionFactor = FASE1_220_VALUE;
  fase2.ConversionFactor = FASE2_220_VALUE;
  fase3.ConversionFactor = FASE3_220_VALUE;
  #endif

  fase1.ReadCount = 0;
  fase2.ReadCount = 0;
  fase3.ReadCount = 0;

  fase1.ReadTotal = 0;
  fase2.ReadTotal = 0;
  fase3.ReadTotal = 0;

  fase1.LastRead = 0;
  fase2.LastRead = 0;
  fase3.LastRead = 0;
}



void ReadFases()
{
  unsigned long t = millis();
  boolean b1 = ReadFase1(t);
  boolean b2 = ReadFase2(t);
  boolean b3 = ReadFase3(t);

  boolean isOkCurrVal = fase1.IsOk && fase2.IsOk && fase3.IsOk;

  if (isOkCurrVal != automaticFSM.IsFaseOk)
  {
    if (automaticFSM.IsFaseOk)
    {
      //muestro la pantalla de error
      ShowErrorFaseView();
      OnFaseError();
      Statistics_FaseError_Begin();
    }
    else
    {
      //muestro la pantalla mormal
      ShowMainView();
      OnFaseOk();
      Statistics_FaseError_End();
    }

    automaticFSM.IsFaseOk = isOkCurrVal;
  }
}


void OnFaseError()
{
  //desactivar el rele para que se corte la alimentacion
  digitalWrite(FASE_OUTPUT_PIN, FASE_OUTPUT_CLOSE_RELE);
}

void OnFaseOk()
{
  //normalizar el rele para que vuelva a funcionar la bomba
  digitalWrite(FASE_OUTPUT_PIN, FASE_OUTPUT_OPEN_RELE);
}


boolean ReadFase1(unsigned long t)
{
  unsigned long delta = deltaMillis(t, fase1.LastRead);
  if (delta < FASE_WAIT_BETWEEN_READS)
    return false;

#ifdef FASE1_ENABLED
  int fase1Val = analogRead(FASE1_INPUT_PIN);
  int volts = mapLocal(fase1Val, 0, fase1.ConversionFactor, 0, fase1.InputVoltsReference);
  //agrego el nuevo valor
  updateFaseValues(&fase1, t, volts);

#else
  fase1.Voltage = fase1.InputVoltsReference;
  fase1.IsOk = true;
#endif

  return true;
}

boolean ReadFase2(unsigned long t)
{
  unsigned long delta = deltaMillis(t, fase2.LastRead);
  if (delta < FASE_WAIT_BETWEEN_READS)
    return false;

#ifdef FASE2_ENABLED
  int fase2Val = analogRead(FASE2_INPUT_PIN);
  int volts = mapLocal(fase2Val, 0, fase2.ConversionFactor, 0, fase2.InputVoltsReference);
  //agrego el nuevo valor
  updateFaseValues(&fase2, t, volts);
#else
  fase2.Voltage = fase2.InputVoltsReference;
  fase2.IsOk = true;
#endif

  return true;
}

boolean ReadFase3(unsigned long t)
{
  unsigned long delta = deltaMillis(t, fase3.LastRead);
  if (delta < FASE_WAIT_BETWEEN_READS)
    return false;

#ifdef FASE3_ENABLED
  int fase3Val = analogRead(FASE3_INPUT_PIN);
  int volts = mapLocal(fase3Val, 0, fase3.ConversionFactor, 0, fase3.InputVoltsReference);
  //Serial.print("Fase 3: ");
  //Serial.println(volts);

  //agrego el nuevo valor
  updateFaseValues(&fase3, t, volts);
#else
  fase3.Voltage = fase3.InputVoltsReference;
  fase3.IsOk = true;
#endif

  return true;
}


void updateFaseValues(Fase * fase, unsigned long readTime, int volts)
{
  fase->ReadCount = fase->ReadCount + 1;
  fase->ReadTotal = fase->ReadTotal + volts;
  fase->LastRead = readTime;
/*
  Serial.print(F("Count: "));
  Serial.print(fase->ReadCount);
  Serial.print(F(" Sum Total: "));
  Serial.println(fase->ReadTotal);
*/
  if (fase->ReadCount >= FASE_READ_COUNT_MAX)
  {
    //calculo el procentaje
    int value = fase->ReadTotal / fase->ReadCount;

    fase->Voltage = value;
    fase->IsOk = fase->Voltage > FASE_MIN_VOLTAGE;
/*
    Serial.print(F("Voltage: "));
    Serial.print(fase->Voltage);
    Serial.print(F(" IsOk: "));
    Serial.println(fase->IsOk);
*/
    //reseteo
    fase->ReadCount = 0;
    fase->ReadTotal = 0;
  }
}



// *************************************************** //
//                  FASE CALIBRATION
// *************************************************** //

int calibrateFase(int pinNumber, int faseNumber, String voltsStr)
{
  voltsStr.trim();
  long inputVolts = voltsStr.toInt();

  if (inputVolts > 0)
  {
    Serial.print(F("Inicio calibracion Fase "));
    Serial.println(faseNumber);
    Serial.print(F("Tension de referencia: "));
    Serial.print(inputVolts);

    //tengo que leer el valor de la fase y sacar un promedio
    float total = 0;

    for (int i = 0; i < 10; i++)
    {
      int fase1Val = analogRead(pinNumber);

      total = total + fase1Val;

      delay(FASE_WAIT_BETWEEN_READS);
    }

    float conversionFactor = total / 10;
    int cFactor = (int) conversionFactor;    

    Serial.print(F("Fin calibracion Fase: "));
    Serial.println(faseNumber);
    Serial.print(F("Constante obtenida [0 - 1023]: "));
    Serial.println(cFactor);

    return cFactor;
  }
  else
    return -1;
}
