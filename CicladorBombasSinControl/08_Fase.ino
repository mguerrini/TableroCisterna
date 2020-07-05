// --- MODO ---
void SetupFase()
{
  fase1.Voltage = 220;
  fase2.Voltage = 220;
  fase3.Voltage = 220;

  fase1.IsOk = true;
  fase2.IsOk = true;
  fase3.IsOk = true;

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
  ReadFase1(t);
  ReadFase2(t);
  ReadFase3(t);

  boolean isOkCurrVal = fase1.IsOk && fase2.IsOk && fase3.IsOk;

  if (isOkCurrVal != automaticFSM.IsFaseOk)
  {
    if (automaticFSM.IsFaseOk)
    {
      //muestro la pantalla mormal
      ShowMainView();
      Statistics_FaseError_End();
    }
    else
    {
      //muestro la pantalla de error
      ShowErrorFaseView();
      Statistics_FaseError_Begin();
    }

    automaticFSM.IsFaseOk = currVal;
  }
}


void OnFaseError()
{
  //desactivar el rele para que se corte la alimentacion   
}

void OnFaseOk()
{
  //normalizar el rele para que vuelva a funcionar la bomba 
}


void ReadFase1(unsigned long t)
{
  unsigned long = delta = deltaMillis(t, fase->LastRead);
  if (delta < FASE_WAIT_BETWEEN_READS)
    return;

  int fase1Val = analogRead(FASE1_INPUT_PIN);
  int volts = map(fase1Val, 0, FASE1_220_VALUE, 0, 220);

  //agrego el nuevo valor
  updateFaseValues(&fase1, t, volts);
}

void ReadFase2(unsigned long t)
{
  unsigned long = delta = deltaMillis(t, fase->LastRead);
  if (delta < FASE_WAIT_BETWEEN_READS)
    return;

  int fase2Val = analogRead(FASE2_INPUT_PIN);
  int volts = map(fase2Val, 0, FASE2_220_VALUE, 0, 220);

  //agrego el nuevo valor
  updateFaseValues(&fase2, bt, volts);
}

void ReadFase3(unsigned long t)
{
  unsigned long = delta = deltaMillis(t, fase->LastRead);
  if (delta < FASE_WAIT_BETWEEN_READS)
    return;

  int fase3Val = analogRead(FASE3_INPUT_PIN);
  int volts = map(fase3Val, 0, FASE3_220_VALUE, 0, 220);

  //agrego el nuevo valor
  updateFaseValues(&fase3, t, volts);
}


void updateFaseValues(Fase * fase, unsigned long readTime, int volts)
{
  fase->ReadCount = fase->ReadCount + 1;
  fase->ReadTotal = fase->ReadTotal + volts;
  fase->LastRead = readTime;

  if (fase->ReadCount >= FASE_READ_COUNT_MAX)
  {
    //calculo el procentaje
    int value = fase->ReadTotal / fase->ReadCount;
    fase->Voltage = value;

    fase->IsOk = fase->Voltage > FASE_MIN_VOLTAGE;

    //reseteo
    fase->ReadCount = 0;
    fase->ReadTotal = 0;
  }
}


unsigned long deltaMillis(unsigned long currRead, unsigned long prevRead)
{
  if (currRead >= prevRead)
    return currRead - prevRead;
  else
    return currRead; //volvio a cero...uso este valor como referencia....pierdo prevRead hasta el maximo..pero pasa 1 vez cada 47 dias
}


boolean IsFaseOk()
{
  static unsigned long startTime = 0;
  static boolean state;
  static boolean isPressed;

  IsButtonPressed(FASE_ERROR_PIN, state, isPressed, startTime);

  return isPressed;
}
