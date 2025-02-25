import serial
import time
import sys
import numpy as np
import csv
from collections import deque
import queue
from threading import Thread, Event
from queue import Queue

# Updated PyQtGraph and Qt imports
import pyqtgraph as pg
from pyqtgraph.Qt import QtWidgets, QtCore

#################################  parameters you might need to change based on your Arduino code       #################################################
port = 'COM4'             # Change this to the port name of the Arduino you are using.
baudrate = 115200         # Change to the Baudrate you are using in the Arduino code.
sample_rate = 400        # Actual sampling rate [50, 100, 200, 400, 800, 1000, 1600, 3200]
average_count = 1         # Sample average count [1, 2, 4, 8, 16, 32]
pulse_width = 18          # Pulse width [15, 16, 17, 18]
#################################   end of parameters you need to edit       ################################################################################


class SerialThread(Thread):
    """
    Thread for reading from the serial port. It now duplicates each sensor sample
    into two separate queues: one for CSV logging and one for plotting.
    """
    def __init__(self, csv_queue, plot_queue, message_queue, command_queue):
        super().__init__()
        self.csv_queue = csv_queue
        self.plot_queue = plot_queue
        self.message_queue = message_queue
        self.command_queue = command_queue
        self.running = Event()
        self.running.set()
        self.arduino = None

    def run(self):
        try:
            self.arduino = serial.Serial(port, baudrate, timeout=1)
            time.sleep(2)  # Allow Arduino time to reset
            if self.arduino.is_open:
                self.message_queue.put("Serial port now open")
            else:
                self.message_queue.put("Failed to open serial port")
                sys.exit(1)

            self._send_configuration()

            while self.running.is_set():
                self._process_commands()
                self._read_serial_data()
                # Minimal sleep to yield CPU if no data is waiting.
                time.sleep(0.001)
        except Exception as e:
            self.message_queue.put(f"Serial error: {str(e)}")
        finally:
            if self.arduino and self.arduino.is_open:
                self.arduino.close()
                self.message_queue.put("Serial port closed")

    def _send_configuration(self):
        """Send initial configuration commands to Arduino."""
        commands = [
            f"samplingRate={sample_rate}",
            f"sampleAverage={average_count}",
            f"pulseWidth={pulse_width}"
        ]
        for cmd in commands:
            self._send_command(cmd.strip())
        time.sleep(1)  # Give Arduino time to apply settings

    def _send_command(self, command):
        """Send a single command to Arduino and process the response."""
        try:
            self.arduino.write((command + "\n").encode('utf-8'))
            time.sleep(0.05)  # Short delay to allow processing
            while self.arduino.in_waiting:
                response = self.arduino.readline().decode('utf-8', errors='ignore').strip()
                if response:
                    self.message_queue.put(response)
        except Exception as e:
            self.message_queue.put(f"Command error: {str(e)}")

    def _process_commands(self):
        """Process any commands from the command queue."""
        try:
            while True:
                cmd = self.command_queue.get_nowait()
                self._send_command(cmd)
        except queue.Empty:
            pass

    def _read_serial_data(self):
        """Read and process incoming serial data."""
        while self.arduino.in_waiting:
            try:
                raw_data = self.arduino.readline()
                line = raw_data.decode('utf-8', errors='ignore').strip()
                if not line:
                    continue

                # Split the incoming line using the comma as the separator.
                parts = line.split(',')
                if len(parts) == 2:
                    try:
                        timestamp = float(parts[0])
                        value = float(parts[1])
                        # Duplicate the sample into both queues.
                        self.csv_queue.put((timestamp, value))
                        self.plot_queue.put((timestamp, value))
                    except ValueError:
                        self.message_queue.put(f"Value parse error: {line}")
                else:
                    # If the line does not match the expected "timestamp,value" format,
                    # assume it's a non-numeric message.
                    self.message_queue.put(line)
            except Exception as e:
                self.message_queue.put(f"Serial read error: {str(e)}")
                break


    def stop(self):
        """Signal the thread to stop."""
        self.running.clear()


