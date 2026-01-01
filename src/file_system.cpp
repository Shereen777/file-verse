#include "../include/odf_types.hpp"
#include <fstream>
#include <sstream>
#include <cstring>
#include <ctime>
#include <vector>
#include <algorithm>
#include <random>
#include <iostream>
#include "IndexGenerator.hpp"
#include "AVL.hpp"
#include "UserSystem.hpp"
#include "FreeSpaceManager.hpp"
#include "FileSystem.hpp"
#include "Session_Instance.hpp"

using namespace std;

mt19937 IndexGenerator::rng;
uniform_int_distribution<uint32_t> IndexGenerator::dist(100000, 999999);

bool parse_config(OMNIHeader& header, const string& config_path) {
    ifstream file(config_path);
    if (!file.is_open()) return false;

    string line, section;
    memset(&header, 0, sizeof(OMNIHeader));
    memcpy(header.magic, "OMNIFS01", 8);
    header.format_version = 0x00010000;

    while (getline(file, line)) {
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        if (line.empty() || line[0] == '#') continue;

        if (line.front() == '[' && line.back() == ']') {
            section = line.substr(1, line.size() - 2);
            transform(section.begin(), section.end(), section.begin(), ::tolower);
            continue;
        }

        size_t eq_pos = line.find('=');
        if (eq_pos == string::npos) continue;

        string key = line.substr(0, eq_pos);
        string value = line.substr(eq_pos + 1);

        auto trim = [](string& s) {
            s.erase(0, s.find_first_not_of(" \t\r\n\""));
            s.erase(s.find_last_not_of(" \t\r\n\"") + 1);
        };
        trim(key);
        trim(value);

        if (section == "filesystem") {
            if (key == "total_size") header.total_size = stoull(value);
            else if (key == "header_size") header.header_size = stoull(value);
            else if (key == "block_size") header.block_size = stoul(value);
            else if (key == "max_users") header.max_users = stoul(value);
        }
        else if (section == "security") {
            if (key == "max_users") header.max_users = stoul(value);
            else if (key == "admin_username")
                strncpy(header.admin_username, value.c_str(), sizeof(header.admin_username) - 1);
            else if (key == "admin_password")
                strncpy(header.admin_password, value.c_str(), sizeof(header.admin_password) - 1);
            else if (key == "require_auth")
                header.require_auth = (value == "true" || value == "1");
        }
    }

    time_t t = time(nullptr);
    tm* tm_info = localtime(&t);
    strftime(header.submission_date, sizeof(header.submission_date), "%Y-%m-%d", tm_info);
    header.user_table_offset = static_cast<uint32_t>(header.header_size);
    header.config_timestamp = static_cast<uint64_t>(t);
    memset(header.config_hash, 0, sizeof(header.config_hash));

    return true;
}

string generate_session_id() {
    static int counter = 0;
    time_t now = time(nullptr);
    return "SESSION_" + to_string(now) + "_" + to_string(counter++);
}

