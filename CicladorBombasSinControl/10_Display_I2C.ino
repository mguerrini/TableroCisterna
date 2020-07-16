#ifdef DISPLAY_20x4_I2C

// --- DISPLAY ---
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4); // set the LCD address to 0x27 for a 16 chars and 2 line display


// ****************************************************************** //
//                          SETUP
// ****************************************************************** //

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

  view.FaseViewLastRefreshTime = 0;
  view.InfoViewNumberActive = 0;
  view.InfoViewNumberActiveTime = 0;

  ShowMainView();
}


// ****************************************************************** //
//                          VISTAS
// ****************************************************************** //

// ============== REFRESH ============== //

void RefreshViews()
{
  if (view.IsMainViewActive)
    return;

  unsigned long delta = 0;

  if (view.IsInfoViewActive)
  {
    delta = deltaMillis(millis(), view.FaseViewLastRefreshTime);

    //la vista de las fases
    if (view.InfoViewNumberActive == 0)
    {
      if (delta > FASE_REFRESH_TIME)
      {
        RefreshFaseView();
        view.FaseViewLastRefreshTime = millis();
      }
    }

    delta = deltaMillis(millis(), view.InfoViewNumberActiveTime);
    //uso el mismo delta para refrescar la ventane de error
    if (delta > INFO_VIEW_VISIBLE_TIME)
      ShowActiveView();
  }

  if (view.IsErrorFaseViewActive)
  {
    delta = deltaMillis(millis(), view.FaseViewLastRefreshTime);
    if (delta > FASE_REFRESH_TIME)
    {
      RefreshFaseView();
      view.FaseViewLastRefreshTime = millis();
    }
  }
}

// ============== SHOW ACTIVE VIEW ============== //

void ShowActiveView()
{
  if (view.IsMainViewActive)
    ShowMainView();
  else
    ShowErrorFaseView();
}


// ****************************************************************** //
//                       FASE VIEW
// ****************************************************************** //

void ShowErrorFaseView()
{
  view.IsMainViewActive = false;
  view.IsErrorFaseViewActive = true;
  view.IsInfoViewActive = false;

  view.FaseViewLastRefreshTime = millis();

  //TODO agrgarle blink a la palabra ERROR de la fase que falla y al titulo
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("**  FALTA DE FASE **"));
  ShowDatosFase();
}

void RefreshFaseView()
{
  lcd.setCursor(3, 1);
  lcd.print(fase1.Voltage);
  lcd.print(F(" V"));

  lcd.setCursor(3, 2);
  lcd.print(fase2.Voltage);
  lcd.print(F(" V"));

  lcd.setCursor(3, 3);
  lcd.print(fase3.Voltage);
  lcd.print(F(" V"));
}


void ShowDatosFase()
{
  ShowDatosFase(F("R"), fase1.IsOk, 1);
  ShowDatosFase(F("S"), fase2.IsOk, 2);
  ShowDatosFase(F("T"), fase3.IsOk, 3);
/*
  lcd.setCursor(0, 1);
  lcd.print(F("R:                  "));
  if (!fase1.IsOk)
  {
    lcd.setCursor(15, 1);
    lcd.print(F("ERROR"));
  }

  lcd.setCursor(0, 2);
  lcd.print(F("S:                  "));
  if (!fase2.IsOk)
  {
    lcd.setCursor(15, 2);
    lcd.print(F("ERROR"));
  }

  lcd.setCursor(0, 3);
  lcd.print(F("T:                  "));
  if (!fase3.IsOk)
  {
    lcd.setCursor(15, 3);
    lcd.print(F("ERROR"));
  }
*/
  RefreshFaseView();
}

void ShowDatosFase(const __FlashStringHelper* fase, boolean isOk, int row)
{
  lcd.setCursor(0, row);
  lcd.print(fase);
  lcd.print(F(":                  "));
  if (!isOk)
  {
    lcd.setCursor(15, row);
    lcd.print(F("ERROR"));
  }
}


// ****************************************************************** //
//                       INFO VIEW
// ****************************************************************** //

void ShowInfoView()
{
  if (!view.IsInfoViewActive)
  {
    view.IsInfoViewActive = true;

    view.InfoViewNumberActive = 0;
    view.FaseViewLastRefreshTime = millis();
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
      ShowDatosFase();
      return;

    case 1:
      //Estadisticas Bomba 1
      PrintBombaView1(&bomba1);
      return;

    case 2:
      PrintBombaView2(&bomba1);
      return;

    case 3:
      //Estadisticas Bomba 2
      PrintBombaView1(&bomba2);
      return;

    case 4:
      PrintBombaView2(&bomba2);
      return;

    case 5:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(F("**** Generales *****"));
      lcd.setCursor(0, 1);
      lcd.print(F("Bomba seleccionada: "));
      lcd.setCursor(19, 1);
      if (bomba1.IsActive)
        lcd.print(BOMBA1);
      else
        lcd.print(BOMBA2);

      lcd.setCursor(0, 2);
      lcd.print(F("Ciclos seguidos:    "));
      lcd.setCursor(17, 2);

      if (bomba1.IsActive)
        lcd.print (bomba1.Uses);
      else
        lcd.print (bomba2.Uses);
      return;

    case 6:
      lcd.setCursor(0, 1);
      lcd.print(F("Errores Fase:       "));
      lcd.setCursor(14, 1);
      lcd.print(statistics.ErrorFaseCount);

      lcd.setCursor(0, 2);
      lcd.print(F("Min. sin fase:      "));
      lcd.setCursor(15, 2);
      lcd.print(statistics.ErrorFaseTotalMinutes);
      return;
  }

  ShowActiveView();
}


