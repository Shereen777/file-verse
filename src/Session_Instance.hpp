#ifndef Session_HPP
#define Session_HPP

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
using namespace std;
// ============================================================================
// SESSION & INSTANCE
// ============================================================================

struct OMNIInstance;

struct Session {
    string session_id;
    UserInfo* user;
    OMNIInstance* instance;
    uint64_t login_time;
    uint64_t last_activity;
    uint32_t operations_count;
    
    Session(const string& id, UserInfo* u, OMNIInstance* inst) 
        : session_id(id), user(u), instance(inst), operations_count(0) {
        login_time = time(nullptr);
        last_activity = login_time;
    }
};

struct OMNIInstance {
    OMNIHeader header;
    fstream omni_file;
    string omni_path;
    
    UserSystem user_system;
    FileSystem file_system;
    FreeSpaceManager free_space;
    
    vector<Session*> sessions;
    
    bool file_open;
    uint32_t admin_index;
    
    OMNIInstance() : file_open(false), admin_index(0) {}
    
    ~OMNIInstance() {
        for (auto* sess : sessions) {
            delete sess;
        }
        if (file_open && omni_file.is_open()) {
            omni_file.close();
        }
    }
    
    uint64_t get_data_offset() const {
        return header.user_table_offset + (header.max_users * sizeof(UserInfo)) + 1024;
    }
};

#endif