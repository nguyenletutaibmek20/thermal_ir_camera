#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>

// put function declarations here:
TFT_eSPI tft = TFT_eSPI(); // Invoke custom library

void setup()
{
  // put your setup code here, to run once:
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH); // Set the Back-light on
  tft.init();

  tft.fillScreen(TFT_BLACK);
  tft.drawRect(0, 0, tft.width(), tft.height(), TFT_GREEN);

  // Set "cursor" at top left corner of display (0,0) and select font 4
  tft.setCursor(0, 4, 4);

  // Set the font colour to be white with a black background
  tft.setTextColor(TFT_WHITE);

  // We can now plot text on screen using the "print" class
  tft.println(" Initialised default\n");
  tft.println(" White text");

  tft.setTextColor(TFT_RED);
  tft.println(" Red text");

  tft.setTextColor(TFT_GREEN);
  tft.println(" Green text");

  tft.setTextColor(TFT_BLUE);
  tft.println(" Blue text");

  delay(5000);
}

void loop()
{
  tft.invertDisplay(false); // Where i is true or false

  tft.fillScreen(TFT_BLACK);
  tft.drawRect(0, 0, tft.width(), tft.height(), TFT_GREEN);

  tft.setCursor(0, 4, 4);

  tft.setTextColor(TFT_WHITE);
  tft.println(" Invert OFF\n");

  tft.println(" White text");

  tft.setTextColor(TFT_RED);
  tft.println(" Red text");

  tft.setTextColor(TFT_GREEN);
  tft.println(" Green text");

  tft.setTextColor(TFT_BLUE);
  tft.println(" Blue text");

  delay(5000);

  // Binary inversion of colours
  tft.invertDisplay(true); // Where i is true or false

  tft.fillScreen(TFT_BLACK);
  tft.drawRect(0, 0, tft.width(), tft.height(), TFT_GREEN);

  tft.setCursor(0, 4, 4);

  tft.setTextColor(TFT_WHITE);
  tft.println(" Invert ON\n");

  tft.println(" White text");

  tft.setTextColor(TFT_RED);
  tft.println(" Red text");

  tft.setTextColor(TFT_GREEN);
  tft.println(" Green text");

  tft.setTextColor(TFT_BLUE);
  tft.println(" Blue text");

  delay(5000);
}
