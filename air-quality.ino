#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include "Adafruit_SGP30.h"
#include "Adafruit_PM25AQI.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SEALEVELPRESSURE_HPA (1013.25)

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


Adafruit_BME680 bme; // I2C
Adafruit_SGP30 sgp;
Adafruit_PM25AQI aqi = Adafruit_PM25AQI();

/* return absolute humidity [mg/m^3] with approximation formula
* @param temperature [°C]
* @param humidity [%RH]
*/
uint32_t getAbsoluteHumidity(float temperature, float humidity) {
    // approximation formula from Sensirion SGP30 Driver Integration chapter 3.15
    const float absoluteHumidity = 216.7f * ((humidity / 100.0f) * 6.112f * exp((17.62f * temperature) / (243.12f + temperature)) / (273.15f + temperature)); // [g/m^3]
    const uint32_t absoluteHumidityScaled = static_cast<uint32_t>(1000.0f * absoluteHumidity); // [mg/m^3]
    return absoluteHumidityScaled;
}

void setup() {
  Serial.begin(115200);
  //while (!Serial) { delay(10); } // Wait for serial console to open!
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) { // Address 0x3D for 128x64
    Serial.println("SSD1306 allocation failed");
    for(;;) {
      Serial.println("SSD1306 allocation failed");
      delay(5000);
    }
  }
  display.clearDisplay();
  //display.setTextSize(2); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Air Quality Sensor");
  display.display();

  display.print("SGP30... ");
  display.display();
  if (! sgp.begin()){
    for(;;) {
      Serial.println("Sensor not found :(");
      delay(5000);
    }
  }
  // If you have a baseline measurement from before you can assign it to start, to 'self-calibrate'
  //sgp.setIAQBaseline(0x8E68, 0x8F41);  // Will vary for each sensor!
  display.println("ready!");
  display.display();
  Serial.print("Found SGP30 serial #");
  Serial.print(sgp.serialnumber[0], HEX);
  Serial.print(sgp.serialnumber[1], HEX);
  Serial.println(sgp.serialnumber[2], HEX);

  display.print("BME680... ");
  display.display();
  if (!bme.begin()) {
    while (1) {
      Serial.println("Could not find a valid BME680 sensor, check wiring!");
      delay(5000);
    }
  }

  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms
  display.println("ready!");
  display.display();

  display.print("PMSA300I... ");
  display.display();
  if (! aqi.begin_I2C()) {
    while (1) {
      Serial.println("Could not find PM 2.5 sensor!");
      delay(5000);
    }
  }
  display.println("ready!");
  display.display();

  delay(2000);
}

int counter = 0;
void loop() {
  
  if (! bme.performReading()) {
    Serial.println("Failed to perform reading of BME680");
  } else {
    Serial.print("Temperature = "); Serial.print(bme.temperature); Serial.println(" *C");
    Serial.print("Pressure = "); Serial.print(bme.pressure / 100.0); Serial.println(" hPa");
    Serial.print("Humidity = "); Serial.print(bme.humidity); Serial.println(" %");
    Serial.print("Gas = "); Serial.print(bme.gas_resistance / 1000.0); Serial.println(" KOhms");

    // If you have a temperature / humidity sensor, you can set the absolute humidity to enable the humditiy compensation for the air quality signals
//    float temperature = 22.1; // [°C]
//    float humidity = 45.2; // [%RH]
    sgp.setHumidity(getAbsoluteHumidity(bme.temperature, bme.humidity));
  }

  if (! sgp.IAQmeasure()) {
    Serial.println("Measurement failed");
    return;
  }
  Serial.print("TVOC "); Serial.print(sgp.TVOC); Serial.print(" ppb\t");
  Serial.print("eCO2 "); Serial.print(sgp.eCO2); Serial.println(" ppm");

  if (! sgp.IAQmeasureRaw()) {
    Serial.println("Raw Measurement failed");
    return;
  }
  Serial.print("Raw H2 "); Serial.print(sgp.rawH2); Serial.print(" \t");
  Serial.print("Raw Ethanol "); Serial.print(sgp.rawEthanol); Serial.println("");

  PM25_AQI_Data data;
  if (! aqi.read(&data)) {
    Serial.println("Could not read from AQI");
    delay(500);  // try again in a bit!
    return;
  }
    Serial.println("AQI reading success");

  Serial.println();
  Serial.println(F("---------------------------------------"));
  Serial.println(F("Concentration Units (standard)"));
  Serial.println(F("---------------------------------------"));
  Serial.print(F("PM 1.0: ")); Serial.print(data.pm10_standard);
  Serial.print(F("\t\tPM 2.5: ")); Serial.print(data.pm25_standard);
  Serial.print(F("\t\tPM 10: ")); Serial.println(data.pm100_standard);
  Serial.println(F("Concentration Units (environmental)"));
  Serial.println(F("---------------------------------------"));
  Serial.print(F("PM 1.0: ")); Serial.print(data.pm10_env);
  Serial.print(F("\t\tPM 2.5: ")); Serial.print(data.pm25_env);
  Serial.print(F("\t\tPM 10: ")); Serial.println(data.pm100_env);
  Serial.println(F("---------------------------------------"));
  Serial.print(F("Particles > 0.3um / 0.1L air:")); Serial.println(data.particles_03um);
  Serial.print(F("Particles > 0.5um / 0.1L air:")); Serial.println(data.particles_05um);
  Serial.print(F("Particles > 1.0um / 0.1L air:")); Serial.println(data.particles_10um);
  Serial.print(F("Particles > 2.5um / 0.1L air:")); Serial.println(data.particles_25um);
  Serial.print(F("Particles > 5.0um / 0.1L air:")); Serial.println(data.particles_50um);
  Serial.print(F("Particles > 10 um / 0.1L air:")); Serial.println(data.particles_100um);
  Serial.println(F("---------------------------------------"));

  display.clearDisplay();
  display.setCursor(0, 0);
  
  display.print("Temperature: "); display.print(bme.temperature); display.println(" *C");
  display.print("Pressure: "); display.print(bme.pressure / 100); display.println(" hPa");
  display.print("Humidity: "); display.print(bme.humidity); display.println(" %");
  //display.print("Gas: "); display.print(bme.gas_resistance / 1000.0); display.println(" KOhms");
  display.print("TVOC: "); display.print(sgp.TVOC); display.println(" ppb");
  display.print("eCO2: "); 
    if (sgp.eCO2 == 400) { display.print("<= "); } 
    display.print(sgp.eCO2); display.println(" ppm");
  //display.print("Raw H2: "); display.println(sgp.rawH2);
  //display.print("Raw Ethanol: "); display.println(sgp.rawEthanol);
  display.print(F("PM 0.3 / 0.1L:")); display.println(data.particles_03um);
  display.print(F("PM 1.0 (STD): ")); display.println(data.pm10_standard);
  display.print(F("PM 2.5 (STD): ")); display.println(data.pm25_standard);

  display.display(); 
  
 
  delay(1000);

  counter++;
  if (counter == 30) {
    counter = 0;

    uint16_t TVOC_base, eCO2_base;
    if (! sgp.getIAQBaseline(&eCO2_base, &TVOC_base)) {
      Serial.println("Failed to get baseline readings");
      return;
    }
    Serial.print("****Baseline values: eCO2: 0x"); Serial.print(eCO2_base, HEX);
    Serial.print(" & TVOC: 0x"); Serial.println(TVOC_base, HEX);
  }
}
