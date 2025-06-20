<img src="https://i.postimg.cc/NFPQmPB0/Mask-group.png" style="width: 50%;" />

# Project BEACON  
A Decentralized, Multi‑Layered Approach for the Convergence and Optimization of Hybrid LoRa–Web Communication Networks

![License](https://img.shields.io/badge/License-MIT-yellow.svg)
![Static Badge](https://img.shields.io/badge/NodeJS-green)
![Static Badge](https://img.shields.io/badge/LoRa-red)
![Static Badge](https://img.shields.io/badge/ESP32-blue)
![MacOS](https://img.shields.io/badge/MacOS-green)
![Windows](https://img.shields.io/badge/Windows-green)

## Table of Contents
- [Quick Start](#quick-start)
- [Overview](#overview)
- [Core Concepts](#core-concepts)
- [System Architecture](#system-architecture)
- [Hardware Components](#hardware-components)
- [Software Stack](#software-stack)
- [Communication Protocols](#communication-protocols)
- [Use Cases](#use-cases)
- [Development Guide](#development-guide)
- [Technical Specifications](#technical-specifications)

## Quick Start

Boot up the beacon device first by connecting it via a Micro‑USB cable. Once powered on, the device will generate its own Wi‑Fi access point—connect your computer or mobile device to that AP.

Next, in your local development environment:

```bash
cd ui
npm install
npm start
```

Then open your browser to http://localhost:3000. The web app will connect to the beacon's AP and communicate over a REST API.

## Overview

Project BEACON is an innovative hybrid communication system that bridges the gap between LoRa (Long Range) radio networks and modern web applications. It creates a decentralized, resilient communication infrastructure that enables web browsers and HTTP-capable devices to seamlessly interact with LoRa-enabled IoT devices and sensor networks.

The system operates as a protocol translator and communication hub, allowing standard web technologies to leverage the long-range, low-power capabilities of LoRa radio while maintaining familiar HTTP/REST interfaces for developers.

## Core Concepts

### Hybrid Communication Architecture
BEACON implements a multi-layered communication stack that combines:
- **LoRa Radio Layer**: Long-range (up to 15km), low-power wireless communication at 915MHz
- **WiFi Access Point Layer**: Local network connectivity for web clients using ESP32's built-in WiFi
- **HTTP/REST API Layer**: Standard web protocols for application integration via Express.js server
- **WebSocket Layer**: Real-time bidirectional communication for live message updates

### Decentralized Network Topology
Unlike traditional IoT systems that rely on cloud infrastructure, BEACON creates a **mesh-capable, edge-first architecture**:
- Each beacon node operates independently without internet dependency
- Nodes can relay messages through multi-hop routing (up to 5 hops configured)
- Self-healing network topology adapts to node failures
- Local processing reduces latency and improves reliability
- Store-and-forward messaging ensures delivery even when destinations are temporarily unreachable

```
    LoRa Node 1 ────┐
                    │
Web Client ← → BEACON Node ← → LoRa Node 2
                    │
    LoRa Node 3 ────┘
```

### Protocol Translation Engine
The system acts as an intelligent protocol translator:
- **Web → LoRa**: Converts HTTP POST requests to structured LoRa radio packets
- **LoRa → Web**: Translates incoming radio messages to JSON format for web consumption
- **Message Routing**: Implements store-and-forward messaging with acknowledgments
- **Format Adaptation**: Handles different payload formats and encoding schemes between protocols

### Message Flow Architecture
1. **HTTP Request**: Web client sends message via REST API to Node.js server
2. **Protocol Translation**: Node.js server forwards request to ESP32 beacon via WiFi
3. **LoRa Transmission**: ESP32 encodes message and broadcasts over 915MHz LoRa radio
4. **Multi-hop Routing**: Message propagates through LoRa mesh network with hop counting
5. **Acknowledgment**: Confirmation flows back through the communication chain
6. **Real-time Updates**: WebSocket connection pushes delivery status to web clients

## System Architecture

```
┌─────────────────┐    ┌──────────────────┐    ┌─────────────────┐
│   Web Client    │    │   BEACON Node    │    │  Remote LoRa    │
│  (Browser/App)  │    │   (ESP32 + UI)   │    │     Devices     │
├─────────────────┤    ├──────────────────┤    ├─────────────────┤
│ • HTML/CSS/JS   │    │ • ESP32 MCU      │    │ • Sensor Nodes  │
│ • WebSocket     │    │ • RFM95 LoRa     │    │ • Actuators     │
│ • REST Client   │    │ • WiFi AP        │    │ • IoT Devices   │
└─────────────────┘    └──────────────────┘    └─────────────────┘
         │                       │                       │
         │ HTTP/WebSocket        │ LoRa Radio           │
         │ (192.168.4.1)        │ (915MHz)             │
    ┌────▼────┐              ┌───▼───┐              ┌────▼────┐
    │ Node.js │              │ESP32  │              │ Remote  │
    │ Server  │◄──────WiFi──►│Bridge │◄──915MHz────►│ LoRa    │
    │ (3000)  │              │       │              │ Network │
    └─────────┘              └───────┘              └─────────┘
```

### Communication Layers

**Layer 1: Physical**
- LoRa radio at 915MHz (North America) with chirp spread spectrum modulation
- WiFi 802.11 b/g/n for local connectivity
- USB serial for programming and debugging

**Layer 2: Data Link**
- LoRa packet structure with error detection and correction
- WiFi frame management
- Message acknowledgment and retry logic

**Layer 3: Network**
- Multi-hop routing with destination-based forwarding
- Node ID addressing (configurable 1-255)
- Hop count limiting to prevent infinite loops

**Layer 4: Transport**
- Reliable message delivery with acknowledgments
- Message queuing and store-and-forward
- Duplicate detection and sequence numbering

**Layer 5: Application**
- HTTP REST API for web integration
- WebSocket for real-time notifications
- JSON message formatting

## Hardware Components

### ESP32 Microcontroller (Primary MCU)
- **Processing Power**: Dual-core Xtensa 240MHz with hardware floating point
- **Memory**: 520KB SRAM, 4MB Flash storage, external SPI flash support
- **Connectivity**: Built-in WiFi 802.11 b/g/n and Bluetooth 4.2/BLE
- **GPIO**: 34 programmable GPIO pins for sensor/actuator expansion
- **Power**: 3.3V operation with multiple sleep modes for battery operation

### LoRa Transceiver Module (RFM95/SX1276)
- **Frequency**: 915MHz (North America) / 868MHz (Europe) / 433MHz (Asia)
- **Range**: Up to 15km line-of-sight, 2-5km in urban environments
- **Power**: 10-100mW configurable transmit power (20dBm max)
- **Sensitivity**: -148dBm receive sensitivity
- **Modulation**: Chirp Spread Spectrum (CSS) for interference resistance
- **Data Rate**: 0.3-37.5 kbps (configurable based on spreading factor)

### Pin Configuration
```cpp
// LoRa module connections to ESP32
#define RFM95_CS   5   // SPI Chip Select (GPIO5)
#define RFM95_RST  14  // Reset Pin (GPIO14)
#define RFM95_INT  25  // Interrupt Pin (GPIO25)

// SPI bus connections (default ESP32 SPI pins)
// MOSI: GPIO23, MISO: GPIO19, SCK: GPIO18
```

### Power Requirements
- **ESP32**: 80mA active, 5µA deep sleep
- **RFM95**: 120mA transmit (20dBm), 10mA receive, 0.2µA sleep
- **Total System**: ~200mA active operation, suitable for battery/solar power

## Software Stack

### Firmware Layer (`hardware/esp/esp.ino`)
**Arduino-based ESP32 firmware providing core functionality:**

**Core Responsibilities:**
- LoRa radio initialization and packet handling using RadioHead library
- WiFi Access Point creation and management (SSID: "LoRaNode_4")
- HTTP server hosting REST API endpoints and diagnostic web interface
- WebSocket server for real-time bidirectional communication
- Message routing table management and store-and-forward logic
- JSON message serialization and deserialization

**Key Features:**
```cpp
// Message structure with routing information
struct Message {
  String content;          // Actual message payload
  bool isAcknowledged;     // Delivery confirmation status
  String timestamp;        // Message creation time
  String type;            // "sent", "received", or "status"
  int senderID;           // Source node identifier
  int receiverID;         // Destination node identifier
};

// Network configuration
#define NODE_ID 4           // This node's unique identifier
#define MAX_HOPS 5          // Maximum routing hops
#define RF95_FREQ 915.0     // Operating frequency in MHz
```

**REST API Endpoints:**
- `GET /` - Diagnostic web interface with network status
- `POST /send` - Send message to specific node ID
- WebSocket on port 81 for real-time message notifications

### Backend Layer (`ui/server.js`)
**Node.js Express server acting as web application host and protocol bridge:**

**Core Responsibilities:**
- HTTP server hosting the chat web application on port 3000
- Proxy/bridge between web clients and ESP32 beacon device
- WebSocket server management for real-time message updates
- Request validation, formatting, and error handling
- CORS handling for cross-origin requests

**Technology Stack:**
- **Express.js 4.21.2**: Fast, unopinionated web framework for Node.js
- **WebSocket (ws 8.18.0)**: WebSocket library for real-time communication
- **node-fetch 3.3.2**: HTTP client for forwarding requests to ESP32

**API Flow:**
```javascript
// 1. Receive POST from web client
app.post('/send', (req, res) => {
  const { destid, message } = req.body;
  
  // 2. Forward to ESP32 device
  fetch('http://192.168.4.1/send', {
    method: 'POST',
    headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
    body: `destid=${destid}&message=${message}`
  })
  // 3. Handle response and update clients
});
```

### Frontend Layer (`ui/public/index.html`)
**Modern chat-style web interface for message composition and monitoring:**

**Core Responsibilities:**
- Responsive chat interface optimized for mobile and desktop
- Real-time message display with sent/received status indicators
- WebSocket client for live communication with backend
- Message composition with destination node selection
- Connection status monitoring and error handling

**Key Features:**
- **Chat Interface**: WhatsApp-style message bubbles with timestamps
- **Real-time Updates**: WebSocket connection for instant message delivery
- **Responsive Design**: Mobile-first CSS with desktop optimization
- **Status Indicators**: Visual feedback for message delivery status
- **Error Handling**: Network disconnection detection and retry logic

```javascript
// WebSocket connection for real-time updates
const ws = new WebSocket('ws://localhost:8081');
ws.onmessage = (event) => {
  const message = JSON.parse(event.data);
  displayMessage(message.content, message.type);
};
```

## Communication Protocols

### LoRa Message Format
The system uses a structured packet format for reliable LoRa communication:

```cpp
// LoRa packet structure (max 255 bytes)
struct LoRaPacket {
  uint8_t senderID;        // Source node identifier (1-255)
  uint8_t receiverID;      // Destination node identifier (0=broadcast)
  uint8_t messageID;       // Unique message identifier for acknowledgment
  uint8_t hopCount;        // Number of hops traversed (prevents loops)
  uint8_t messageType;     // DATA, ACK, ROUTE_REQ, ROUTE_REPLY
  uint8_t payloadLength;   // Length of actual message content
  char payload[240];       // UTF-8 encoded message content
  uint16_t checksum;       // CRC16 for error detection
} __attribute__((packed));
```

### Message Types
- **DATA (0x01)**: Standard user message
- **ACK (0x02)**: Acknowledgment of received message
- **BROADCAST (0x03)**: Network-wide announcement
- **ROUTE_REQ (0x04)**: Route discovery request
- **ROUTE_REPLY (0x05)**: Route discovery response
- **KEEPALIVE (0x06)**: Node presence announcement

### HTTP API Specification

**Send Message Endpoint:**
```http
POST /send HTTP/1.1
Host: localhost:3000
Content-Type: application/x-www-form-urlencoded

destid=3&message=Hello%20from%20web%20client
```

**Response Codes:**
- `200 OK`: Message queued successfully
- `400 Bad Request`: Invalid destination ID or message format
- `500 Internal Server Error`: LoRa transmission failed
- `503 Service Unavailable`: Beacon device not reachable

### WebSocket Event Format
Real-time events are transmitted as JSON over WebSocket:

```javascript
// Incoming message from LoRa network
{
  "type": "message_received",
  "sender": 3,
  "receiver": 4,
  "content": "Sensor reading: 23.5°C",
  "timestamp": "2025-06-20T15:30:45Z",
  "rssi": -67,
  "snr": 8.5
}

// Message delivery confirmation
{
  "type": "message_ack",
  "messageId": "msg_20250620_153045_001",
  "status": "delivered",
  "hops": 2,
  "timestamp": "2025-06-20T15:30:48Z"
}

// Network status update
{
  "type": "network_status",
  "nodesOnline": 5,
  "lastSeen": {
    "1": "2025-06-20T15:29:12Z",
    "2": "2025-06-20T15:30:01Z",
    "3": "2025-06-20T15:30:45Z"
  }
}
```

## Use Cases

### 1. Emergency Communication Networks
**Scenario**: Natural disaster disables cellular and internet infrastructure, emergency responders need reliable communication.

**BEACON Solution**:
- Deploy battery-powered beacon nodes at strategic locations
- Create mesh network covering disaster zone
- Emergency workers use web interface on tablets/phones
- Messages automatically route through available nodes
- Works completely offline without external infrastructure

**Benefits**: 
- Self-healing network adapts to node failures
- Long-range coverage (15km+ between nodes)
- Battery operation for extended deployments
- Familiar web interface requires no specialized training

### 2. Agricultural IoT Monitoring
**Scenario**: Large farm with sensors monitoring soil moisture, weather, and crop conditions across thousands of acres.

**BEACON Solution**:
- LoRa sensor nodes throughout fields report to beacon gateways
- Farm management web app displays real-time sensor data
- Low-power sensors operate for years on battery power
- Multi-hop routing extends coverage across entire farm

**Benefits**:
- Ultra-low power consumption extends battery life
- Wide area coverage without cellular infrastructure costs
- Easy integration with existing farm management software
- Scalable architecture supports hundreds of sensors

### 3. Smart City Infrastructure
**Scenario**: Municipal sensors for air quality, traffic flow, and utility monitoring across urban environment.

**BEACON Solution**:
- Distributed beacon nodes create citywide IoT backbone
- Various sensors connect via LoRa to nearest beacon
- City dashboard aggregates data from all sensor networks
- Redundant routing ensures reliable data collection

**Benefits**:
- Reduced infrastructure costs compared to cellular IoT
- Multiple communication paths increase reliability
- Real-time monitoring enables rapid response to issues
- Open architecture supports diverse sensor types

### 4. Industrial Asset Tracking
**Scenario**: Manufacturing facility needs to track equipment, inventory, and personnel across large industrial site.

**BEACON Solution**:
- LoRa asset tags on equipment and inventory
- Personnel carry LoRa-enabled safety devices
- Beacon nodes throughout facility create tracking network
- Web dashboard shows real-time location and status

**Benefits**:
- Long battery life reduces maintenance overhead
- LoRa penetrates industrial environments better than WiFi
- Scalable to thousands of tracked assets
- Integration with existing enterprise systems via REST API

### 5. Research and Education
**Scenario**: University IoT lab teaching mesh networking and protocol design concepts.

**BEACON Solution**:
- Hands-on BEACON testbed for experimenting with:
  - Mesh routing algorithms
  - Protocol optimization
  - Network resilience testing
  - Cross-layer communication design

**Benefits**:
- Visual web interface makes abstract concepts tangible
- Configurable parameters allow experimentation
- Real-world applicable technology
- Open source enables modification and extension

## Development Guide

### Setting Up Development Environment

**Arduino IDE Configuration:**
```bash
# Install ESP32 board support
# File → Preferences → Additional Board Manager URLs:
# https://dl.espressif.com/dl/package_esp32_index.json

# Install required libraries:
# Tools → Manage Libraries
# - RadioHead by Mike McCauley
# - ArduinoJson by Benoit Blanchon
# - WebSockets by Markus Sattler
```

**Node.js Development:**
```bash
# Install Node.js dependencies
cd ui
npm install

# Start development server with auto-reload
npm run dev  # if available, or npm start

# Optional: Install nodemon for auto-restart
npm install -g nodemon
nodemon server.js
```

### Adding New LoRa Nodes

**1. Configure Node Parameters:**
```cpp
// In hardware/esp/esp.ino
#define NODE_ID 5              // Unique identifier (1-255)
#define RF95_FREQ 915.0        // Match network frequency
const char* ssid = "LoRaNode_5"; // Unique WiFi SSID
```

**2. Implement Custom Message Handlers:**
```cpp
void processCustomMessage(String payload, int senderID) {
  if (payload.startsWith("SENSOR:")) {
    // Handle sensor data
    float value = payload.substring(7).toFloat();
    storeSensorReading(senderID, value);
  }
  // Add more custom protocols as needed
}
```

**3. Update Routing Logic:**
```cpp
// Add node to known nodes list
struct KnownNode {
  int nodeID;
  unsigned long lastSeen;
  int rssi;
  int hopCount;
};

KnownNode knownNodes[MAX_NODES];
```

### Extending the Web Interface

**Adding New API Endpoints:**
```javascript
// In ui/server.js
app.get('/api/network-status', (req, res) => {
  // Query beacon device for network information
  fetch('http://192.168.4.1/network-status')
    .then(response => response.json())
    .then(data => res.json(data))
    .catch(err => res.status(500).json({error: err.message}));
});
```

**Real-time Data Visualization:**
```html
<!-- Add to ui/public/index.html -->
<div id="network-graph">
  <canvas id="topology-canvas"></canvas>
</div>

<script>
function drawNetworkTopology(nodes, connections) {
  const canvas = document.getElementById('topology-canvas');
  const ctx = canvas.getContext('2d');
  
  // Draw nodes and connections
  nodes.forEach(node => {
    ctx.beginPath();
    ctx.arc(node.x, node.y, 20, 0, 2 * Math.PI);
    ctx.fillStyle = node.online ? '#4CAF50' : '#F44336';
    ctx.fill();
  });
}
</script>
```

### Custom Message Types and Protocols

**Define Application-Specific Messages:**
```cpp
// Custom message types for specific applications
#define MSG_SENSOR_DATA     0x10
#define MSG_ACTUATOR_CMD    0x20
#define MSG_STATUS_REPORT   0x30
#define MSG_CONFIG_UPDATE   0x40

// Message processing in main loop
void processReceivedMessage(uint8_t* packet, int length) {
  uint8_t messageType = packet[4];
  
  switch(messageType) {
    case MSG_SENSOR_DATA:
      processSensorData(&packet[6], packet[5]); // payload, length
      break;
    case MSG_ACTUATOR_CMD:
      executeActuatorCommand(&packet[6], packet[5]);
      break;
    // Add more handlers...
  }
}
```

**JSON Message Formatting:**
```javascript
// Structured message format for web interface
const message = {
  type: 'sensor_reading',
  nodeId: 3,
  timestamp: new Date().toISOString(),
  data: {
    temperature: 23.5,
    humidity: 67.2,
    battery: 3.7
  },
  location: {
    lat: 40.7128,
    lon: -74.0060
  }
};
```

### Network Optimization Strategies

**Frequency Planning:**
```cpp
// Use different frequencies for different node clusters
// to reduce interference and increase capacity
#define CLUSTER_A_FREQ  915.0  // Primary network
#define CLUSTER_B_FREQ  915.5  // Secondary network
#define EMERGENCY_FREQ  916.0  // Emergency communications
```

**Adaptive Power Management:**
```cpp
// Adjust transmit power based on distance and RSSI
void adaptTransmitPower(int targetRSSI, int currentRSSI) {
  if (currentRSSI < targetRSSI - 10) {
    // Increase power if signal too weak
    rf95.setTxPower(min(20, rf95.getTxPower() + 2));
  } else if (currentRSSI > targetRSSI + 10) {
    // Decrease power if signal too strong (save battery)
    rf95.setTxPower(max(5, rf95.getTxPower() - 2));
  }
}
```

**Intelligent Routing:**
```cpp
// Route selection based on hop count and signal quality
struct Route {
  int destinationID;
  int nextHopID;
  int hopCount;
  int rssi;
  unsigned long lastUsed;
};

int selectBestRoute(int destinationID) {
  Route* bestRoute = nullptr;
  int bestScore = -1000;
  
  for (int i = 0; i < routeTableSize; i++) {
    if (routeTable[i].destinationID == destinationID) {
      // Score based on hop count and signal quality
      int score = routeTable[i].rssi - (routeTable[i].hopCount * 20);
      if (score > bestScore) {
        bestScore = score;
        bestRoute = &routeTable[i];
      }
    }
  }
  
  return bestRoute ? bestRoute->nextHopID : -1;
}
```

### Testing and Debugging

**Serial Monitor Debugging:**
```cpp
// Enable detailed logging for development
#define DEBUG_LEVEL 2  // 0=none, 1=basic, 2=verbose

void debugPrint(int level, String message) {
  if (DEBUG_LEVEL >= level) {
    Serial.print("[");
    Serial.print(millis());
    Serial.print("] ");
    Serial.println(message);
  }
}
```

**Network Analysis Tools:**
```javascript
// Web-based network analyzer
function analyzeNetworkPerformance() {
  const stats = {
    messagesPerMinute: messageCount / upTimeMinutes,
    averageRSSI: totalRSSI / messageCount,
    deliveryRate: acknowledgedMessages / sentMessages,
    averageHops: totalHops / deliveredMessages
  };
  
  console.log('Network Performance:', stats);
  return stats;
}
```

## Technical Specifications

### RF Characteristics
| Parameter | Specification | Notes |
|-----------|---------------|-------|
| **Frequency** | 915MHz (US), 868MHz (EU) | ISM band, license-free |
| **Modulation** | LoRa CSS | Chirp Spread Spectrum |
| **Bandwidth** | 125/250/500 kHz | Configurable |
| **Spreading Factor** | 7-12 | Higher SF = longer range, lower data rate |
| **Coding Rate** | 4/5, 4/6, 4/7, 4/8 | Forward error correction |
| **Output Power** | 2-20 dBm | Up to 100mW |
| **Sensitivity** | -148 dBm | At SF12, 125kHz BW |
| **Range** | 2-15 km | Depending on environment and settings |

### Network Capacity
| Parameter | Value | Limitations |
|-----------|-------|-------------|
| **Max Nodes** | 255 | Limited by 8-bit node ID |
| **Max Hops** | 5 | Configurable, prevents loops |
| **Message Size** | 240 bytes | LoRa payload limitation |
| **Data Rate** | 0.3-37.5 kbps | Depends on SF and BW |
| **Duty Cycle** | <1% (EU), <36s/hour (US) | Regulatory requirements |

### Power Consumption
| Component | Active | Sleep | Notes |
|-----------|--------|-------|-------|
| **ESP32** | 80 mA | 5 µA | WiFi active/deep sleep |
| **RFM95 TX** | 120 mA | 0.2 µA | At 20dBm output |
| **RFM95 RX** | 10 mA | 0.2 µA | Receive mode/sleep |
| **Total System** | ~200 mA | ~5 µA | Typical operation |

### Performance Benchmarks
- **Message Latency**: 100-500ms (local network)
- **Multi-hop Latency**: +200ms per hop
- **Throughput**: 1-10 messages/minute (sustainable)
- **Battery Life**: 7-30 days (2000mAh, depending on usage)
- **Concurrent Users**: 10-50 (web interface)

## Troubleshooting

### Common Issues

**1. LoRa Communication Failure**
```cpp
// Check SPI connections and power supply
if (!rf95.init()) {
  Serial.println("Check wiring: CS, RST, INT pins");
  Serial.println("Verify 3.3V power supply");
}
```

**2. WiFi AP Not Visible**
```cpp
// Ensure unique SSID for each node
const char* ssid = "LoRaNode_X";  // X = unique number
WiFi.softAP(ssid, password);
```

**3. Web Interface Connection Issues**
```bash
# Check IP addresses
# ESP32 AP: 192.168.4.1
# Node.js server: localhost:3000
# Ensure client connected to correct WiFi network
```

**4. Message Delivery Problems**
- Verify node IDs are unique and within range (1-255)
- Check antenna connections and orientation
- Monitor for interference on 915MHz band
- Ensure nodes are within communication range

### Performance Optimization

**Reduce Power Consumption:**
```cpp
// Implement sleep modes for battery operation
void enterDeepSleep(int seconds) {
  esp_sleep_enable_timer_wakeup(seconds * 1000000);
  esp_deep_sleep_start();
}
```

**Improve Range:**
```cpp
// Optimize LoRa parameters for maximum range
rf95.setModemConfig(RH_RF95::Bw125Cr48Sf4096);  // SF12, slowest but longest range
rf95.setTxPower(20, false);  // Maximum power
```

**Increase Throughput:**
```cpp
// Optimize for higher data rates
rf95.setModemConfig(RH_RF95::Bw500Cr45Sf128);  // SF7, fastest
rf95.setTxPower(14, false);  // Moderate power
```

## License

MIT License

Copyright (c) 2025 Project BEACON

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

## Contributing

We welcome contributions to Project BEACON! Please follow these guidelines:

### Development Workflow
1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Make your changes with appropriate tests
4. Commit with descriptive messages (`git commit -m 'Add amazing feature'`)
5. Push to your branch (`git push origin feature/amazing-feature`)
6. Open a Pull Request with detailed description

### Code Style
- **Arduino/C++**: Follow Arduino coding style guide
- **JavaScript**: Use ES6+ features, async/await for promises
- **Documentation**: Update README and inline comments for new features

### Testing
- Test hardware changes with multiple ESP32/LoRa modules
- Verify web interface across different browsers
- Test network scenarios (single hop, multi-hop, node failures)

## Support and Community

### Getting Help
- **Issues**: Report bugs and request features via GitHub Issues
- **Discussions**: Join community discussions for questions and ideas
- **Documentation**: Check this README and inline code comments

### Roadmap
- [ ] Mobile app for iOS/Android
- [ ] Enhanced mesh routing algorithms
- [ ] GPS integration for location-based services
- [ ] Encryption and security features
- [ ] Integration with popular IoT platforms
- [ ] Advanced network monitoring and analytics

---

**Project BEACON** - Bridging the gap between LoRa and the Web, one message at a time.
