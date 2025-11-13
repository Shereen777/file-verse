#ifndef USER_SYSTEM_HPP
#define USER_SYSTEM_HPP

#include "../include/odf_types.hpp"
#include <iostream>
#include <string>
#include <cstring>

using namespace std;

// ============================================================================
// SINGLE AVL TREE FOR USER MANAGEMENT (indexed by user_index)
// ============================================================================

struct AVLNode {
    UserInfo user;           // Complete user data stored in the node
    int height;
    AVLNode* left;
    AVLNode* right;
    
    AVLNode(const UserInfo& u) 
        : user(u), height(1), left(nullptr), right(nullptr) {}
};

class UserAVL {
private:
    AVLNode* root;
    
    int height(AVLNode* node) {
        return node ? node->height : 0;
    }
    
    int balance_factor(AVLNode* node) {
        return node ? height(node->left) - height(node->right) : 0;
    }
    
    void update_height(AVLNode* node) {
        if (node) {
            node->height = 1 + max(height(node->left), height(node->right));
        }
    }
    
    AVLNode* rotate_right(AVLNode* y) {
        AVLNode* x = y->left;
        AVLNode* T2 = x->right;
        x->right = y;
        y->left = T2;
        update_height(y);
        update_height(x);
        return x;
    }
    
    AVLNode* rotate_left(AVLNode* x) {
        AVLNode* y = x->right;
        AVLNode* T2 = y->left;
        y->left = x;
        x->right = T2;
        update_height(x);
        update_height(y);
        return y;
    }
    
    AVLNode* insert_helper(AVLNode* node, const UserInfo& user) {
        if (!node) return new AVLNode(user);
        
        uint32_t idx = user.user_index;
        
        if (idx < node->user.user_index)
            node->left = insert_helper(node->left, user);
        else if (idx > node->user.user_index)
            node->right = insert_helper(node->right, user);
        else {
            // Update existing user (e.g., for password change, last_login, etc.)
            node->user = user;
            return node;
        }
        
        update_height(node);
        int balance = balance_factor(node);
        
        // Left Left
        if (balance > 1 && idx < node->left->user.user_index)
            return rotate_right(node);
        
        // Right Right
        if (balance < -1 && idx > node->right->user.user_index)
            return rotate_left(node);
        
        // Left Right
        if (balance > 1 && idx > node->left->user.user_index) {
            node->left = rotate_left(node->left);
            return rotate_right(node);
        }
        
        // Right Left
        if (balance < -1 && idx < node->right->user.user_index) {
            node->right = rotate_right(node->right);
            return rotate_left(node);
        }
        
        return node;
    }
    
    AVLNode* find_helper(AVLNode* node, uint32_t user_index) {
        if (!node) return nullptr;
        
        if (user_index == node->user.user_index) 
            return node;
        
        if (user_index < node->user.user_index) 
            return find_helper(node->left, user_index);
        
        return find_helper(node->right, user_index);
    }
    
    // For username search - O(n) in worst case, but we don't use this often
    AVLNode* find_by_username_helper(AVLNode* node, const string& username) {
        if (!node) return nullptr;
        
        // Check current node
        if (strcmp(node->user.username, username.c_str()) == 0) {
            return node;
        }
        
        // Search both subtrees
        AVLNode* left_result = find_by_username_helper(node->left, username);
        if (left_result) return left_result;
        
        return find_by_username_helper(node->right, username);
    }
    
    void count_active_helper(AVLNode* node, int& count) {
        if (!node) return;
        
        if (node->user.is_active) count++;
        
        count_active_helper(node->left, count);
        count_active_helper(node->right, count);
    }
    
    void collect_active_users_helper(AVLNode* node, UserInfo* arr, int& index) {
        if (!node) return;
        
        collect_active_users_helper(node->left, arr, index);
        
        if (node->user.is_active) {
            arr[index++] = node->user;
        }
        
        collect_active_users_helper(node->right, arr, index);
    }
    
    void delete_tree(AVLNode* node) {
        if (!node) return;
        delete_tree(node->left);
        delete_tree(node->right);
        delete node;
    }
    
public:
    UserAVL() : root(nullptr) {}
    
    ~UserAVL() { 
        delete_tree(root); 
    }
    
    // Insert/Update user - O(log n)
    void insert(const UserInfo& user) {
        root = insert_helper(root, user);
    }
    
    // Find by index - O(log n) - PRIMARY OPERATION
    UserInfo* find_by_index(uint32_t user_index) {
        AVLNode* node = find_helper(root, user_index);
        return node ? &(node->user) : nullptr;
    }
    
    // Find by username - O(n) - RARE ADMIN OPERATION
    // Only used when admin searches by username
    UserInfo* find_by_username(const string& username) {
        AVLNode* node = find_by_username_helper(root, username);
        return node ? &(node->user) : nullptr;
    }
    
    // Check if username exists (for validation during user creation)
    bool username_exists(const string& username) {
        return find_by_username(username) != nullptr;
    }
    
    // Count active users - O(n)
    int count_active() {
        int count = 0;
        count_active_helper(root, count);
        return count;
    }
    
    // Get all active users as array - O(n)
    void get_all_active(UserInfo* arr, int& count) {
        count = 0;
        collect_active_users_helper(root, arr, count);
    }
};

// ============================================================================
// USER SYSTEM WRAPPER
// ============================================================================

struct UserSystem {
    UserAVL tree;
    
    uint32_t add_user(const UserInfo& user) {
        // Check if username already exists (O(n) - but only during creation)
        if (tree.username_exists(user.username)) {
            cout << "✗ Username '" << user.username << "' already exists\n";
            return 0;
        }
        
        // Check if index already exists (O(log n))
        if (tree.find_by_index(user.user_index) != nullptr) {
            cout << "✗ Index " << user.user_index << " already exists\n";
            return 0;
        }
        
        tree.insert(user);
        
        cout << "✓ User created: " << user.username 
             << " (Index: " << user.user_index << ")\n";
        
        return user.user_index;
    }
    
    // Primary lookup - O(log n)
    UserInfo* find_user_by_index(uint32_t index) {
        return tree.find_by_index(index);
    }
    
    // Rare admin operation - O(n)
    UserInfo* find_user_by_username(const string& username) {
        return tree.find_by_username(username);
    }
    
    int get_user_count() {
        return tree.count_active();
    }
    
    // For user_list function
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

#endif // USER_SYSTEM_HPP