int fs_init(void** instance, const char* omni_path, const char* config_path) {
    OMNIInstance* inst = new OMNIInstance();
    inst->omni_path = omni_path;
    
    if (!parse_config(inst->header, config_path)) {
        delete inst;
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_CONFIG);
    }

    std::string admin_username = inst->header.admin_username;
    std::string admin_password = inst->header.admin_password;

    inst->omni_file.open(omni_path, ios::in | ios::out | ios::binary);
    bool file_exists = inst->omni_file.is_open();
    
    if (file_exists) {
        inst->omni_file.seekg(0, ios::beg);
        inst->omni_file.read(reinterpret_cast<char*>(&inst->header), sizeof(OMNIHeader));
        
        if (memcmp(inst->header.magic, "OMNIFS01", 8) != 0) {
            inst->omni_file.close();
            delete inst;
            return static_cast<int>(OFSErrorCodes::ERROR_IO_ERROR);
        }
        
        inst->omni_file.seekg(inst->header.user_table_offset, ios::beg);
        uint32_t num_users = 0;
        inst->omni_file.read(reinterpret_cast<char*>(&num_users), sizeof(uint32_t));
        
        for (uint32_t i = 0; i < num_users; i++) {
            UserInfo user;
            inst->omni_file.read(reinterpret_cast<char*>(&user), sizeof(UserInfo));
            if (user.is_active) {
                inst->user_system.add_user(user);
            }
        }

        if (inst->user_system.get_user_count() == 0) {
            UserInfo admin{};
            strncpy(admin.username, admin_username.c_str(), sizeof(admin.username) - 1);
            strncpy(admin.password_hash, admin_password.c_str(), sizeof(admin.password_hash) - 1);
            admin.role = UserRole::ADMIN;
            admin.is_active = 1;
            admin.created_time = time(nullptr);
            admin.last_login = 0;
            IndexGenerator::initialize();
            admin.user_index = IndexGenerator::generate();
            memset(admin.reserved, 0, sizeof(admin.reserved));

            inst->user_system.add_user(admin);
            inst->omni_file.seekp(inst->header.user_table_offset, ios::beg);
            uint32_t new_user_count = 1;
            inst->omni_file.write(reinterpret_cast<const char*>(&new_user_count), sizeof(uint32_t));
            inst->omni_file.write(reinterpret_cast<const char*>(&admin), sizeof(UserInfo));
            inst->omni_file.flush();
        }

    } else {
        inst->omni_file.open(omni_path, ios::out | ios::binary);
        if (!inst->omni_file.is_open()) {
            delete inst;
            return static_cast<int>(OFSErrorCodes::ERROR_IO_ERROR);
        }
        inst->omni_file.close();
        inst->omni_file.open(omni_path, ios::in | ios::out | ios::binary);

        inst->omni_file.seekp(0, ios::beg);
        inst->omni_file.write(reinterpret_cast<const char*>(&inst->header), sizeof(OMNIHeader));

        UserInfo admin{};
        strncpy(admin.username, admin_username.c_str(), sizeof(admin.username) - 1);
        strncpy(admin.password_hash, admin_password.c_str(), sizeof(admin.password_hash) - 1);
        admin.role = UserRole::ADMIN;
        admin.is_active = 1;
        admin.created_time = time(nullptr);
        admin.last_login = 0;
        IndexGenerator::initialize();
        admin.user_index = IndexGenerator::generate();
        memset(admin.reserved, 0, sizeof(admin.reserved));

        inst->user_system.add_user(admin);

        uint32_t num_users = 1;
        inst->omni_file.seekp(inst->header.user_table_offset, ios::beg);
        inst->omni_file.write(reinterpret_cast<const char*>(&num_users), sizeof(uint32_t));
        inst->omni_file.write(reinterpret_cast<const char*>(&admin), sizeof(UserInfo));
        inst->omni_file.flush();
    }
    
    inst->file_open = true;
    
    uint64_t data_size = inst->header.total_size - inst->get_data_offset();
    uint32_t num_blocks = data_size / inst->header.block_size;
    inst->free_space.initialize(num_blocks);
    
    *instance = inst;
    return static_cast<int>(OFSErrorCodes::SUCCESS);
}

void fs_shutdown(void* instance) {
    if (!instance) return;
    OMNIInstance* inst = static_cast<OMNIInstance*>(instance);
    delete inst;
}

int fs_format(const char* omni_path, const char* config_path) {
    void* instance;
    return fs_init(&instance, omni_path, config_path);
}

