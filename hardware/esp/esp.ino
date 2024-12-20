#include <SPI.h>
#include <RH_RF95.h>
#include "BluetoothSerial.h"

// Updated pins for ESP32
#define RFM95_CS 5        // Chip Select
#define RFM95_RST 14      // Reset
#define RFM95_INT 25      // Interrupt (G0)

// LoRa settings
#define RF95_FREQ 915.0   // LoRa frequency
#define NODE_ID 3         // This node's ID

RH_RF95 rf95(RFM95_CS, RFM95_INT);
BluetoothSerial SerialBT; // Create a BluetoothSerial object

void setup() {
  Serial.begin(115200);
  while (!Serial);

  // Initialize Bluetooth
  if (!SerialBT.begin("Node3LoRa")) { // You can rename "Node3LoRa" to any name you want.
    Serial.println("Bluetooth init failed");
    while (1);
  }
  Serial.println("Bluetooth initialized. Device name: Node3LoRa");

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
    SerialBT.println("LoRa init failed");
    while (1);
  }

  rf95.setFrequency(RF95_FREQ);
  
  Serial.println("LoRa Node Initialized.");
  SerialBT.println("LoRa Node Initialized.");
  Serial.println("Enter messages in format: <destination_id>:<message>");
  SerialBT.println("Enter messages in format: <destination_id>:<message>");
  Serial.println("Example: 1:Hello Node 1!");
  SerialBT.println("Example: 1:Hello Node 1!");
}

void loop() {
  // Check for incoming serial data (from USB)
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    handleUserInput(input);
  }

  // Check for incoming Bluetooth data
  if (SerialBT.available()) {
    String inputBT = SerialBT.readStringUntil('\n');
    handleUserInput(inputBT);
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
        if (SerialBT.hasClient()) {
          SerialBT.println(receivedMsg);
        }
      } else {
        Serial.println("Message not for this node.");
        if (SerialBT.hasClient()) {
          SerialBT.println("Message not for this node.");
        }
      }
    }
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
    if (SerialBT.hasClient()) {
      SerialBT.print("Sending: ");
      SerialBT.println(message);
    }

    rf95.send((uint8_t *)message, strlen(message));
    rf95.waitPacketSent();
  } else {
    Serial.println("Invalid format. Use: <destination_id>:<message>");
    if (SerialBT.hasClient()) {
      SerialBT.println("Invalid format. Use: <destination_id>:<message>");
    }
  }
}
