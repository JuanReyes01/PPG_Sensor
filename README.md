# Plotter Setup

This document describes the setup for connecting the sensor, installing the Arduino code, running the Python plotting script, and installing the required Python libraries.

---

## Connections

The sensor has 4 pins that need to be connected to the Arduino as follows:

- **Orange (Vcc)** -> `3.3V`
- **Brown (GND)** -> `GND`
- **Purple (SCL)** -> `SCL`
- **Green (SDA)** -> `SDA`

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

## Required Python Libraries

Before running the Python script, make sure to install the following libraries:

- **Seaborn**
- **Matplotlib**
- **Pandas**
- **PySerial** (for serial communication)
- **NumPy**
- **PyQtGraph**

You can install them using `pip` by running the following command:

```bash
pip install seaborn matplotlib pandas pyserial numpy pyqtgraph
```
## Some Notes

- If you are getting the values `-1`, try pressing the Arduino reset button (the only one) and wait a few seconds.
- If the error persists, disconnect the orange cable and reconnect it, then press the reset button again.





