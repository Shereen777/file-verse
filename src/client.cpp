#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sstream>

using namespace std;

class FSClient {
private:
    string server_host;
    int server_port;
    string session_id;
    int request_counter;
    
    string send_request(const string& request) {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            return "{\"status\":\"error\",\"error_message\":\"Socket creation failed\"}";
        }
        
        sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(server_port);
        
        if (inet_pton(AF_INET, server_host.c_str(), &server_addr.sin_addr) <= 0) {
            close(sock);
            return "{\"status\":\"error\",\"error_message\":\"Invalid address\"}";
        }
        
        if (connect(sock, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            close(sock);
            return "{\"status\":\"error\",\"error_message\":\"Connection failed\"}";
        }
        
        send(sock, request.c_str(), request.length(), 0);
        
        char buffer[8192] = {0};
        int bytes_read = read(sock, buffer, sizeof(buffer) - 1);
        close(sock);
        
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            return string(buffer);
        }
        
        return "{\"status\":\"error\",\"error_message\":\"No response\"}";
    }
    
    string get_json_value(const string& json, const string& key) {
        size_t pos = json.find("\"" + key + "\"");
        if (pos == string::npos) return "";
        
        pos = json.find(":", pos);
        if (pos == string::npos) return "";
        
        pos = json.find("\"", pos);
        if (pos == string::npos) return "";
        
        size_t end = json.find("\"", pos + 1);
        if (end == string::npos) return "";
        
        return json.substr(pos + 1, end - pos - 1);
    }
    
    string generate_request_id() {
        return "REQ_" + to_string(time(nullptr)) + "_" + to_string(request_counter++);
    }
    
public:
    FSClient(const string& host = "127.0.0.1", int port = 8080) 
        : server_host(host), server_port(port), request_counter(0) {}
    
    bool init(const string& config_path = "omnifs.conf", const string& omni_path = "omnifs.dat") {
        string request = "{\"operation\":\"init\",\"session_id\":\"\",\"request_id\":\"" + 
                        generate_request_id() + "\",\"parameters\":{\"config_path\":\"" + 
                        config_path + "\",\"omni_path\":\"" + omni_path + "\"}}";
        
        string response = send_request(request);
        return response.find("\"status\":\"success\"") != string::npos;
    }
    
    bool login(uint32_t user_index, const string& password) {
        string request = "{\"operation\":\"login\",\"session_id\":\"\",\"request_id\":\"" + 
                        generate_request_id() + "\",\"parameters\":{\"user_index\":" + 
                        to_string(user_index) + ",\"password\":\"" + password + "\"}}";
        
        string response = send_request(request);
        if (response.find("\"status\":\"success\"") != string::npos) {
            session_id = get_json_value(response, "session_id");
            return !session_id.empty();
        }
        return false;
    }
    
    bool logout() {
        string request = "{\"operation\":\"logout\",\"session_id\":\"" + session_id + 
                        "\",\"request_id\":\"" + generate_request_id() + "\",\"parameters\":{}}";
        
        string response = send_request(request);
        session_id = "";
        return response.find("\"status\":\"success\"") != string::npos;
    }
    
    string create_user(const string& username, const string& password, int role) {
        string request = "{\"operation\":\"user_create\",\"session_id\":\"" + session_id + 
                        "\",\"request_id\":\"" + generate_request_id() + 
                        "\",\"parameters\":{\"username\":\"" + username + 
                        "\",\"password\":\"" + password + "\",\"role\":" + to_string(role) + "}}";
        
        return send_request(request);
    }
    
    string delete_user(uint32_t user_index) {
        string request = "{\"operation\":\"user_delete\",\"session_id\":\"" + session_id + 
                        "\",\"request_id\":\"" + generate_request_id() + 
                        "\",\"parameters\":{\"user_index\":" + to_string(user_index) + "}}";
        
        return send_request(request);
    }
    
    string list_users() {
        string request = "{\"operation\":\"user_list\",\"session_id\":\"" + session_id + 
                        "\",\"request_id\":\"" + generate_request_id() + "\",\"parameters\":{}}";
        
        return send_request(request);
    }
    
    string create_file(const string& path, const string& data) {
        string request = "{\"operation\":\"file_create\",\"session_id\":\"" + session_id + 
                        "\",\"request_id\":\"" + generate_request_id() + 
                        "\",\"parameters\":{\"path\":\"" + path + "\",\"data\":\"" + data + "\"}}";
        
        return send_request(request);
    }
    
    string read_file(const string& path) {
        string request = "{\"operation\":\"file_read\",\"session_id\":\"" + session_id + 
                        "\",\"request_id\":\"" + generate_request_id() + 
                        "\",\"parameters\":{\"path\":\"" + path + "\"}}";
        
        return send_request(request);
    }
    
    string delete_file(const string& path) {
        string request = "{\"operation\":\"file_delete\",\"session_id\":\"" + session_id + 
                        "\",\"request_id\":\"" + generate_request_id() + 
                        "\",\"parameters\":{\"path\":\"" + path + "\"}}";
        
        return send_request(request);
    }
    
    string rename_file(const string& old_path, const string& new_path) {
        string request = "{\"operation\":\"file_rename\",\"session_id\":\"" + session_id + 
                        "\",\"request_id\":\"" + generate_request_id() + 
                        "\",\"parameters\":{\"old_path\":\"" + old_path + 
                        "\",\"new_path\":\"" + new_path + "\"}}";
        
        return send_request(request);
    }
    
    string create_directory(const string& path) {
        string request = "{\"operation\":\"dir_create\",\"session_id\":\"" + session_id + 
                        "\",\"request_id\":\"" + generate_request_id() + 
                        "\",\"parameters\":{\"path\":\"" + path + "\"}}";
        
        return send_request(request);
    }
    
    string list_directory(const string& path) {
        string request = "{\"operation\":\"dir_list\",\"session_id\":\"" + session_id + 
                        "\",\"request_id\":\"" + generate_request_id() + 
                        "\",\"parameters\":{\"path\":\"" + path + "\"}}";
        
        return send_request(request);
    }
    
    string delete_directory(const string& path) {
        string request = "{\"operation\":\"dir_delete\",\"session_id\":\"" + session_id + 
                        "\",\"request_id\":\"" + generate_request_id() + 
                        "\",\"parameters\":{\"path\":\"" + path + "\"}}";
        
        return send_request(request);
    }
    
    string get_stats() {
        string request = "{\"operation\":\"get_stats\",\"session_id\":\"" + session_id + 
                        "\",\"request_id\":\"" + generate_request_id() + "\",\"parameters\":{}}";
        
        return send_request(request);
    }
    
    string get_session_id() const { return session_id; }
};

