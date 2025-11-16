// #include "../include/odf_types.hpp"
// #include <iostream>
// #include <iomanip>
// #include <cstring>
// #include <fstream>
// #include "file_system.cpp"
// #include "../include/odf_types.hpp"
// #include "IndexGenerator.hpp"
// #include "AVL.hpp"
// #include "UserSystem.hpp"
// #include "FreeSpaceManager.hpp"

// using namespace std;

// int fs_init(void** instance, const char* omni_path, const char* config_path);
// void fs_shutdown(void* instance);
// int fs_format(const char* omni_path, const char* config_path);

// int user_login(void** session, void* instance, uint32_t user_index, const char* password);
// int user_logout(void* session);

// int user_create(void* session, const char* username, const char* password, UserRole role, uint32_t &out_index);
// int user_delete(void* session, uint32_t user_index);
// int user_list(void* session, UserInfo** users, int* count);
// int get_session_info(void* session, SessionInfo* info);

// int file_create(void* session, const char* path, const char* data, size_t size);
// int file_read(void* session, const char* path, char** buffer, size_t* size);
// int file_edit(void* session, const char* path, const char* data, size_t size, uint index);
// int file_delete(void* session, const char* path);
// int file_truncate(void* session, const char* path);
// int file_exists(void* session, const char* path);
// int file_rename(void* session, const char* old_path, const char* new_path);

// int dir_create(void* session, const char* path);
// int dir_list(void* session, const char* path, FileEntry** entries, int* count);
// int dir_delete(void* session, const char* path);
// int dir_exists(void* session, const char* path);

// int get_metadata(void* session, const char* path, FileMetadata* meta);
// int set_permissions(void* session, const char* path, uint32_t permissions);
// int get_stats(void* session, FSStats* stats);

// void free_buffer(void* buffer);
// const char* get_error_message(int error_code);

// uint32_t get_admin_index(void* instance);

// void print_separator(const string& title = "") {
//     cout << "\n" << string(60, '=') << "\n";
//     if (!title.empty()) {
//         cout << "  " << title << "\n";
//         cout << string(60, '=') << "\n";
//     }
// }

// void print_result(const string& operation, int result) {
//     if (result == static_cast<int>(OFSErrorCodes::SUCCESS)) {
//         cout << "✓ " << operation << " - SUCCESS\n";
//     } else {
//         cout << "✗ " << operation << " - FAILED: " << get_error_message(result) << "\n";
//     }
// }

// void print_file_entry(const FileEntry& entry) {
//     cout << "  ";
//     if (entry.getType() == EntryType::DIRECTORY) {
//         cout << "[DIR]  ";
//     } else {
//         cout << "[FILE] ";
//     }
//     cout << left << setw(30) << entry.name;
//     cout << right << setw(10) << entry.size << " bytes";
//     cout << "  Owner: " << entry.owner;
//     cout << "\n";
// }

// int main() {
//     cout << "\n" << string(70, '=') << "\n";
//     cout << "  OMNIFS - Complete File System Demonstration with User Isolation\n";
//     cout << string(70, '=') << "\n\n";
    
//     const char* omni_path = "omnifs.dat";
//     const char* config_path = "omnifs.conf";
    
//     {
//         ofstream conf(config_path);
//         conf << "[filesystem]\n";
//         conf << "total_size = 2097152\n";
//         conf << "header_size = 4096\n";
//         conf << "block_size = 4096\n";
//         conf << "max_users = 100\n";
//         conf << "\n[security]\n";
//         conf << "admin_username = admin\n";
//         conf << "admin_password = password123\n";
//         conf << "require_auth = true\n";
//         conf.close();
//     }
    
//     cout << "[1] Initializing filesystem...\n";
//     void* instance = nullptr;
//     int result = fs_init(&instance, omni_path, config_path);
//     if (result != static_cast<int>(OFSErrorCodes::SUCCESS)) {
//         cout << "✗ Failed: " << get_error_message(result) << "\n";
//         return 1;
//     }
//     cout << "✓ Filesystem initialized\n";
//     cout << "✓ /users directory structure created\n\n";
    
//     OMNIInstance* inst = static_cast<OMNIInstance*>(instance);
//     UserInfo* users = nullptr;
//     int count = 0;
//     inst->user_system.get_all_users(&users, &count);

