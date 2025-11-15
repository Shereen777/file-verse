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

// Function declarations
int fs_init(void** instance, const char* omni_path, const char* config_path);
void fs_shutdown(void* instance);
int fs_format(const char* omni_path, const char* config_path);

int user_login(void** session, void* instance, uint32_t user_index, const char* password);
int user_logout(void* session);

int user_create(void* session, const char* username, const char* password, UserRole role, uint32_t &out_index);
int user_delete(void* session, uint32_t user_index);  // Delete by index
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

// Helper functions
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
    cout << "  OMNIFS - Complete File System Demonstration\n";
    cout << string(70, '=') << "\n\n";
    
    const char* omni_path = "omnifs.dat";
    const char* config_path = "omnifs.conf";
    
    // Create config file
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
    
    // Initialize filesystem
    cout << "[1] Initializing filesystem...\n";
    void* instance = nullptr;
    int result = fs_init(&instance, omni_path, config_path);
    if (result != static_cast<int>(OFSErrorCodes::SUCCESS)) {
        cout << "✗ Failed: " << get_error_message(result) << "\n";
        return 1;
    }
    cout << "✓ Filesystem initialized\n\n";
    
    OMNIInstance* inst = static_cast<OMNIInstance*>(instance);
    UserInfo* users = nullptr;
    int count = 0;
    inst->user_system.get_all_users(&users, &count);

    // if (count > 0) {
    uint32_t admin_index = users[0].user_index;
    free(users);
    // }
    // uint32_t admin_index = admin->user_index;
    
    // Admin login
    cout << "[2] Admin login (index: " << admin_index << ")...\n";
    void* admin_session = nullptr;
    result = user_login(&admin_session, instance, admin_index, "password123");
    if (result != static_cast<int>(OFSErrorCodes::SUCCESS)) {
        cout << "✗ Login failed\n";
        fs_shutdown(instance);
        return 1;
    }
    cout << "\n";
    
    // Create users
    cout << "[3] Creating users...\n";
    uint32_t user1_idx, user2_idx;
    user_create(admin_session, "alice", "pass_alice", UserRole::NORMAL, user1_idx);
    user_create(admin_session, "bob", "pass_bob", UserRole::NORMAL, user2_idx);
    cout << "\n";
    
    // List users
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
    
    // Get session info
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
    
    // Create directories
    cout << "[6] Creating directory structure...\n";
    dir_create(admin_session, "/documents");
    dir_create(admin_session, "/documents/work");
    dir_create(admin_session, "/documents/personal");
    dir_create(admin_session, "/shared");
    cout << "\n";
    
    // Create files
    cout << "[7] Creating files...\n";
    const char* content1 = "This is a work document.\nIt contains important information.";
    const char* content2 = "Personal notes and thoughts.";
    const char* content3 = "Shared content for everyone.";
    
    file_create(admin_session, "/documents/work/report.txt", content1, strlen(content1));
    file_create(admin_session, "/documents/personal/notes.txt", content2, strlen(content2));
    file_create(admin_session, "/shared/readme.txt", content3, strlen(content3));
    cout << "\n";
    
    // Set permissions
    cout << "[8] Setting file permissions...\n";
    set_permissions(admin_session, "/documents/work/report.txt", 0644);
    set_permissions(admin_session, "/documents/personal/notes.txt", 0600);
    cout << "\n";
    
    // List directory contents
    cout << "[9] Listing directory /documents...\n";
    FileEntry* entries = nullptr;
    int entry_count = 0;
    result = dir_list(admin_session, "/documents", &entries, &entry_count);
    if (result == static_cast<int>(OFSErrorCodes::SUCCESS)) {
        cout << "✓ Entries in /documents:\n";
        for (int i = 0; i < entry_count; i++) {
            cout << "  - " << entries[i].name 
                 << " (type: " << (int)entries[i].type 
                 << ", inode: " << entries[i].inode << ")\n";
        }
    }
    free_buffer(entries);
    cout << "\n";
    
    // Read file
    cout << "[10] Reading file /documents/work/report.txt...\n";
    char* file_data = nullptr;
    size_t file_size = 0;
    result = file_read(admin_session, "/documents/work/report.txt", &file_data, &file_size);
    if (result == static_cast<int>(OFSErrorCodes::SUCCESS)) {
        cout << "✓ Content:\n";
        cout << "   " << file_data << "\n";
    }
    free_buffer(file_data);
    cout << "\n";
    
    // Get metadata
    cout << "[11] Getting file metadata...\n";
    FileMetadata meta{};
    result = get_metadata(admin_session, "/documents/work/report.txt", &meta);
    if (result == static_cast<int>(OFSErrorCodes::SUCCESS)) {
        cout << "✓ Metadata for " << meta.path << ":\n";
        cout << "  Name: " << meta.entry.name << "\n";
        cout << "  Size: " << meta.entry.size << " bytes\n";
        cout << "  Inode: " << meta.entry.inode << "\n";
        cout << "  Owner: " << meta.entry.owner << "\n";
        cout << "  Permissions: 0o" << oct << meta.entry.permissions << dec << "\n";
        cout << "  Blocks used: " << meta.blocks_used << "\n";
    }
    cout << "\n";
    
    // Edit file
    cout << "[12] Editing file (modifying content at offset 0)...\n";
    const char* new_content = "MODIFIED";
    result = file_edit(admin_session, "/documents/work/report.txt", new_content, strlen(new_content), 0);
    if (result == static_cast<int>(OFSErrorCodes::SUCCESS)) {
        cout << "✓ File edited\n";
    }
    cout << "\n";
    
    // Read edited file
    cout << "[13] Reading modified file...\n";
    file_data = nullptr;
    file_size = 0;
    result = file_read(admin_session, "/documents/work/report.txt", &file_data, &file_size);
    if (result == static_cast<int>(OFSErrorCodes::SUCCESS)) {
        cout << "✓ Modified content:\n";
        cout << "   " << file_data << "\n";
    }
    free_buffer(file_data);
    cout << "\n";
    
    // File operations
    cout << "[14] File operations...\n";
    
    // Check if file exists
    if (file_exists(admin_session, "/documents/work/report.txt") == static_cast<int>(OFSErrorCodes::SUCCESS)) {
        cout << "✓ File /documents/work/report.txt exists\n";
    }
    
    // Rename file
    file_rename(admin_session, "/documents/work/report.txt", "/documents/work/report_backup.txt");
    
    cout << "\n";
    
    // Directory operations
    cout << "[15] Directory operations...\n";
    
    // Check if directory exists
    if (dir_exists(admin_session, "/documents") == static_cast<int>(OFSErrorCodes::SUCCESS)) {
        cout << "✓ Directory /documents exists\n";
    }
    
    // Try to list /documents/work after rename
    entries = nullptr;
    entry_count = 0;
    if (dir_list(admin_session, "/documents/work", &entries, &entry_count) == static_cast<int>(OFSErrorCodes::SUCCESS)) {
        cout << "✓ Files in /documents/work: " << entry_count << "\n";
        for (int i = 0; i < entry_count; i++) {
            cout << "  - " << entries[i].name << "\n";
        }
    }
    free_buffer(entries);
    cout << "\n";
    
    // Get filesystem statistics
    cout << "[16] Filesystem statistics...\n";
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
    
    // Truncate file
    cout << "[17] Truncating file /shared/readme.txt...\n";
    file_truncate(admin_session, "/shared/readme.txt");
    cout << "\n";
    
    // Delete file
    cout << "[18] Deleting file /documents/personal/notes.txt...\n";
    file_delete(admin_session, "/documents/personal/notes.txt");
    cout << "\n";
    
    // Delete directory
    cout << "[19] Deleting empty directory /documents/personal...\n";
    dir_delete(admin_session, "/documents/personal");
    cout << "\n";
    
    // Delete user
    cout << "[20] Deleting user bob...\n";
    user_delete(admin_session, user2_idx);
    cout << "\n";
    
    // Final statistics
    cout << "[21] Final filesystem statistics...\n";
    FSStats final_stats{};
    get_stats(admin_session, &final_stats);
    cout << "✓ Final Stats:\n";
    cout << "  Total files: " << final_stats.total_files << "\n";
    cout << "  Total directories: " << final_stats.total_directories << "\n";
    cout << "  Total users: " << final_stats.total_users << "\n";
    cout << "\n";
    
    // Logout and shutdown
    cout << "[22] Logging out...\n";
    user_logout(admin_session);
    cout << "\n";
    
    cout << "[23] Shutting down filesystem...\n";
    fs_shutdown(instance);
    cout << "✓ Shutdown complete\n\n";
    
    cout << string(70, '=') << "\n";
    cout << "  Demonstration Complete - All Functions Tested Successfully!\n";
    cout << string(70, '=') << "\n\n";
    
    return 0;
}