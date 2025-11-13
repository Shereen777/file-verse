#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <stack>
#include <string>
#include <algorithm>
using namespace std;

class BST {
private:
    struct Node {
        int data;
        Node* left;
        Node* right;
        Node* parent;
        Node(int value, Node* _l = nullptr, Node* _r = nullptr, Node* _p = nullptr) : data{ value }, left{ _l }, right{ _r }, parent{ _p } {}
        ~Node() {}
    };
    Node* root;
    void insert(Node*& r, int value, Node* parent = nullptr) {
        if (!r) {
            r = new Node(value, nullptr, nullptr, parent);
            return;
        }
        if (value <= r->data)
            insert(r->left, value, r);
        else
            insert(r->right, value, r);
    }
    bool search(Node* const& r, int val) const {
        if (!r)
            return false;
        if (r->data == val)
            return true;
        if (val < r->data)
            return search(r->left, val);
        else
            return search(r->right, val);
    }
    bool search(int value) const {
        return search(root, value);
    }
    void inorder(Node* const& r, vector<int>& V) const {
        if (!r)
            return;
        inorder(r->left, V);
        V.push_back(r->data);
        inorder(r->right, V);
    }
    void RNL(Node* const& r, vector<int>& V) const {
        if (!r)
            return;
        RNL(r->right, V);
        V.push_back(r->data);
        RNL(r->left, V);
    }
    void inorderSimp(Node* const& r) const {
        if (!r)
            return;
        inorderSimp(r->left);
        cout << r->data << " ";
        inorderSimp(r->right);
    }
    void postorder(Node* const& r) const {
        if (!r)
            return;
        postorder(r->left);
        postorder(r->right);
        cout << r->data << " ";
    }
    void preorder(Node* const& r) const {
        if (!r)
            return;
        cout << r->data << " ";
        preorder(r->left);
        preorder(r->right);
    }
    int size(Node* const& r) const {
        if (!r)
            return 0;
        return size(r->left) + size(r->right) + 1;
    }
    void copy(Node*& r, Node* other) {
        if (other == nullptr)
            return;
        r = new Node(other->data);
        copy(r->left, other->left);
        copy(r->right, other->right);
    }
    void write_to_file(ofstream& Wtr, Node*& r) {
        if (!r)
            return;
        Wtr << r->data << " ";
        write_to_file(Wtr, r->left);
        write_to_file(Wtr, r->right);

    }
    bool isPrime(int val) const {
        if (val == 2 or val == 3)
            return true;
        if (val < 2 or val % 2 == 0)
            return false;
        for (int d = 3;d < val;d += 2) {
            if (val % d == 0)
                return false;
        }
        return true;
    }
    void read_from_file(ifstream& Rdr) {
        if (!Rdr)
            return;
        int val;
        Rdr >> val;
        insert(val);
        read_from_file(Rdr);
    }
    int prime_count(Node* n) const {
        if (!n)
            return 0;
        return prime_count(n->right) + prime_count(n->left) + isPrime(n->data);
    }
    int even_count(Node* n) const {
        if (!n)
            return 0;
        return even_count(n->right) + even_count(n->left) + (n->data % 2 == 0);
    }
    void printLeaves(Node* const& r) const {
        if (!r)
            return;
        if (!r->right and r->right == r->left)
            cout << r->data << " ";
        printLeaves(r->left);
        printLeaves(r->right);
    }
    void countLeaves(Node* const& r, int& c) const {
        if (!r)
            return;
        if (!r->right and r->right == r->left)
            c++;
        countLeaves(r->left, c);
        countLeaves(r->right, c);
    }
    void printRootsAtK(Node* const& r, int k) const {
        if (!r)
            return;
        if (k == 0) {
            cout << r->data << " ";
            return;
        }
        printRootsAtK(r->left, k - 1);
        printRootsAtK(r->right, k - 1);
    }
    int height(Node* const& n)const {
        if (!n)
            return 0;
        return max(height(n->right), height(n->left)) + 1;
    }
    bool isPictoricallySame(Node* const& n, Node* const& other) const {
        if (!n and n == other)
            return true;
        if (!n or !other)
            return false;
        return isPictoricallySame(n->left, other->left) and isPictoricallySame(n->right, other->right);
    }
    bool isValueWiseSame(Node* const& r, Node* const& other) const {
        vector<int> V1;
        inorder(r, V1);
        vector<int> V2;
        inorder(other, V2);
        return V1 == V2;
    }
    void deleteLeaves(Node*& r) {
        Node* tempr = r->right;
        Node* templ = r->left;
        if (!r)
            return;
        if (!r->right and r->right == r->left)
        {
            if (r->parent) {
                if (r->parent->right == r)
                    r->parent->right = nullptr;
                else
                    r->parent->left = nullptr;
            }
            delete r;
            r = nullptr;
        }
        if (templ)
            deleteLeaves(templ);
        if (tempr)
            deleteLeaves(tempr);
    }
    int TreeMinimum(Node* const& r)const {
        if (!r)
            return -1;
        if (!r->left)
            return r->data;
        TreeMinimum(r->left);
    }
    int TreeMaximum(Node* const& r)const {
        if (!r)
            return -1;
        if (!r->right)
            return r->data;
        TreeMaximum(r->right);
    }
    bool isBalanced(Node* const& r)const {
        if (!r)
            return true;
        if (abs(height(r->right) - height(r->left)) > 1)
            return false;
        return isBalanced(r->right) and isBalanced(r->left);
    }
    bool isLeaf(Node* const& r) const {
        return !r->right and r->right == r->left;
    }
    Node* closestLeaf(queue<Node*>& nodes)const {
        Node* r = nodes.front();
        nodes.pop();
        if (isLeaf(r))
            return r;
        if (r->left)
            nodes.push(r->left);
        if (r->right)
            nodes.push(r->right);
        return closestLeaf(nodes);
    }
    void farthestLeaf(queue<Node*>& nodes, Node*& prev)const {
        if (nodes.empty())
            return;
        Node* r = nodes.front();
        nodes.pop();
        if (isLeaf(r))
            prev = r;
        if (r->left)
            nodes.push(r->left);
        if (r->right)
            nodes.push(r->right);
        return farthestLeaf(nodes, prev);
    }
    void printPath(Node* const& r)const {
        if (!r)
            return;
        printPath(r->parent);
        cout << r->data << " ";
    }
    Node* searchNodeptr(Node* const& r, int val) const {
        if (!r)
            return nullptr;
        if (r->data == val)
            return r;
        if (val < r->data)
            return searchNodeptr(r->left, val);
        else
            return searchNodeptr(r->right, val);
    }
    void printSuccessorLNR(int v, Node* r) {
        if (r->right) {
            r = r->right;
            cout << TreeMinimum(r);
            return;
        }
        if (r->parent and r->parent->left == r) {
            cout << r->parent->data;
            return;
        }
        if (r->parent and r->parent->right == r) {
            r = r->parent;
            if (r->parent and r->parent->left == r) {
                cout << r->parent->data;
                return;
            }
        }
    }
    void printPredecessorLNR(int v, Node* r) {
        if (r->left) {
            r = r->left;
            cout << TreeMaximum(r);
            return;
        }
        if (r->parent) {
            r = r->parent;
            return;
        }
    }
public:
    // --- Constructors / Destructor ---
    BST() :root{ nullptr } {

    }
    void deallocate(Node*& r) {
        if (!r)
            return;
        deallocate(r->right);
        deallocate(r->left);
        delete r;
        r = nullptr;
    }