void PrintBombaView1(Bomba* bomba)
{
  int number = bomba->Number;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("***** Bomba "));
  lcd.print(number);
  lcd.print(F("  *****"));

  lcd.setCursor(0, 1);
  lcd.print(F("# Usos:             "));
  lcd.setCursor(8, 1);

  if (number == BOMBA1)
    lcd.print(statistics.Bomba1Uses);
  else
    lcd.print(statistics.Bomba2Uses);

  lcd.setCursor(0, 2);
  lcd.print(F("Tiempo ON:         m"));
  lcd.setCursor(11, 2);
  if (number == BOMBA1)
    lcd.print(statistics.Bomba1TotalMinutes);
  else
    lcd.print(statistics.Bomba2TotalMinutes);

  lcd.setCursor(0, 3);
  lcd.print(F("Llenado:             s"));
  lcd.setCursor(9, 3);
  lcd.print(bomba->FillTimeSecondsAverage);
}

void PrintBombaView2(Bomba* bomba)
{
  lcd.setCursor(0, 1);
  lcd.print(F("Contactor:          "));
  if (bomba->State == BOMBA_STATE_ERROR_CONTACTOR_ABIERTO)
  {
    lcd.setCursor(11, 1);
    lcd.print(F("ERROR"));
  }
  else
  {
    lcd.setCursor(11, 1);
    lcd.print(F("OK"));
  }

  lcd.setCursor(0, 2);
  lcd.print(F("Termico:            "));
  if (bomba->IsTermicoOk)
  {
    lcd.setCursor(9, 2);
    lcd.print(F("OK"));
  }
  else
  {
    lcd.setCursor(9, 2);
    lcd.print(F("ERROR"));
  }

  lcd.setCursor(0, 3);
  lcd.print(F("Errores termico:    "));
  lcd.setCursor(17, 3);
  if (bomba->Number == BOMBA1)
    lcd.print(statistics.Bomba1ErrorTermicoCount);
  else
    lcd.print(statistics.Bomba2ErrorTermicoCount);
}


// ****************************************************************** //
//                          MAIN VIEW
// ****************************************************************** //

void ShowMainView()
{
  view.IsMainViewActive = true;
  view.IsErrorFaseViewActive = false;
  view.IsInfoViewActive = false;

  lcd.clear();
  PrintInitialText();
  UpdateBombaDisplay(&bomba1);
  UpdateBombaDisplay(&bomba2);

  UpdateCisternaDisplay();
  UpdateTanqueDisplay();

  UpdateActiveBombaDisplay();

  UpdateDisplayMode();
}

void UpdateBombaDisplay(Bomba* bomba)
{
  if (!view.IsMainViewActive)
    return;

  int row = 0;

  if (bomba->Number == BOMBA2)
    row = 1;

  lcd.setCursor(6, row);

  if (!bomba->IsEnabled)
    lcd.print  (F("Fuera de linea"));
  else
  {
    if (bomba->State == BOMBA_STATE_ON)
    {
      lcd.print(F("ON    00:00:00"));
      UpdateBombaWorkingTime(bomba);
    }
    else if (bomba->State == BOMBA_STATE_OFF)
      lcd.print(F("OFF           "));
    else if (bomba->State == BOMBA_STATE_ERROR_CONTACTOR_ABIERTO)
      lcd.print(F("Err.Contactor "));
    else if (bomba->State == BOMBA_STATE_ERROR_CONTACTOR_CERRADO)
      lcd.print(F("Err.Contactor "));
    else if (bomba->State == BOMBA_STATE_ERROR_TERMICO)
      lcd.print(F("Err.Termico   "));
  }
}

/*
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
    lcd.print  (F("Fuera de linea"));
  else
  {
    if (state == BOMBA_STATE_ON)
    {
      lcd.print(F("ON            "));
      ShowWorkingTimeBomba(bomba);
    }
    else if (state == BOMBA_STATE_OFF)
      lcd.print(F("OFF           "));
    else if (state == BOMBA_STATE_ERROR_CONTACTOR_ABIERTO)
      lcd.print(F("Err.Contactor "));
    else if (state == BOMBA_STATE_ERROR_CONTACTOR_CERRADO)
      lcd.print(F("Err.Contactor "));
    else if (state == BOMBA_STATE_ERROR_TERMICO)
      lcd.print(F("Err.Termico   "));
  }
  }

  void ShowBombaWorkingTime(Bomba* bomba)
  {
  if (bomba->Number == BOMBA1)
    lcd.setCursor(12, 0);
  else
    lcd.setCursor(12, 1);

  lcd.print(F("00:00:00"));

  UpdateBombaWorkingTime(bomba);
  }
*/


void UpdateBombaWorkingTime(Bomba* bomba)
{
  unsigned long totalSec = GetBombaWorkingTimeInSeconds(bomba);

  static int h = 0;
  static int m = 0;
  static int s = 0;

  int lastH = h;
  int lastM = m;
  int lastS = s;

  convertSeconds2HMS(totalSec, h, m, s);

  int row = 0;
  if (bomba->Number == BOMBA2)
    row = 1;

  if (h != lastH)
  {
    lcd.setCursor(12, row);
    if (h < 10)
      lcd.print(F("0"));

    lcd.print(h);
  }

  if (m != lastM)
  {
    lcd.setCursor(15, row);
    if (m < 10)
      lcd.print(F("0"));

    lcd.print(m);
  }

  if (s != lastS)
  {
    lcd.setCursor(18, row);
    if (s < 10)
      lcd.print(F("0"));

    lcd.print(s);
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

void UpdateDisplayMode()
{
#ifdef MODO_OUTPUT_VIEW_ENABLED
  if (!view.IsMainViewActive)
    return;

  lcd.setCursor(19, 3);

  if (IsAutomaticMode())
    lcd.print(F("A"));
  else
    lcd.print(F("M"));
#endif
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
