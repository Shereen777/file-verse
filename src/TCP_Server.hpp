#ifndef TCP_SERVER_HPP
#define TCP_SERVER_HPP

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <mutex>
#include <queue>
#include <cstring>
#include <iostream>
#include <nlohmann/json.hpp>
#include "Socket_Handler.hpp"

using json = nlohmann::json;
using namespace std;

class TCPServer {
private:
    int server_socket;
    int port;
    bool running;
    OMNIInstance* instance;
    SocketHandler* handler;
    mutex client_mutex;
    
    static const int BUFFER_SIZE = 65536; // 64KB buffer for large requests
    static const int MAX_CLIENTS = 50;
    
    // Handle individual client connection
    void handle_client(int client_socket, struct sockaddr_in client_addr) {
        char buffer[BUFFER_SIZE];
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        
        cout << "[CLIENT] Connected from " << client_ip << ":" << ntohs(client_addr.sin_port) << "\n";
        
        while (running) {
            memset(buffer, 0, BUFFER_SIZE);
            int bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
            
            // Connection closed or error
            if (bytes_received <= 0) {
                if (bytes_received < 0) {
                    cerr << "[ERROR] recv failed: " << strerror(errno) << "\n";
                }
                break;
            }
            
            buffer[bytes_received] = '\0';
            
            // Parse incoming JSON request
            json response;
            try {
                json request = json::parse(buffer);
                
                cout << "[REQUEST] Op: " << request.value("operation", "unknown") 
                     << " | ReqID: " << request.value("request_id", "unknown") << "\n";
                
                // Process request through handler
                response = handler->process_request(request);
                
            } catch (const json::exception& e) {
                cerr << "[PARSE_ERROR] " << e.what() << "\n";
                response = {
                    {"status", "error"},
                    {"operation", "unknown"},
                    {"request_id", "invalid"},
                    {"error_code", static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION)},
                    {"error_message", string("JSON parse error: ") + e.what()}
                };
            } catch (const exception& e) {
                cerr << "[HANDLER_ERROR] " << e.what() << "\n";
                response = {
                    {"status", "error"},
                    {"operation", "unknown"},
                    {"request_id", "invalid"},
                    {"error_code", static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION)},
                    {"error_message", string("Handler error: ") + e.what()}
                };
            }
            
            // Send JSON response
            string response_str = response.dump();
            if (send(client_socket, response_str.c_str(), response_str.length(), 0) < 0) {
                cerr << "[ERROR] send failed: " << strerror(errno) << "\n";
                break;
            }
            
            cout << "[RESPONSE] Status: " << response.value("status", "unknown") << "\n";
        }
        
        close(client_socket);
        cout << "[CLIENT] Disconnected from " << client_ip << "\n";
    }
    
    // Accept connections in separate thread
    void accept_loop() {
        cout << "[SERVER] Listening for connections on port " << port << "...\n";
        
        while (running) {
            struct sockaddr_in client_addr;
            socklen_t addr_len = sizeof(client_addr);
            
            int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &addr_len);
            
            if (client_socket < 0) {
                if (running) {
                    cerr << "[ERROR] accept failed: " << strerror(errno) << "\n";
                }
                continue;
            }
            
            // Set socket timeout (10 seconds)
            struct timeval tv;
            tv.tv_sec = 10;
            tv.tv_usec = 0;
            setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
            
            // Handle client in separate thread
            thread client_thread(&TCPServer::handle_client, this, client_socket, client_addr);
            client_thread.detach();
        }
    }
    
public:
    TCPServer(int p, OMNIInstance* inst) 
        : port(p), instance(inst), server_socket(-1), running(false) {
        handler = new SocketHandler(instance);
    }
    
    ~TCPServer() {
        stop();
        if (handler) delete handler;
    }
    
    bool start() {
        // Create socket
        server_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (server_socket < 0) {
            cerr << "[ERROR] socket creation failed: " << strerror(errno) << "\n";
            return false;
        }
        
        // Allow address reuse
        int opt = 1;
        if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt)) < 0) {
            cerr << "[ERROR] setsockopt failed: " << strerror(errno) << "\n";
            close(server_socket);
            return false;
        }
        
        // Bind socket
        struct sockaddr_in server_addr;
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        server_addr.sin_port = htons(port);
        
        if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            cerr << "[ERROR] bind failed: " << strerror(errno) << "\n";
            close(server_socket);
            return false;
        }
        
        // Listen
        if (listen(server_socket, MAX_CLIENTS) < 0) {
            cerr << "[ERROR] listen failed: " << strerror(errno) << "\n";
            close(server_socket);
            return false;
        }
        
        running = true;
        cout << "[SERVER] Started on port " << port << "\n";
        
        // Accept connections in background thread
        thread accept_thread(&TCPServer::accept_loop, this);
        accept_thread.detach();
        
        return true;
    }
    
    void stop() {
        running = false;
        if (server_socket >= 0) {
            close(server_socket);
            server_socket = -1;
        }
        cout << "[SERVER] Stopped\n";
    }
    
    bool is_running() const {
        return running;
    }
};

#endif