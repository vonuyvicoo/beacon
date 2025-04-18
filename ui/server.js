const express = require('express');
const path = require('path');
const WebSocket = require('ws');

// If on Node 18+, you can use global fetch; otherwise install "node-fetch":
// const fetch = require('node-fetch');

const app = express();

// 1) Parse URL-encoded bodies
app.use(express.urlencoded({ extended: true }));

// 2) Serve static files from "public"
app.use(express.static(path.join(__dirname, 'public')));

// 3) POST /send: forward to 192.168.4.1/send, then broadcast locally
app.post('/send', (req, res) => {
  const { destid, message } = req.body;
  console.log(`Received POST: destid=${destid}, message=${message}`);

  // Forward to the ESP-like device at 192.168.4.1
  fetch('http://192.168.4.1/send', {
    method: 'POST',
    headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
    body: `destid=${encodeURIComponent(destid)}&message=${encodeURIComponent(message)}`
  })
    .then((response) => response.text())
    .then((responseText) => {
      console.log('Response from 192.168.4.1/send:', responseText);

      // Broadcast the message to local WebSocket clients
      //broadcast(message);

      res.sendStatus(200);
    })
    .catch((err) => {
      console.error('Error forwarding to 192.168.4.1/send:', err);
      res.sendStatus(500);
    });
});

// 4) Start HTTP server on port 3000
const server = app.listen(3000, () => {
  console.log('HTTP server running on http://localhost:3000');
});

// 5) Create a local WebSocket server on port 81
const wss = new WebSocket.Server({ port: 8081 }, () => {
  console.log('Local WebSocket server listening on ws://localhost:8081');
});

// 6) Broadcast helper
function broadcast(msg) {
  wss.clients.forEach((client) => {
    if (client.readyState === WebSocket.OPEN) {
      client.send(msg);
    }
  });
}

// 7) Handle local WebSocket connections
wss.on('connection', (ws) => {
  console.log('New local client connected.');

  // If you want to handle messages from local clients and forward them
  // to the remote device's WS, do it here:
  ws.on('message', (msg) => {
    console.log('Local client says:', msg);
    // e.g., forward to remoteWS if you want (see below)
    // if (remoteWS && remoteWS.readyState === WebSocket.OPEN) {
    //remoteWS.send(msg);
    // }
  });

  ws.on('close', () => {
    console.log('Local client disconnected.');
  });
});

// 8) Connect to the remote WebSocket at 192.168.4.1:81
const remoteWS = new WebSocket('ws://192.168.4.1:81');

// 9) Handle remote WS open/close
remoteWS.on('open', (r) => {
    console.log(111);
  console.log('Connected to remote WS at ws://192.168.4.1:81');
});

remoteWS.on('close', () => {
  console.log('Disconnected from remote WS');
});

// 10) When remote WS sends a message, broadcast to local clients
remoteWS.on('message', (data) => {
    const messageStr = data.toString('utf8');

    console.log('Received from remote WS:', messageStr);
  
    // If you're broadcasting it to local clients, send the string version
    broadcast(messageStr);
});
