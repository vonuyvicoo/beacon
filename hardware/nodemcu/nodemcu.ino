#include <SPI.h>
#include <RH_RF95.h>

// Updated pins for ESP8266 NodeMCU3
#define RFM95_CS D8       // Chip Select (CS -> GPIO15)
#define RFM95_RST D2      // Reset (RST -> GPIO0)
#define RFM95_INT D4      // Interrupt (G0 -> GPIO2)

// LoRa settings
#define RF95_FREQ 915.0   // LoRa frequency
#define NODE_ID 2         // This node's ID

RH_RF95 rf95(RFM95_CS, RFM95_INT);

void setup() {
  Serial.begin(115200);
  while (!Serial);

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
  // Check for incoming serial data
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    
    // Parse destination ID and message
    int commaIndex = input.indexOf(':');
    if (commaIndex != -1) {
      int destID = input.substring(0, commaIndex).toInt();
      String userMessage = input.substring(commaIndex + 1);
      
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

  // Check for incoming LoRa messages
  if (rf95.available()) {
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    if (rf95.recv(buf, &len)) {
      buf[len] = '\0';  // Null-terminate for string handling

      // Parse the message
      int sourceID, destID;
      char payload[50];
      sscanf((char *)buf, "[%d][%d][%49[^]]]", &sourceID, &destID, payload);

      // Check if the message is for this node
      if (destID == NODE_ID) {
        Serial.print("Message from Node ");
        Serial.print(sourceID);
        Serial.print(": ");
        Serial.println(payload);
      } else {
        Serial.println("Message not for this node.");
      }
    }
  }
}
