import express from 'express';
import path from 'path';
import WebSocket from 'ws';

const wss = new WebSocket.Server({ port: 8080 });

wss.on('connection', (ws) => {
    console.log('New client connected');

    ws.on('message', (message) => {

        console.log(message.toString());

        const data = JSON.parse(message.toString());
        if (data.type === 'offer') {
            // Forward offer to the other peer
            wss.clients.forEach(client => {
                if (client !== ws && client.readyState === WebSocket.OPEN) {
                    client.send(JSON.stringify(data));
                }
            });
        } else if (data.type === 'answer') {
            // Forward answer to the offerer
            wss.clients.forEach(client => {
                if (client !== ws && client.readyState === WebSocket.OPEN) {
                    client.send(JSON.stringify(data));
                }
            });
        } else if (data.type === 'candidate') {
            // Forward ICE candidate to the other peer
            wss.clients.forEach(client => {
                if (client !== ws && client.readyState === WebSocket.OPEN) {
                    client.send(JSON.stringify(data));
                }
            });
        }
    });

    ws.on('close', () => {
        console.log('Client disconnected');
    });
});

console.log('Signaling server running on ws://192.168.0.213:8080');

const app = express();
const port = 3000;

// Serve static files from the "public" directory
app.use(express.static(path.join(__dirname, '../public')));

app.get('/', (req, res) => {
    res.sendFile(path.join(__dirname, '../public/index.html'));
});

app.listen(port, () => {
    console.log(`Server is running on http://localhost:${port}`);
});