#include "../include/odf_types.hpp"
#include <iostream>
#include <iomanip>
#include <cstring>
#include <fstream>
#include "file_system.cpp"
#include "../include/odf_types.hpp"
#include "IndexGenerator.hpp"
#include "AVL.hpp"
#include "UserSystem.hpp"
#include "FreeSpaceManager.hpp"

using namespace std;

int fs_init(void** instance, const char* omni_path, const char* config_path);
void fs_shutdown(void* instance);
int fs_format(const char* omni_path, const char* config_path);

int user_login(void** session, void* instance, uint32_t user_index, const char* password);
int user_logout(void* session);

int user_create(void* session, const char* username, const char* password, UserRole role, uint32_t &out_index);
int user_delete(void* session, uint32_t user_index);
int user_list(void* session, UserInfo** users, int* count);
int get_session_info(void* session, SessionInfo* info);

int file_create(void* session, const char* path, const char* data, size_t size);
int file_read(void* session, const char* path, char** buffer, size_t* size);
int file_edit(void* session, const char* path, const char* data, size_t size, uint index);
int file_delete(void* session, const char* path);
int file_truncate(void* session, const char* path);
int file_exists(void* session, const char* path);
int file_rename(void* session, const char* old_path, const char* new_path);

int dir_create(void* session, const char* path);
int dir_list(void* session, const char* path, FileEntry** entries, int* count);
int dir_delete(void* session, const char* path);
int dir_exists(void* session, const char* path);

int get_metadata(void* session, const char* path, FileMetadata* meta);
int set_permissions(void* session, const char* path, uint32_t permissions);
int get_stats(void* session, FSStats* stats);

void free_buffer(void* buffer);
const char* get_error_message(int error_code);

uint32_t get_admin_index(void* instance);

void print_separator(const string& title = "") {
    cout << "\n" << string(60, '=') << "\n";
    if (!title.empty()) {
        cout << "  " << title << "\n";
        cout << string(60, '=') << "\n";
    }
}

void print_result(const string& operation, int result) {
    if (result == static_cast<int>(OFSErrorCodes::SUCCESS)) {
        cout << "✓ " << operation << " - SUCCESS\n";
    } else {
        cout << "✗ " << operation << " - FAILED: " << get_error_message(result) << "\n";
    }
}

void print_file_entry(const FileEntry& entry) {
    cout << "  ";
    if (entry.getType() == EntryType::DIRECTORY) {
        cout << "[DIR]  ";
    } else {
        cout << "[FILE] ";
    }
    cout << left << setw(30) << entry.name;
    cout << right << setw(10) << entry.size << " bytes";
    cout << "  Owner: " << entry.owner;
    cout << "\n";
}

