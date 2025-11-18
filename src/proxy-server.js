// proxy-server.js
// WebSocket to TCP Proxy for OmniFS
// This bridges browser WebSocket connections to your C++ TCP server

const WebSocket = require('ws');
const net = require('net');

// Configuration
const WS_PORT = 8081;           // WebSocket port for browser
const TCP_HOST = '127.0.0.1';   // Your C++ server address
const TCP_PORT = 8080;          // Your C++ server port

// Create WebSocket server
const wss = new WebSocket.Server({ port: WS_PORT });

console.log('========================================');
console.log('OmniFS WebSocket Proxy Server');
console.log('========================================');
console.log(`WebSocket: ws://localhost:${WS_PORT}`);
console.log(`TCP Target: ${TCP_HOST}:${TCP_PORT}`);
console.log('========================================\n');

// Handle WebSocket connections from browser
wss.on('connection', (ws, req) => {
    const clientIP = req.socket.remoteAddress;
    console.log(`[${new Date().toISOString()}] Browser connected from ${clientIP}`);
    
    // Handle messages from browser
    ws.on('message', (message) => {
        const msgStr = message.toString();
        console.log(`\n[WS→TCP] Received from browser (${msgStr.length} bytes)`);
        console.log(`Request: ${msgStr.substring(0, 100)}${msgStr.length > 100 ? '...' : ''}`);
        
        // Create TCP connection to C++ server
        const tcpClient = net.createConnection({ 
            host: TCP_HOST, 
            port: TCP_PORT 
        }, () => {
            console.log(`[TCP] Connected to C++ server at ${TCP_HOST}:${TCP_PORT}`);
            
            // Send the JSON request to C++ server
            tcpClient.write(msgStr);
            console.log(`[TCP→C++] Sent request to C++ server`);
        });
        
        // Receive response from C++ server
        tcpClient.on('data', (data) => {
            const response = data.toString();
            console.log(`[C++→TCP] Received from C++ server (${response.length} bytes)`);
            console.log(`Response: ${response.substring(0, 100)}${response.length > 100 ? '...' : ''}`);
            
            // Send response back to browser via WebSocket
            if (ws.readyState === WebSocket.OPEN) {
                ws.send(response);
                console.log(`[TCP→WS] Forwarded response to browser`);
            }
            
            // Close TCP connection
            tcpClient.end();
        });
        
        // Handle TCP errors
        tcpClient.on('error', (err) => {
            console.error(`[TCP ERROR] ${err.message}`);
            
            // Send error back to browser
            if (ws.readyState === WebSocket.OPEN) {
                ws.send(JSON.stringify({
                    status: 'error',
                    error_message: `Cannot connect to C++ server: ${err.message}`,
                    error_code: -1
                }));
            }
        });
        
        tcpClient.on('end', () => {
            console.log(`[TCP] Connection to C++ server closed\n`);
        });
    });
    
    // Handle browser disconnect
    ws.on('close', () => {
        console.log(`[${new Date().toISOString()}] Browser disconnected\n`);
    });
    
    // Handle WebSocket errors
    ws.on('error', (err) => {
        console.error(`[WS ERROR] ${err.message}`);
    });
});

// Handle proxy server errors
wss.on('error', (err) => {
    console.error(`[SERVER ERROR] ${err.message}`);
    if (err.code === 'EADDRINUSE') {
        console.error(`\nPort ${WS_PORT} is already in use!`);
        console.error('Please close the other application or change WS_PORT in this file.\n');
        process.exit(1);
    }
});

console.log('✓ Proxy server is running...');
console.log('✓ Waiting for browser connections...');
console.log('\nPress Ctrl+C to stop\n');