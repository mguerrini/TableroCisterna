#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

// OLED display TWI address
#define OLED_ADDR   0x3C

Adafruit_SSD1306 display(-1);

void setup() {
  // initialize and clear display
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.clearDisplay();
  display.display();

  // display a line of text
  display.setFont(NULL);
  display.setTextColor(WHITE);

  display.setTextSize(2);
  display.setCursor(116,0);
  display.print("A");

  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("B1 (*): ON");

  display.setCursor(0,8);
  display.print("B2 ( ): OFF");

  display.setCursor(0,16);
  display.print("Tanque: Normal");

  display.setCursor(0,24);
  display.print("Cisterna: Normal");


  // update display with all of the above graphics
  display.display();
}

void loop() {
  // put your main code here, to run repeatedly:

}
