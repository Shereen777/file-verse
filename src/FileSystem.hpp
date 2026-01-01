#ifndef FILE_HPP
#define FILE_HPP

struct FSNode; //its basically the directory

struct AVLFSNode {
    uint32_t child_id;
    FSNode* fs_node;
    int height;
    AVLFSNode* left;
    AVLFSNode* right;
    
    AVLFSNode(uint32_t id, FSNode* node) 
        : child_id(id), fs_node(node), height(1), left(nullptr), right(nullptr) {}
};

struct NameMapNode {
    string name;
    uint32_t child_id;
    int height;
    NameMapNode* left;
    NameMapNode* right;
    
    NameMapNode(const string& n, uint32_t id)
        : name(n), child_id(id), height(1), left(nullptr), right(nullptr) {}
};

class AVLFSTree {
private:
    AVLFSNode* root;
    NameMapNode* name_map_root;
    
    int height(AVLFSNode* node) {
        return node ? node->height : 0;
    }
    
    int balance_factor(AVLFSNode* node) {
        return node ? height(node->left) - height(node->right) : 0;
    }
    
    void update_height(AVLFSNode* node) {
        if (node) {
            node->height = 1 + max(height(node->left), height(node->right));
        }
    }
    
    AVLFSNode* rotate_right(AVLFSNode* y) {
        AVLFSNode* x = y->left;
        AVLFSNode* T2 = x->right;
        x->right = y;
        y->left = T2;
        update_height(y);
        update_height(x);
        return x;
    }
    
    AVLFSNode* rotate_left(AVLFSNode* x) {
        AVLFSNode* y = x->right;
        AVLFSNode* T2 = y->left;
        y->left = x;
        x->right = T2;
        update_height(x);
        update_height(y);
        return y;
    }
    
    AVLFSNode* insert_helper(AVLFSNode* node, uint32_t child_id, FSNode* fs_node) {
        if (!node) return new AVLFSNode(child_id, fs_node);
        
        if (child_id < node->child_id)
            node->left = insert_helper(node->left, child_id, fs_node);
        else if (child_id > node->child_id)
            node->right = insert_helper(node->right, child_id, fs_node);
        else
            return node;
        
        update_height(node);
        int balance = balance_factor(node);
        
        if (balance > 1 && child_id < node->left->child_id)
            return rotate_right(node);
        if (balance < -1 && child_id > node->right->child_id)
            return rotate_left(node);
        if (balance > 1 && child_id > node->left->child_id) {
            node->left = rotate_left(node->left);
            return rotate_right(node);
        }
        if (balance < -1 && child_id < node->right->child_id) {
            node->right = rotate_right(node->right);
            return rotate_left(node);
        }
        
        return node;
    }
    
    AVLFSNode* find_by_id_helper(AVLFSNode* node, uint32_t child_id) {
        if (!node) return nullptr;
        if (node->child_id == child_id) return node;
        if (child_id < node->child_id) return find_by_id_helper(node->left, child_id);
        return find_by_id_helper(node->right, child_id);
    }
    
    AVLFSNode* find_min(AVLFSNode* node) {
        while (node && node->left) node = node->left;
        return node;
    }
    
