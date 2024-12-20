#include <SPI.h>
#include <RH_RF95.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// Updated pins for ESP32
#define RFM95_CS 5        // Chip Select
#define RFM95_RST 14      // Reset
#define RFM95_INT 25      // Interrupt (G0)

// LoRa settings
#define RF95_FREQ 915.0   // LoRa frequency
#define NODE_ID 3         // This node's ID

RH_RF95 rf95(RFM95_CS, RFM95_INT);

// BLE settings
#define SERVICE_UUID "12345678-1234-1234-1234-123456789abc"
#define CHARACTERISTIC_UUID_RX "12345678-1234-1234-1234-123456789abd"
#define CHARACTERISTIC_UUID_TX "12345678-1234-1234-1234-123456789abe"

BLECharacteristic *txCharacteristic;
bool deviceConnected = false;
String rxValue = "";

// Callback for BLE Server
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }
};

// Callback for BLE Characteristic
class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    rxValue = pCharacteristic->getValue(); // Adjusted to use String
    if (rxValue.length() > 0) {
      Serial.println("Received over BLE: " + rxValue);
    }
  }
};

void setup() {
  Serial.begin(115200);
  while (!Serial);

  // Initialize BLE
  BLEDevice::init("Node3LoRa");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  txCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_TX,
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
  txCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic *rxCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID_RX,
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  rxCharacteristic->setCallbacks(new MyCallbacks());

  pService->start();
  pServer->getAdvertising()->start();
  Serial.println("BLE initialized. Device name: Node3LoRa");

  // Initialize LoRa module
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  if (!rf95.init()) {
    Serial.println("LoRa init failed");
    while (1);
  }

  rf95.setFrequency(RF95_FREQ);
  Serial.println("LoRa Node Initialized.");
  Serial.println("Enter messages in format: <destination_id>:<message>");
  Serial.println("Example: 1:Hello Node 1!");
}

void loop() {
  // check serial
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    handleUserInput(input);
  }
  // Check for incoming LoRa messages
  if (rf95.available()) {
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    if (rf95.recv(buf, &len)) {
      buf[len] = '\0';  // Null-terminate for string handling

      int sourceID, destID;
      char payload[50];
      sscanf((char *)buf, "[%d][%d][%49[^]]]", &sourceID, &destID, payload);

      // Check if the message is for this node
      if (destID == NODE_ID) {
        String receivedMsg = "Message from Node " + String(sourceID) + ": " + String(payload);
        Serial.println(receivedMsg);

        if (deviceConnected) {
          txCharacteristic->setValue(receivedMsg.c_str());
          txCharacteristic->notify();
        }
      } else {
        Serial.println("Message not for this node.");
      }
    }
  }

  // Process received BLE commands
  if (rxValue.length() > 0) {
    handleUserInput(rxValue);
    rxValue = "";
  }
}

void handleUserInput(const String &input) {
  int delimiterIndex = input.indexOf(':');
  if (delimiterIndex != -1) {
    int destID = input.substring(0, delimiterIndex).toInt();
    String userMessage = input.substring(delimiterIndex + 1);

    // Format the complete message
    char message[50];
    snprintf(message, sizeof(message), "[%d][%d][%s]", NODE_ID, destID, userMessage.c_str());

    Serial.print("Sending: ");
    Serial.println(message);

    rf95.send((uint8_t *)message, strlen(message));
    rf95.waitPacketSent();
  } else {
    Serial.println("Invalid format. Use: <destination_id>:<message>");
  }
}
