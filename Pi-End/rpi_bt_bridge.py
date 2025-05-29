#!/usr/bin/env python3
import asyncio, serial, json, time
from gpiozero import DigitalOutputDevice
from bumble.device import Device
from bumble.transport.hci_uart import HCI_UART_Transport
from bumble.rfcomm.profile import SerialPortProfile

# ---------- RS-485 side ----------------------------------------------------
ser485  = serial.Serial('/dev/serial0', 512000, timeout=0.02)
dir_pin = DigitalOutputDevice(17, active_high=True, initial_value=False)

# ---------- Bumble SPP server ---------------------------------------------
async def main():
    transport = HCI_UART_Transport('/dev/serial1')          # Pi’s BT HCI
    device    = Device.with_hci_transport(transport)
    device.name = 'Sploot Eddy'                  # Bluetooth device name
    spp       = SerialPortProfile('Doral Telemetry', server_channel=1) 
    device.add_profile(spp)
    await device.power_on()
    print('SPP advertised on channel 1; waiting for laptop…')

    async for sock in spp.accept():                         # blocks until connect
        print('Laptop connected:', sock.peer_address)
        asyncio.create_task(relay(sock))

async def relay(sock):
    buf = bytearray()
    try:
        while True:
            buf.extend(ser485.read(512))
            while b'\n' in buf:
                pkt, _, buf = buf.partition(b'\n')
                await sock.send(pkt + b'\n')                # RS-485 → laptop
    finally:
        await sock.close()

asyncio.run(main())
