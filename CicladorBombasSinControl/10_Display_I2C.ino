#ifdef DISPLAY_20x4_I2C

// --- DISPLAY ---
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4); // set the LCD address to 0x27 for a 16 chars and 2 line display

typedef struct {
  boolean IsMainViewActive = true;
  boolean IsErrorFaseViewActive = false;
  boolean IsInfoViewActive = false;

  byte InfoViewNumberActive = 0;
  unsigned long InfoViewNumberActiveTime = 0;
} View;

View view = {};

void SetupDisplay()
{
  lcd.init();                      // initialize the lcd
  lcd.init();
  // Print a message to the LCD.
  lcd.clear();
  lcd.backlight();

  //tres tipos de ventanas
  view.IsMainViewActive = true;
  view.IsErrorFaseViewActive = false;
  view.IsInfoViewActive = false;

  view.InfoViewNumberActive = 0;
  view.InfoViewNumberActiveTime = 0;

  PrintInitialText();
}

//---------------------- VISTAS ----------------------

void UpdateView()
{
  if (!view.IsInfoViewActive)
    return;

  unsigned long curr = millis();
  unsigned long delta = curr - view.InfoViewNumberActiveTime;

  if (delta > INFO_VIEW_VISIBLE_TIME)
  {
    ShowMainView();
  }
}

void ShowErrorFaseView()
{
  view.IsMainViewActive = false;
  view.IsErrorFaseViewActive = true;
  view.IsInfoViewActive = false;
  /*
    lcd.clear();
    lcd.setCursor(5, 1);
    lcd.print(F("ERROR"));
    lcd.setCursor(3, 2);
    lcd.print(F("FALLA DE FASE"));
  */
  //TODO agrgarle blink a la palabra ERROR de la fase que falla y al titulo
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("*** FALLA DE FASE **"));
  lcd.print(F("Fase 1:             "));
  lcd.setCursor(8, 1);
  lcd.print(fase.Fase1Voltage);
  lcd.print(F(" V"));
  if (!fase.IsFase1Ok)
  {
    lcd.setCursor(16, 1);
    lcd.print(F("ERROR"));
  }


  lcd.setCursor(0, 2);
  lcd.print(F("Fase 2:             "));
  lcd.setCursor(8, 2);
  lcd.print(fase.Fase2Voltage);
  lcd.print(F(" V"));
  if (!fase.IsFase2Ok)
  {
    lcd.setCursor(16, 2);
    lcd.print(F("ERROR"));
  }

  lcd.setCursor(0, 3);
  lcd.print(F("Fase 3:             "));
  lcd.setCursor(8, 3);
  lcd.print(fase.Fase2Voltage);
  lcd.print(F(" V"));
  if (!fase.IsFase2Ok)
  {
    lcd.setCursor(16, 3);
    lcd.print(F("ERROR"));
  }
}

void ShowMainView()
{
  view.IsMainViewActive = true;
  view.IsErrorFaseViewActive = false;
  view.IsInfoViewActive = false;

  lcd.clear();
  PrintInitialText();
  UpdateBomba1Display();
  UpdateBomba2Display();

  UpdateCisternaDisplay();
  UpdateTanqueDisplay();
}

void ShowInfoView()
{
  if (!view.IsInfoViewActive)
  {
    view.IsMainViewActive = false;
    view.IsErrorFaseViewActive = false;
    view.IsInfoViewActive = true;

    view.InfoViewNumberActive = 0;
  }
  else
  {
    view.InfoViewNumberActive = view.InfoViewNumberActive + 1;
  }

  //muestro la vista
  ShowNextInfoView();

  //indico la hora de visualizacion
  view.InfoViewNumberActiveTime =  millis();
}

void ShowNextInfoView()
{
  switch (view.InfoViewNumberActive)
  {
    case 0:
      //Fases
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(F(" Tension de Entrada "));
      lcd.print(F("Fase 1:             "));
      lcd.setCursor(8, 1);
      lcd.print(fase.Fase1Voltage);
      lcd.print(F(" V"));

      lcd.setCursor(0, 2);
      lcd.print(F("Fase 2:             "));
      lcd.setCursor(8, 2);
      lcd.print(fase.Fase2Voltage);
      lcd.print(F(" V"));

      lcd.setCursor(0, 3);
      lcd.print(F("Fase 3:             "));
      lcd.setCursor(8, 3);
      lcd.print(fase.Fase3Voltage);
      lcd.print(F(" V"));
      break;

    case 1:
      //Estadisticas Bomba 1
      printBombaView1(&bomba1);
      break;

    case 2:
      printBombaView2(&bomba1);
      break;

    case 3:
      //Estadisticas Bomba 1
      printBombaView1(&bomba2);
      break;

    case 4:
      printBombaView2(&bomba2);
      break;
      
    case 5:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(F("**** Generales *****"));
      lcd.setCursor(0, 1);
      lcd.print(F("Bomba seleccionada:"));
      lcd.setCursor(0, 1);
      lcd.print(GetActiveBombaNumber());

      lcd.setCursor(0, 2);
      lcd.print(F("Ciclos seguidos:"));
      lcd.setCursor(16, 2);
      lcd.print(GetActiveBombaUses());
      return;

    case 6:
      lcd.setCursor(0, 1);
      lcd.print(F("Errores Fase:       "));
      lcd.setCursor(13, 1);
      lcd.print(statistics.ErrorFaseCount);

      lcd.setCursor(0, 2);
      lcd.print(F("Min. sin fase:      "));
      lcd.setCursor(14, 2);
      lcd.print(statistics.ErrorFaseTotalMinutes);
      return;
  }

  ShowMainView();
}

