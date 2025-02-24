//MAX30101 code for PPG reading compatible with the python script
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
                   MAX30101_LED_CURRENT_50MA,
                   false))
  {
    //Serial.println("Sensor not found. Check wiring!");
    Serial.println("-1");
    while (1);
  }
}

void loop() {
  // Update the sensor readings and process the signal
  if(sensor.update()){
  float ppgNormalized = sensor.cardiogram();
  Serial.println(ppgNormalized);
  }
  
}