int user_login(void** session, void* instance, uint32_t user_index, const char* password) {
    if (!instance || !password) {
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION);
    }
    
    OMNIInstance* inst = static_cast<OMNIInstance*>(instance);
    UserInfo* user = inst->user_system.find_user_by_index(user_index);
    
    if (!user || !user->is_active) {
        cout << "✗ Login failed: User index " << user_index << " not found\n";
        return static_cast<int>(OFSErrorCodes::ERROR_NOT_FOUND);
    }
    
    if (strcmp(user->password_hash, password) != 0) {
        cout << "✗ Login failed: Invalid password for index " << user_index << "\n";
        return static_cast<int>(OFSErrorCodes::ERROR_PERMISSION_DENIED);
    }
    
    string session_id = generate_session_id();
    Session* sess = new Session(session_id, user, inst);
    inst->sessions.push_back(sess);
    
    user->last_login = time(nullptr);
    
    cout << "✓ Login successful: " << user->username << " (Index: " << user_index << ")\n";
    
    *session = sess;
    return static_cast<int>(OFSErrorCodes::SUCCESS);
}

int user_logout(void* session) {
    if (!session) {
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_SESSION);
    }
    
    Session* sess = static_cast<Session*>(session);
    OMNIInstance* inst = sess->instance;
    
    auto it = find(inst->sessions.begin(), inst->sessions.end(), sess);
    if (it != inst->sessions.end()) {
        inst->sessions.erase(it);
    }
    delete sess;
    
    cout << "✓ User logged out\n";
    return static_cast<int>(OFSErrorCodes::SUCCESS);
}
int user_create(void* session, const char* username, const char* password, UserRole role, uint32_t &out_index) {
    if (!session || !username || !password) {
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION);
    }
    
    Session* sess = static_cast<Session*>(session);
    
    if (sess->user->role != UserRole::ADMIN) {
        return static_cast<int>(OFSErrorCodes::ERROR_PERMISSION_DENIED);
    }
    
    OMNIInstance* inst = sess->instance;
    
    uint32_t new_index = IndexGenerator::generate();
    while (inst->user_system.find_user_by_index(new_index) != nullptr) {
        new_index = IndexGenerator::generate();
    }
    
    UserInfo new_user{};
    strncpy(new_user.username, username, sizeof(new_user.username) - 1);
    strncpy(new_user.password_hash, password, sizeof(new_user.password_hash) - 1);
    new_user.role = role;
    new_user.user_index = new_index;
    new_user.is_active = 1;
    new_user.created_time = time(nullptr);
    new_user.last_login = 0;
    memset(new_user.reserved, 0, sizeof(new_user.reserved));
    
    uint32_t result_idx = inst->user_system.add_user(new_user);
    if (result_idx == 0) {
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION);
    }
    
    // NEW: Create user's home directory
    if (!inst->file_system.create_user_directory(username)) {
        // Rollback user creation if directory creation fails
        inst->user_system.tree.remove(new_index);
        return static_cast<int>(OFSErrorCodes::ERROR_IO_ERROR);
    }
    
    out_index = new_index;
    sess->operations_count++;
    sess->last_activity = time(nullptr);
    
    cout << "✓ User directory created: /users/" << username << "\n";
    
    return static_cast<int>(OFSErrorCodes::SUCCESS);
}

int user_delete(void* session, uint32_t user_index) {
    if (!session) {
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION);
    }
    
    Session* sess = static_cast<Session*>(session);
    
    if (sess->user->role != UserRole::ADMIN) {
        return static_cast<int>(OFSErrorCodes::ERROR_PERMISSION_DENIED);
    }
    
    OMNIInstance* inst = sess->instance;
    UserInfo* user = inst->user_system.find_user_by_index(user_index);
    
    if (!user) {
        return static_cast<int>(OFSErrorCodes::ERROR_NOT_FOUND);
    }
    
    user->is_active = 0;
    cout << "✓ User deleted: " << user->username << "\n";
    
    sess->operations_count++;
    sess->last_activity = time(nullptr);
    
    return static_cast<int>(OFSErrorCodes::SUCCESS);
}

