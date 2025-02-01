#include <SPI.h>
#include <RH_RF95.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>

// Previous pin and settings definitions remain the same
#define RFM95_CS 5
#define RFM95_RST 14
#define RFM95_INT 25
#define RF95_FREQ 915.0
#define NODE_ID 4
#define MAX_HOPS 5

const char* ssid = "LoRaNode_4";
const char* password = "lora1234";

RH_RF95 rf95(RFM95_CS, RFM95_INT);
WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

// Enhanced message structure
struct Message {
  String content;
  bool isAcknowledged;
  String timestamp;
  String type;  // "sent", "received", or "status"
};

Message messageLog[10];
int messageIndex = 0;

// Setup function remains largely the same
void setup() {
  // Previous setup code remains the same...
  Serial.begin(115200);
  while (!Serial);

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
  
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  server.on("/", handleRoot);
  server.on("/send", HTTP_POST, handleSend);
  server.begin();

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void handleRoot() {
  String html = R"html(
    <!DOCTYPE html>
    <html>
    <head>
        <title>LoRa Node Control</title>
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <style>
            body {
                font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
                margin: 0;
                padding: 20px;
                background-color: #f5f5f5;
                display: flex;
                flex-direction: column;
                align-items: center;
            }
            .container {
                max-width: 800px;
                width: 100%;
                background: white;
                padding: 20px;
                border-radius: 10px;
                box-shadow: 0 2px 4px rgba(0,0,0,0.1);
            }
            h1 {
                color: #333;
                text-align: center;
                margin-bottom: 30px;
            }
            .form-group {
                margin: 20px 0;
                text-align: center;
            }
            input {
                width: 100%;
                max-width: 300px;
                padding: 10px;
                margin: 8px 0;
                border: 1px solid #ddd;
                border-radius: 4px;
                box-sizing: border-box;
            }
            input[type="submit"] {
                background-color: #4CAF50;
                color: white;
                padding: 12px 20px;
                border: none;
                border-radius: 4px;
                cursor: pointer;
                font-size: 16px;
                transition: background-color 0.3s;
            }
            input[type="submit"]:hover {
                background-color: #45a049;
            }
            .message-log {
                border: 1px solid #ddd;
                padding: 15px;
                margin: 20px 0;
                height: 400px;
                overflow-y: auto;
                border-radius: 4px;
                background: #fff;
            }
            .message {
                padding: 10px;
                margin: 5px 0;
                border-radius: 4px;
                animation: fadeIn 0.5s;
            }
            .message.sent {
                background: #e3f2fd;
                margin-left: 20%;
                border-left: 4px solid #2196F3;
            }
            .message.received {
                background: #f5f5f5;
                margin-right: 20%;
                border-left: 4px solid #4CAF50;
            }
            .message.status {
                background: #fff3e0;
                text-align: center;
                font-style: italic;
                font-size: 0.9em;
            }
            .timestamp {
                font-size: 0.8em;
                color: #666;
                margin-top: 5px;
            }
            .acknowledged {
                color: #4CAF50;
                font-size: 0.8em;
                margin-top: 5px;
            }
            @keyframes fadeIn {
                from { opacity: 0; transform: translateY(10px); }
                to { opacity: 1; transform: translateY(0); }
            }
        </style>
        <script>
            var ws = new WebSocket('ws://' + window.location.hostname + ':81/');
            
            ws.onmessage = function(event) {
                var messageData = JSON.parse(event.data);
                var log = document.getElementById('messageLog');
                var messageDiv = document.createElement('div');
                messageDiv.className = 'message ' + messageData.type;
                
                var content = messageData.content;
                if (messageData.isAcknowledged) {
                    content += '<div class="acknowledged">✓ Delivered</div>';
                }
                content += '<div class="timestamp">' + messageData.timestamp + '</div>';
                
                messageDiv.innerHTML = content;
                log.insertBefore(messageDiv, log.firstChild);
            };
            
            function sendMessage() {
                var destid = document.getElementById('destid').value;
                var message = document.getElementById('message').value;
                if (!destid || !message) {
                    alert('Please fill in all fields');
                    return false;
                }
                var xhr = new XMLHttpRequest();
                xhr.open('POST', '/send', true);
                xhr.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
                xhr.send('destid=' + destid + '&message=' + message);
                document.getElementById('message').value = '';
                return false;
            }
        </script>
    </head>
    <body>
        <div class="container">
            <h1>LoRa Node )html" + String(NODE_ID) + R"html(</h1>
            <div class="form-group">
                <form onsubmit="return sendMessage();">
                    <input type="number" id="destid" placeholder="Destination Node ID" required><br>
                    <input type="text" id="message" placeholder="Enter your message" required><br>
                    <input type="submit" value="Send Message">
                </form>
            </div>
            <div id="messageLog" class="message-log">
            </div>
        </div>
    </body>
    </html>
  )html";

  server.send(200, "text/html", html);
}