void print_json_pretty(const string& json) {
    cout << json << "\n\n";
}

int main(int argc, char* argv[]) {
    string host = "127.0.0.1";
    int port = 8080;
    
    if (argc > 1) host = argv[1];
    if (argc > 2) port = atoi(argv[2]);
    
    FSClient client(host, port);
    
    cout << "=== OMNIFS Client Demo ===\n\n";
    
    cout << "[1] Initializing filesystem...\n";
    if (client.init()) {
        cout << "✓ Filesystem initialized\n\n";
    } else {
        cout << "✗ Initialization failed\n";
        return 1;
    }
    
    cout << "[2] Logging in as admin (user_index: 191110)...\n";
    if (client.login(191110, "password123")) {
        cout << "✓ Login successful (Session: " << client.get_session_id() << ")\n\n";
    } else {
        cout << "✗ Login failed. Trying with different index...\n";
        for (uint32_t idx = 100000; idx < 999999; idx += 100) {
            if (client.login(idx, "password123")) {
                cout << "✓ Login successful with index " << idx << "\n\n";
                break;
            }
        }
    }
    
    cout << "[3] Creating users...\n";
    print_json_pretty(client.create_user("alice", "pass_alice", 1));
    print_json_pretty(client.create_user("bob", "pass_bob", 1));
    
    cout << "[4] Listing all users...\n";
    print_json_pretty(client.list_users());
    
    cout << "[5] Creating directories...\n";
    print_json_pretty(client.create_directory("/documents"));
    print_json_pretty(client.create_directory("/documents/work"));
    
    cout << "[6] Creating files...\n";
    print_json_pretty(client.create_file("/documents/work/report.txt", "This is a test report."));
    print_json_pretty(client.create_file("/documents/notes.txt", "Some notes here."));
    
    cout << "[7] Listing directory...\n";
    print_json_pretty(client.list_directory("/documents"));
    
    cout << "[8] Reading file...\n";
    print_json_pretty(client.read_file("/documents/work/report.txt"));
    
    cout << "[9] Getting filesystem stats...\n";
    print_json_pretty(client.get_stats());
    
    cout << "[10] Renaming file...\n";
    print_json_pretty(client.rename_file("/documents/notes.txt", "/documents/notes_backup.txt"));
    
    cout << "[11] Deleting file...\n";
    print_json_pretty(client.delete_file("/documents/work/report.txt"));
    
    cout << "[12] Deleting directory...\n";
    print_json_pretty(client.delete_directory("/documents/work"));
    
    cout << "[13] Final stats...\n";
    print_json_pretty(client.get_stats());
    
    cout << "[14] Logging out...\n";
    if (client.logout()) {
        cout << "✓ Logout successful\n\n";
    }
    
    cout << "=== Demo Complete ===\n";
    
    return 0;
}