    ~BST() {
        deallocate(root);
    }

    BST(const BST& other) {
        copy(root, other.root);
    }
    BST& operator=(const BST& other) {
        copy(root, other.root);
        return *this;
    }

    // --- Core Operations ---
    void insert(int value) {
        insert(root, value);
    }

    void clear() {
        deallocate(root);
    }

    // --- Traversals ---
    void inorder() const {
        cout << "InOrder\n";
        vector<int> V;
        inorder(root, V);
        for (int i = 0;i < V.size();i++)
            cout << V[i] << " ";
        cout << "\n";
    }
    vector<int> LNR(Node* n) const {
        vector<int> V;
        inorder(n, V);
        return V;
    }
    vector<int> RNL(Node * n) const {
        vector<int> V;
        RNL(n, V);
        return V;
    }
    void preorder() const {
        cout << "PreOrder\n";
        preorder(root);
        cout << "\n";
    }
    void postorder() const {
        cout << "PostOrder\n";
        postorder(root);
        cout << "\n";
    }

    // --- Utilities ---
    int size() const {
        return size(root);
    }
    bool empty() const {
        if (!root)
            return true;
        return false;
    }
    int prime_count() const {
        return prime_count(root);
    }
    int even_count() const {
        return even_count(root);
    }
    int height()const {
        return height(root);
    }
    void read_from_file(const char* file_name) {
        ifstream Rdr(file_name);
        read_from_file(Rdr);
    }
    void write_to_file(const char* file_name) {
        ofstream Wtr(file_name);
        write_to_file(Wtr, root);
    }
    void printRootsAtK(int k) const {
        printRootsAtK(root, k);
    }
    void printLeaves() const {
        cout << "Leaves\n";
        printLeaves(root);
        cout << "\n";
    }
    bool isPictoricallySame(const BST& other) const {
        return isPictoricallySame(root, other.root);
    }
    bool isValueWiseSame(const BST& other)const {
        return isValueWiseSame(root, other.root);
    }
    bool PictoricallyPlusValueWise(const BST& other) {
        return isValueWiseSame(root, other.root) and isPictoricallySame(root, other.root);
    }
    void deleteLeaves() {
        deleteLeaves(root);
    }
    int TreeMinimum() const {
        return TreeMinimum(root);
    }
    int TreeMaximum() const {
        return TreeMaximum(root);
    }
    bool isBalanced()const {
        return isBalanced(root);
    }
    int closestLeaf()const {
        queue<Node*> nodes;
        if (root)
            nodes.push(root);
        else
            return -1;
        Node* n = closestLeaf(nodes);
        return n->data;
    }
    int farthestLeaf()const {
        Node* prev = nullptr;
        queue<Node*> nodes;
        if (root)
            nodes.push(root);
        else
            return -1;
        farthestLeaf(nodes, prev);
        if (prev)
            return prev->data;
        //return -1;
    }
    void printPathClosest()const {
        queue<Node*> nodes;
        if (root)
            nodes.push(root);
        else
            return;
        Node* closest = closestLeaf(nodes);
        printPath(closest);
        cout << "\n";
    }
    void printPathFarthest()const {
        queue<Node*> nodes;
        if (root)
            nodes.push(root);
        else
            return;
        Node* farthest;
        farthestLeaf(nodes, farthest);
        printPath(farthest);
        cout << "\n";
    }
    int countInternalNodes() {
        int s = size();
        int c = 0;
        countLeaves(root, c);
        if (isLeaf(root))
            return s - c;
        return s - c - 1;
    }
    void printSuccessorLNR(int v) {
        if (!search(v))
            return;
        Node* n = searchNodeptr(root, v);
        if (n)
            printSuccessorLNR(v, n);
        else
            cout << "Value not Found";
    }
    void printPredecessorLNR(int v) {
        if (!search(v))
            return;
        Node* n = searchNodeptr(root, v);
        if (n)
            printPredecessorLNR(v, n);
        else
            cout << "Value not Found";
    }
    /*bool isMirror(Node* r) {
        if (!r)
            return false;
        vector<int> rL = LNR(r->left);
        vector<int> rR = RNL(r->right);
        return rL == rR;
    }*/
    int findind(vector<int> in,int val,int si,int ei) {
        for (int i = si;i <= ei;i++) {
            if (in[i] == val)
                return i;
        }
        return -1;
    }

