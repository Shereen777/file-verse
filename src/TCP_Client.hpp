#ifndef TCP_CLIENT_HPP
#define TCP_CLIENT_HPP

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <nlohmann/json.hpp>
#include <chrono>

using json = nlohmann::json;
using namespace std;

class TCPClient {
private:
    int socket_fd;
    string host;
    int port;
    
    static const int BUFFER_SIZE = 65536;
    
public:
    TCPClient(const string& h, int p) : host(h), port(p), socket_fd(-1) {}
    
    ~TCPClient() {
        disconnect();
    }
    
    bool connect() {
        socket_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (socket_fd < 0) {
            cerr << "[ERROR] socket creation failed: " << strerror(errno) << "\n";
            return false;
        }
        
        struct sockaddr_in server_addr;
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        
        if (inet_pton(AF_INET, host.c_str(), &server_addr.sin_addr) <= 0) {
            cerr << "[ERROR] inet_pton failed: " << strerror(errno) << "\n";
            close(socket_fd);
            return false;
        }
        
        if (::connect(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            cerr << "[ERROR] connect failed: " << strerror(errno) << "\n";
            close(socket_fd);
            return false;
        }
        
        cout << "[CLIENT] Connected to " << host << ":" << port << "\n";
        return true;
    }
    
    void disconnect() {
        if (socket_fd >= 0) {
            close(socket_fd);
            socket_fd = -1;
        }
    }
    
    json send_request(const json& request) {
        if (socket_fd < 0) {
            cerr << "[ERROR] Not connected\n";
            return {
                {"status", "error"},
                {"error_message", "Not connected to server"}
            };
        }
        
        string request_str = request.dump();
        cout << "[SEND] " << request_str.substr(0, 100) << "...\n";
        
        if (send(socket_fd, request_str.c_str(), request_str.length(), 0) < 0) {
            cerr << "[ERROR] send failed: " << strerror(errno) << "\n";
            return {
                {"status", "error"},
                {"error_message", "Send failed"}
            };
        }
        
        char buffer[BUFFER_SIZE];
        memset(buffer, 0, BUFFER_SIZE);
        
        int bytes_received = recv(socket_fd, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received <= 0) {
            cerr << "[ERROR] recv failed: " << strerror(errno) << "\n";
            return {
                {"status", "error"},
                {"error_message", "Recv failed"}
            };
        }
        
        buffer[bytes_received] = '\0';
        
        try {
            json response = json::parse(buffer);
            cout << "[RECV] " << response.dump().substr(0, 100) << "...\n";
            return response;
        } catch (const json::exception& e) {
            cerr << "[ERROR] JSON parse failed: " << e.what() << "\n";
            return {
                {"status", "error"},
                {"error_message", string("JSON parse failed: ") + e.what()}
            };
        }
    }
    
    bool is_connected() const {
        return socket_fd >= 0;
    }
};

// ============ TEST MAIN ============

int run_client_tests() {
    TCPClient client("127.0.0.1", 9999);
    
    if (!client.connect()) {
        cerr << "[FATAL] Failed to connect to server\n";
        return 1;
    }
    
    cout << "\n" << string(70, '=') << "\n";
    cout << "  OMNIFS TCP Client - Testing Socket Communication\n";
    cout << string(70, '=') << "\n\n";
    
    // Test 1: Login
    cout << "[TEST 1] Admin Login\n";
    json login_req = {
        {"operation", "login"},
        {"parameters", {
            {"user_index", 1},
            {"password", "password123"}
        }},
        {"request_id", "REQ001"}
    };
    
    json login_resp = client.send_request(login_req);
    cout << "Response: " << login_resp.dump(2) << "\n\n";
    
    if (login_resp["status"] != "success") {
        cerr << "[FAILED] Login failed\n";
        return 1;
    }
    
    string admin_session = login_resp["data"]["session_id"];
    cout << "[SUCCESS] Admin session: " << admin_session << "\n\n";
    
    // Test 2: Create File
    cout << "[TEST 2] Create File\n";
    json create_req = {
        {"operation", "file_create"},
        {"parameters", {
            {"session_id", admin_session},
            {"path", "/test.txt"},
            {"data", "Hello from network!"}
        }},
        {"request_id", "REQ002"}
    };
    
    json create_resp = client.send_request(create_req);
    cout << "Response: " << create_resp.dump(2) << "\n\n";
    
    if (create_resp["status"] != "success") {
        cerr << "[FAILED] File creation failed\n";
    } else {
        cout << "[SUCCESS] File created\n\n";
    }
    
    // Test 3: Read File
    cout << "[TEST 3] Read File\n";
    json read_req = {
        {"operation", "file_read"},
        {"parameters", {
            {"session_id", admin_session},
            {"path", "/test.txt"}
        }},
        {"request_id", "REQ003"}
    };
    
    json read_resp = client.send_request(read_req);
    cout << "Response: " << read_resp.dump(2) << "\n\n";
    
    if (read_resp["status"] == "success") {
        cout << "[SUCCESS] File content: " << read_resp["data"]["content"] << "\n\n";
    }
    
    // Test 4: Get Stats
    cout << "[TEST 4] Get Filesystem Stats\n";
    json stats_req = {
        {"operation", "get_stats"},
        {"parameters", {
            {"session_id", admin_session}
        }},
        {"request_id", "REQ004"}
    };
    
    json stats_resp = client.send_request(stats_req);
    cout << "Response: " << stats_resp.dump(2) << "\n\n";
    
    // Test 5: Invalid Path (should fail)
    cout << "[TEST 5] Invalid Path (expect error)\n";
    json invalid_req = {
        {"operation", "file_read"},
        {"parameters", {
            {"session_id", admin_session},
            {"path", ""}
        }},
        {"request_id", "REQ005"}
    };
    
    json invalid_resp = client.send_request(invalid_req);
    cout << "Response: " << invalid_resp.dump(2) << "\n\n";
    
    if (invalid_resp["status"] == "error") {
        cout << "[SUCCESS] Correctly rejected invalid path\n\n";
    }
    
    // Test 6: Logout
    cout << "[TEST 6] Logout\n";
    json logout_req = {
        {"operation", "logout"},
        {"parameters", {
            {"session_id", admin_session}
        }},
        {"request_id", "REQ006"}
    };
    
    json logout_resp = client.send_request(logout_req);
    cout << "Response: " << logout_resp.dump(2) << "\n\n";
    
    cout << string(70, '=') << "\n";
    cout << "  All tests completed!\n";
    cout << string(70, '=') << "\n\n";
    
    return 0;
}

#endif