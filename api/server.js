const express = require('express');
const bodyParser = require('body-parser');

const app = express();
const port = 7798;
const middleware_endpoint = 'http://localhost:5345/send';

// Middleware to parse JSON body
app.use(bodyParser.json());

// POST endpoint
app.post('/send', (req, res) => {
    const { node_ID, message } = req.body;

    // forward message to http://localhost:5353/send POST
    fetch(middleware_endpoint, {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({ node_ID, message })
    })
        .then(response => response.json())
        .then(data => {
            res.json(data);
        })
        .catch(error => {
            console.error('Error:', error);
            res.status(500).json({ error: 'Internal Server Error' });
        });
});

// Start server
app.listen(port, () => {
    console.log(`Server running at http://localhost:${port}`);
});
