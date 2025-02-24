#include <Wire.h>
#include "MAX30101.h"

MAX30101 sensor;

// Global configuration variables using the proper enum types
// Using SPO2_HR mode for full functionality (SpO2 + heart rate)
Mode sensorMode = MAX30101_MODE_SPO2_HR;
SamplingRate samplingRate = MAX30101_SAMPLING_RATE_400HZ;
SampleAveragingCount sampleAverage = MAX30101_SAMPLE_AVERAGE_COUNT_4;
LEDPulseWidth pulseWidth = MAX30101_PULSE_WIDTH_411US_ADC_18;
LEDCurrent ledCurrent = MAX30101_LED_CURRENT_50MA;
bool useFIFO = false;

// Helper function to map a numeric string to a SamplingRate enum
SamplingRate getSamplingRateFromString(const String &valueStr) {
  int value = valueStr.toInt();
  if (value == 50)    return MAX30101_SAMPLING_RATE_50HZ;
  else if (value == 100)  return MAX30101_SAMPLING_RATE_100HZ;
  else if (value == 200)  return MAX30101_SAMPLING_RATE_200HZ;
  else if (value == 400)  return MAX30101_SAMPLING_RATE_400HZ;
  else if (value == 800)  return MAX30101_SAMPLING_RATE_800HZ;
  else if (value == 1000) return MAX30101_SAMPLING_RATE_1000HZ;
  else if (value == 1600) return MAX30101_SAMPLING_RATE_1600HZ;
  else if (value == 3200) return MAX30101_SAMPLING_RATE_3200HZ;
  return samplingRate;
}

// Helper function to map a numeric string to a SampleAveragingCount enum
SampleAveragingCount getSampleAverageFromString(const String &valueStr) {
  int value = valueStr.toInt();
  if (value == 1)    return MAX30101_SAMPLE_AVERAGE_COUNT_1;
  else if (value == 2)   return MAX30101_SAMPLE_AVERAGE_COUNT_2;
  else if (value == 4)   return MAX30101_SAMPLE_AVERAGE_COUNT_4;
  else if (value == 8)   return MAX30101_SAMPLE_AVERAGE_COUNT_8;
  else if (value == 16)  return MAX30101_SAMPLE_AVERAGE_COUNT_16;
  else if (value == 32)  return MAX30101_SAMPLE_AVERAGE_COUNT_32;
  return sampleAverage;
}

// Helper function to map a numeric string to a LEDPulseWidth enum
LEDPulseWidth getPulseWidthFromString(const String &valueStr) {
  int value = valueStr.toInt();
  if (value == 15)    return MAX30101_PULSE_WIDTH_69US_ADC_15;
  else if (value == 16)   return MAX30101_PULSE_WIDTH_118US_ADC_16;
  else if (value == 17)   return MAX30101_PULSE_WIDTH_215US_ADC_17;
  else if (value == 18)   return MAX30101_PULSE_WIDTH_411US_ADC_18;
  return pulseWidth;
}

// Function to (re)initialize the sensor with the current configuration
void reinitializeSensor() {
  // sensor.begin returns 0 on success, nonzero on error
  if (sensor.begin(sensorMode, samplingRate, sampleAverage, pulseWidth, ledCurrent, useFIFO)) {
    Serial.println("ERROR: Sensor not found. Check wiring!");
  } else {
    Serial.println("SUCCESS: Sensor reinitialized");
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin();

  // Initialize the sensor with the default configuration.
  if (sensor.begin(sensorMode, samplingRate, sampleAverage, pulseWidth, ledCurrent, useFIFO)) {
    Serial.println("ERROR: Sensor not found. Check wiring!");
    while (1);
  }
}

void loop() {
  // Process serial commands to update configuration
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();  // Remove extra whitespace

    int separatorIndex = input.indexOf('=');
    if (separatorIndex != -1) {
      String paramName = input.substring(0, separatorIndex);
      String valueStr  = input.substring(separatorIndex + 1);
      
      // Update configuration based on parameter name
      if (paramName.equals("samplingRate")) {
        samplingRate = getSamplingRateFromString(valueStr);
        Serial.print("Updated samplingRate to: ");
        Serial.println(valueStr);
      } else if (paramName.equals("sampleAverage")) {
        sampleAverage = getSampleAverageFromString(valueStr);
        Serial.print("Updated sampleAverage to: ");
        Serial.println(valueStr);
      } else if (paramName.equals("pulseWidth")) {
        pulseWidth = getPulseWidthFromString(valueStr);
        Serial.print("Updated pulseWidth to: ");
        Serial.println(valueStr);
      } else {
        Serial.print("Unknown parameter: ");
        Serial.println(paramName);
      }
      
      // Reinitialize sensor with new settings
      reinitializeSensor();
    } else {
      Serial.println("Invalid command format. Expected: parameter=value");
    }
  }

  // Update sensor readings and print normalized PPG data
  if (sensor.update()) {
    float ppgNormalized = sensor.cardiogram();
    Serial.println(ppgNormalized, 4);  // Print with 4 decimals
  }
}
