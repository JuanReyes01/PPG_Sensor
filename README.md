# Plotter Setup

This document describes the setup for connecting the sensor, installing the Arduino code, running the Python plotting script, and installing the required Python libraries.

---

## Connections

The sensor has 4 pins that need to be connected to the Arduino as follows:

- **Vcc** -> `3.3V`
- **GND** -> `GND`
- **SCL** -> `SCL`
- **SDA** -> `SDA`

> **Note:** There is a fifth connection called `int`. In the current implementation, the Arduino only has 4 pins connected to a 5-pin socket, leaving this one unconnected.

---

## Arduino Code

The Arduino code compatible with this version of the plotter is **PPG_with_python**.

- **Important:** If you change the Arduino code, you will need to update the Python code accordingly.
- **Installation:** If you are unsure how to install the Arduino library, decompress the provided zip file into the Arduino library folder (typically located in your `Documents` folder).

---

## Python

This Python code takes the output from the Arduino, plots it live, and saves the output to a CSV file.

- **Normalization:** The Arduino code already normalizes the PPG output by removing any outliers.

---

## Some Notes

- If you are getting the message 'No sensor found', try pressing the Arduino reset button (the only one) and wait a few seconds.
- If the error persists, make sure you are pressing the sensor firmly in between your fingers.

# MAX30101 Configuration Documentation

How key parameters in the MAX30101 sensor configuration affect its performance. The primary parameters that can be adjusted are:

- **Sample Rate**
- **Average Count**
- **Pulse Width**

For further details, please refer to the [MAX30101 datasheet](https://www.analog.com/media/en/technical-documentation/data-sheets/max30101.pdf).

---

## Sample Rate

NOTE: after further inspection I have found that the library code for **sample rate** is inside a if(false) meaning it is not being utilized.

The **sample rate** determines how frequently the sensor collects data from its photodiodes.

- **Definition:**  
  The number of samples per second (Hz) that the sensor outputs.

- **Implications:**
  - **Higher Sample Rate:**
    - Provides more data points per second, improving temporal resolution.
    - May increase noise if the rate is too high.
  - **Lower Sample Rate:**
    - Yields fewer data points, which can help reduce noise.

---

## Average Count

The **average count** specifies how many individual readings are combined to produce a smoother output signal.

- **Definition:**  
  The number of samples that are averaged together to reduce noise.

- **Implications:**
  - **Higher Average Count:**
    - Increases smoothing by reducing the effect of random noise.
    - Can lower the sensor’s responsiveness to rapid changes in the signal.
  - **Lower Average Count:**
    - Allows the sensor to respond more quickly to changes.
    - May result in a noisier output due to less averaging.

---

## Pulse Width

The **pulse width** sets the duration for which the LED is active during each measurement cycle.

- **Definition:**  
  The duration of the LED pulse, which determines the integration time for the sensor's analog-to-digital conversion.

- **Implications:**
  - **Longer Pulse Width:**
    - Increases the amount of light collected, enhancing the signal-to-noise ratio (SNR) and measurement resolution.
    - May risk saturating the sensor if the LED current is too high or if ambient light conditions are strong.
  - **Shorter Pulse Width:**
    - Limits the light collected, reducing the chance of saturation.
    - May result in lower SNR due to a shorter integration period.

---
