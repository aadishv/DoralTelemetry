import json
import time
import threading 
import serial
import bluetooth
from gpiozero import DigitalOutputDevice

# -------- RS485 Side ------------------------------------------
UART = "/dev/serial0" # Serial port for RS485
BAUD = 512_000 # Baudrate for RS485
ser485 = serial.Serial(UART, BAUD, timeout=0.02)

#DIR pin : low = receive, high = transmit
dir_pin = DigitalOutputDevice(17, active_high=True, initial_value=False)

# -------- Bluetooth SPP Server Side ------------------------
UUID = "00001101-0000-1000-8000-00805F9B34FB" # SPP UUID
server = bluetooth.BluetoothSocket(bluetooth.RFCOMM)
server.bind(("", bluetooth.PORT_ANY))
server.listen(1)
port = server.getsockname()[1]

bluetooth.advertise_service(
    server, "Sploot Eddy",
    service_id=UUID,
    service_classes=[UUID, bluetooth.SERIAL_PORT_CLASS],
    profiles=[bluetooth.SERIAL_PORT_PROFILE]
)

print(f"waiting on RFCOMM channel {port}...")
client, info = server.accept()
print("Laptop Connected:", info[0])

# -------- RS485 to Bluetooth Relay Thread -------------------

def rs485_to_bt():
    buf = bytearray()
    while True:
        buf.extend(ser485.read(512))
        while b"\n" in buf:
            pkt, _, buf = buf.partition(b"\n")
            try:
                client.send(pkt + b"\n")
            except OSError:
                return
            
threading.Thread(target=rs485_to_bt, daemon=True).start()