int user_list(void* session, UserInfo** users, int* count) {
    if (!session or !users or !count) {
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION);
    }
    
    Session* sess = static_cast<Session*>(session);
    
    if (sess->user->role != UserRole::ADMIN) {
        return static_cast<int>(OFSErrorCodes::ERROR_PERMISSION_DENIED);
    }
    
    OMNIInstance* inst = sess->instance;
    
    // Get all active users - O(n)
    inst->user_system.get_all_users(users, count);
    
    sess->operations_count++;
    sess->last_activity = time(nullptr);
    
    return static_cast<int>(OFSErrorCodes::SUCCESS);
}

int get_session_info(void* session, SessionInfo* info) {
    if (!session || !info) {
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION);
    }
    
    Session* sess = static_cast<Session*>(session);
    
    strncpy(info->session_id, sess->session_id.c_str(), sizeof(info->session_id) - 1);
    info->session_id[sizeof(info->session_id) - 1] = '\0';
    
    info->user = *(sess->user);
    info->login_time = sess->login_time;
    info->last_activity = sess->last_activity;
    info->operations_count = sess->operations_count;
    
    memset(info->reserved, 0, sizeof(info->reserved));
    
    return static_cast<int>(OFSErrorCodes::SUCCESS);
}

int file_create(void* session, const char* path, const char* data, size_t size) {
    if (!session || !path) {
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION);
    }
    
    Session* sess = static_cast<Session*>(session);
    OMNIInstance* inst = sess->instance;
    
    FSNode* existing = inst->file_system.find_node(path);
    if (existing) {
        return static_cast<int>(OFSErrorCodes::ERROR_FILE_EXISTS);
    }
    
    FSNode* node = inst->file_system.create_node(path, EntryType::FILE, sess->user->username);
    if (!node) {
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_PATH);
    }
    
    if (data && size > 0) {
        uint32_t blocks_needed = (size + inst->header.block_size - 1) / inst->header.block_size;
        int start_block = inst->free_space.allocate_blocks(blocks_needed);
        
        if (start_block == -1) {
            inst->file_system.delete_node(path);
            return static_cast<int>(OFSErrorCodes::ERROR_NO_SPACE);
        }
        
        node->start_block = start_block;
        node->num_blocks = blocks_needed;
        node->size = size;
        
        uint64_t offset = inst->get_data_offset() + (start_block * inst->header.block_size);
        inst->omni_file.seekp(offset, ios::beg);
        inst->omni_file.write(data, size);
        inst->omni_file.flush();
    }
    
    cout << "✓ File created: " << path << " (" << size << " bytes)\n";
    sess->operations_count++;
    sess->last_activity = time(nullptr);
    
    return static_cast<int>(OFSErrorCodes::SUCCESS);
}

int file_read(void* session, const char* path, char** buffer, size_t* size) {
    if (!session || !path || !buffer || !size) {
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION);
    }
    
    Session* sess = static_cast<Session*>(session);
    OMNIInstance* inst = sess->instance;
    
    FSNode* node = inst->file_system.find_node(path);
    if (!node) {
        return static_cast<int>(OFSErrorCodes::ERROR_NOT_FOUND);
    }
    
    if (node->type != EntryType::FILE) {
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION);
    }
    
    char* data = (char*)malloc(node->size + 1);
    if (!data) {
        return static_cast<int>(OFSErrorCodes::ERROR_IO_ERROR);
    }
    
    if (node->size > 0) {
        uint64_t offset = inst->get_data_offset() + (node->start_block * inst->header.block_size);
        inst->omni_file.seekg(offset, ios::beg);
        inst->omni_file.read(data, node->size);
    }
    
    data[node->size] = '\0';
    *buffer = data;
    *size = node->size;
    
    cout << "✓ File read: " << path << " (" << node->size << " bytes)\n";
    sess->operations_count++;
    sess->last_activity = time(nullptr);
    
    return static_cast<int>(OFSErrorCodes::SUCCESS);
}

