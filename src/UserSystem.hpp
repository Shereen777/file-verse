#ifndef USER_SYSTEM_HPP
#define USER_SYSTEM_HPP

#include "../include/odf_types.hpp"
#include <iostream>
#include <string>
#include <cstring>
#include "AVL.hpp"

using namespace std;

struct UserSystem {
    UserAVL tree;
    
    uint32_t add_user(const UserInfo& user) {
        if (tree.user_exists(user.user_index)) {
            cout << "✗ Username '" << user.username << "' already exists\n";
            return 0;
        }
        
        if (tree.find_by_index(user.user_index) != nullptr) {
            cout << "✗ Index " << user.user_index << " already exists\n";
            return 0;
        }
        
        tree.insert(user);
        
        cout << "✓ User created: " << user.username 
             << " (Index: " << user.user_index << ")\n";
        
        return user.user_index;
    }
    
    UserInfo* find_user_by_index(uint32_t index) {
        return tree.find_by_index(index);
    }
    
    bool remove(uint32_t user_index) {
        UserInfo* user = tree.find_by_index(user_index);
        if (!user) {
            cout << "✗ User with index " << user_index << " not found\n";
            return false;
        }
        
        // Mark as inactive instead of removing from tree
        user->is_active = 0;
        cout << "✓ User removed: " << user->username << " (Index: " << user_index << ")\n";
        
        return true;
    }
    
    int get_user_count() {
        return tree.count_active();
    }
    
    void get_all_users(UserInfo** users, int* count) {
        int active_count = tree.count_active();
        
        if (active_count == 0) {
            *users = nullptr;
            *count = 0;
            return;
        }
        
        UserInfo* arr = new UserInfo[active_count];
        int idx = 0;
        tree.get_all_active(arr, idx);
        
        *users = arr;
        *count = idx;
    }
};

#endif