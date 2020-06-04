// --- DISPLAY ---
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>


// --- DISPLAY ---
#define OLED_ADDR   0x3C

Adafruit_SSD1306 display(-1);



void SetupDisplay()
{
  // initialize and clear display
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.clearDisplay();
  display.display();

  // display a line of text
  display.setFont(NULL);
  display.setTextColor(WHITE);

  PrintInitialText();
}


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
  UpdateBombaDisplay(bomba2.IsEnabled, bomba2.State, 8);
}

void UpdateBombaDisplay(bool enabled, int state, int row)
{
  display.setTextColor(WHITE, BLACK);
  display.setTextSize(1);
  display.setCursor(35, row);

  if (!enabled)
      display.print(F("Deshabilitada"));
  else
  {
    if (state == BOMBA_STATE_ON)
      display.print(F("ON           "));
    else if (state == BOMBA_STATE_OFF)
      display.print(F("OFF          "));
    else if (state == BOMBA_STATE_ERROR_CONTACTOR_ABIERTO)
      display.print(F("Err.Contactor"));
    else if (state == BOMBA_STATE_ERROR_CONTACTOR_CERRADO)
      display.print(F("Err.Contactor"));
    else if (state == BOMBA_STATE_ERROR_TERMICO)
      display.print(F("Err.Termico  "));
  }

  display.display();
}

void UpdateCisternaDisplay()
{
  display.setTextColor(WHITE, BLACK);
  display.setTextSize(1);

  display.setCursor(54, 24);
  if (sensores.IsCisternaSensorMinVal)
    display.print(F("Minimo"));
  else
    display.print(F("Normal"));

  display.display();
}

void UpdateTanqueDisplay()
{
  display.setTextColor(WHITE, BLACK);
  display.setTextSize(1);

  display.setCursor(54, 16);
  if (sensores.IsTanqueSensorMinVal)
    display.print(F("Minimo"));
  else
    display.print(F("Normal"));

  display.display();
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
  display.setTextColor(WHITE, BLACK);
  display.setTextSize(2);
  display.setCursor(116, 0);
  display.print(mode);
  display.display();
}


void UpdateActiveBombaDisplay()
{
  display.setTextColor(WHITE, BLACK);
  display.setTextSize(1);

  if (bomba1.IsActive)
  {
    display.setCursor(18, 0);
    display.print(F("*"));
    display.setCursor(18, 8);
    display.print(F(" "));
  }
  else if (bomba2.IsActive)
  {
    display.setCursor(18, 0);
    display.print(F(" "));
    display.setCursor(18, 8);
    display.print(F("*"));
  }
  else
  {
    display.setCursor(18, 0);
    display.print(F(" "));
    display.setCursor(18, 8);
    display.print(F(" "));
  }

  display.display();
}


void PrintInitialText()
{
  display.setTextSize(2);
  display.setCursor(116, 0);
  if (_mode == AUTO)
    display.print(F("A"));
  else
    display.print(F("M"));

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(F("B1( )"));
  display.setCursor(29, 0);
  display.print(F(":"));

  display.setCursor(0, 8);
  display.print(F("B2( )"));
  display.setCursor(29, 8);
  display.print(F(":"));

  display.setCursor(0, 16);
  display.print(F("Tanque:"));

  display.setCursor(0, 24);
  display.print(F("Cisterna:"));

  // update display with all of the above graphics
  display.display();
}