void printBombaView1(Bomba* bomba)
{
  int number = bomba->Number;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("***** Bomba "));
  lcd.print(number);
  lcd.print(F(" ******"));
  lcd.print(F("Usos:               "));
  lcd.setCursor(6, 1);

  if (number == BOMBA1)
    lcd.print(statistics.Bomba1Uses);
  else
    lcd.print(statistics.Bomba2Uses);

  lcd.setCursor(0, 2);
  lcd.print(F("Minutos:            "));
  lcd.setCursor(9, 2);
  if (number == BOMBA1)
    lcd.print(statistics.Bomba1TotalMinutes);
  else
    lcd.print(statistics.Bomba2TotalMinutes);

  lcd.setCursor(0, 2);
  lcd.print(F("T. llenado min.:     "));
  lcd.setCursor(11, 2);
  lcd.print(bomba->FillTimeAverage);
}

void PrintBombaView2(Bomba* bomba)
{
  lcd.setCursor(0, 1);
  lcd.print(F("Contactor: "));
  if (bomba->IsContactorOk)
  {
    lcd.setCursor(11, 1);
    lcd.print(F("OK       "));
  }
  else
  {
    lcd.setCursor(11, 1);
    lcd.print(F("ERROR    "));
  }

  lcd.setCursor(0, 2);
  lcd.print(F("Termico:             "));
  if (bomba->IsTermicoOk)
  {
    lcd.setCursor(9, 2);
    lcd.print(F("OK         "));
  }
  else
  {
    lcd.setCursor(9, 2);
    lcd.print(F("ERROR      "));
  }

  lcd.setCursor(0, 3);
  lcd.print(F("Errores térmico:    "));
  lcd.setCursor(16, 3);
  if (bomba->Number == BOMBA1)
    lcd.print(statistics.Bomba1TotalMinutes);
  else
    lcd.print(statistics.Bomba2TotalMinutes);  
}


// -------- ACTUALIZACIONES ---------

void UpdateBombaDisplay(Bomba* bomba)
{
  if (bomba->Number == BOMBA1)
    UpdateBomba1Display();
  else
    UpdateBomba2Display();
}

void UpdateBomba1Display()
{
  UpdateBombaDisplay(bomba1.IsEnabled, bomba1.State, 0);
}

void UpdateBomba2Display()
{
  UpdateBombaDisplay(bomba2.IsEnabled, bomba2.State, 1);
}

void UpdateBombaDisplay(bool enabled, int state, int row)
{
  if (!view.IsMainViewActive)
    return;

  lcd.setCursor(6, row);

  if (!enabled)
    lcd.print  (F("Fuera de línea"));
  else
  {
    if (state == BOMBA_STATE_ON)
      lcd.print(F("Encendida     "));
    else if (state == BOMBA_STATE_OFF)
      lcd.print(F("Apagada       "));
    else if (state == BOMBA_STATE_ERROR_CONTACTOR_ABIERTO)
      lcd.print(F("Err.Contactor "));
    else if (state == BOMBA_STATE_ERROR_CONTACTOR_CERRADO)
      lcd.print(F("Err.Contactor "));
    else if (state == BOMBA_STATE_ERROR_TERMICO)
      lcd.print(F("Err.Termico   "));
  }
}

void UpdateCisternaDisplay()
{
  if (!view.IsMainViewActive)
    return;

  lcd.setCursor(9, 3);
  if (sensores.IsCisternaSensorMinVal)
    lcd.print(F("Minimo"));
  else
    lcd.print(F("Normal"));
}

void UpdateTanqueDisplay()
{
  if (!view.IsMainViewActive)
    return;

  lcd.setCursor(7, 2);
  if (sensores.IsTanqueSensorMinVal)
  {
    if (IsBombaOn(&bomba1) || IsBombaOn(&bomba2))
      lcd.print(F("Cargando   "));
    else
      lcd.print(F("Minimo     "));
  }
  else
    lcd.print(F("Descargando"));
}

void UpdateDisplayToManualMode()
{
  UpdateDisplayMode("M");
}

void UpdateDisplayToAutoMode()
{
  UpdateDisplayMode("A");
}

void UpdateDisplayMode(const char* mode)
{
  if (!view.IsMainViewActive)
    return;

  lcd.setCursor(19, 0);
  lcd.print(mode);
}


void UpdateActiveBombaDisplay()
{
  if (!view.IsMainViewActive)
    return;

  if (bomba1.IsActive)
  {
    lcd.setCursor(3, 0);
    lcd.print(F("*"));
    lcd.setCursor(3, 1);
    lcd.print(F(" "));
  }
  else if (bomba2.IsActive)
  {
    lcd.setCursor(3, 0);
    lcd.print(F(" "));
    lcd.setCursor(3, 1);
    lcd.print(F("*"));
  }
  else
  {
    lcd.setCursor(3, 0);
    lcd.print(F(" "));
    lcd.setCursor(3, 1);
    lcd.print(F(" "));
  }
}


void PrintInitialText()
{
  if (!view.IsMainViewActive)
    return;

  lcd.setCursor(19, 0);
  if (IsAutomaticMode() == AUTO)
    lcd.print(F("A"));
  else
    lcd.print(F("M"));

  lcd.setCursor(0, 0);
  lcd.print(F("B1( ): "));

  lcd.setCursor(0, 1);
  lcd.print(F("B2( ): "));

  lcd.setCursor(0, 2);
  lcd.print(F("Tanque: "));

  lcd.setCursor(0, 3);
  lcd.print(F("Cisterna: "));
}

#endif