int file_edit(void* session, const char* path, const char* data, size_t size, uint index) {
    if (!session || !path || !data) {
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION);
    }
    
    Session* sess = static_cast<Session*>(session);
    OMNIInstance* inst = sess->instance;
    
    FSNode* node = inst->file_system.find_node(path);
    if (!node) {
        return static_cast<int>(OFSErrorCodes::ERROR_NOT_FOUND);
    }
    
    if (node->type != EntryType::FILE) {
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION);
    }
    
    if (node->owner != sess->user->username && sess->user->role != UserRole::ADMIN) {
        return static_cast<int>(OFSErrorCodes::ERROR_PERMISSION_DENIED);
    }
    
    if (index + size > node->size) {
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION);
    }
    
    uint64_t offset = inst->get_data_offset() + (node->start_block * inst->header.block_size) + index;
    inst->omni_file.seekp(offset, ios::beg);
    inst->omni_file.write(data, size);
    inst->omni_file.flush();
    
    node->modified_time = time(nullptr);
    
    cout << "✓ File edited: " << path << " (offset: " << index << ", size: " << size << ")\n";
    sess->operations_count++;
    sess->last_activity = time(nullptr);
    
    return static_cast<int>(OFSErrorCodes::SUCCESS);
}

int file_delete(void* session, const char* path) {
    if (!session || !path) {
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION);
    }
    
    Session* sess = static_cast<Session*>(session);
    OMNIInstance* inst = sess->instance;
    
    FSNode* node = inst->file_system.find_node(path);
    if (!node) {
        return static_cast<int>(OFSErrorCodes::ERROR_NOT_FOUND);
    }
    
    if (node->type != EntryType::FILE) {
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION);
    }
    
    if (node->owner != sess->user->username && sess->user->role != UserRole::ADMIN) {
        return static_cast<int>(OFSErrorCodes::ERROR_PERMISSION_DENIED);
    }
    
    if (node->num_blocks > 0) {
        inst->free_space.free_blocks(node->start_block, node->num_blocks);
    }
    
    if (!inst->file_system.delete_node(path)) {
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION);
    }
    
    cout << "✓ File deleted: " << path << "\n";
    sess->operations_count++;
    sess->last_activity = time(nullptr);
    
    return static_cast<int>(OFSErrorCodes::SUCCESS);
}

int file_truncate(void* session, const char* path) {
    if (!session || !path) {
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION);
    }
    
    Session* sess = static_cast<Session*>(session);
    OMNIInstance* inst = sess->instance;
    
    FSNode* node = inst->file_system.find_node(path);
    if (!node) {
        return static_cast<int>(OFSErrorCodes::ERROR_NOT_FOUND);
    }
    
    if (node->type != EntryType::FILE) {
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION);
    }
    
    if (node->owner != sess->user->username && sess->user->role != UserRole::ADMIN) {
        return static_cast<int>(OFSErrorCodes::ERROR_PERMISSION_DENIED);
    }
    
    const char* pattern = "siruamr";
    size_t pattern_len = strlen(pattern);
    
    if (node->size > 0) {
        uint64_t offset = inst->get_data_offset() + (node->start_block * inst->header.block_size);
        inst->omni_file.seekp(offset, ios::beg);
        
        for (size_t i = 0; i < node->size; i += pattern_len) {
            size_t write_len = min(pattern_len, node->size - i);
            inst->omni_file.write(pattern, write_len);
        }
        inst->omni_file.flush();
    }
    
    node->modified_time = time(nullptr);
    
    cout << "✓ File truncated: " << path << "\n";
    sess->operations_count++;
    sess->last_activity = time(nullptr);
    
    return static_cast<int>(OFSErrorCodes::SUCCESS);
}

int file_exists(void* session, const char* path) {
    if (!session || !path) {
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION);
    }
    
    Session* sess = static_cast<Session*>(session);
    OMNIInstance* inst = sess->instance;
    
    FSNode* node = inst->file_system.find_node(path);
    if (!node || node->type != EntryType::FILE) {
        return static_cast<int>(OFSErrorCodes::ERROR_NOT_FOUND);
    }
    
    sess->operations_count++;
    sess->last_activity = time(nullptr);
    
    return static_cast<int>(OFSErrorCodes::SUCCESS);
}

