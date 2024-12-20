import asyncio
from bleak import BleakScanner

async def scan_ble_devices():
    devices = await BleakScanner.discover()
    for device in devices:
        print(f"Name: {device.name}, Address: {device.address}, UUIDs: {device.metadata.get('uuids')}")

asyncio.run(scan_ble_devices())