    AVLFSNode* remove_helper(AVLFSNode* node, uint32_t child_id, bool& deleted) {
        if (!node) {
            deleted = false;
            return nullptr;
        }
        
        if (child_id < node->child_id) {
            node->left = remove_helper(node->left, child_id, deleted);
        } else if (child_id > node->child_id) {
            node->right = remove_helper(node->right, child_id, deleted);
        } else {
            deleted = true;
            if (!node->left || !node->right) {
                AVLFSNode* temp = node->left ? node->left : node->right;
                if (!temp) {
                    delete node;
                    return nullptr;
                }
                *node = *temp;
                delete temp;
            } else {
                AVLFSNode* temp = find_min(node->right);
                node->child_id = temp->child_id;
                node->fs_node = temp->fs_node;
                node->right = remove_helper(node->right, temp->child_id, deleted);
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
        
    int count_nodes(AVLFSNode* node) {
        if (!node) return 0;
        return 1 + count_nodes(node->left) + count_nodes(node->right);
    }
    
    void delete_id_tree(AVLFSNode* node) {
        if (!node) return;
        delete_id_tree(node->left);
        delete_id_tree(node->right);
        delete node;
    }
    
    int height(NameMapNode* node) {
        return node ? node->height : 0;
    }
    
    int balance_factor(NameMapNode* node) {
        return node ? height(node->left) - height(node->right) : 0;
    }
    
    void update_height(NameMapNode* node) {
        if (node) {
            node->height = 1 + max(height(node->left), height(node->right));
        }
    }
    
    NameMapNode* rotate_right(NameMapNode* y) {
        NameMapNode* x = y->left;
        NameMapNode* T2 = x->right;
        x->right = y;
        y->left = T2;
        update_height(y);
        update_height(x);
        return x;
    }
    
    NameMapNode* rotate_left(NameMapNode* x) {
        NameMapNode* y = x->right;
        NameMapNode* T2 = y->left;
        y->left = x;
        x->right = T2;
        update_height(x);
        update_height(y);
        return y;
    }
    
    NameMapNode* insert_name_helper(NameMapNode* node, const string& name, uint32_t child_id) {
        if (!node) return new NameMapNode(name, child_id);
        
        if (name < node->name)
            node->left = insert_name_helper(node->left, name, child_id);
        else if (name > node->name)
            node->right = insert_name_helper(node->right, name, child_id);
        else
            return node;
        
        update_height(node);
        int balance = balance_factor(node);
        
        if (balance > 1 && name < node->left->name)
            return rotate_right(node);
        if (balance < -1 && name > node->right->name)
            return rotate_left(node);
        if (balance > 1 && name > node->left->name) {
            node->left = rotate_left(node->left);
            return rotate_right(node);
        }
        if (balance < -1 && name < node->right->name) {
            node->right = rotate_right(node->right);
            return rotate_left(node);
        }
        
        return node;
    }
    
    NameMapNode* find_by_name_helper(NameMapNode* node, const string& name) {
        if (!node) return nullptr;
        if (node->name == name) return node;
        if (name < node->name) return find_by_name_helper(node->left, name);
        return find_by_name_helper(node->right, name);
    }
    
    NameMapNode* remove_name_helper(NameMapNode* node, const string& name, bool& deleted) {
        if (!node) {
            deleted = false;
            return nullptr;
        }
        
        if (name < node->name) {
            node->left = remove_name_helper(node->left, name, deleted);
        } else if (name > node->name) {
            node->right = remove_name_helper(node->right, name, deleted);
        } else {
            deleted = true;
            if (!node->left || !node->right) {
                NameMapNode* temp = node->left ? node->left : node->right;
                if (!temp) {
                    delete node;
                    return nullptr;
                }
                *node = *temp;
                delete temp;
            }
        }
        
        if (!node) return node;
        update_height(node);
        return balance_name_tree(node);
    }
    
    NameMapNode* balance_name_tree(NameMapNode* node) {
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
    
    void delete_name_tree(NameMapNode* node) {
        if (!node) return;
        delete_name_tree(node->left);
        delete_name_tree(node->right);
        delete node;
    }
    
public:
    AVLFSTree() : root(nullptr), name_map_root(nullptr) {}
    
    ~AVLFSTree() {
        delete_id_tree(root);
        delete_name_tree(name_map_root);
    }
    
    void insert(uint32_t child_id, const string& name, FSNode* fs_node) {
        root = insert_helper(root, child_id, fs_node);
        name_map_root = insert_name_helper(name_map_root, name, child_id);
    }

    void inorder_collect(AVLFSNode* node, vector<FSNode*>& result) {
        if (!node) return;
        inorder_collect(node->left, result);
        result.push_back(node->fs_node);
        inorder_collect(node->right, result);
    }

    
    FSNode* find(const string& name) {
        NameMapNode* name_node = find_by_name_helper(name_map_root, name);
        if (!name_node) return nullptr;
        
        AVLFSNode* id_node = find_by_id_helper(root, name_node->child_id);
        return id_node ? id_node->fs_node : nullptr;
    }
    
    FSNode* find_by_id(uint32_t child_id) {
        AVLFSNode* node = find_by_id_helper(root, child_id);
        return node ? node->fs_node : nullptr;
    }
    
    bool remove(const string& name) {
        NameMapNode* name_node = find_by_name_helper(name_map_root, name);
        if (!name_node) return false;
        
        uint32_t child_id = name_node->child_id;
        
        bool deleted_id = false, deleted_name = false;
        root = remove_helper(root, child_id, deleted_id);
        name_map_root = remove_name_helper(name_map_root, name, deleted_name);
        
        return deleted_id && deleted_name;
    }
    
    vector<FSNode*> get_all_sorted() {
        vector<FSNode*> result;
        inorder_collect(root, result);
        return result;
    }
    
    int size() {
        return count_nodes(root);
    }
    
    bool empty() {
        return root == nullptr;
    }

    AVLFSNode* getRoot(){
        return root;
    }
};

struct FSNode {
    string name;
    string full_path;
    EntryType type;
    FSNode* parent;
    AVLFSTree children;
    
    string owner;
    uint32_t permissions;
    uint64_t size;
    uint64_t created_time;
    uint64_t modified_time;
    uint32_t inode;
    uint32_t start_block;
    uint32_t num_blocks;
    uint32_t next_child_id;

    FSNode(const string& n, EntryType t, FSNode* p = nullptr) 
        : name(n), type(t), parent(p), permissions(0755), size(0),
          created_time(0), modified_time(0), inode(0), 
          start_block(0), num_blocks(0), next_child_id(1) {
        
        if (parent && parent->parent) {
            full_path = parent->full_path + "/" + name;
        } else if (parent) {
            full_path = "/" + name;
        } else {
            full_path = "/";
        }
    }
    
    uint32_t generate_child_id() {
        return (++next_child_id) + (rand() % 1000000);
    }
    
    void add_child(FSNode* child) {
        uint32_t child_id = generate_child_id();
        children.insert(child_id, child->name, child);
    }
    
    FSNode* find_child(const string& name) {
        return children.find(name);
    }
    
    bool remove_child(const string& name) {
        return children.remove(name);
    }
    
    vector<FSNode*> get_children() {
        return children.get_all_sorted();
    }
    
    bool has_children() {
        return !children.empty();
    }
    
    int children_count() {
        return children.size();
    }
};

class FileSystem {
private:
    FSNode* root;
    uint32_t next_inode;
    
    void delete_tree(FSNode* node) {
        if (!node) return;
        vector<FSNode*> children = node->get_children();
        for (auto* child : children) {
            delete_tree(child);
        }
        delete node;
    }
    
    vector<string> split_path(const string& path) {
        vector<string> result;
        if (path.empty() || path == "/") return result;
        
        stringstream ss(path);
        string component;
        
        while (getline(ss, component, '/')) {
            if (!component.empty()) {
                result.push_back(component);
            }
        }
        return result;
    }
    
    // NEW: Helper to resolve user-specific paths
    string resolve_user_path(const string& path, const string& username, bool is_admin) {
        // Admins can access any path
        if (is_admin) {
            return path;
        }
        
        // If path starts with /users/, allow it as-is (for explicit cross-user access by admins)
        if (path.find("/users/") == 0) {
            return path;
        }
        
        // If path is absolute but not in /users/, prepend user's directory
        if (path[0] == '/') {
            return "/users/" + username + path;
        }
        
        // Relative paths get prepended with user's directory
        return "/users/" + username + "/" + path;
    }
    
public:
    FileSystem() : next_inode(1) {
        root = new FSNode("/", EntryType::DIRECTORY, nullptr);
        root->inode = 0;
        root->created_time = time(nullptr);
        root->modified_time = root->created_time;
    }
    
    ~FileSystem() {
        delete_tree(root);
    }
    
    // NEW: Initialize /users directory structure
    bool ensure_users_directory() {
        FSNode* users_dir = root->find_child("users");
        if (!users_dir) {
            users_dir = new FSNode("users", EntryType::DIRECTORY, root);
            users_dir->owner = "system";
            users_dir->inode = next_inode++;
            users_dir->created_time = time(nullptr);
            users_dir->modified_time = users_dir->created_time;
            users_dir->permissions = 0755;
            root->add_child(users_dir);
        }
        return true;
    }
    
    // NEW: Create user home directory
    bool create_user_directory(const string& username) {
        ensure_users_directory();
        
        FSNode* users_dir = root->find_child("users");
        if (!users_dir) return false;
        
        // Check if user directory already exists
        if (users_dir->find_child(username)) {
            return true; // Already exists, that's fine
        }
        
        FSNode* user_dir = new FSNode(username, EntryType::DIRECTORY, users_dir);
        user_dir->owner = username;
        user_dir->inode = next_inode++;
        user_dir->created_time = time(nullptr);
        user_dir->modified_time = user_dir->created_time;
        user_dir->permissions = 0755;
        users_dir->add_child(user_dir);
        
        return true;
    }
    
    FSNode* find_node(const string& path) {
        if (path == "/" || path.empty()) return root;
        
        vector<string> components = split_path(path);
        FSNode* current = root;
        
        for (const auto& comp : components) {
            current = current->find_child(comp);
            if (!current) return nullptr;
        }
        return current;
    }
    
    // NEW: Find node with user context
    FSNode* find_node_for_user(const string& path, const string& username, bool is_admin) {
        string resolved_path = resolve_user_path(path, username, is_admin);
        return find_node(resolved_path);
    }
    
    FSNode* create_node(const string& path, EntryType type, const string& owner) {
        size_t last_slash = path.find_last_of('/');
        string parent_path = (last_slash == 0) ? "/" : path.substr(0, last_slash);
        string name = path.substr(last_slash + 1);
        
        if (name.empty()) return nullptr;
        
        FSNode* parent = find_node(parent_path);
        if (!parent || parent->type != EntryType::DIRECTORY) return nullptr;
        
        if (parent->find_child(name)) return nullptr;
        
        FSNode* node = new FSNode(name, type, parent);
        node->owner = owner;
        node->inode = next_inode++;
        node->created_time = time(nullptr);
        node->modified_time = node->created_time;
        node->permissions = (type == EntryType::DIRECTORY) ? 0755 : 0644;
        
        parent->add_child(node);
        return node;
    }
    
    FSNode* create_node_for_user(const string& path, EntryType type, const string& owner, bool is_admin) {
        string resolved_path = resolve_user_path(path, owner, is_admin);
        return create_node(resolved_path, type, owner);
    }
    
    bool delete_node(const string& path) {
        FSNode* node = find_node(path);
        if (!node || node == root) return false;
        
        if (node->type == EntryType::DIRECTORY && node->has_children()) {
            return false;
        }
        
        node->parent->remove_child(node->name);
        delete node;
        return true;
    }
    
    // NEW: Delete node with user context
    bool delete_node_for_user(const string& path, const string& username, bool is_admin) {
        string resolved_path = resolve_user_path(path, username, is_admin);
        return delete_node(resolved_path);
    }
    
    FSNode* get_root() { return root; }
};

#endif