int file_rename(void* session, const char* old_path, const char* new_path) {
    if (!session || !old_path || !new_path) {
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION);
    }
    
    Session* sess = static_cast<Session*>(session);
    OMNIInstance* inst = sess->instance;
    
    FSNode* node = inst->file_system.find_node(old_path);
    if (!node) {
        return static_cast<int>(OFSErrorCodes::ERROR_NOT_FOUND);
    }
    
    if (inst->file_system.find_node(new_path)) {
        return static_cast<int>(OFSErrorCodes::ERROR_FILE_EXISTS);
    }
    
    if (node->owner != sess->user->username && sess->user->role != UserRole::ADMIN) {
        return static_cast<int>(OFSErrorCodes::ERROR_PERMISSION_DENIED);
    }
    
    size_t last_slash = new_path ? string(new_path).find_last_of('/') : string::npos;
    string new_name = new_path ? string(new_path).substr(last_slash + 1) : "";
    
    node->name = new_name;
    node->modified_time = time(nullptr);
    
    cout << "✓ File renamed: " << old_path << " -> " << new_path << "\n";
    sess->operations_count++;
    sess->last_activity = time(nullptr);
    
    return static_cast<int>(OFSErrorCodes::SUCCESS);
}

int dir_create(void* session, const char* path) {
    if (!session || !path) {
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION);
    }
    
    Session* sess = static_cast<Session*>(session);
    OMNIInstance* inst = sess->instance;
    
    FSNode* existing = inst->file_system.find_node(path);
    if (existing) {
        return static_cast<int>(OFSErrorCodes::ERROR_FILE_EXISTS);
    }
    
    FSNode* node = inst->file_system.create_node(path, EntryType::DIRECTORY, sess->user->username);
    if (!node) {
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_PATH);
    }
    
    cout << "✓ Directory created: " << path << "\n";
    sess->operations_count++;
    sess->last_activity = time(nullptr);
    
    return static_cast<int>(OFSErrorCodes::SUCCESS);
}

int dir_list(void* session, const char* path, FileEntry** entries, int* count) {
    if (!session || !path || !entries || !count) {
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION);
    }

    Session* sess = static_cast<Session*>(session);
    OMNIInstance* inst = sess->instance;

    FSNode* node = inst->file_system.find_node(path);
    if (!node) {
        return static_cast<int>(OFSErrorCodes::ERROR_NOT_FOUND);
    }

    if (node->type != EntryType::DIRECTORY) {
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION);
    }

    // ✅ Collect all children using inorder traversal
    std::vector<FSNode*> children;
    node->children.inorder_collect(node->children.getRoot(),children);

    int num_children = children.size();
    FileEntry* entry_array = (FileEntry*)malloc(num_children * sizeof(FileEntry));
    if (!entry_array && num_children > 0) {
        return static_cast<int>(OFSErrorCodes::ERROR_IO_ERROR);
    }

    for (int i = 0; i < num_children; i++) {
        FSNode* child = children[i];

        strncpy(entry_array[i].name, child->name.c_str(), sizeof(entry_array[i].name) - 1);
        entry_array[i].name[sizeof(entry_array[i].name) - 1] = '\0';

        entry_array[i].type = static_cast<uint8_t>(child->type);
        entry_array[i].size = child->size;
        entry_array[i].permissions = child->permissions;
        entry_array[i].created_time = child->created_time;
        entry_array[i].modified_time = child->modified_time;

        strncpy(entry_array[i].owner, child->owner.c_str(), sizeof(entry_array[i].owner) - 1);
        entry_array[i].owner[sizeof(entry_array[i].owner) - 1] = '\0';

        entry_array[i].inode = child->inode;
        memset(entry_array[i].reserved, 0, sizeof(entry_array[i].reserved));
    }

    *entries = entry_array;
    *count = num_children;

    std::cout << "✓ Directory listed: " << path << " (" << num_children << " entries)\n";
    sess->operations_count++;
    sess->last_activity = time(nullptr);

    return static_cast<int>(OFSErrorCodes::SUCCESS);
}