void addToMessageLog(String content, bool isAcknowledged, String type) {
  String timestamp = String(millis() / 1000) + "s";  // Simple timestamp
  messageLog[messageIndex] = {content, isAcknowledged, timestamp, type};
  
  // Create JSON for WebSocket
  String json = "{\"content\":\"" + content + "\",\"isAcknowledged\":" + 
                (isAcknowledged ? "true" : "false") + ",\"timestamp\":\"" + 
                timestamp + "\",\"type\":\"" + type + "\"}";
  
  webSocket.broadcastTXT(json);
  messageIndex = (messageIndex + 1) % 10;
}

// Modified message handling in loop()
void loop() {
  server.handleClient();
  webSocket.loop();

  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    handleUserInput(input);
  }

  if (rf95.available()) {
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    if (rf95.recv(buf, &len)) {
      buf[len] = '\0';

      int sourceID, destID, hopCount;
      char payload[50];
      sscanf((char *)buf, "[%d][%d][%d][%49[^]]]", &sourceID, &destID, &hopCount, payload);

      if (destID == NODE_ID || destID == 999) {
        String receivedMsg = String(payload);
        
        if (receivedMsg == "Ack") {
          // Update the sent message status
          String statusMsg = "Message delivered to Node " + String(sourceID);
          addToMessageLog(statusMsg, true, "status");
        } else if (receivedMsg == "PING") {
          addToMessageLog("Ping received from Node " + String(sourceID), false, "status");
          char ackMessage[50];
          snprintf(ackMessage, sizeof(ackMessage), "[%d][%d][%d][%s]", NODE_ID, sourceID, 0, "Online");
          rf95.send((uint8_t *)ackMessage, strlen(ackMessage));
          rf95.waitPacketSent();
        } else {
          addToMessageLog("From Node " + String(sourceID) + ": " + receivedMsg, false, "received");
          // Send acknowledgment
          char ackMessage[50];
          snprintf(ackMessage, sizeof(ackMessage), "[%d][%d][%d][%s]", NODE_ID, sourceID, 0, "Ack");
          rf95.send((uint8_t *)ackMessage, strlen(ackMessage));
          rf95.waitPacketSent();
        }
      }
    }
  }
}

void handleSend() {
  if (server.hasArg("destid") && server.hasArg("message")) {
    int destID = server.arg("destid").toInt();
    String message = server.arg("message");
    
    char loraMessage[50];
    snprintf(loraMessage, sizeof(loraMessage), "[%d][%d][%d][%s]", NODE_ID, destID, 0, message.c_str());
    
    rf95.send((uint8_t *)loraMessage, strlen(loraMessage));
    rf95.waitPacketSent();
    
    addToMessageLog("To Node " + String(destID) + ": " + message, false, "sent");
    server.send(200, "text/plain", "Message sent");
  }
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED:
      Serial.printf("[%u] Connected!\n", num);
      // Send existing messages to newly connected client
      for (int i = 0; i < 10; i++) {
        if (messageLog[i].content != "") {
          String json = "{\"content\":\"" + messageLog[i].content + 
                       "\",\"isAcknowledged\":" + (messageLog[i].isAcknowledged ? "true" : "false") + 
                       ",\"timestamp\":\"" + messageLog[i].timestamp + 
                       "\",\"type\":\"" + messageLog[i].type + "\"}";
          webSocket.sendTXT(num, json);
        }
      }
      break;
  }
}

void handleUserInput(const String &input) {
  if (input == "!ping") {
    char pingMessage[50];
    snprintf(pingMessage, sizeof(pingMessage), "[%d][%d][%d][%s]", NODE_ID, 999, 0, "PING");
    Serial.println("Pinging all nodes...");
    rf95.send((uint8_t *)pingMessage, strlen(pingMessage));
    rf95.waitPacketSent();
    addToMessageLog("Pinging all nodes...", false, "status");
  } else {
    int delimiterIndex = input.indexOf(':');
    if (delimiterIndex != -1) {
      int destID = input.substring(0, delimiterIndex).toInt();
      String userMessage = input.substring(delimiterIndex + 1);
      char message[50];
      snprintf(message, sizeof(message), "[%d][%d][%d][%s]", NODE_ID, destID, 0, userMessage.c_str());
      rf95.send((uint8_t *)message, strlen(message));
      rf95.waitPacketSent();
      addToMessageLog("To Node " + String(destID) + ": " + userMessage, false, "sent");
    }
  }
}