//     uint32_t admin_index = users[0].user_index;
//     free(users);
    
//     cout << "[2] Admin login (index: " << admin_index << ")...\n";
//     void* admin_session = nullptr;
//     result = user_login(&admin_session, instance, admin_index, "password123");
//     if (result != static_cast<int>(OFSErrorCodes::SUCCESS)) {
//         cout << "✗ Login failed\n";
//         fs_shutdown(instance);
//         return 1;
//     }
//     cout << "\n";
    
//     cout << "[3] Creating users (with home directories)...\n";
//     uint32_t user1_idx, user2_idx;
//     user_create(admin_session, "alice", "pass_alice", UserRole::NORMAL, user1_idx);
//     user_create(admin_session, "bob", "pass_bob", UserRole::NORMAL, user2_idx);
//     cout << "✓ User directories created: /users/alice and /users/bob\n\n";
    
//     cout << "[4] Listing all users...\n";
//     UserInfo* user_list_ptr = nullptr;
//     int user_count = 0;
//     user_list(admin_session, &user_list_ptr, &user_count);
//     cout << "✓ Total users: " << user_count << "\n";
//     for (int i = 0; i < user_count; i++) {
//         cout << "  - " << user_list_ptr[i].username 
//              << " (index: " << user_list_ptr[i].user_index 
//              << ", role: " << (int)user_list_ptr[i].role << ")\n";
//     }
//     free_buffer(user_list_ptr);
//     cout << "\n";
    
//     cout << "[5] Getting session information...\n";
//     SessionInfo sess_info{};
//     result = get_session_info(admin_session, &sess_info);
//     if (result == static_cast<int>(OFSErrorCodes::SUCCESS)) {
//         cout << "✓ Session Info:\n";
//         cout << "  Session ID: " << sess_info.session_id << "\n";
//         cout << "  User: " << sess_info.user.username << "\n";
//         cout << "  Operations: " << sess_info.operations_count << "\n";
//     }
//     cout << "\n";
    
//     cout << "[6] Admin creating directory structure (no user prefix for admin)...\n";
//     dir_create(admin_session, "/shared");
//     dir_create(admin_session, "/shared/public");
//     cout << "✓ Admin can access root directly\n\n";
    
//     cout << "[7] Logging out admin and logging in as Alice...\n";
//     user_logout(admin_session);
//     void* alice_session = nullptr;
//     result = user_login(&alice_session, instance, user1_idx, "pass_alice");
//     if (result != static_cast<int>(OFSErrorCodes::SUCCESS)) {
//         cout << "✗ Alice login failed\n";
//         fs_shutdown(instance);
//         return 1;
//     }
//     cout << "✓ Alice logged in\n\n";
    
//     cout << "[8] Alice creating her directory structure...\n";
//     cout << "   (Path '/documents' becomes '/users/alice/documents')\n";
//     dir_create(alice_session, "/documents");
//     dir_create(alice_session, "/documents/work");
//     dir_create(alice_session, "/documents/personal");
//     cout << "\n";
    
//     cout << "[9] Alice creating files in her workspace...\n";
//     cout << "   (Path '/documents/work/report.txt' becomes '/users/alice/documents/work/report.txt')\n";
//     const char* content1 = "This is Alice's work document.\nIt contains important information.";
//     const char* content2 = "Alice's personal notes and thoughts.";
    
//     file_create(alice_session, "/documents/work/report.txt", content1, strlen(content1));
//     file_create(alice_session, "/documents/personal/notes.txt", content2, strlen(content2));
//     cout << "\n";
    
//     cout << "[10] Alice setting file permissions...\n";
//     set_permissions(alice_session, "/documents/work/report.txt", 0644);
//     set_permissions(alice_session, "/documents/personal/notes.txt", 0600);
//     cout << "\n";
    
//     cout << "[11] Alice listing her /documents directory...\n";
//     FileEntry* entries = nullptr;
//     int entry_count = 0;
//     result = dir_list(alice_session, "/documents", &entries, &entry_count);
//     if (result == static_cast<int>(OFSErrorCodes::SUCCESS)) {
//         cout << "✓ Entries in Alice's /documents (actual: /users/alice/documents):\n";
//         for (int i = 0; i < entry_count; i++) {
//             cout << "  - " << entries[i].name 
//                  << " (type: " << (int)entries[i].type 
//                  << ", owner: " << entries[i].owner << ")\n";
//         }
//     }
//     free_buffer(entries);
//     cout << "\n";
    
