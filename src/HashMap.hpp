#ifndef HASHMAP_HPP
#define HASHMAP_HPP

#include <vector>
#include <utility>
#include <functional>
#include <stdexcept>
#include <algorithm>
#include "SinglyLinkedList.hpp"

using namespace std;

template <typename K, typename V>
class HashMap {
private:
    struct Entry {
        K key;
        V value;
        Entry(const K& k, const V& v) : key(k), value(v) {}
    };

    vector<LinkedList<Entry>> table;
    size_t current_size;
    float max_load_factor;

    size_t hash_key(const K& key) const {
        return std::hash<K>{}(key) % table.size();
    }

    void rehash() {
        size_t new_capacity = table.size() * 2;
        vector<LinkedList<Entry>> new_table(new_capacity);

        for (auto& bucket : table) {
            for (auto& entry : bucket) {
                size_t new_index = std::hash<K>{}(entry.key) % new_capacity;
                new_table[new_index].push_back(entry);
            }
        }
        table.swap(new_table);
    }

public:
    HashMap(size_t capacity = 16, float load_factor = 0.75f)
        : table(capacity), current_size(0), max_load_factor(load_factor) {}

    bool insert(const K& key, const V& value) {
        size_t index = hash_key(key);
        
        for (auto& entry : table[index]) {
            if (entry.key == key) {
                entry.value = value;
                return false;
            }
        }
        
        table[index].emplace_back(key, value);
        current_size++;

        if ((float)current_size / table.size() > max_load_factor)
            rehash();

        return true;
    }

    bool contains(const K& key) const {
        size_t index = hash_key(key);
        for (const auto& entry : table[index]) {
            if (entry.key == key)
                return true;
        }
        return false;
    }

    bool erase(const K& key) {
        size_t index = hash_key(key);
        auto& bucket = table[index];
        
        for (auto it = bucket.begin(); it != bucket.end(); ++it) {
            if (it->key == key) {
                bucket.erase(it);
                current_size--;
                return true;
            }
        }
        return false;
    }

    V* get(const K& key) {
        size_t index = hash_key(key);
        for (auto& entry : table[index]) {
            if (entry.key == key)
                return &entry.value;
        }
        return nullptr;
    }

    const V* get(const K& key) const {
        size_t index = hash_key(key);
        for (const auto& entry : table[index]) {
            if (entry.key == key)
                return &entry.value;
        }
        return nullptr;
    }

    vector<K> keys() const {
        vector<K> result;
        result.reserve(current_size);
        for (const auto& bucket : table) {
            for (const auto& entry : bucket) {
                result.push_back(entry.key);
            }
        }
        return result;
    }

    size_t size() const {
        return current_size;
    }

    void clear() {
        for (auto& bucket : table)
            bucket.clear();
        current_size = 0;
    }
};

#endif