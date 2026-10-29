#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

// Display pins (software SPI)
#define TFT_CS   7
#define TFT_DC   A5
#define TFT_RST  A4
#define TFT_MOSI 4
#define TFT_SCLK 8

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

// Flow sensor
const uint8_t FLOW_PIN = 2;               // INT0 on the Uno
const float PULSES_PER_LITER = 5880.0;    // YF-S401 datasheet: F = 98 * Q(L/min) -> 98*60 pulses/L. Calibrate yours!

volatile unsigned long pulseCount = 0;
float totalLiters = 0.0;
unsigned long lastUpdate = 0;

void onPulse() {
  pulseCount++;
}

void setup() {
  Serial.begin(9600);

  pinMode(FLOW_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(FLOW_PIN), onPulse, FALLING);

  tft.initR(INITR_BLACKTAB);   // try INITR_GREENTAB or INITR_REDTAB if colors/edges look off
  tft.setRotation(1);
  tft.fillScreen(ST77XX_BLACK);

  tft.setTextSize(1);
  tft.setTextColor(ST77XX_CYAN, ST77XX_BLACK);
  tft.setCursor(0, 5);
  tft.print("Flow rate (L/min)");
  tft.setCursor(0, 55);
  tft.print("Total (L)");

  lastUpdate = millis();
}

void loop() {
  unsigned long now = millis();
  unsigned long elapsed = now - lastUpdate;

  if (elapsed >= 1000) {
    // Safely grab and reset the pulse count
    noInterrupts();
    unsigned long pulses = pulseCount;
    pulseCount = 0;
    interrupts();
    lastUpdate = now;

    float seconds = elapsed / 1000.0;
    float liters = pulses / PULSES_PER_LITER;
    float flowLpm = (liters / seconds) * 60.0;
    totalLiters += liters;

    Serial.print(flowLpm, 2);
    Serial.print(" L/min  total ");
    Serial.println(totalLiters, 3);

    // Drawing text with a background color overwrites old digits without a full clear
    tft.setTextSize(3);
    tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
    tft.setCursor(0, 18);
    tft.print(flowLpm, 2);
    tft.print("  ");

    tft.setTextSize(2);
    tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK);
    tft.setCursor(0, 68);
    tft.print(totalLiters, 3);
    tft.print("  ");
  }
}