//     cout << "[12] Alice reading her file...\n";
//     char* file_data = nullptr;
//     size_t file_size = 0;
//     result = file_read(alice_session, "/documents/work/report.txt", &file_data, &file_size);
//     if (result == static_cast<int>(OFSErrorCodes::SUCCESS)) {
//         cout << "✓ Content from /users/alice/documents/work/report.txt:\n";
//         cout << "   " << file_data << "\n";
//     }
//     free_buffer(file_data);
//     cout << "\n";
    
//     cout << "[13] Alice getting file metadata...\n";
//     FileMetadata meta{};
//     result = get_metadata(alice_session, "/documents/work/report.txt", &meta);
//     if (result == static_cast<int>(OFSErrorCodes::SUCCESS)) {
//         cout << "✓ Metadata:\n";
//         cout << "  Full path: " << meta.path << "\n";
//         cout << "  Name: " << meta.entry.name << "\n";
//         cout << "  Size: " << meta.entry.size << " bytes\n";
//         cout << "  Owner: " << meta.entry.owner << "\n";
//         cout << "  Permissions: 0o" << oct << meta.entry.permissions << dec << "\n";
//     }
//     cout << "\n";
    
//     cout << "[14] Alice editing her file...\n";
//     const char* new_content = "MODIFIED";
//     result = file_edit(alice_session, "/documents/work/report.txt", new_content, strlen(new_content), 0);
//     if (result == static_cast<int>(OFSErrorCodes::SUCCESS)) {
//         cout << "✓ File edited\n";
//     }
//     cout << "\n";
    
//     cout << "[15] Alice reading modified file...\n";
//     file_data = nullptr;
//     file_size = 0;
//     result = file_read(alice_session, "/documents/work/report.txt", &file_data, &file_size);
//     if (result == static_cast<int>(OFSErrorCodes::SUCCESS)) {
//         cout << "✓ Modified content:\n";
//         cout << "   " << file_data << "\n";
//     }
//     free_buffer(file_data);
//     cout << "\n";
    
//     cout << "[16] Alice performing file operations...\n";
//     if (file_exists(alice_session, "/documents/work/report.txt") == static_cast<int>(OFSErrorCodes::SUCCESS)) {
//         cout << "✓ File exists in Alice's workspace\n";
//     }
//     file_rename(alice_session, "/documents/work/report.txt", "/documents/work/report_backup.txt");
//     cout << "\n";
    
//     cout << "[17] Logging out Alice and logging in as Bob...\n";
//     user_logout(alice_session);
//     void* bob_session = nullptr;
//     result = user_login(&bob_session, instance, user2_idx, "pass_bob");
//     if (result != static_cast<int>(OFSErrorCodes::SUCCESS)) {
//         cout << "✗ Bob login failed\n";
//         fs_shutdown(instance);
//         return 1;
//     }
//     cout << "✓ Bob logged in\n\n";
    
//     cout << "[18] Bob creating his directory structure...\n";
//     cout << "   (Bob's paths are isolated to /users/bob/...)\n";
//     dir_create(bob_session, "/projects");
//     dir_create(bob_session, "/projects/code");
//     cout << "\n";
    
//     cout << "[19] Bob creating files in his workspace...\n";
//     const char* bob_content = "Bob's project code and notes.";
//     file_create(bob_session, "/projects/code/main.cpp", bob_content, strlen(bob_content));
//     cout << "\n";
    
//     cout << "[20] Bob trying to access Alice's file (should fail)...\n";
//     file_data = nullptr;
//     result = file_read(bob_session, "/users/alice/documents/work/report_backup.txt", &file_data, &file_size);
//     if (result != static_cast<int>(OFSErrorCodes::SUCCESS)) {
//         cout << "✓ Correctly denied: Bob cannot access Alice's files\n";
//         cout << "   Error: " << get_error_message(result) << "\n";
//     }
//     cout << "\n";
    