    void buildTree(vector<int>& in, vector<int>& pre, int si, int ei, Node*& r, int &ci, Node*& head) {
        if (si > ei or ei < 0 or si < 0 or ci >= pre.size())
            return;
        int mid = pre[ci++];
        int mi = findind(in, mid, si, ei);
        if (mi == -1)
            return;
        r = new Node(mid);
        if (!head)
            head = r;
        buildTree(in, pre, si, mi - 1, r->left, ci, head);
        buildTree(in, pre, mi + 1, ei, r->right, ci, head);
    }
    void buildTree(vector<int>& pre, vector<int>& in) {
        Node* r = nullptr;
        //Node* h = nullptr;
        int ci = 0;
        buildTree(in, pre, 0, pre.size() - 1, r, ci, root);
        //return h;
    }


    //void buildTree(vector<int> in, vector<int> &post,int si,int ei,Node* &r) {
    //    if (si > ei or ei < 0 or si < 0)
    //        return;
    //    if (post.size() < 1)
    //        return;
    //    int mid = post[post.size() - 1];
    //    int mi = findind(in, mid, si, ei);
    //    if (mi == -1)
    //        return;
    //    post.pop_back();
    //    r = new Node(mid);
    //    if (!root)
    //        root = r;
    //    buildTree(in, post, mi + 1, ei , r->right);
    //    buildTree(in, post, si, mi - 1, r->left);
    //}
    //void buildTree(vector<int> in, vector<int> post) {
    //    Node* r = nullptr;
    //    buildTree(in, post, 0, post.size() - 1, r);
    //}
    void largestValues(queue<Node*>& q, vector<int>& V, int max) {
        if (q.empty())
            return;
        if (q.size() == 1 and q.front() == nullptr) {
            V.push_back(max);
            return;
        }
        Node* r = q.front();
        q.pop();
        if (r == nullptr) {
            q.push(r);
            V.push_back(max);
            max = 298091110;
            r = q.front();
            q.pop();
        }
        if (r->data > max)
            max = r->data;
    if (r->right)
        q.push(r->right);
    if (r->left)
        q.push(r->left);
    largestValues(q, V, max);
}
    vector<int> largestValues(Node* root) {
        queue<Node*> q;
        vector<int> V;
        if (!root)
            return V;
        q.push(root);
        //V.push_back(root->data);
        q.push(nullptr);
        int max = -100000;
        largestValues(q, V, max);
        return V;
    }