class CSVThread(Thread):
    """
    Thread for handling CSV file writing.
    It now reads from its dedicated queue so it doesn't interfere with the plot.
    """
    def __init__(self, csv_queue, filename='ppg_data.csv'):
        super().__init__()
        self.csv_queue = csv_queue
        self.filename = filename
        self.running = Event()
        self.running.set()
        self.start_time = time.time()

    def run(self):
        try:
            with open(self.filename, 'w', newline='') as csv_file:
                writer = csv.writer(csv_file)
                writer.writerow(["Time (s)", "PPG Value"])
                while self.running.is_set() or not self.csv_queue.empty():
                    try:
                        timestamp, value = self.csv_queue.get(timeout=0.1)
                        # Write the Arduino timestamp directly:
                        writer.writerow([timestamp, value])
                    except queue.Empty:
                        continue
        except Exception as e:
            print(f"CSV error: {str(e)}")


    def stop(self):
        """Signal the thread to stop."""
        self.running.clear()


class MessageThread(Thread):
    """
    Thread for handling non-numeric messages.
    """
    def __init__(self, message_queue):
        super().__init__()
        self.message_queue = message_queue
        self.running = Event()
        self.running.set()

    def run(self):
        while self.running.is_set() or not self.message_queue.empty():
            try:
                message = self.message_queue.get(timeout=0.1)
                print(f"[{time.strftime('%H:%M:%S')}] {message}")
            except queue.Empty:
                continue

    def stop(self):
        """Signal the thread to stop."""
        self.running.clear()


class MainWindow(pg.GraphicsLayoutWidget):
    def __init__(self, plot_queue):
        super().__init__(show=True, title="Live PPG Plot")
        self.plot_queue = plot_queue

        self.resize(800, 500)
        self.setWindowTitle('Live PPG Data Plotter (Press Q to Quit)')

        # Create plot area
        self.plot = self.addPlot(title="PPG Data")
        self.plot.setLabel('left', "PPG Signal", units='a.u.')
        self.plot.setLabel('bottom', "Time (s)")
        self.curve = self.plot.plot(pen='y')

        # Data buffers using deque for a fixed-length rolling window
        self.max_points = 1000  # Number of points to display
        self.data_buffer = deque(maxlen=self.max_points)
        self.x_buffer = deque(maxlen=self.max_points)

        # Store the first timestamp to create a relative time axis
        self.first_timestamp = None


    def keyPressEvent(self, event):
        """Handle key presses (Q to quit)."""
        if event.key() == QtCore.Qt.Key_Q:
            print("Q pressed, exiting...")
            QtWidgets.QApplication.quit()
        else:
            super().keyPressEvent(event)

    def update_plot(self):
        """Update the plot with new data from the plot queue."""
        while not self.plot_queue.empty():
            try:
                timestamp, value = self.plot_queue.get_nowait()
                # Initialize first_timestamp if not set.
                if self.first_timestamp is None:
                    self.first_timestamp = timestamp
                # Calculate relative time.
                rel_time = timestamp - self.first_timestamp
                self.data_buffer.append(value)
                self.x_buffer.append(rel_time)
            except queue.Empty:
                break
        self.curve.setData(np.array(self.x_buffer), np.array(self.data_buffer))



def main():
    # Create separate queues for CSV logging, plotting, messages, and commands.
    csv_queue = Queue()
    plot_queue = Queue()
    message_queue = Queue()
    command_queue = Queue()

    # Create and start threads.
    serial_thread = SerialThread(csv_queue, plot_queue, message_queue, command_queue)
    csv_thread = CSVThread(csv_queue)
    message_thread = MessageThread(message_queue)

    serial_thread.start()
    csv_thread.start()
    message_thread.start()

    # Set up GUI.
    app = QtWidgets.QApplication([])
    win = MainWindow(plot_queue)

    # Set up a timer to update the plot regularly.
    timer = QtCore.QTimer()
    timer.timeout.connect(win.update_plot)
    timer.start(50)  # Update every 50 ms

    print("Starting live plot. Press Q in the window to quit.")
    app.exec_()

    # Cleanup on exit.
    serial_thread.stop()
    csv_thread.stop()
    message_thread.stop()

    serial_thread.join()
    csv_thread.join()
    message_thread.join()

    print("All threads stopped and resources closed.")


if __name__ == '__main__':
    main()
