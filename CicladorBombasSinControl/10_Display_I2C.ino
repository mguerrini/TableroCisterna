#ifdef DISPLAY_20x4_I2C

// --- DISPLAY ---
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4); // set the LCD address to 0x27 for a 16 chars and 2 line display

boolean IsMainViewActive = true;
boolean IsErrorFaseViewActive = false;
boolean IsInfoViewActive = false;
byte ActiveInfoViewNumber = 0;

void SetupDisplay()
{
  lcd.init();                      // initialize the lcd
  lcd.init();
  // Print a message to the LCD.
  lcd.clear();
  lcd.backlight();

  IsMainViewActive = true;
  IsErrorFaseViewActive = false;
  IsInfoViewActive = false;
  ActiveInfoViewNumber = 0;

  PrintInitialText();
}

//---------------------- VISTAS ----------------------

void ShowErrorFaseView()
{
  lcd.clear();
  lcd.setCursor(5, 1);
  lcd.print(F("ERROR"));
  lcd.setCursor(5, 2);
  lcd.print(F("FASE CAIDA"));


  IsMainViewActive = false;
  IsErrorFaseViewActive = true;
  IsInfoViewActive = false;
}

void ShowMainView()
{
  lcd.clear();
  PrintInitialText();
  UpdateBomba1Display();
  UpdateBomba2Display();

  UpdateCisternaDisplay();
  UpdateTanqueDisplay();

  IsMainViewActive = true;
  IsErrorFaseViewActive = false;
  IsInfoViewActive = false;

}

void ShowInfoView()
{
  if (IsInfoViewActive)
  {
    //me fijo cual es la ventana activa y muestro la siguiente.
  }
  else
    ActiveInfoViewNumber = 0;


  IsMainViewActive = false;
  IsErrorFaseViewActive = false;
  IsInfoViewActive = true;

}

void ShowBomba1InfoView()
{

}

void ShowBomba2InfoView()
{

}

void ShowOtherInfoView()
{

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
  lcd.setCursor(6, row);

  if (!enabled)
    lcd.print(F("Deshabilitada"));
  else
  {
    if (state == BOMBA_STATE_ON)
      lcd.print(F("ON           "));
    else if (state == BOMBA_STATE_OFF)
      lcd.print(F("OFF          "));
    else if (state == BOMBA_STATE_ERROR_CONTACTOR_ABIERTO)
      lcd.print(F("Err.Contactor"));
    else if (state == BOMBA_STATE_ERROR_CONTACTOR_CERRADO)
      lcd.print(F("Err.Contactor"));
    else if (state == BOMBA_STATE_ERROR_TERMICO)
      lcd.print(F("Err.Termico  "));
  }
}

void UpdateCisternaDisplay()
{
  lcd.setCursor(9, 3);
  if (sensores.IsCisternaSensorMinVal)
    lcd.print(F("Minimo"));
  else
    lcd.print(F("Normal"));
}

void UpdateTanqueDisplay()
{
  lcd.setCursor(7, 2);
  if (sensores.IsTanqueSensorMinVal)
    lcd.print(F("Minimo"));
  else
    lcd.print(F("Normal"));
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
  lcd.setCursor(19, 0);
  lcd.print(mode);
}


void UpdateActiveBombaDisplay()
{
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
  lcd.setCursor(19, 0);
  if (_mode == AUTO)
    lcd.print(F("A"));
  else
    lcd.print(F("M"));

  lcd.setCursor(0, 0);
  lcd.print(F("B1( ):"));

  lcd.setCursor(0, 1);
  lcd.print(F("B2( ):"));

  lcd.setCursor(0, 2);
  lcd.print(F("Tanque:"));

  lcd.setCursor(0, 3);
  lcd.print(F("Cisterna:"));
}

#endif
