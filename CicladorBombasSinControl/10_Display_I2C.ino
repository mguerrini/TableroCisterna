#ifdef DISPLAY_20x4_I2C

// --- DISPLAY ---
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x3F, 20, 4); // set the LCD address to 0x27 for a 16 chars and 2 line display


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

    return;
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



void ShowDatosFase()
{
  ShowDatosFase(F("R"), fase1.IsOk, 1);
  ShowDatosFase(F("S"), fase2.IsOk, 2);
  ShowDatosFase(F("T"), fase3.IsOk, 3);

  RefreshFaseView();
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
  int len;
  int uses;

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
      PrintBombaView(&bomba1);
      return;

    case 2:
      PrintBombaView(&bomba2);
      return;

    case 3:
      //Estadisticas Tanque
      PrintTanqueView(&bomba1);
      return;

    case 4:
      PrintTanqueView(&bomba2);
      return;

    case 5:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(F("** Falta de Fase ***"));

      lcd.setCursor(0, 2);
      lcd.print(F("Cantidad:"));
      len = GetLen(statistics.ErrorFaseCount);
      lcd.setCursor(20 - len, 2);
      lcd.print(statistics.ErrorFaseCount);

      lcd.setCursor(0, 3);
      lcd.print(F("Tiempo:"));
      PrintTimeFromSeconds(statistics.ErrorFaseTotalSeconds, 3, false);
      return;

    case 6:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(F("**** Generales *****"));
      lcd.setCursor(0, 2);
      lcd.print(F("Bomba seleccionada:"));
      lcd.setCursor(19, 2);
      if (bomba1.IsActive)
        lcd.print(BOMBA1);
      else
        lcd.print(BOMBA2);

      lcd.setCursor(0, 3);
      lcd.print(F("Ciclos seguidos:"));
      uses = bomba1.Uses;
      if (bomba2.IsActive)
        uses  = bomba2.Uses;
      len = GetLen(uses);
      lcd.setCursor(20 - len, 3);
      lcd.print(uses);

      return;
  }

  ShowActiveView();
}


void PrintBombaView(Bomba* bomba)
{
  int number = bomba->Number;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("***** Bomba "));
  lcd.print(number);
  lcd.print(F(" ******"));

  lcd.setCursor(0, 1);
  lcd.print(F("# Usos:"));
  int n = statistics.Bomba1Uses;
  if (number == BOMBA2)
    n = statistics.Bomba2Uses;
  int len = GetLen(n);

  lcd.setCursor(20 - len, 1);
  lcd.print(n);

  lcd.setCursor(0, 2);
  lcd.print(F("Tiempo ON:"));
  if (number == BOMBA1)
    PrintTimeFromSeconds(statistics.Bomba1TotalSeconds, 2, false);
  else
    PrintTimeFromSeconds(statistics.Bomba2TotalSeconds, 2, false);

  lcd.setCursor(0, 3);
  lcd.print(F("Errores termico:    "));

  n = statistics.Bomba1ErrorTermicoCount;
  if (bomba->Number == BOMBA2)
    n = statistics.Bomba2ErrorTermicoCount;
  len = GetLen(n);

  lcd.setCursor(20 - len, 3);
  lcd.print(n);
}


void PrintTanqueView(Bomba* bomba)
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("****** Tanque ******"));
  
  lcd.setCursor(0, 1);
  if (bomba->Number == BOMBA1)
    lcd.print(F("-- Llenado con B1 --"));
  else
    lcd.print(F("-- Llenado con B2 --"));
  
  lcd.setCursor(0, 2);
  lcd.print(F("Promedio:"));
  PrintTimeFromSeconds(bomba->FillTimeSecondsAverage, 2, true);

  lcd.setCursor(0, 3);
  lcd.print(F("Maximo:"));
  PrintTimeFromSeconds(bomba->FillTimeSecondsMaximum, 3, true);
  