//     cout << "[21] Bob listing his /projects directory...\n";
//     entries = nullptr;
//     entry_count = 0;
//     result = dir_list(bob_session, "/projects", &entries, &entry_count);
//     if (result == static_cast<int>(OFSErrorCodes::SUCCESS)) {
//         cout << "✓ Entries in Bob's /projects (actual: /users/bob/projects):\n";
//         for (int i = 0; i < entry_count; i++) {
//             cout << "  - " << entries[i].name 
//                  << " (owner: " << entries[i].owner << ")\n";
//         }
//     }
//     free_buffer(entries);
//     cout << "\n";
    
//     cout << "[22] Logging out Bob and logging back in as Admin...\n";
//     user_logout(bob_session);
//     result = user_login(&admin_session, instance, admin_index, "password123");
//     cout << "\n";
    
//     cout << "[23] Admin accessing Alice's files directly...\n";
//     file_data = nullptr;
//     result = file_read(admin_session, "/users/alice/documents/work/report_backup.txt", &file_data, &file_size);
//     if (result == static_cast<int>(OFSErrorCodes::SUCCESS)) {
//         cout << "✓ Admin can access any user's files:\n";
//         cout << "   " << file_data << "\n";
//     }
//     free_buffer(file_data);
//     cout << "\n";
    
//     cout << "[24] Admin listing /users directory...\n";
//     entries = nullptr;
//     entry_count = 0;
//     result = dir_list(admin_session, "/users", &entries, &entry_count);
//     if (result == static_cast<int>(OFSErrorCodes::SUCCESS)) {
//         cout << "✓ User directories:\n";
//         for (int i = 0; i < entry_count; i++) {
//             cout << "  - " << entries[i].name << " (type: DIR)\n";
//         }
//     }
//     free_buffer(entries);
//     cout << "\n";
    
//     cout << "[25] Getting filesystem statistics...\n";
//     FSStats stats{};
//     result = get_stats(admin_session, &stats);
//     if (result == static_cast<int>(OFSErrorCodes::SUCCESS)) {
//         cout << "✓ Filesystem Stats:\n";
//         cout << "  Total size: " << stats.total_size << " bytes\n";
//         cout << "  Used space: " << stats.used_space << " bytes\n";
//         cout << "  Free space: " << stats.free_space << " bytes\n";
//         cout << "  Total files: " << stats.total_files << "\n";
//         cout << "  Total directories: " << stats.total_directories << "\n";
//         cout << "  Total users: " << stats.total_users << "\n";
//         cout << "  Active sessions: " << stats.active_sessions << "\n";
//         cout << "  Fragmentation: " << stats.fragmentation << "%\n";
//     }
//     cout << "\n";
    
//     cout << "[26] Admin creating file in shared space...\n";
//     const char* content3 = "Shared content accessible by admin.";
//     file_create(admin_session, "/shared/readme.txt", content3, strlen(content3));
//     file_truncate(admin_session, "/shared/readme.txt");
//     cout << "\n";
    
//     cout << "[27] Cleanup operations...\n";
//     file_delete(admin_session, "/users/alice/documents/personal/notes.txt");
//     dir_delete(admin_session, "/users/alice/documents/personal");
//     cout << "\n";
    
//     cout << "[28] Deleting user Bob (and his home directory will be orphaned)...\n";
//     user_delete(admin_session, user2_idx);
//     cout << "   Note: /users/bob directory remains but user cannot login\n\n";
    
//     cout << "[29] Final filesystem statistics...\n";
//     FSStats final_stats{};
//     get_stats(admin_session, &final_stats);
//     cout << "✓ Final Stats:\n";
//     cout << "  Total files: " << final_stats.total_files << "\n";
//     cout << "  Total directories: " << final_stats.total_directories << "\n";
//     cout << "  Total users: " << final_stats.total_users << "\n";
//     cout << "\n";
    
//     cout << "[30] Logging out and shutting down...\n";
//     user_logout(admin_session);
//     fs_shutdown(instance);
//     cout << "✓ Shutdown complete\n\n";
    
//     cout << string(70, '=') << "\n";
//     cout << "  User Isolation Testing Complete!\n";
//     cout << "  Key Features Demonstrated:\n";
//     cout << "  - Each user has isolated /users/<username>/ directory\n";
//     cout << "  - Regular users can only access their own files\n";
//     cout << "  - Admin can access all files system-wide\n";
//     cout << "  - Path resolution is transparent to users\n";
//     cout << string(70, '=') << "\n\n";
    
//     return 0;
// }

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