    void sumNumbers(stack<string>& s, Node* r, int &sum) {
        if (!r)
            return;
        string val;
        if (s.size() > 0) {
            val = s.top();
        }
        val.append(to_string(r->data));
        s.push(val);
        if (isLeaf(r)) {
            sum += stoi(s.top());
        }
        sumNumbers(s, r->left, sum);
        sumNumbers(s, r->right, sum);
        if(s.size() > 0)
            s.pop();
    }

    int sumNumbers(Node* r) {
        stack<string> s;
        int sum = 0;
        sumNumbers(s, r, sum);
        return sum;
    }
    bool isValidBST(Node* r) {
        if (!r)
            return true;
        int prev = NULL;
        int curr = NULL;

        return  isValidBSTrec(r, prev, curr);
    }
    bool isValidBSTrec(Node* r, int &prev, int &curr ) {
        if (!r)
            return true;
        // if(root->left and root->left->val>=root->val)
        //     return false;
        // if(root->right and root->right->val<=root->val)
        //     return false;
        // return isValidBST(root->right) and isValidBST(root->left);
        bool L = isValidBSTrec(r->left, prev, curr);
        prev = curr;
        curr = r->data;
        if (prev and prev >= curr)
            return false;
        bool R = isValidBSTrec(r->right, prev, curr);
        return L and R;
    }
    bool isBSTree() {
        return isValidBST(root);
    }

};
