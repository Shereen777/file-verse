#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <map>
#include <sstream>
#include <fstream>
#include <pthread.h>
#include "file_system.cpp"
#include "BoundedBlockingQueue.hpp"

using namespace std;

void* fs_instance = nullptr;
map<string, void*> active_sessions;
pthread_mutex_t sessions_mutex = PTHREAD_MUTEX_INITIALIZER;

struct ClientRequest {
    int client_fd;
    string request_data;
    
    ClientRequest() : client_fd(-1) {}
    ClientRequest(int fd, const string& data) : client_fd(fd), request_data(data) {}
};

BoundedBlockingQueue<ClientRequest>* request_queue = nullptr;
bool server_running = true;

string json_escape(const string& str) {
    string result;
    for (char c : str) {
        if (c == '"') result += "\\\"";
        else if (c == '\\') result += "\\\\";
        else if (c == '\n') result += "\\n";
        else if (c == '\r') result += "\\r";
        else if (c == '\t') result += "\\t";
        else result += c;
    }
    return result;
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

int get_json_int(const string& json, const string& key) {
    size_t pos = json.find("\"" + key + "\"");
    if (pos == string::npos) return 0;
    
    pos = json.find(":", pos);
    if (pos == string::npos) return 0;
    
    while (pos < json.length() && (json[pos] == ':' || json[pos] == ' ')) pos++;
    
    string num;
    while (pos < json.length() && (isdigit(json[pos]) || json[pos] == '-')) {
        num += json[pos++];
    }
    
    return num.empty() ? 0 : stoi(num);
}

string create_response(const string& status, const string& operation, const string& request_id, const string& data_json) {
    return "{\"status\":\"" + status + "\",\"operation\":\"" + operation + 
           "\",\"request_id\":\"" + request_id + "\",\"data\":" + data_json + "}";
}

string create_error_response(const string& operation, const string& request_id, int error_code) {
    return "{\"status\":\"error\",\"operation\":\"" + operation + 
           "\",\"request_id\":\"" + request_id + "\",\"error_code\":" + to_string(error_code) + 
           ",\"error_message\":\"" + json_escape(get_error_message(error_code)) + "\"}";
}

string handle_init(const string& params) {
    string config_path = get_json_value(params, "config_path");
    string omni_path = get_json_value(params, "omni_path");
    
    if (config_path.empty()) config_path = "omnifs.conf";
    if (omni_path.empty()) omni_path = "omnifs.dat";
    
    int result = fs_init(&fs_instance, omni_path.c_str(), config_path.c_str());
    
    if (result == static_cast<int>(OFSErrorCodes::SUCCESS)) {
        return "{\"initialized\":true}";
    }
    return "{\"initialized\":false}";
}

string handle_login(const string& params, string& session_id) {
    uint32_t user_index = get_json_int(params, "user_index");
    string password = get_json_value(params, "password");
    
    void* session = nullptr;
    int result = user_login(&session, fs_instance, user_index, password.c_str());
    
    if (result == static_cast<int>(OFSErrorCodes::SUCCESS)) {
        session_id = "SESSION_" + to_string(time(nullptr)) + "_" + to_string(user_index);
        
        pthread_mutex_lock(&sessions_mutex);
        active_sessions[session_id] = session;
        pthread_mutex_unlock(&sessions_mutex);
        
        return "{\"session_id\":\"" + session_id + "\",\"user_index\":" + to_string(user_index) + "}";
    }
    return "{}";
}

string handle_logout(const string& session_id) {
    pthread_mutex_lock(&sessions_mutex);
    auto it = active_sessions.find(session_id);
    if (it == active_sessions.end()) {
        pthread_mutex_unlock(&sessions_mutex);
        return "{\"logged_out\":false}";
    }
    
    user_logout(it->second);
    active_sessions.erase(it);
    pthread_mutex_unlock(&sessions_mutex);
    
    return "{\"logged_out\":true}";
}

string handle_user_create(void* session, const string& params) {
    string username = get_json_value(params, "username");
    string password = get_json_value(params, "password");
    int role_int = get_json_int(params, "role");
    UserRole role = static_cast<UserRole>(role_int);
    
    uint32_t new_index;
    int result = user_create(session, username.c_str(), password.c_str(), role, new_index);
    
    if (result == static_cast<int>(OFSErrorCodes::SUCCESS)) {
        return "{\"user_index\":" + to_string(new_index) + ",\"username\":\"" + username + "\"}";
    }
    return "{}";
}

string handle_user_delete(void* session, const string& params) {
    uint32_t user_index = get_json_int(params, "user_index");
    int result = user_delete(session, user_index);
    return "{\"deleted\":" + string(result == static_cast<int>(OFSErrorCodes::SUCCESS) ? "true" : "false") + "}";
}

string handle_user_list(void* session) {
    UserInfo* users = nullptr;
    int count = 0;
    int result = user_list(session, &users, &count);
    
    if (result != static_cast<int>(OFSErrorCodes::SUCCESS)) {
        return "{\"users\":[]}";
    }
    
    string json = "{\"users\":[";
    for (int i = 0; i < count; i++) {
        if (i > 0) json += ",";
        json += "{\"username\":\"" + string(users[i].username) + "\"," +
                "\"user_index\":" + to_string(users[i].user_index) + "," +
                "\"role\":" + to_string(static_cast<int>(users[i].role)) + "}";
    }
    json += "]}";
    free_buffer(users);
    return json;
}

string handle_file_create(void* session, const string& params) {
    string path = get_json_value(params, "path");
    string data = get_json_value(params, "data");
    int result = file_create(session, path.c_str(), data.c_str(), data.length());
    return "{\"created\":" + string(result == static_cast<int>(OFSErrorCodes::SUCCESS) ? "true" : "false") + "}";
}

string handle_file_read(void* session, const string& params) {
    string path = get_json_value(params, "path");
    char* buffer = nullptr;
    size_t size = 0;
    int result = file_read(session, path.c_str(), &buffer, &size);
    
    if (result != static_cast<int>(OFSErrorCodes::SUCCESS)) {
        return "{\"content\":\"\",\"size\":0}";
    }
    
    string content = buffer ? string(buffer, size) : "";
    free_buffer(buffer);
    return "{\"content\":\"" + json_escape(content) + "\",\"size\":" + to_string(size) + "}";
}

string handle_file_delete(void* session, const string& params) {
    string path = get_json_value(params, "path");
    int result = file_delete(session, path.c_str());
    return "{\"deleted\":" + string(result == static_cast<int>(OFSErrorCodes::SUCCESS) ? "true" : "false") + "}";
}

string handle_file_rename(void* session, const string& params) {
    string old_path = get_json_value(params, "old_path");
    string new_path = get_json_value(params, "new_path");
    int result = file_rename(session, old_path.c_str(), new_path.c_str());
    return "{\"renamed\":" + string(result == static_cast<int>(OFSErrorCodes::SUCCESS) ? "true" : "false") + "}";
}

string handle_dir_create(void* session, const string& params) {
    string path = get_json_value(params, "path");
    int result = dir_create(session, path.c_str());
    return "{\"created\":" + string(result == static_cast<int>(OFSErrorCodes::SUCCESS) ? "true" : "false") + "}";
}

string handle_dir_list(void* session, const string& params) {
    string path = get_json_value(params, "path");
    FileEntry* entries = nullptr;
    int count = 0;
    int result = dir_list(session, path.c_str(), &entries, &count);
    
    if (result != static_cast<int>(OFSErrorCodes::SUCCESS)) {
        return "{\"entries\":[]}";
    }
    
    string json = "{\"entries\":[";
    for (int i = 0; i < count; i++) {
        if (i > 0) json += ",";
        json += "{\"name\":\"" + string(entries[i].name) + "\"," +
                "\"type\":" + to_string(entries[i].type) + "," +
                "\"size\":" + to_string(entries[i].size) + "," +
                "\"owner\":\"" + string(entries[i].owner) + "\"}";
    }
    json += "]}";
    free_buffer(entries);
    return json;
}

string handle_dir_delete(void* session, const string& params) {
    string path = get_json_value(params, "path");
    int result = dir_delete(session, path.c_str());
    return "{\"deleted\":" + string(result == static_cast<int>(OFSErrorCodes::SUCCESS) ? "true" : "false") + "}";
}

string handle_get_stats(void* session) {
    FSStats stats;
    int result = get_stats(session, &stats);
    
    if (result != static_cast<int>(OFSErrorCodes::SUCCESS)) {
        return "{}";
    }
    
    return "{\"total_size\":" + to_string(stats.total_size) + 
           ",\"used_space\":" + to_string(stats.used_space) +
           ",\"free_space\":" + to_string(stats.free_space) +
           ",\"total_files\":" + to_string(stats.total_files) +
           ",\"total_directories\":" + to_string(stats.total_directories) +
           ",\"total_users\":" + to_string(stats.total_users) +
           ",\"active_sessions\":" + to_string(stats.active_sessions) + "}";
}

string process_request(const string& request) {
    string operation = get_json_value(request, "operation");
    string session_id = get_json_value(request, "session_id");
    string request_id = get_json_value(request, "request_id");
    
    size_t params_start = request.find("\"parameters\"");
    string params = "";
    if (params_start != string::npos) {
        size_t brace_start = request.find("{", params_start);
        if (brace_start != string::npos) {
            int brace_count = 1;
            size_t pos = brace_start + 1;
            while (pos < request.length() && brace_count > 0) {
                if (request[pos] == '{') brace_count++;
                else if (request[pos] == '}') brace_count--;
                pos++;
            }
            params = request.substr(brace_start, pos - brace_start);
        }
    }
    
    void* session = nullptr;
    if (!session_id.empty()) {
        pthread_mutex_lock(&sessions_mutex);
        if (active_sessions.find(session_id) != active_sessions.end()) {
            session = active_sessions[session_id];
        }
        pthread_mutex_unlock(&sessions_mutex);
    }
    
    int result = 0;
    string data_json;
    
    if (operation == "init") {
        data_json = handle_init(params);
        result = static_cast<int>(OFSErrorCodes::SUCCESS);
    }
    else if (operation == "login") {
        string new_session_id;
        data_json = handle_login(params, new_session_id);
        result = new_session_id.empty() ? static_cast<int>(OFSErrorCodes::ERROR_PERMISSION_DENIED) : static_cast<int>(OFSErrorCodes::SUCCESS);
    }
    else if (operation == "logout") {
        data_json = handle_logout(session_id);
        result = static_cast<int>(OFSErrorCodes::SUCCESS);
    }
    else if (!session) {
        return create_error_response(operation, request_id, static_cast<int>(OFSErrorCodes::ERROR_INVALID_SESSION));
    }
    else if (operation == "user_create") {
        data_json = handle_user_create(session, params);
        result = data_json == "{}" ? static_cast<int>(OFSErrorCodes::ERROR_PERMISSION_DENIED) : static_cast<int>(OFSErrorCodes::SUCCESS);
    }
    else if (operation == "user_delete") {
        data_json = handle_user_delete(session, params);
        result = static_cast<int>(OFSErrorCodes::SUCCESS);
    }
    else if (operation == "user_list") {
        data_json = handle_user_list(session);
        result = static_cast<int>(OFSErrorCodes::SUCCESS);
    }
    else if (operation == "file_create") {
        data_json = handle_file_create(session, params);
        result = static_cast<int>(OFSErrorCodes::SUCCESS);
    }
    else if (operation == "file_read") {
        data_json = handle_file_read(session, params);
        result = static_cast<int>(OFSErrorCodes::SUCCESS);
    }
    else if (operation == "file_delete") {
        data_json = handle_file_delete(session, params);
        result = static_cast<int>(OFSErrorCodes::SUCCESS);
    }
    else if (operation == "file_rename") {
        data_json = handle_file_rename(session, params);
        result = static_cast<int>(OFSErrorCodes::SUCCESS);
    }
    else if (operation == "dir_create") {
        data_json = handle_dir_create(session, params);
        result = static_cast<int>(OFSErrorCodes::SUCCESS);
    }
    else if (operation == "dir_list") {
        data_json = handle_dir_list(session, params);
        result = static_cast<int>(OFSErrorCodes::SUCCESS);
    }
    else if (operation == "dir_delete") {
        data_json = handle_dir_delete(session, params);
        result = static_cast<int>(OFSErrorCodes::SUCCESS);
    }
    else if (operation == "get_stats") {
        data_json = handle_get_stats(session);
        result = static_cast<int>(OFSErrorCodes::SUCCESS);
    }
    else {
        return create_error_response(operation, request_id, static_cast<int>(OFSErrorCodes::ERROR_NOT_IMPLEMENTED));
    }
    
    if (result == static_cast<int>(OFSErrorCodes::SUCCESS)) {
        return create_response("success", operation, request_id, data_json);
    }
    return create_error_response(operation, request_id, result);
}

void* worker_thread(void* arg) {
    int thread_id = *((int*)arg);
    cout << "Worker thread " << thread_id << " started\n";
    
    while (server_running) {
        ClientRequest req = request_queue->dequeue();
        
        if (req.client_fd == -1) break;
        
        string response = process_request(req.request_data);
        send(req.client_fd, response.c_str(), response.length(), 0);
        close(req.client_fd);
    }
    
    cout << "Worker thread " << thread_id << " stopped\n";
    return nullptr;
}

int main(int argc, char* argv[]) {
    int port = 8080;
    int num_workers = 4;
    int queue_size = 100;
    
    if (argc > 1) port = atoi(argv[1]);
    if (argc > 2) num_workers = atoi(argv[2]);
    if (argc > 3) queue_size = atoi(argv[3]);
    
    request_queue = new BoundedBlockingQueue<ClientRequest>(queue_size);
    
    pthread_t* workers = new pthread_t[num_workers];
    int* worker_ids = new int[num_workers];
    
    for (int i = 0; i < num_workers; i++) {
        worker_ids[i] = i;
        pthread_create(&workers[i], nullptr, worker_thread, &worker_ids[i]);
    }
    
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        cerr << "Socket creation failed\n";
        return 1;
    }
    
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    
    if (bind(server_fd, (sockaddr*)&address, sizeof(address)) < 0) {
        cerr << "Bind failed\n";
        return 1;
    }
    
    if (listen(server_fd, 50) < 0) {
        cerr << "Listen failed\n";
        return 1;
    }
    
    cout << "========================================\n";
    cout << "OMNIFS Server Configuration:\n";
    cout << "  Port: " << port << "\n";
    cout << "  Worker Threads: " << num_workers << "\n";
    cout << "  Queue Size: " << queue_size << "\n";
    cout << "========================================\n";
    cout << "Server is running... Press Ctrl+C to stop\n\n";
    
    while (server_running) {
        sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (sockaddr*)&client_addr, &client_len);
        
        if (client_fd < 0) continue;
        
        char buffer[4096] = {0};
        int bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
        
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            ClientRequest req(client_fd, string(buffer));
            request_queue->enqueue(req);
        } else {
            close(client_fd);
        }
    }
    
    server_running = false;
    for (int i = 0; i < num_workers; i++) {
        request_queue->enqueue(ClientRequest(-1, ""));
    }
    
    for (int i = 0; i < num_workers; i++) {
        pthread_join(workers[i], nullptr);
    }
    
    delete[] workers;
    delete[] worker_ids;
    delete request_queue;
    
    if (fs_instance) fs_shutdown(fs_instance);
    close(server_fd);
    
    cout << "\nServer shutdown complete\n";
    return 0;
}