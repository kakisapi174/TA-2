import serial
import numpy as np
import matplotlib.pyplot as plt
from collections import deque
import threading
import time

# ================= CONFIG =================
PORT = "COM5"          # <<< CHANGE THIS
BAUD = 230400
FS = 10000             # SpikerBox sample rate
WINDOW_SEC = 2.0
MIN_WINDOW = 0.2
MAX_WINDOW = 5.0

DISPLAY_SIZE = int(FS * WINDOW_SEC)
buffer = deque(maxlen=DISPLAY_SIZE)
paused = False

# ================= SERIAL =================
ser = serial.Serial(PORT, BAUD, timeout=0)

input_buffer = []
cBufTail = 0

# ================= FRAME DECODER =================
def handle_data(data):
    global cBufTail
    input_buffer.extend(data)

    while cBufTail + 2 < len(input_buffer):
        MSB = input_buffer[cBufTail] & 0xFF

        if MSB > 127:
            MSB &= 0x7F
            LSB = input_buffer[cBufTail + 1] & 0x7F
            value = (MSB << 7) | LSB
            buffer.append(value - 512)
            cBufTail += 2
        else:
            cBufTail += 1

    if cBufTail > 1000:
        del input_buffer[:cBufTail]
        cBufTail = 0

# ================= SERIAL THREAD =================
def serial_thread():
    while True:
        data = ser.read(1024)
        if data:
            handle_data(list(data))
        time.sleep(0.001)

threading.Thread(target=serial_thread, daemon=True).start()

# ================= PLOT =================
plt.ion()
fig, ax = plt.subplots()

x = np.linspace(-WINDOW_SEC, 0, DISPLAY_SIZE)
line, = ax.plot(x, np.zeros(DISPLAY_SIZE))

ax.set_ylim(-550, 550)
ax.set_xlim(-WINDOW_SEC, 0)
ax.set_title("Human SpikerBox – USB Real-Time Signal")
ax.set_xlabel("Time (s)")
ax.set_ylabel("Amplitude (ADC counts)")

# ================= KEY HANDLER =================
def on_key(event):
    global paused, WINDOW_SEC, DISPLAY_SIZE, buffer, x

    if event.key == ' ':
        paused = not paused
        return

    if event.key == 'up':
        WINDOW_SEC = min(WINDOW_SEC * 1.25, MAX_WINDOW)

    elif event.key == 'down':
        WINDOW_SEC = max(WINDOW_SEC / 1.25, MIN_WINDOW)

    DISPLAY_SIZE = int(FS * WINDOW_SEC)
    buffer = deque(buffer, maxlen=DISPLAY_SIZE)

    x = np.linspace(-WINDOW_SEC, 0, DISPLAY_SIZE)
    line.set_xdata(x)
    ax.set_xlim(-WINDOW_SEC, 0)

fig.canvas.mpl_connect('key_press_event', on_key)

# ================= MAIN LOOP =================
while True:
    if len(buffer) == DISPLAY_SIZE and not paused:
        line.set_ydata(np.array(buffer))
        fig.canvas.draw()
        fig.canvas.flush_events()

    time.sleep(0.01)
