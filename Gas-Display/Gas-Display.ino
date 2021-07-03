//SGP30 needs 15 seconds to initialize calibration after power on.
//The screen will display TVOC and CO2

#include <M5StickC.h>
#include "Adafruit_SGP30.h"

Adafruit_SGP30 sgp;
int i = 15;
long last_millis = 0;

int screenSelection = 0;

int lastDisplayUpdate = -100000;
int displayUpdateDelay = 1000;

bool displayCanUpdate() {
  return ((millis() - lastDisplayUpdate) > displayUpdateDelay);
}

void header(const char *string, uint16_t screenColor)
{
    M5.Lcd.fillScreen(screenColor);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.fillRect(0, 0, 80, 20, TFT_BLACK);
    M5.Lcd.setTextDatum(TC_DATUM);
    M5.Lcd.drawString(string, 80, 2, 2); 
    M5.Lcd.setTextDatum(TL_DATUM);
}

void setup() {
  M5.begin(true, false, false);
  Wire.begin(32,33);
  M5.Lcd.setRotation(3);
  Serial.begin(115200);
  Serial.println("SGP30 test");
  if (! sgp.begin()){
    Serial.println("Sensor not found :(");
    while (1);
  }
  
  Serial.print("Found SGP30 serial #");
  Serial.print(sgp.serialnumber[0], HEX);
  Serial.print(sgp.serialnumber[1], HEX);
  Serial.println(sgp.serialnumber[2], HEX);
  initWait();
}

void displayValue(char *title, float value, char *units) {
  header(title, BLACK);
  M5.Lcd.setTextSize(2);
  M5.Lcd.drawNumber(value, 40, 25 , 2);
  M5.Lcd.setTextSize(1);
  M5.Lcd.drawString(units, 120, 65, 2);
}

void carbonDisplay() {
  displayValue("Carbon", sgp.eCO2, "ppm");
}

void tvocDisplay() {
  displayValue("TVOC", sgp.TVOC, "ppb");
}

void ethanolDisplay() {
  displayValue("Ethanol", sgp.rawEthanol, "?");
}

void h2Display() {
  displayValue("H2", sgp.rawH2, "?");
}

void handleDisplay(bool force = false) {
  if(!force && !displayCanUpdate()) return;
  lastDisplayUpdate = millis();

  if (!sgp.IAQmeasure()) {
    Serial.println("Measure failure");
    return;
  }
  if (! sgp.IAQmeasureRaw()) {
    Serial.println("Raw Measurement failed");
    return;
  }
  
  // Serial output
  Serial.print("TVOC "); Serial.print(sgp.TVOC); Serial.print(" ppb\t");
  Serial.print("eCO2 "); Serial.print(sgp.eCO2); Serial.println(" ppm");
  Serial.print("Raw H2 "); Serial.print(sgp.rawH2); Serial.print(" \t");
  Serial.print("Raw Ethanol "); Serial.print(sgp.rawEthanol); Serial.println("");
  
  switch(screenSelection) {
    case 0:
      carbonDisplay();
      break;
    case 1:
      tvocDisplay();
      break;
    case 2:
      h2Display();
      break;
    case 3:
      ethanolDisplay();
      break;
    default:
      screenSelection = 0;
      return handleDisplay(true);
  }
}

void handleButtons() {
  if(M5.BtnB.wasPressed()) {
    Serial.printf("BtnB %d\n", screenSelection);
    screenSelection++;
    handleDisplay(true);
  }
}

void initWait() {
  int waitSeconds = 15;
  float counter = 0;
  while(counter != waitSeconds) {
    header("Starting...", BLACK);
    
    counter += 1;
    float progress = (counter/waitSeconds);
    int width = 100 * progress;
    M5.Lcd.fillRect(30, 30, width, 30, GREEN);
    delay(1000);
  }
  header("Ready", BLACK);
}

void loop() {
  M5.update();
  handleButtons();
  handleDisplay();
}