/*
  lcd.setCursor(0, 3);
  lcd.print(F("Errores termico:    "));

  int n = statistics.Bomba1ErrorTermicoCount;
  if (bomba->Number == BOMBA2)
    n = statistics.Bomba2ErrorTermicoCount;
  int len = GetLen(n);

  lcd.setCursor(20 - len, 3);
  lcd.print(n);

  
    lcd.setCursor(0, 1);
    lcd.print(F("Contactor:          "));
    if (bomba->State == BOMBA_STATE_ERROR_CONTACTOR_ABIERTO)
    {
    lcd.setCursor(15, 1);
    lcd.print(F("ERROR"));
    }
    else
    {
    lcd.setCursor(18, 1);
    lcd.print(F("OK"));
    }

    lcd.setCursor(0, 2);
    lcd.print(F("Termico:            "));
    if (bomba->IsTermicoOk)
    {
    lcd.setCursor(18, 2);
    lcd.print(F("OK"));
    }
    else
    {
    lcd.setCursor(15, 2);
    lcd.print(F("ERROR"));
    }

    lcd.setCursor(0, 3);
    lcd.print(F("Errores termico:    "));

    int n = statistics.Bomba1ErrorTermicoCount;
    if (bomba->Number == BOMBA2)
    n = statistics.Bomba2ErrorTermicoCount;
    int len = GetLen(n);

    lcd.setCursor(20 - len, 3);
    lcd.print(n);
  */
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
  if (view.IsInfoViewActive || view.IsErrorFaseViewActive)
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
    else if (bomba->State == BOMBA_STATE_ERROR_CONTACTOR_ABIERTO || bomba->State == BOMBA_STATE_ERROR_CONTACTOR_CERRADO)
      lcd.print(F("Err. Contactor"));
    else if (bomba->State == BOMBA_STATE_ERROR_TERMICO)
      lcd.print(F("Err. Termico  "));
    else if (bomba->State == BOMBA_STATE_ERROR_FILL_TIMEOUT)
      lcd.print(F("Err. Bomba    "));
  }
}


void UpdateBombaWorkingTime(Bomba* bomba)
{
  if (view.IsInfoViewActive || view.IsErrorFaseViewActive)
    return;

  unsigned long totalSec = GetBombaWorkingTimeInSeconds(bomba);

  static int h = 0;
  static int m = 0;
  static int s = 0;

  int row = 0;
  if (bomba->Number == BOMBA2)
    row = 1;

  UpdateTime(totalSec, row, h, m, s, true);
}

/*
  void PrintTimeFromMinutes(unsigned long totalMin, int row)
  {
  int h = -1;
  int m = -1;
  int s = -1;

  lcd.setCursor(12, row);
  lcd.print(F("00:00:00"));

  unsigned long secs = totalMin * 60;
  UpdateTime(secs, row, h, m, s);
  }
*/

void PrintTimeFromSeconds(unsigned long totalSec, int row, boolean printSeconds)
{
  int h = -1;
  int m = -1;
  int s = -1;

  if (printSeconds)
  {
    lcd.setCursor(12, row);
    lcd.print(F("00:00:00"));
  }
  else
  {
    lcd.setCursor(15, row);
    lcd.print(F("00:00"));
  }

  UpdateTime(totalSec, row, h, m, s, printSeconds);
}


void UpdateTime(unsigned long totalSec, int row, int &h, int &m, int &s, boolean printSeconds)
{
  int lastH = h;
  int lastM = m;
  int lastS = s;

  convertSeconds2HMS(totalSec, h, m, s);

  int deltaCursor = 0;
  if (!printSeconds)
    deltaCursor = 3;

  if (h != lastH)
  {
    if (h < 100)
    {
      lcd.setCursor(12 + deltaCursor, row);
      if (h < 10)
        lcd.print(F("0"));
    }
    else if (h < 1000)
      lcd.setCursor(11 + deltaCursor, row);
    else if (h < 10000)
      lcd.setCursor(10 + deltaCursor, row);
    else if (h < 100000)
      lcd.setCursor(9 + deltaCursor, row);

    lcd.print(h);
  }

  if (m != lastM)
  {
    lcd.setCursor(15 + deltaCursor, row);
    if (m < 10)
      lcd.print(F("0"));

    lcd.print(m);
  }

  if (printSeconds && s != lastS)
  {
    lcd.setCursor(18, row);
    if (s < 10)
      lcd.print(F("0"));

    lcd.print(s);
  }
}


void UpdateCisternaDisplay()
{
  if (view.IsInfoViewActive || view.IsErrorFaseViewActive)
    return;

  lcd.setCursor(9, 3);
  if (sensores.IsCisternaSensorMinVal)
    lcd.print(F("Minimo"));
  else
    lcd.print(F("Normal"));
}


void UpdateTanqueDisplay()
{
  if (view.IsInfoViewActive || view.IsErrorFaseViewActive)
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
  if (view.IsInfoViewActive || view.IsErrorFaseViewActive)
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
  if (view.IsInfoViewActive || view.IsErrorFaseViewActive)
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
