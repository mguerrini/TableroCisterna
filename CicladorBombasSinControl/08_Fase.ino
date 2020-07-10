#include <EEPROM.h>

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

  int voltRef = 0;
  int cal = 0;
  EEPROM.get(FASE1_TENSION_ENTRADA_ADDR, voltRef);
  EEPROM.put(FASE1_FACTOR_CONVERSION_ADDR, cal);

  fase1.InputVoltsReference = voltRef;
  fase1.ConversionFactor = cal;

  EEPROM.put(FASE2_TENSION_ENTRADA_ADDR, voltRef);
  EEPROM.put(FASE2_FACTOR_CONVERSION_ADDR, cal);

  fase2.InputVoltsReference = voltRef;
  fase2.ConversionFactor = cal;

  EEPROM.put(FASE3_TENSION_ENTRADA_ADDR, voltRef);
  EEPROM.put(FASE3_FACTOR_CONVERSION_ADDR, cal);

  fase3.InputVoltsReference = voltRef;
  fase3.ConversionFactor = cal;

  Serial.print(F("Fase 1 - Tension de referencia: "));
  Serial.print(fase1.InputVoltsReference);
  Serial.print(F(", Valor de calibración: "));
  Serial.println(fase1.ConversionFactor);

  Serial.print(F("Fase 2 - Tension de referencia: "));
  Serial.print(fase2.InputVoltsReference);
  Serial.print(F(", Valor de calibración: "));
  Serial.println(fase2.ConversionFactor);

  Serial.print(F("Fase 3 - Tension de referencia: "));
  Serial.print(fase3.InputVoltsReference);
  Serial.print(F(", Valor de calibración: "));
  Serial.println(fase3.ConversionFactor);

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

void calibrateFase(int pinNumber, int faseNumber, String voltsStr)
{
  voltsStr.trim();
  long inputVolts = voltsStr.toInt();

  if (inputVolts <= 0)
  {
    Serial.println(F("La tension de referencia es 0 o no está definida. No es posible calibrar la fase."));
    return;
  }

  int calibration = doCalibrateFase(pinNumber, faseNumber, inputVolts);

  if (calibration > 0)
  {
    if (faseNumber == 1)
    {
      EEPROM.put(FASE1_TENSION_ENTRADA_ADDR, inputVolts);
      EEPROM.put(FASE1_FACTOR_CONVERSION_ADDR, calibration);

      //actualizo los valores actuales
      fase1.InputVoltsReference = inputVolts;
      fase1.ConversionFactor = calibration;

    }
    else if (faseNumber == 2)
    {
      EEPROM.put(FASE2_TENSION_ENTRADA_ADDR, inputVolts);
      EEPROM.put(FASE2_FACTOR_CONVERSION_ADDR, calibration);

      //actualizo los valores actuales
      fase2.InputVoltsReference = inputVolts;
      fase2.ConversionFactor = calibration;
    }
    else
    {
      EEPROM.put(FASE3_TENSION_ENTRADA_ADDR, inputVolts);
      EEPROM.put(FASE3_FACTOR_CONVERSION_ADDR, calibration);

      //actualizo los valores actuales
      fase3.InputVoltsReference = inputVolts;
      fase3.ConversionFactor = calibration;
    }

    Serial.print(F("Calibracion fase "));
    Serial.print(faseNumber);
    Serial.println(F(" guardada."));

    Serial.print(F("Tension de entrada: "));
    Serial.println(inputVolts);

    Serial.print(F("Valor de calibración: "));
    Serial.println(calibration);
  }
}

int doCalibrateFase(int pinNumber, int faseNumber, int inputVolts)
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

    Serial.print(F("Valor leido: "));
    Serial.println(fase1Val);

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
