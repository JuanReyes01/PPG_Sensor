//MAX30101 basic usage example
//By: Juan Camilo Reyes - juancreyes201@gmail.com
// Original MAX30100 code by Raivis Strogonovs and updated by Ben Rose for max30101
// https://morf.lv/implementing-pulse-oximeter-using-max30100

#include <Wire.h>
#include "MAX30101.h"


MAX30101 sensor;

void setup() {
  Serial.begin(115200);
  Wire.begin();

  // Initialize the sensor in SpO2 & heart rate mode
  if (sensor.begin(MAX30101_MODE_SPO2_HR,
                   MAX30101_SAMPLING_RATE_400HZ,
                   MAX30101_SAMPLE_AVERAGE_COUNT_4,
                   MAX30101_PULSE_WIDTH_411US_ADC_18,
                   MAX30101_LED_CURRENT_27_1MA,
                   true))
  {
    Serial.println("Sensor not found. Check wiring!");
    while (1);
  }

  // Set custom LED currents for optimal signal
  // Adjust these values based on your sensor datasheet recommendations
  byte customRedCurrent = 0x7F; // Example value for the red LED current
  byte customIRCurrent  = 0xA0; // Example value for the IR LED current
  sensor.setLEDCurrents(customRedCurrent, customIRCurrent);

  Serial.println("Custom LED currents set.");
}

void loop() {
  // Update the sensor readings and process the signal
  if (sensor.update()) {

    // Read raw FIFO values for debugging (commented for now)
    /*
    fifo_t fifo = sensor.readFIFO();
    Serial.print("Raw IR: ");
    Serial.print(fifo.rawIR);
    Serial.print(" | Raw Red: ");
    Serial.println(fifo.rawRed);
    */
    
    // Get the processed PPG waveform (cardiogram)
    // The normalized cardiogram is scaled for display or further processing
    float ppgNormalized = sensor.cardiogram();
    // The raw cardiogram value represents the filtered IR signal output
    float ppgRaw = sensor.rawCardiogram();

    Serial.print("PPG (normalized cardiogram): ");
    Serial.println(ppgNormalized);
    Serial.print("PPG (raw cardiogram): ");
    Serial.println(ppgRaw);

    // If a valid pulse is detected, also print heart rate and SpO₂
    if (sensor.pulseValid()) {
      Serial.print("BPM: ");
      Serial.println(sensor.BPM());
      Serial.print("SpO2: ");
      Serial.println(sensor.oxygen());
    }
  }
  delay(10); // Adjust delay based on your application's needs
}
