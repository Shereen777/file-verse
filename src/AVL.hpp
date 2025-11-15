#ifndef AVL_HPP
#define AVL_HPP

#include "../include/odf_types.hpp"
#include <iostream>
#include <string>
#include <cstring>

using namespace std;

struct AVLNode {
    UserInfo user;
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
            node->user = user;
            return node;
        }
        
        update_height(node);
        int balance = balance_factor(node);
        
        if (balance > 1 && idx < node->left->user.user_index)
            return rotate_right(node);
        
        if (balance < -1 && idx > node->right->user.user_index)
            return rotate_left(node);
        
        if (balance > 1 && idx > node->left->user.user_index) {
            node->left = rotate_left(node->left);
            return rotate_right(node);
        }
        
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
    
    AVLNode* find_min(AVLNode* node) {
        while (node && node->left) {
            node = node->left;
        }
        return node;
    }
    
    AVLNode* remove_helper(AVLNode* node, uint32_t user_index, bool& deleted) {
        if (!node) {
            deleted = false;
            return nullptr;
        }
        
        if (user_index < node->user.user_index) {
            node->left = remove_helper(node->left, user_index, deleted);
        } else if (user_index > node->user.user_index) {
            node->right = remove_helper(node->right, user_index, deleted);
        } else {
            deleted = true;
            
            if (!node->left || !node->right) {
                AVLNode* temp = node->left ? node->left : node->right;
                if (!temp) {
                    delete node;
                    return nullptr;
                }
                AVLNode* to_delete = node;
                node = temp;
                delete to_delete;
            } else {
                AVLNode* temp = find_min(node->right);
                node->user = temp->user;
                node->right = remove_helper(node->right, temp->user.user_index, deleted);
            }
        }
        
        if (!node) return node;
        
        update_height(node);
        int balance = balance_factor(node);
        
        if (balance > 1 && balance_factor(node->left) >= 0)
            return rotate_right(node);
        
        if (balance > 1 && balance_factor(node->left) < 0) {
            node->left = rotate_left(node->left);
            return rotate_right(node);
        }
        
        if (balance < -1 && balance_factor(node->right) <= 0)
            return rotate_left(node);
        
        if (balance < -1 && balance_factor(node->right) > 0) {
            node->right = rotate_right(node->right);
            return rotate_left(node);
        }
        
        return node;
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
    
    void insert(const UserInfo& user) {
        root = insert_helper(root, user);
    }
    
    UserInfo* find_by_index(uint32_t user_index) {
        AVLNode* node = find_helper(root, user_index);
        return node ? &(node->user) : nullptr;
    }
    
    bool user_exists(const uint32_t& ind) {
        return find_by_index(ind) != nullptr;
    }
    
    bool remove(uint32_t user_index) {
        bool deleted = false;
        root = remove_helper(root, user_index, deleted);
        return deleted;
    }
    
    int count_active() {
        int count = 0;
        count_active_helper(root, count);
        return count;
    }
    
    void get_all_active(UserInfo* arr, int& count) {
        count = 0;
        collect_active_users_helper(root, arr, count);
    }
};

#endif