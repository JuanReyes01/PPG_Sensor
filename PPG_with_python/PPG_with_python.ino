/*
 * This code includes a companion python script plot_ppg_data.py
 * It will simply transfer data collection to the Python script to make it more user-friendly
 * All important changes to how the sensor collects data should be made in this script
 */


 #include <Wire.h>
 #include "MAX30101.h"
 #include <EEPROM.h>
 
 MAX30101 sensor;
 
 // Global configuration variables
 Mode sensorMode = MAX30101_MODE_SPO2_HR;
 SamplingRate samplingRate = MAX30101_SAMPLING_RATE_3200HZ;
 SampleAveragingCount sampleAverage = MAX30101_SAMPLE_AVERAGE_COUNT_1;
 LEDPulseWidth pulseWidth = MAX30101_PULSE_WIDTH_411US_ADC_18;
 LEDCurrent ledCurrent = MAX30101_LED_CURRENT_50MA;
 bool useFIFO = false;
 
 // EEPROM addresses for each parameter
 const int ADDR_SAMPLING_RATE = 0;
 const int ADDR_SAMPLE_AVERAGE  = 1;
 const int ADDR_PULSE_WIDTH     = 2;
 
 void saveConfig() {
   EEPROM.write(ADDR_SAMPLING_RATE, (uint8_t)samplingRate);
   EEPROM.write(ADDR_SAMPLE_AVERAGE, (uint8_t)sampleAverage);
   EEPROM.write(ADDR_PULSE_WIDTH, (uint8_t)pulseWidth);
 }
 
 void loadConfig() {
 
   byte storedSamplingRate = EEPROM.read(ADDR_SAMPLING_RATE);
   byte storedSampleAverage  = EEPROM.read(ADDR_SAMPLE_AVERAGE);
   byte storedPulseWidth     = EEPROM.read(ADDR_PULSE_WIDTH);
 
   // Update only if the stored values seem valid. (or at least an little effort)
   if (storedSamplingRate != 0) {
     samplingRate = (SamplingRate)storedSamplingRate;
   }
   if (storedSampleAverage != 0) {
     sampleAverage = (SampleAveragingCount)storedSampleAverage;
   }
   if (storedPulseWidth != 0) {
     pulseWidth = (LEDPulseWidth)storedPulseWidth;
   }
 }
 
 const char* getSamplingRateString(SamplingRate rate) {
   switch (rate) {
     case MAX30101_SAMPLING_RATE_50HZ: return "50 Hz";
     case MAX30101_SAMPLING_RATE_100HZ: return "100 Hz";
     case MAX30101_SAMPLING_RATE_200HZ: return "200 Hz";
     case MAX30101_SAMPLING_RATE_400HZ: return "400 Hz";
     case MAX30101_SAMPLING_RATE_800HZ: return "800 Hz";
     case MAX30101_SAMPLING_RATE_1000HZ: return "1000 Hz";
     case MAX30101_SAMPLING_RATE_1600HZ: return "1600 Hz";
     case MAX30101_SAMPLING_RATE_3200HZ: return "3200 Hz";
     default: return "Unknown";
   }
 }
 
 const char* getSampleAverageString(SampleAveragingCount avg) {
   switch (avg) {
     case MAX30101_SAMPLE_AVERAGE_COUNT_1: return "1";
     case MAX30101_SAMPLE_AVERAGE_COUNT_2: return "2";
     case MAX30101_SAMPLE_AVERAGE_COUNT_4: return "4";
     case MAX30101_SAMPLE_AVERAGE_COUNT_8: return "8";
     case MAX30101_SAMPLE_AVERAGE_COUNT_16: return "16";
     case MAX30101_SAMPLE_AVERAGE_COUNT_32: return "32";
     default: return "Unknown";
   }
 }
 
 const char* getPulseWidthString(LEDPulseWidth width) {
   switch (width) {
     case MAX30101_PULSE_WIDTH_69US_ADC_15: return "69us (ADC 15)";
     case MAX30101_PULSE_WIDTH_118US_ADC_16: return "118us (ADC 16)";
     case MAX30101_PULSE_WIDTH_215US_ADC_17: return "215us (ADC 17)";
     case MAX30101_PULSE_WIDTH_411US_ADC_18: return "411us (ADC 18)";
     default: return "Unknown";
   }
 }
 
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
 
 LEDPulseWidth getPulseWidthFromString(const String &valueStr) {
   int value = valueStr.toInt();
   if (value == 15)    return MAX30101_PULSE_WIDTH_69US_ADC_15;
   else if (value == 16)   return MAX30101_PULSE_WIDTH_118US_ADC_16;
   else if (value == 17)   return MAX30101_PULSE_WIDTH_215US_ADC_17;
   else if (value == 18)   return MAX30101_PULSE_WIDTH_411US_ADC_18;
   return pulseWidth;
 }
 
 void reinitializeSensor() {
   if (sensor.begin(sensorMode, samplingRate, sampleAverage, pulseWidth, ledCurrent, useFIFO)) {
     Serial.println("ERROR: Sensor not found. Check wiring!");
   } else {
     Serial.println("SUCCESS: Sensor reinitialized");
   }
 }
 
 void setup() {
   Serial.begin(115200);
   Wire.begin();
 
   // Load saved configuration
   loadConfig();
 
   // Initialize the sensor with the configuration.
   if (sensor.begin(sensorMode, samplingRate, sampleAverage, pulseWidth, ledCurrent, useFIFO)) {
     Serial.println("ERROR: Sensor not found. Check wiring!");
     while (1);
   }
 
   // Print current parameters on startup
   Serial.println("\nCurrent Parameters:");
   Serial.print("Sampling Rate: ");
   Serial.println(getSamplingRateString(samplingRate));
   Serial.print("Sample Averaging: ");
   Serial.println(getSampleAverageString(sampleAverage));
   Serial.print("Pulse Width: ");
   Serial.println(getPulseWidthString(pulseWidth));
 }
 
 void loop() {
   // Check for configuration commands first
   if (Serial.available() > 0) {
     String input = Serial.readStringUntil('\n');
     input.trim();
 
     int separatorIndex = input.indexOf('=');
     if (separatorIndex != -1) {
       String paramName = input.substring(0, separatorIndex);
       String valueStr  = input.substring(separatorIndex + 1);
       
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
       
       saveConfig();      // Save the updated config
       reinitializeSensor();
 
     } else {
       Serial.println("Invalid command format. Expected: parameter=value");
     }
   }
 
   // Update sensor reading and print timestamp + sensor value
   if (sensor.update()) {
     float ppgNormalized = sensor.cardiogram();
     // Use micros() for high-resolution timing (in microseconds) then convert to seconds:
     unsigned long t = micros();
     float timestamp = t / 1000000.0;
     
     // Print timestamp and sensor value separated by a comma.
     Serial.print(timestamp, 6);  // 6 decimal places for microsecond resolution
     Serial.print(",");
     Serial.println(ppgNormalized, 4);
   }
 }