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
#define MAX_HOPS 5        // Maximum number of hops a message can take

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
  Serial.println("Special Commands:");
  Serial.println("  !ping               - Ping all nodes");
}

void loop() {
  // Check for incoming serial data
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

      int sourceID, destID, hopCount;
      char payload[50];
      sscanf((char *)buf, "[%d][%d][%d][%49[^]]]", &sourceID, &destID, &hopCount, payload);

      // Check if the message is for this node
      if (destID == NODE_ID) {
        String receivedMsg = "Message from Node " + String(sourceID) + ": " + String(payload);
        Serial.println(receivedMsg);

        if (deviceConnected) {
          txCharacteristic->setValue(receivedMsg.c_str());
          txCharacteristic->notify();
        }

        // Handle special messages (ping, ack)
        if (String(payload) == "PING") {
          Serial.println("Ping received. Sending acknowledgment...");
          char ackMessage[50];
          snprintf(ackMessage, sizeof(ackMessage), "[%d][%d][%d][%s]", NODE_ID, sourceID, 0, "Online");
          rf95.send((uint8_t *)ackMessage, strlen(ackMessage));
          rf95.waitPacketSent();
        } else if (String(payload) == "Ack") {
          Serial.println("Acknowledgment received from Node " + String(sourceID));
        } else {
          // Send acknowledgment for regular messages
          char ackMessage[50];
          snprintf(ackMessage, sizeof(ackMessage), "[%d][%d][%d][%s]", NODE_ID, sourceID, 0, "Ack");
          rf95.send((uint8_t *)ackMessage, strlen(ackMessage));
          rf95.waitPacketSent();
        }
      } else if (hopCount < MAX_HOPS) {
        // Forward the message
        hopCount++;
        char message[50];
        snprintf(message, sizeof(message), "[%d][%d][%d][%s]", sourceID, destID, hopCount, payload);

        Serial.print("Forwarding message to Node ");
        Serial.print(destID);
        Serial.print(" with hop count ");
        Serial.println(hopCount);

        rf95.send((uint8_t *)message, strlen(message));
        rf95.waitPacketSent();
      } else {
        Serial.println("Message dropped. Maximum hop count reached.");
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
  if (input == "!ping") {
    // Send a ping message to all nodes
    char pingMessage[50];
    snprintf(pingMessage, sizeof(pingMessage), "[%d][%d][%d][%s]", NODE_ID, 999, 0, "PING");
    Serial.println("Pinging all nodes...");

    rf95.send((uint8_t *)pingMessage, strlen(pingMessage));
    rf95.waitPacketSent();
  } else {
    int delimiterIndex = input.indexOf(':');
    if (delimiterIndex != -1) {
      int destID = input.substring(0, delimiterIndex).toInt();
      String userMessage = input.substring(delimiterIndex + 1);

      // Format the complete message
      char message[50];
      snprintf(message, sizeof(message), "[%d][%d][%d][%s]", NODE_ID, destID, 0, userMessage.c_str());

      Serial.print("Sending: ");
      Serial.println(message);

      rf95.send((uint8_t *)message, strlen(message));
      rf95.waitPacketSent();
    } else {
      Serial.println("Invalid format. Use: <destination_id>:<message>");
    }
  }
}