#ifndef AVL_HPP
#define AVL_HPP

#include "../include/odf_types.hpp"
#include <fstream>
#include <sstream>
#include <cstring>
#include <ctime>
#include <vector>
#include <algorithm>
#include <random>
#include <iostream>

using namespace std;

struct AVLUserNode {
    uint32_t user_index;
    UserInfo* user_data;
    int height;
    AVLUserNode* left;
    AVLUserNode* right;
    
    AVLUserNode(uint32_t idx, UserInfo* data) 
        : user_index(idx), user_data(data), height(1), left(nullptr), right(nullptr) {}
};

class AVLIndexTree {
private:
    AVLUserNode* root;
    
    int height(AVLUserNode* node) {
        return node ? node->height : 0;
    }
    
    int balance_factor(AVLUserNode* node) {
        return node ? height(node->left) - height(node->right) : 0;
    }
    
    void update_height(AVLUserNode* node) {
        if (node) {
            node->height = 1 + max(height(node->left), height(node->right));
        }
    }
    
    AVLUserNode* rotate_right(AVLUserNode* y) {
        AVLUserNode* x = y->left;
        AVLUserNode* T2 = x->right;
        x->right = y;
        y->left = T2;
        update_height(y);
        update_height(x);
        return x;
    }
    
    AVLUserNode* rotate_left(AVLUserNode* x) {
        AVLUserNode* y = x->right;
        AVLUserNode* T2 = y->left;
        y->left = x;
        x->right = T2;
        update_height(x);
        update_height(y);
        return y;
    }
    
    AVLUserNode* insert_helper(AVLUserNode* node, uint32_t user_index, UserInfo* user_data) {
        if (!node) return new AVLUserNode(user_index, user_data);
        
        if (user_index < node->user_index)
            node->left = insert_helper(node->left, user_index, user_data);
        else if (user_index > node->user_index)
            node->right = insert_helper(node->right, user_index, user_data);
        else
            return node;
        
        update_height(node);
        int balance = balance_factor(node);
        
        if (balance > 1 && user_index < node->left->user_index)
            return rotate_right(node);
        if (balance < -1 && user_index > node->right->user_index)
            return rotate_left(node);
        if (balance > 1 && user_index > node->left->user_index) {
            node->left = rotate_left(node->left);
            return rotate_right(node);
        }
        if (balance < -1 && user_index < node->right->user_index) {
            node->right = rotate_right(node->right);
            return rotate_left(node);
        }
        
        return node;
    }
    
    AVLUserNode* find_helper(AVLUserNode* node, uint32_t user_index) {
        if (!node) return nullptr;
        if (node->user_index == user_index) return node;
        if (user_index < node->user_index) return find_helper(node->left, user_index);
        return find_helper(node->right, user_index);
    }
    
    void delete_tree(AVLUserNode* node) {
        if (!node) return;
        delete_tree(node->left);
        delete_tree(node->right);
        delete node;
    }
    
public:
    AVLIndexTree() : root(nullptr) {}
    ~AVLIndexTree() { delete_tree(root); }
    
    void insert(uint32_t user_index, UserInfo* user_data) {
        root = insert_helper(root, user_index, user_data);
    }
    
    UserInfo* find(uint32_t user_index) {
        AVLUserNode* node = find_helper(root, user_index);
        return node ? node->user_data : nullptr;
    }
};

struct AVLNameNode {
    string username;
    uint32_t user_index;
    int height;
    AVLNameNode* left;
    AVLNameNode* right;
    
    AVLNameNode(const string& name, uint32_t idx) 
        : username(name), user_index(idx), height(1), left(nullptr), right(nullptr) {}
};

class AVLNameTree {
private:
    AVLNameNode* root;
    
    int height(AVLNameNode* node) {
        return node ? node->height : 0;
    }
    
    int balance_factor(AVLNameNode* node) {
        return node ? height(node->left) - height(node->right) : 0;
    }
    
    void update_height(AVLNameNode* node) {
        if (node) {
            node->height = 1 + max(height(node->left), height(node->right));
        }
    }
    
    AVLNameNode* rotate_right(AVLNameNode* y) {
        AVLNameNode* x = y->left;
        AVLNameNode* T2 = x->right;
        x->right = y;
        y->left = T2;
        update_height(y);
        update_height(x);
        return x;
    }
    
    AVLNameNode* rotate_left(AVLNameNode* x) {
        AVLNameNode* y = x->right;
        AVLNameNode* T2 = y->left;
        y->left = x;
        x->right = T2;
        update_height(x);
        update_height(y);
        return y;
    }
    
    AVLNameNode* insert_helper(AVLNameNode* node, const string& username, uint32_t user_index) {
        if (!node) return new AVLNameNode(username, user_index);
        
        if (username < node->username)
            node->left = insert_helper(node->left, username, user_index);
        else if (username > node->username)
            node->right = insert_helper(node->right, username, user_index);
        else
            return node;
        
        update_height(node);
        int balance = balance_factor(node);
        
        if (balance > 1 && username < node->left->username)
            return rotate_right(node);
        if (balance < -1 && username > node->right->username)
            return rotate_left(node);
        if (balance > 1 && username > node->left->username) {
            node->left = rotate_left(node->left);
            return rotate_right(node);
        }
        if (balance < -1 && username < node->right->username) {
            node->right = rotate_right(node->right);
            return rotate_left(node);
        }
        
        return node;
    }
    
    AVLNameNode* find_helper(AVLNameNode* node, const string& username) {
        if (!node) return nullptr;
        if (node->username == username) return node;
        if (username < node->username) return find_helper(node->left, username);
        return find_helper(node->right, username);
    }
    
    void delete_tree(AVLNameNode* node) {
        if (!node) return;
        delete_tree(node->left);
        delete_tree(node->right);
        delete node;
    }
    
public:
    AVLNameTree() : root(nullptr) {}
    ~AVLNameTree() { delete_tree(root); }
    
    void insert(const string& username, uint32_t user_index) {
        root = insert_helper(root, username, user_index);
    }
    
    uint32_t find(const string& username) {
        AVLNameNode* node = find_helper(root, username);
        return node ? node->user_index : 0;
    }
};

#endif