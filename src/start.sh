#!/bin/bash

# OmniFS Complete Startup Script
# This starts all necessary servers for the web interface

echo "=========================================="
echo "Starting OmniFS System"
echo "=========================================="

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check if C++ server exists
if [ ! -f "./main" ]; then
    echo -e "${RED}✗ C++ server not found!${NC}"
    echo "Please compile first: g++ -o server server.cpp file_system.cpp -lpthread"
    exit 1
fi

# Check if Node.js is installed
if ! command -v node &> /dev/null; then
    echo -e "${RED}✗ Node.js not found!${NC}"
    echo "Please install Node.js from https://nodejs.org"
    exit 1
fi

# Check if npm dependencies are installed
if [ ! -d "node_modules" ]; then
    echo -e "${YELLOW}Installing npm dependencies...${NC}"
    npm install
fi

# Kill any existing processes on our ports
echo "Checking for existing processes..."
lsof -ti:8080 | xargs kill -9 2>/dev/null
lsof -ti:8081 | xargs kill -9 2>/dev/null
lsof -ti:3000 | xargs kill -9 2>/dev/null

echo ""

# Start C++ server in background
echo -e "${GREEN}Starting C++ Server (port 8080)...${NC}"
./main 8080 > server.log 2>&1 &
SERVER_PID=$!
sleep 1

# Check if server started
if ! ps -p $SERVER_PID > /dev/null; then
    echo -e "${RED}✗ Failed to start C++ server${NC}"
    cat server.log
    exit 1
fi
echo -e "${GREEN}✓ C++ Server running (PID: $SERVER_PID)${NC}"

# Start WebSocket proxy in background
echo -e "${GREEN}Starting WebSocket Proxy (port 8081)...${NC}"
node proxy-server.js > proxy.log 2>&1 &
PROXY_PID=$!
sleep 1

# Check if proxy started
if ! ps -p $PROXY_PID > /dev/null; then
    echo -e "${RED}✗ Failed to start proxy server${NC}"
    cat proxy.log
    kill $SERVER_PID 2>/dev/null
    exit 1
fi
echo -e "${GREEN}✓ Proxy Server running (PID: $PROXY_PID)${NC}"

# Start HTTP server for web interface
echo -e "${GREEN}Starting Web Server (port 3000)...${NC}"
if command -v python3 &> /dev/null; then
    python3 -m http.server 3000 > web.log 2>&1 &
    HTTP_PID=$!
elif command -v python &> /dev/null; then
    python -m http.server 3000 > web.log 2>&1 &
    HTTP_PID=$!
else
    echo -e "${YELLOW}Python not found, trying npx http-server...${NC}"
    npx http-server -p 3000 > web.log 2>&1 &
    HTTP_PID=$!
fi

sleep 1

if ! ps -p $HTTP_PID > /dev/null; then
    echo -e "${RED}✗ Failed to start web server${NC}"
    kill $SERVER_PID $PROXY_PID 2>/dev/null
    exit 1
fi
echo -e "${GREEN}✓ Web Server running (PID: $HTTP_PID)${NC}"

echo ""
echo "=========================================="
echo -e "${GREEN}✓ All servers started successfully!${NC}"
echo "=========================================="
echo "C++ Server:    http://localhost:8080"
echo "Proxy Server:  ws://localhost:8081"
echo "Web Interface: http://localhost:3000"
echo ""
echo "Open your browser and navigate to:"
echo -e "${YELLOW}http://localhost:3000/index.html${NC}"
echo ""
echo "=========================================="
echo "Logs are being written to:"
echo "  - server.log (C++ server)"
echo "  - proxy.log (WebSocket proxy)"
echo "  - web.log (HTTP server)"
echo ""
echo -e "Press ${RED}Ctrl+C${NC} to stop all servers"
echo "=========================================="

# Cleanup function
cleanup() {
    echo ""
    echo "Shutting down all servers..."
    kill $SERVER_PID $PROXY_PID $HTTP_PID 2>/dev/null
    echo "All servers stopped."
    exit 0
}

# Trap Ctrl+C
trap cleanup SIGINT SIGTERM

# Wait for all processes
wait