int dir_delete(void* session, const char* path) {
    if (!session || !path) {
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION);
    }
    
    Session* sess = static_cast<Session*>(session);
    OMNIInstance* inst = sess->instance;
    
    FSNode* node = inst->file_system.find_node(path);
    if (!node) {
        return static_cast<int>(OFSErrorCodes::ERROR_NOT_FOUND);
    }
    
    if (node->type != EntryType::DIRECTORY) {
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION);
    }
    
    if (!node->children.empty()) {
        return static_cast<int>(OFSErrorCodes::ERROR_DIRECTORY_NOT_EMPTY);
    }
    
    if (node->owner != sess->user->username && sess->user->role != UserRole::ADMIN) {
        return static_cast<int>(OFSErrorCodes::ERROR_PERMISSION_DENIED);
    }
    
    if (!inst->file_system.delete_node(path)) {
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION);
    }
    
    cout << "✓ Directory deleted: " << path << "\n";
    sess->operations_count++;
    sess->last_activity = time(nullptr);
    
    return static_cast<int>(OFSErrorCodes::SUCCESS);
}

int dir_exists(void* session, const char* path) {
    if (!session || !path) {
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION);
    }
    
    Session* sess = static_cast<Session*>(session);
    OMNIInstance* inst = sess->instance;
    
    FSNode* node = inst->file_system.find_node(path);
    if (!node || node->type != EntryType::DIRECTORY) {
        return static_cast<int>(OFSErrorCodes::ERROR_NOT_FOUND);
    }
    
    sess->operations_count++;
    sess->last_activity = time(nullptr);
    
    return static_cast<int>(OFSErrorCodes::SUCCESS);
}


void count_files_recursive(FSNode* node, uint32_t& files, uint32_t& dirs, uint64_t& total_size) {
    if (!node) return;

    if (node->type == EntryType::DIRECTORY) {
        dirs++;

        // Collect children from AVL tree
        std::vector<FSNode*> children;
        node->children.inorder_collect(node->children.getRoot(),children);  // Assuming 'children_root' is your AVL root

        for (auto* child : children) {
            count_files_recursive(child, files, dirs, total_size);
        }
    } else {
        files++;
        total_size += node->size;
    }
}


int get_metadata(void* session, const char* path, FileMetadata* meta) {
    if (!session || !path || !meta) {
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION);
    }
    
    Session* sess = static_cast<Session*>(session);
    OMNIInstance* inst = sess->instance;
    
    FSNode* node = inst->file_system.find_node(path);
    if (!node) {
        return static_cast<int>(OFSErrorCodes::ERROR_NOT_FOUND);
    }
    
    strncpy(meta->path, path, sizeof(meta->path) - 1);
    meta->path[sizeof(meta->path) - 1] = '\0';
    
    strncpy(meta->entry.name, node->name.c_str(), sizeof(meta->entry.name) - 1);
    meta->entry.name[sizeof(meta->entry.name) - 1] = '\0';
    
    meta->entry.type = static_cast<uint8_t>(node->type);
    meta->entry.size = node->size;
    meta->entry.permissions = node->permissions;
    meta->entry.created_time = node->created_time;
    meta->entry.modified_time = node->modified_time;
    
    strncpy(meta->entry.owner, node->owner.c_str(), sizeof(meta->entry.owner) - 1);
    meta->entry.owner[sizeof(meta->entry.owner) - 1] = '\0';
    
    meta->entry.inode = node->inode;
    meta->blocks_used = node->num_blocks;
    meta->actual_size = node->num_blocks * inst->header.block_size;
    
    memset(meta->entry.reserved, 0, sizeof(meta->entry.reserved));
    memset(meta->reserved, 0, sizeof(meta->reserved));
    
    sess->operations_count++;
    sess->last_activity = time(nullptr);
    
    return static_cast<int>(OFSErrorCodes::SUCCESS);
}

