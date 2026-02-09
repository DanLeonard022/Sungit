#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <LiquidCrystal_I2C.h> //For LCD

#define RAIN_ANALOG_PIN 34   // ADC1 pin (safe with WiFi)
#define ADC_MAX 4095
#define I2C_SDA 21
#define I2C_SCL 22

#define LCD_ADDR 0x27
bool bmeFound = false; //check if bme exists
int outputType = 0;
// outputType options
// 0 - All
// 1 - Temperature
// 2 - Humidity
// 3 - Pressure
// 4 - Rain

bool lcdAvailable = false;

Adafruit_BME280 bme;
LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);

void setup() {
  Serial.begin(115200);
  delay(1000);

  Wire.begin(I2C_SDA, I2C_SCL); //BME - ESPN, 3v3 - VIN,  GND - GND, SCL - G22, SDA - G21 //Wiring
                                //LCD - ESPN, 5V - VCC,  GND - GND, SCL - G22, SDA - G21
                                //Rain - VCC - 3V3, GND - GND, DO - G34, AO - G27

  analogReadResolution(12);
  pinMode(RAIN_ANALOG_PIN, INPUT);

  searchBus();

  Serial.println("start");

  checkBME();
  checkLCD();

}

void loop() {
  if (!bmeFound) {
    Serial.println("BME280 not found");
    delay(2000);
    return;
  }

  char line[64] = "";

  switch (outputType) {
    case 0: // All
      snprintf(line, sizeof(line), "%s %s %s %s",
               readTemp(),
               readHumidity(),
               readPressure(),
               readRain());
      break;

    case 1:
      snprintf(line, sizeof(line), "%s", readTemp());
      //break;

    case 2:
      snprintf(line, sizeof(line), "%s", readHumidity());
      //break;

    case 3:
      snprintf(line, sizeof(line), "%s", readPressure());
      //break;
    
    case 4: // Rain
      snprintf(line, sizeof(line), "%s", readRain());
      //break;
  }

  printMessage(line);   // LCD or Serial output
  delay(2000);
  lcd.clear();
}


void space(){
  Serial.println("");
}

void activeCheck(){
  Serial.println("active");
  delay(1000);
}

const char* readTemp() {
  static char buffer[16];
  float temperature = bme.readTemperature();
  snprintf(buffer, sizeof(buffer), "T: %.2fC", temperature);
  return buffer;
}

const char* readHumidity() {
  static char buffer[16];
  float humidity = bme.readHumidity();
  snprintf(buffer, sizeof(buffer), "H: %.2f%%", humidity);
  return buffer;
}

const char* readPressure() {
  static char buffer[20];
  float pressure = bme.readPressure() / 100.0F;
  snprintf(buffer, sizeof(buffer), "P: %.2fhPa", pressure);
  return buffer;
}

int readRainRaw() {
  const int samples = 10;
  int sum = 0;
  for (int i = 0; i < samples; i++) {
    sum += analogRead(RAIN_ANALOG_PIN);
    delay(5);
  }
  return sum / samples;
}

const char* readRain() {
  static char buffer[20];

  int raw = readRainRaw();
  int rainLevel = ADC_MAX - raw;  // invert (wet = higher)
  float percent = (rainLevel * 100.0) / ADC_MAX;

  if (percent < 20) {
    snprintf(buffer, sizeof(buffer), "R: NAN");
  } else if (percent < 45) {
    snprintf(buffer, sizeof(buffer), "R: LITE");
  } else if (percent < 75) {
    snprintf(buffer, sizeof(buffer), "R: MED");
  } else {
    snprintf(buffer, sizeof(buffer), "R: HEV");
  }

  return buffer;
}


void checkBME(){
  if (!bme.begin(0x76) && !bme.begin(0x77)) {
    Serial.println("BME280 not found");
    bmeFound = false;
  } else {
    Serial.println("BME280 initialized");
    bmeFound = true;
  }
}

void checkLCD() {
  lcdAvailable = detectLCD(LCD_ADDR);

  if (lcdAvailable) {
    delay(100);                 // allow LCD power stabilization
    lcd.begin(16, 2);           // IMPORTANT on ESP32
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("LCD READY");
    Serial.println("LCD detected");
  } else {
    Serial.println("LCD not found");
  }
}

void searchBus(){
   Serial.println("Scanning I2C bus...");
  
  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.print("Found I2C device at 0x");
      if (addr < 16) Serial.print("0");
      Serial.println(addr, HEX);
      delay(1);
    }
  }
  Serial.println("Done.");
}

bool detectLCD(uint8_t address) {
  Wire.beginTransmission(address);
  return (Wire.endTransmission() == 0);
}

// ---------- PRINT Message ----------
void printMessage(const char* message) {
  if (lcdAvailable) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(message);
    Serial.println("lcd print");
  } else {
    Serial.println(message);
  }
}


