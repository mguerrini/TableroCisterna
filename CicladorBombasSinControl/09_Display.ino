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
  display.setCursor(42, row);

  if (!enabled)
    display.print("Deshabilitada");
  else
  {
    if (state == BOMBA_STATE_ON)
      display.print("ON");
    else if (state == BOMBA_STATE_OFF)
      display.print("OFF");
    else if (state == BOMBA_STATE_ERROR_CONTACTOR_ABIERTO)
      display.print("Error contactor");
    else if (state == BOMBA_STATE_ERROR_CONTACTOR_CERRADO)
      display.print("Error contactor");
    else if (state == BOMBA_STATE_ERROR_TERMICO)
      display.print("Error termico");
  }

  display.display();
}

void UpdateCisternaDisplay()
{
  display.setTextColor(WHITE, BLACK);
  display.setTextSize(1);

  display.setCursor(54, 24);
  if (sensores.IsCisternaSensorMinVal)
    display.print("Minimo");
  else
    display.print("Normal");

  display.display();
}

void UpdateTanqueDisplay()
{
  display.setTextColor(WHITE, BLACK);
  display.setTextSize(1);

  display.setCursor(54, 16);
  if (sensores.IsTanqueSensorMinVal)
    display.print("Minimo");
  else
    display.print("Normal");

  display.display();
}


void UpdateDisplayToManual()
{
  UpdateDisplayMode("M");
}

void UpdateDisplayToAuto()
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


void UpdateActiveBombaDisplay(byte activeNumber)
{
  display.setTextColor(WHITE, BLACK);
  display.setTextSize(1);
  if (activeNumber == BOMBA1)
  {
    display.setCursor(24, 0);
    display.print("*");
    display.setCursor(24, 8);
    display.print(" ");
  }
  else if (activeNumber == BOMBA2)

  {
    display.setCursor(24, 0);
    display.print(" ");
    display.setCursor(24, 8);
    display.print("*");
  }
  else
  {
    display.setCursor(24, 0);
    display.print(" ");
    display.setCursor(24, 8);
    display.print(" ");
  }

  display.display();
}


void PrintInitialText()
{
  display.setTextSize(2);
  display.setCursor(116, 0);
  if (_mode == AUTO)
    display.print("A");
  else
    display.print("M");

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("B1 ( ):");

  display.setCursor(0, 8);
  display.print("B2 ( ):");

  display.setCursor(0, 16);
  display.print("Tanque:");

  display.setCursor(0, 24);
  display.print("Cisterna:");

  // update display with all of the above graphics
  display.display();
}
