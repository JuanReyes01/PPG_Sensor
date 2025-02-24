import serial
import time
import sys
import numpy as np
import csv
from collections import deque

# Updated PyQtGraph and Qt imports
import pyqtgraph as pg
from pyqtgraph.Qt import QtWidgets, QtCore

#################################  parameters you might need to change based on your Arduino code       #################################################
port = 'COM4'             # Change this to the port name of the Arduino you are using.
baudrate = 115200         # Change to the Baudrate you are using in the Arduino code.
sample_rate = 3200        # Actual sampling rate [50, 100, 200, 400, 800, 1000, 1600, 3200]
average_count = 1        # Sample average count [1, 2, 4, 8, 16, 32]
pulse_width = 18          # Pulse width [15, 16, 17, 18]
#################################   end of parameters you need to edit       ################################################################################

# Global serial connection
arduino = None

def send_command(command):
    """Send a command to the Arduino and print its response."""
    print(f"Sending command: {command}")
    arduino.write((command + "\n").encode('utf-8'))
    time.sleep(0.1)  # Short delay to allow Arduino to process
    while arduino.in_waiting:
        response = arduino.readline().decode('utf-8').strip()
        if "Updated" in response or "SUCCES" in response:
            print(f"Arduino response: {response}")

# Subclass GraphicsLayoutWidget to catch key presses (e.g., Q to quit)
class MainWindow(pg.GraphicsLayoutWidget):
    def keyPressEvent(self, event):
        if event.key() == QtCore.Qt.Key_Q:
            print("Q pressed, exiting...")
            QtWidgets.QApplication.quit()
        else:
            super().keyPressEvent(event)

def main():
    global arduino

    # -------------------- Set Up Serial Connection and Arduino --------------------
    try:
        arduino = serial.Serial(port, baudrate, timeout=1)
    except Exception as e:
        print(f"Failed to open serial port {port}: {e}")
        sys.exit(1)

    time.sleep(2)  # Wait for Arduino to initialize
    if arduino.is_open:
        print("Serial port now open\n")
    else:
        print("Failed to open serial port")
        sys.exit(1)

    # Send configuration commands to the Arduino
    send_command(f"samplingRate={sample_rate}")
    send_command(f"sampleAverage={average_count}")
    send_command(f"pulseWidth={pulse_width}")
    time.sleep(3)  # Wait for Arduino to apply settings

    # -------------------- Set Up CSV Logging --------------------
    csv_filename = "ppg_data.csv"
    try:
        csv_file = open(csv_filename, "w", newline="")
    except Exception as e:
        print(f"Error opening CSV file for writing: {e}")
        sys.exit(1)
    csv_writer = csv.writer(csv_file)
    csv_writer.writerow(["Time (s)", "PPG Value"])
    start_time = time.time()

    # -------------------- Set Up PyQtGraph Window --------------------
    app = QtWidgets.QApplication([])  # Updated: using QtWidgets.QApplication
    win = MainWindow(show=True, title="Live PPG Plot")
    win.resize(800, 500)
    win.setWindowTitle('Live PPG Data Plotter (Press Q to Quit)')

    # Create a plot area in the window
    p = win.addPlot(title="PPG Data")
    p.setLabel('left', "PPG Signal", units='a.u.')
    p.setLabel('bottom', "Sample Number")
    curve = p.plot(pen='y')

    # Data buffers using deque for a fixed-length rolling window
    max_points = 1000  # Number of points to display
    data_buffer = deque(maxlen=max_points)
    x_buffer = deque(maxlen=max_points)
    sample_counter = 0  # Keep track of sample numbers

    # -------------------- Timer Callback for Reading Data, Updating Plot, and Saving to CSV --------------------
    def update():
        nonlocal sample_counter
        # Read all available lines from serial
        while arduino.in_waiting:
            try:
                line = arduino.readline().decode('utf-8').strip()
            except Exception as e:
                print(f"Error reading line: {e}")
                continue

            # Try converting the line to a float value
            try:
                value = float(line)
            except ValueError:
                # If conversion fails, skip this line (could be an error message)
                continue

            # Calculate elapsed time since start
            current_time = time.time() - start_time

            # Write the elapsed time and value to the CSV file
            csv_writer.writerow([current_time, value])
            csv_file.flush()  # Ensure the data is written to disk immediately

            # Append new data point to buffers for live plotting
            data_buffer.append(value)
            x_buffer.append(sample_counter)
            sample_counter += 1

        # Update the curve with new data if available
        if len(x_buffer) > 0:
            curve.setData(np.array(x_buffer), np.array(data_buffer))

    # Create a QTimer to call the update function periodically
    timer = QtCore.QTimer()
    timer.timeout.connect(update)
    timer.start(50)  # update every 50 ms

    # -------------------- Start Qt Event Loop --------------------
    print("Starting live plot. Press Q in the window to quit.")
    app.exec_()

    # Cleanup on exit
    if arduino is not None and arduino.is_open:
        arduino.close()
    csv_file.close()
    print("Serial port and CSV file closed.")

if __name__ == '__main__':
    main()