int set_permissions(void* session, const char* path, uint32_t permissions) {
    if (!session || !path) {
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION);
    }
    
    Session* sess = static_cast<Session*>(session);
    OMNIInstance* inst = sess->instance;
    
    FSNode* node = inst->file_system.find_node(path);
    if (!node) {
        return static_cast<int>(OFSErrorCodes::ERROR_NOT_FOUND);
    }
    
    if (node->owner != sess->user->username && sess->user->role != UserRole::ADMIN) {
        return static_cast<int>(OFSErrorCodes::ERROR_PERMISSION_DENIED);
    }
    
    node->permissions = permissions;
    node->modified_time = time(nullptr);
    
    cout << "✓ Permissions set: " << path << " (0o" << oct << permissions << dec << ")\n";
    sess->operations_count++;
    sess->last_activity = time(nullptr);
    
    return static_cast<int>(OFSErrorCodes::SUCCESS);
}

int get_stats(void* session, FSStats* stats) {
    if (!session || !stats) {
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION);
    }
    
    Session* sess = static_cast<Session*>(session);
    OMNIInstance* inst = sess->instance;
    
    uint32_t total_files = 0;
    uint32_t total_dirs = 0;
    uint64_t used_size = 0;
    
    count_files_recursive(inst->file_system.get_root(), total_files, total_dirs, used_size);
    
    stats->total_size = inst->header.total_size;
    stats->used_space = used_size;
    stats->free_space = inst->free_space.get_free_blocks() * inst->header.block_size;
    stats->total_files = total_files;
    stats->total_directories = total_dirs > 0 ? total_dirs - 1 : 0;
    stats->total_users = inst->user_system.get_user_count();
    stats->active_sessions = inst->sessions.size();
    
    uint32_t total_blocks = inst->free_space.get_total_blocks();
    uint32_t used_blocks = total_blocks - inst->free_space.get_free_blocks();
    stats->fragmentation = (total_blocks > 0) ? 
        (static_cast<double>(used_blocks) / total_blocks * 100.0) : 0.0;
    
    memset(stats->reserved, 0, sizeof(stats->reserved));
    
    sess->operations_count++;
    sess->last_activity = time(nullptr);
    
    return static_cast<int>(OFSErrorCodes::SUCCESS);
}

void free_buffer(void* buffer) {
    if (buffer) {
        free(buffer);
    }
}

const char* get_error_message(int error_code) {
    switch (static_cast<OFSErrorCodes>(error_code)) {
        case OFSErrorCodes::SUCCESS:
            return "Operation completed successfully";
        case OFSErrorCodes::ERROR_NOT_FOUND:
            return "File/directory/user not found";
        case OFSErrorCodes::ERROR_PERMISSION_DENIED:
            return "Permission denied";
        case OFSErrorCodes::ERROR_IO_ERROR:
            return "I/O error occurred";
        case OFSErrorCodes::ERROR_INVALID_PATH:
            return "Invalid path";
        case OFSErrorCodes::ERROR_FILE_EXISTS:
            return "File/directory already exists";
        case OFSErrorCodes::ERROR_NO_SPACE:
            return "Insufficient space";
        case OFSErrorCodes::ERROR_INVALID_CONFIG:
            return "Invalid configuration";
        case OFSErrorCodes::ERROR_NOT_IMPLEMENTED:
            return "Feature not implemented";
        case OFSErrorCodes::ERROR_INVALID_SESSION:
            return "Invalid session";
        case OFSErrorCodes::ERROR_DIRECTORY_NOT_EMPTY:
            return "Directory not empty";
        case OFSErrorCodes::ERROR_INVALID_OPERATION:
            return "Invalid operation";
        default:
            return "Unknown error";
    }
}