int main() {
    cout << "\n" << string(70, '=') << "\n";
    cout << "  OMNIFS - Complete File System Demonstration with User Isolation\n";
    cout << string(70, '=') << "\n\n";
    
    const char* omni_path = "omnifs.dat";
    const char* config_path = "omnifs.conf";
    
    {
        ofstream conf(config_path);
        conf << "[filesystem]\n";
        conf << "total_size = 2097152\n";
        conf << "header_size = 4096\n";
        conf << "block_size = 4096\n";
        conf << "max_users = 100\n";
        conf << "\n[security]\n";
        conf << "admin_username = admin\n";
        conf << "admin_password = password123\n";
        conf << "require_auth = true\n";
        conf.close();
    }
    
    cout << "[1] Initializing filesystem...\n";
    void* instance = nullptr;
    int result = fs_init(&instance, omni_path, config_path);
    if (result != static_cast<int>(OFSErrorCodes::SUCCESS)) {
        cout << "✗ Failed: " << get_error_message(result) << "\n";
        return 1;
    }
    cout << "✓ Filesystem initialized\n";
    cout << "✓ /users directory structure created\n\n";
    
    OMNIInstance* inst = static_cast<OMNIInstance*>(instance);
    UserInfo* users = nullptr;
    int count = 0;
    inst->user_system.get_all_users(&users, &count);

    uint32_t admin_index = users[0].user_index;
    free(users);
    
    cout << "[2] Admin login (index: " << admin_index << ")...\n";
    void* admin_session = nullptr;
    result = user_login(&admin_session, instance, admin_index, "password123");
    if (result != static_cast<int>(OFSErrorCodes::SUCCESS)) {
        cout << "✗ Login failed\n";
        fs_shutdown(instance);
        return 1;
    }
    cout << "\n";
    
    cout << "[3] Creating users (with home directories)...\n";
    uint32_t user1_idx, user2_idx;
    user_create(admin_session, "alice", "pass_alice", UserRole::NORMAL, user1_idx);
    user_create(admin_session, "bob", "pass_bob", UserRole::NORMAL, user2_idx);
    cout << "✓ User directories created: /users/alice and /users/bob\n\n";
    
    cout << "[4] Listing all users...\n";
    UserInfo* user_list_ptr = nullptr;
    int user_count = 0;
    user_list(admin_session, &user_list_ptr, &user_count);
    cout << "✓ Total users: " << user_count << "\n";
    for (int i = 0; i < user_count; i++) {
        cout << "  - " << user_list_ptr[i].username 
             << " (index: " << user_list_ptr[i].user_index 
             << ", role: " << (int)user_list_ptr[i].role << ")\n";
    }
    free_buffer(user_list_ptr);
    cout << "\n";
    
    cout << "[5] Getting session information...\n";
    SessionInfo sess_info{};
    result = get_session_info(admin_session, &sess_info);
    if (result == static_cast<int>(OFSErrorCodes::SUCCESS)) {
        cout << "✓ Session Info:\n";
        cout << "  Session ID: " << sess_info.session_id << "\n";
        cout << "  User: " << sess_info.user.username << "\n";
        cout << "  Operations: " << sess_info.operations_count << "\n";
    }
    cout << "\n";
    
    cout << "[6] Admin creating directory structure (no user prefix for admin)...\n";
    dir_create(admin_session, "/shared");
    dir_create(admin_session, "/shared/public");
    cout << "✓ Admin can access root directly\n\n";
    
    cout << "[7] Logging out admin and logging in as Alice...\n";
    user_logout(admin_session);
    void* alice_session = nullptr;
    result = user_login(&alice_session, instance, user1_idx, "pass_alice");
    if (result != static_cast<int>(OFSErrorCodes::SUCCESS)) {
        cout << "✗ Alice login failed\n";
        fs_shutdown(instance);
        return 1;
    }
    cout << "✓ Alice logged in\n\n";
    
    cout << "[8] Alice creating her directory structure...\n";
    cout << "   (Path '/documents' becomes '/users/alice/documents')\n";
    dir_create(alice_session, "/documents");
    dir_create(alice_session, "/documents/work");
    dir_create(alice_session, "/documents/personal");
    cout << "\n";
    
    cout << "[9] Alice creating files in her workspace...\n";
    cout << "   (Path '/documents/work/report.txt' becomes '/users/alice/documents/work/report.txt')\n";
    const char* content1 = "This is Alice's work document.\nIt contains important information.";
    const char* content2 = "Alice's personal notes and thoughts.";
    
    file_create(alice_session, "/documents/work/report.txt", content1, strlen(content1));
    file_create(alice_session, "/documents/personal/notes.txt", content2, strlen(content2));
    cout << "\n";
    
    cout << "[10] Alice setting file permissions...\n";
    set_permissions(alice_session, "/documents/work/report.txt", 0644);
    set_permissions(alice_session, "/documents/personal/notes.txt", 0600);
    cout << "\n";
    
    cout << "[11] Alice listing her /documents directory...\n";
    FileEntry* entries = nullptr;
    int entry_count = 0;
    result = dir_list(alice_session, "/documents", &entries, &entry_count);
    if (result == static_cast<int>(OFSErrorCodes::SUCCESS)) {
        cout << "✓ Entries in Alice's /documents (actual: /users/alice/documents):\n";
        for (int i = 0; i < entry_count; i++) {
            cout << "  - " << entries[i].name 
                 << " (type: " << (int)entries[i].type 
                 << ", owner: " << entries[i].owner << ")\n";
        }
    }
    free_buffer(entries);
    cout << "\n";
    
    cout << "[12] Alice reading her file...\n";
    char* file_data = nullptr;
    size_t file_size = 0;
    result = file_read(alice_session, "/documents/work/report.txt", &file_data, &file_size);
    if (result == static_cast<int>(OFSErrorCodes::SUCCESS)) {
        cout << "✓ Content from /users/alice/documents/work/report.txt:\n";
        cout << "   " << file_data << "\n";
    }
    free_buffer(file_data);
    cout << "\n";
    
    cout << "[13] Alice getting file metadata...\n";
    FileMetadata meta{};
    result = get_metadata(alice_session, "/documents/work/report.txt", &meta);
    if (result == static_cast<int>(OFSErrorCodes::SUCCESS)) {
        cout << "✓ Metadata:\n";
        cout << "  Full path: " << meta.path << "\n";
        cout << "  Name: " << meta.entry.name << "\n";
        cout << "  Size: " << meta.entry.size << " bytes\n";
        cout << "  Owner: " << meta.entry.owner << "\n";
        cout << "  Permissions: 0o" << oct << meta.entry.permissions << dec << "\n";
    }
    cout << "\n";
    
    cout << "[14] Alice editing her file...\n";
    const char* new_content = "MODIFIED";
    result = file_edit(alice_session, "/documents/work/report.txt", new_content, strlen(new_content), 0);
    if (result == static_cast<int>(OFSErrorCodes::SUCCESS)) {
        cout << "✓ File edited\n";
    }
    cout << "\n";
    
    cout << "[15] Alice reading modified file...\n";
    file_data = nullptr;
    file_size = 0;
    result = file_read(alice_session, "/documents/work/report.txt", &file_data, &file_size);
    if (result == static_cast<int>(OFSErrorCodes::SUCCESS)) {
        cout << "✓ Modified content:\n";
        cout << "   " << file_data << "\n";
    }
    free_buffer(file_data);
    cout << "\n";
    
    cout << "[16] Alice performing file operations...\n";
    if (file_exists(alice_session, "/documents/work/report.txt") == static_cast<int>(OFSErrorCodes::SUCCESS)) {
        cout << "✓ File exists in Alice's workspace\n";
    }
    file_rename(alice_session, "/documents/work/report.txt", "/documents/work/report_backup.txt");
    cout << "\n";
    
    cout << "[17] Logging out Alice and logging in as Bob...\n";
    user_logout(alice_session);
    void* bob_session = nullptr;
    result = user_login(&bob_session, instance, user2_idx, "pass_bob");
    if (result != static_cast<int>(OFSErrorCodes::SUCCESS)) {
        cout << "✗ Bob login failed\n";
        fs_shutdown(instance);
        return 1;
    }
    cout << "✓ Bob logged in\n\n";
    
    cout << "[18] Bob creating his directory structure...\n";
    cout << "   (Bob's paths are isolated to /users/bob/...)\n";
    dir_create(bob_session, "/projects");
    dir_create(bob_session, "/projects/code");
    cout << "\n";
    
    cout << "[19] Bob creating files in his workspace...\n";
    const char* bob_content = "Bob's project code and notes.";
    file_create(bob_session, "/projects/code/main.cpp", bob_content, strlen(bob_content));
    cout << "\n";
    
    cout << "[20] Bob trying to access Alice's file (should fail)...\n";
    file_data = nullptr;
    result = file_read(bob_session, "/users/alice/documents/work/report_backup.txt", &file_data, &file_size);
    if (result != static_cast<int>(OFSErrorCodes::SUCCESS)) {
        cout << "✓ Correctly denied: Bob cannot access Alice's files\n";
        cout << "   Error: " << get_error_message(result) << "\n";
    }
    cout << "\n";
    
    cout << "[21] Bob listing his /projects directory...\n";
    entries = nullptr;
    entry_count = 0;
    result = dir_list(bob_session, "/projects", &entries, &entry_count);
    if (result == static_cast<int>(OFSErrorCodes::SUCCESS)) {
        cout << "✓ Entries in Bob's /projects (actual: /users/bob/projects):\n";
        for (int i = 0; i < entry_count; i++) {
            cout << "  - " << entries[i].name 
                 << " (owner: " << entries[i].owner << ")\n";
        }
    }
    free_buffer(entries);
    cout << "\n";
    
    cout << "[22] Logging out Bob and logging back in as Admin...\n";
    user_logout(bob_session);
    result = user_login(&admin_session, instance, admin_index, "password123");
    cout << "\n";
    
    cout << "[23] Admin accessing Alice's files directly...\n";
    file_data = nullptr;
    result = file_read(admin_session, "/users/alice/documents/work/report_backup.txt", &file_data, &file_size);
    if (result == static_cast<int>(OFSErrorCodes::SUCCESS)) {
        cout << "✓ Admin can access any user's files:\n";
        cout << "   " << file_data << "\n";
    }
    free_buffer(file_data);
    cout << "\n";
    
    cout << "[24] Admin listing /users directory...\n";
    entries = nullptr;
    entry_count = 0;
    result = dir_list(admin_session, "/users", &entries, &entry_count);
    if (result == static_cast<int>(OFSErrorCodes::SUCCESS)) {
        cout << "✓ User directories:\n";
        for (int i = 0; i < entry_count; i++) {
            cout << "  - " << entries[i].name << " (type: DIR)\n";
        }
    }
    free_buffer(entries);
    cout << "\n";
    
    cout << "[25] Getting filesystem statistics...\n";
    FSStats stats{};
    result = get_stats(admin_session, &stats);
    if (result == static_cast<int>(OFSErrorCodes::SUCCESS)) {
        cout << "✓ Filesystem Stats:\n";
        cout << "  Total size: " << stats.total_size << " bytes\n";
        cout << "  Used space: " << stats.used_space << " bytes\n";
        cout << "  Free space: " << stats.free_space << " bytes\n";
        cout << "  Total files: " << stats.total_files << "\n";
        cout << "  Total directories: " << stats.total_directories << "\n";
        cout << "  Total users: " << stats.total_users << "\n";
        cout << "  Active sessions: " << stats.active_sessions << "\n";
        cout << "  Fragmentation: " << stats.fragmentation << "%\n";
    }
    cout << "\n";
    
    cout << "[26] Admin creating file in shared space...\n";
    const char* content3 = "Shared content accessible by admin.";
    file_create(admin_session, "/shared/readme.txt", content3, strlen(content3));
    file_truncate(admin_session, "/shared/readme.txt");
    cout << "\n";
    
    cout << "[27] Cleanup operations...\n";
    file_delete(admin_session, "/users/alice/documents/personal/notes.txt");
    dir_delete(admin_session, "/users/alice/documents/personal");
    cout << "\n";
    
    cout << "[28] Deleting user Bob (and his home directory will be orphaned)...\n";
    user_delete(admin_session, user2_idx);
    cout << "   Note: /users/bob directory remains but user cannot login\n\n";
    
    cout << "[29] Final filesystem statistics...\n";
    FSStats final_stats{};
    get_stats(admin_session, &final_stats);
    cout << "✓ Final Stats:\n";
    cout << "  Total files: " << final_stats.total_files << "\n";
    cout << "  Total directories: " << final_stats.total_directories << "\n";
    cout << "  Total users: " << final_stats.total_users << "\n";
    cout << "\n";
    
    cout << "[30] Logging out and shutting down...\n";
    user_logout(admin_session);
    fs_shutdown(instance);
    cout << "✓ Shutdown complete\n\n";
    
    cout << string(70, '=') << "\n";
    cout << "  User Isolation Testing Complete!\n";
    cout << "  Key Features Demonstrated:\n";
    cout << "  - Each user has isolated /users/<username>/ directory\n";
    cout << "  - Regular users can only access their own files\n";
    cout << "  - Admin can access all files system-wide\n";
    cout << "  - Path resolution is transparent to users\n";
    cout << string(70, '=') << "\n\n";
    
    return 0;
}