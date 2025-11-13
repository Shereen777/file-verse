#pragma once
#include <string>
#include <vector>
#include "odf_types.hpp"
using namespace std;

struct UserTable {
    vector<UserInfo> users;
    int next_id;

    UserTable() : next_id(0) {}

    int add_user(const string &username, const string &password, UserRole role) {
        if (username.empty() || password.empty()) {
            return -1;
        }

        int idx = users.size();
        users.push_back(UserInfo(username, password, role, next_id++));
        return idx;
    }

    UserInfo* get_user(int index) {
        if (index < 0 || index >= users.size()) {
            return nullptr;
        }
        return &users[index];
    }

    bool deactivate_user(int index) {
        if (index < 0 || index >= users.size()) {
            return false;
        }
        users[index].is_active = false;
        return true;
    }

    int size() const {
        return users.size();
    }
};

struct HashEntry {
    string key;      // username
    int index;       // user index in users[]
    bool occupied;
    bool deleted;    // tombstone for proper probing

    HashEntry() : key(""), index(-1), occupied(false), deleted(false) {}
};

const int HASH_SIZE = 211; // prime number recommended

struct UsernameHashTable {
    vector<HashEntry> table;
    int size;        // number of occupied entries

    UsernameHashTable() : size(0) {
        table.resize(HASH_SIZE);
    }

    int hash(const string &key) const {
        unsigned long h = 5381;
        for (char c : key)
            h = ((h << 5) + h) + c;
        return h % HASH_SIZE;
    }

    bool insert(const string &key, int index) {
        // Check if table is too full (load factor > 0.7)
        if (size >= HASH_SIZE * 0.7) {
            return false; // Should rehash in production
        }

        int h = hash(key);
        int start = h;
        int first_deleted = -1;

        do {
            // If slot is empty or deleted, we can use it
            if (!table[h].occupied) {
                // Use first deleted slot if we found one, otherwise current slot
                int insert_pos = (first_deleted != -1) ? first_deleted : h;
                table[insert_pos].key = key;
                table[insert_pos].index = index;
                table[insert_pos].occupied = true;
                table[insert_pos].deleted = false;
                size++;
                return true;
            }
            
            // Track first deleted slot for reuse
            if (table[h].deleted && first_deleted == -1) {
                first_deleted = h;
            }
            
            // Check for duplicate key
            if (table[h].key == key && !table[h].deleted) {
                return false; // Key already exists
            }
            
            h = (h + 1) % HASH_SIZE; // linear probing
        } while (h != start);
        
        return false; // table full
    }

    int search(const string &key) const {
        int h = hash(key);
        int start = h;
        
        do {
            // If we hit an empty slot that was never used, key doesn't exist
            if (!table[h].occupied && !table[h].deleted) {
                return -1;
            }
            
            // Found the key and it's not deleted
            if (table[h].occupied && !table[h].deleted && table[h].key == key) {
                return table[h].index;
            }
            
            h = (h + 1) % HASH_SIZE;
        } while (h != start);
        
        return -1;
    }

    bool remove(const string &key) {
        int h = hash(key);
        int start = h;
        
        do {
            // If we hit an empty slot that was never used, key doesn't exist
            if (!table[h].occupied && !table[h].deleted) {
                return false;
            }
            
            // Found the key and it's not already deleted
            if (table[h].occupied && !table[h].deleted && table[h].key == key) {
                table[h].deleted = true;  // Mark as deleted (tombstone)
                size--;
                return true;
            }
            
            h = (h + 1) % HASH_SIZE;
        } while (h != start);
        
        return false;
    }

    // Helper method to check if username exists
    bool exists(const string &key) const {
        return search(key) != -1;
    }
};

struct UserSystem {
    UserTable users;
    UsernameHashTable username_map;

    int register_user(const string &username, const string &password, UserRole role) {
        // Check if username already exists
        if (username_map.exists(username)) {
            return -1; // Username already taken
        }

        // Add user to the user table
        int idx = users.add_user(username, password, role);
        if (idx == -1) {
            return -1; // Failed to add user
        }

        // Insert into hash table
        if (!username_map.insert(username, idx)) {
            // Hash table insertion failed, should rollback user addition
            // (In production, implement proper rollback)
            return -1;
        }

        return idx;
    }

    UserInfo* get_user_by_name(const string &username) {
        int idx = username_map.search(username);
        if (idx == -1 || idx >= users.users.size()) {
            return nullptr;
        }
        return &users.users[idx];
    }

    const UserInfo* get_user_by_name(const string &username) const {
        int idx = username_map.search(username);
        if (idx == -1 || idx >= users.users.size()) {
            return nullptr;
        }
        return &users.users[idx];
    }

    bool delete_user(const string &username) {
        int idx = username_map.search(username);
        if (idx == -1) {
            return false;
        }

        // Remove from hash table
        if (!username_map.remove(username)) {
            return false;
        }
        return true;
    }
};