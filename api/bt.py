import asyncio
from bleak import BleakClient
from aiohttp import web

# Define your UUIDs
SERVICE_UUID = "12345678-1234-1234-1234-123456789abc"
CHARACTERISTIC_UUID_RX = "12345678-1234-1234-1234-123456789abd"
CHARACTERISTIC_UUID_TX = "12345678-1234-1234-1234-123456789abe"

# Define the BLE device address (replace with your device's address)
DEVICE_ADDRESS = "94:E6:86:3B:9C:F6"  # Replace with your BLE device's address

# Global variables to store the BLE client
ble_client = None

# Notification handler
def notification_handler(sender, data):
    print(f"Received data from {sender}: {data.decode()}")

# Function to initialize the BLE client
async def initialize_ble_client():
    global ble_client
    ble_client = BleakClient(DEVICE_ADDRESS, timeout=60.0)
    await ble_client.connect()

    if ble_client.is_connected:
        print(f"Connected to {DEVICE_ADDRESS}")

        # Start receiving notifications
        await ble_client.start_notify(CHARACTERISTIC_UUID_TX, notification_handler)
        print("Subscribed to notifications on TX characteristic")
    else:
        print("Failed to connect to device")
        ble_client = None

# Function to handle HTTP POST requests
async def handle_post(request):
    global ble_client

    if not ble_client or not ble_client.is_connected:
        return web.json_response({"error": "BLE client is not connected"}, status=500)

    try:
        data = await request.json()
        node_id = data.get("node_ID")
        message = data.get("message")

        if not node_id or not message:
            return web.json_response({"error": "Missing 'node_ID' or 'message' parameters"}, status=400)

        # Construct and send the message
        full_message = f"{node_id}:{message}"
        await ble_client.write_gatt_char(CHARACTERISTIC_UUID_RX, full_message.encode())
        print(f"Sent message: {full_message}")

        return web.json_response({"status": "success", "message": "Message sent successfully"})
    except Exception as e:
        print(f"Error handling POST request: {e}")
        return web.json_response({"error": str(e)}, status=500)

# Function to stop the BLE client
async def cleanup(app):
    global ble_client
    if ble_client and ble_client.is_connected:
        await ble_client.stop_notify(CHARACTERISTIC_UUID_TX)
        await ble_client.disconnect()
        print("Disconnected BLE client")

# Main function to run the server
async def main():
    # Initialize BLE client
    await initialize_ble_client()

    # Set up the HTTP server
    app = web.Application()
    app.router.add_post("/send", handle_post)
    app.on_cleanup.append(cleanup)

    print("Starting HTTP server on port 5345...")
    runner = web.AppRunner(app)
    await runner.setup()
    site = web.TCPSite(runner, "0.0.0.0", 5345)
    await site.start()

    # Keep the server running
    try:
        while True:
            await asyncio.sleep(3600)  # Keep the event loop running
    except KeyboardInterrupt:
        print("Shutting down server...")

# Run the main function
asyncio.run(main())
