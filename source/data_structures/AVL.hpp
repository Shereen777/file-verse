#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
using namespace std;

class AVL {
    struct Node {
        int data;
        Node* left;
        Node* right;
        Node* parent;
        int h;
        Node(int value, Node* _l = nullptr, Node* _r = nullptr, Node* _p = nullptr, int _h = 0) : data{ value }, left{ _l }, right{ _r }, parent{ _p }, h{ _h } {}
        ~Node() {}
    };
    Node* root;
    Node* insert(Node*& r, int value, Node* &parent) {
        if (!r and !parent) {
            r = new Node(value, nullptr, nullptr, nullptr);
            return r;
        }
        if (!r) {
            if (!parent->left and !parent->right)
                parent->h++;
            r = new Node(value, nullptr, nullptr, parent);
            return r;
        }
        if (value <= r->data) {
            auto n = insert(r->left, value, r);
            if (parent and r->h >= parent-> h) {
                parent->h = r->h + 1;
            }
            return n;
        }
        else {
            auto n = insert(r->right, value, r);
            if (parent and r->h >= parent->h) {
                parent->h = r->h + 1;
            }
            return n;
        }
    }
	//Left rotation
    Node* leftRotation(Node* r) {
        Node* temp = r;
        r = r->right;
        r->parent = temp->parent;
        temp->parent = r;
        temp->right = r->left;
        r->left = temp;
        if (r->parent == nullptr)
            root = r;
        temp->h = height(temp) - 1;
        return r;
    }
    //Right rotation
    Node* rightRotation(Node* r) {
        Node* temp = r;
        r = r->left;
        r->parent = temp->parent;
        temp->parent = r;
        temp->left = r->right;
        r->right = temp;
        if (r->parent == nullptr)
            root = r;
        temp->h = height(temp) - 1;
        return r;
    }
    int height(Node* const& n)const {
        if (!n)
            return 0;
        return max(height(n->right), height(n->left)) + 1;
    }
    void recCheck(Node* r) {
        if (!r)
            return;
        int lh,rh;
        lh = !r->left ? 0 : r->left->h;
        rh = !r->right? 0 : r->right->h;
        int h = lh - rh;
        if (h < -1) {
            if (r->right) {
                lh = !r->right->left ? 0 : r->right->left->h;
                rh = !r->right->right ? 0 : r->right->right->h;
                h = lh - rh;
                if (h > 0)
                {
                    //if (r->right->parent and r->right->parent->left == r->right) {
                    //    r->right = rightRotation(r->right);
                    //    if (r->right->parent)
                    //        r->right->parent->left = r->right;
                    //}
                    //else {
                        r->right = rightRotation(r->right);
                        if (r->right->parent)
                            r->right->parent->right = r->right;
                    //}
                }
            }
            if (r->parent and r->parent->right == r) {
                r = leftRotation(r);
                if (r->parent)
                    r->parent->right = r;
                return;
            }
            r = leftRotation(r);
            if (r->parent)
                r->parent->left = r;
            return;
        }
        if (h > 1) {
            if (r->left) {
                lh = !r->left->left ? 0 : r->left->left->h;
                rh = !r->left->right ? 0 : r->left->right->h;
                h = lh - rh;
                if (h < 0){
                    /*if (r->left->parent and r->left->parent->right == r->left) {
                        r->left = leftRotation(r->left);
                        if (r->left->parent)
                            r->left->parent->right = r->left;
                    }
                    else {*/
                        r->left = leftRotation(r->left);
                        if (r->left->parent)
                            r->left->parent->left = r->left;
                    //}
                }
            }
            if (r->parent and r->parent->left == r) {
                r = rightRotation(r);
                if (r->parent)
                    r->parent->left = r;
                return;
            }
            r = rightRotation(r);
            if (r->parent)
                r->parent->right = r;
            return;
        }
        recCheck(r->parent);
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
    Node* TreeMinimum(Node* const& r)const {
        if (!r)
            return nullptr;
        if (!r->left)
            return r;
        return TreeMinimum(r->left);
    }
    Node* SuccessorLNR(int v, Node* r) {
        if (r->right) {
            r = r->right;
            return TreeMinimum(r);
        }
        if (r->parent and r->parent->left == r) {
            return r->parent;
        }
        if (r->parent and r->parent->right == r) {
            r = r->parent;
            if (r->parent and r->parent->left == r) {
                return r->parent;
            }
        }
    }
    Node* TreeMaximum(Node* const& r)const {
        if (!r)
            return nullptr;
        if (!r->right)
            return r;
        return TreeMaximum(r->right);
    }
    Node* PredecessorLNR(int v, Node* r) {
        if (r->left) {
            r = r->left;
            return TreeMaximum(r);
        }
        if (r->parent) {
            r = r->parent;
            return r;
        }
    }
    bool isLeaf(Node* const& r) const {
        return !r->right and r->right == r->left;
    }
    Node* deleteLeaf(Node*& r) {
        if (r->parent and r->parent->right == r)
            r->parent->right = nullptr;
        if (r->parent and r->parent->left == r)
            r->parent->left = nullptr;
        if (r == root)
            root = nullptr;
        Node* temp = r->parent;
        delete r;
        r = nullptr;
        return temp;
    }
    void Delete(Node* n, int val) {
        if (isLeaf(n)) {
            Node* d = deleteLeaf(n);
            recCheck(d);
            return;
        }
        if (n and !n->left and n->right) {
            Node* p = SuccessorLNR(val, n);
            if (p) {
                swap(p->data, n->data);
            }
            Node* d = deleteLeaf(p);
            recCheck(d);
            return;
        }
        if (n and !n->right and n->left) {
            Node* p = PredecessorLNR(val, n);
            if (p) {
                swap(p->data, n->data);
            }
            Node* d = deleteLeaf(p);
            recCheck(d);
            return;
        }
        if (n) {
            Node* p = PredecessorLNR(val, n);
            if (p) {
                swap(p->data, n->data);
            }
            Delete(p, val);
        }
    }
public:
    AVL() :root{} {}
    void insert(int val) {
        Node* s = nullptr;
        Node* r = insert(root, val, s);
        recCheck(r);
    }
    void Delete(int val) {
        if (!search(root,val))
            return;
        Node* n = searchNodeptr(root, val);
        Delete(n, val);
    }
};



//#pragma once
//#include <iostream>
//#include <fstream>
//#include <vector>
//#include <queue>
//#include <Windows.h>
//using namespace std;
//
//class AVL {
//    struct Node {
//        int data;
//        Node* left;
//        Node* right;
//        Node* parent;
//        Node(int value, Node* _l = nullptr, Node* _r = nullptr, Node* _p = nullptr) : data{ value }, left{ _l }, right{ _r }, parent{ _p } {}
//        ~Node() {}
//    };
//    Node* root;
//    Node* insert(Node*& r, int value, Node*& parent) {
//        if (!r and !parent) {
//            r = new Node(value, nullptr, nullptr, nullptr);
//            return r;
//        }
//        if (!r) {
//            r = new Node(value, nullptr, nullptr, parent);
//            return r;
//        }
//        if (value <= r->data)
//            return insert(r->left, value, r);
//        else
//            return insert(r->right, value, r);
//    }
//    //Left rotation
//    Node* leftRotation(Node* r) {
//        Node* temp = r;
//        r = r->right;
//        r->parent = temp->parent;
//        temp->parent = r;
//        temp->right = r->left;
//        r->left = temp;
//        if (r->parent == nullptr)
//            root = r;
//        return r;
//    }
//    //Right rotation
//    Node* rightRotation(Node* r) {
//        Node* temp = r;
//        r = r->left;
//        r->parent = temp->parent;
//        temp->parent = r;
//        temp->left = r->right;
//        r->right = temp;
//        if (r->parent == nullptr)
//            root = r;
//        return r;
//    }
//    int height(Node* const& n)const {
//        if (!n)
//            return 0;
//        return max(height(n->right), height(n->left)) + 1;
//    }
//    void recCheck(Node* r) {
//        if (!r)
//            return;
//        int h = height(r->left) - height(r->right);
//        if (h < -1) {
//            if (r->right) {
//                h = height(r->right->left) - height(r->right->right);
//                if (h > 0)
//                {
//                    if (r->right->parent and r->right->parent->left == r->right) {
//                        r->right = rightRotation(r->right);
//                        if (r->right->parent)
//                            r->right->parent->left = r->right;
//                    }
//                    else {
//                        r->right = rightRotation(r->right);
//                        if (r->right->parent)
//                            r->right->parent->right = r->right;
//                    }
//                }
//            }
//            if (r->parent and r->parent->right == r) {
//                r = leftRotation(r);
//                if (r->parent)
//                    r->parent->right = r;
//                return;
//            }
//            r = leftRotation(r);
//            if (r->parent)
//                r->parent->left = r;
//            return;
//        }
//        if (h > 1) {
//            if (r->left) {
//                h = height(r->left->left) - height(r->left->right);
//                if (h < 0) {
//                    if (r->left->parent and r->left->parent->right == r->left) {
//                        r->left = leftRotation(r->left);
//                        if (r->left->parent)
//                            r->left->parent->right = r->left;
//                    }
//                    else {
//                        r->left = leftRotation(r->left);
//                        if (r->left->parent)
//                            r->left->parent->left = r->left;
//                    }
//                }
//            }
//            if (r->parent and r->parent->left == r) {
//                r = rightRotation(r);
//                if (r->parent)
//                    r->parent->left = r;
//                return;
//            }
//            r = rightRotation(r);
//            if (r->parent)
//                r->parent->right = r;
//            return;
//        }
//        recCheck(r->parent);
//    }
//    void gotoXY(int x, int y) {
//        COORD coord;
//        coord.X = x;
//        coord.Y = y;
//        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
//    }
//
//    // Recursive function to print tree in 2D
//    void printTree(Node* root, int x, int y, int offset) {
//        if (root == nullptr) return;
//
//        // Print current node at (x, y)
//        gotoXY(x, y);
//        cout << root->data;
//
//        // Print left child
//        if (root->left) {
//            gotoXY(x - offset / 2, y + 1);
//            cout << "/";
//            printTree(root->left, x - offset, y + 2, offset / 2);
//        }
//
//        // Print right child
//        if (root->right) {
//            gotoXY(x + offset / 2, y + 1);
//            cout << "\\";
//            printTree(root->right, x + offset, y + 2, offset / 2);
//        }
//    }
//    bool search(Node* const& r, int val) const {
//        if (!r)
//            return false;
//        if (r->data == val)
//            return true;
//        if (val < r->data)
//            return search(r->left, val);
//        else
//            return search(r->right, val);
//    }
//    Node* searchNodeptr(Node* const& r, int val) const {
//        if (!r)
//            return nullptr;
//        if (r->data == val)
//            return r;
//        if (val < r->data)
//            return searchNodeptr(r->left, val);
//        else
//            return searchNodeptr(r->right, val);
//    }
//    Node* TreeMinimum(Node* const& r)const {
//        if (!r)
//            return nullptr;
//        if (!r->left)
//            return r;
//        return TreeMinimum(r->left);
//    }
//    Node* SuccessorLNR(int v, Node* r) {
//        if (r->right) {
//            r = r->right;
//            return TreeMinimum(r);
//        }
//        if (r->parent and r->parent->left == r) {
//            return r->parent;
//        }
//        if (r->parent and r->parent->right == r) {
//            r = r->parent;
//            if (r->parent and r->parent->left == r) {
//                return r->parent;
//            }
//        }
//    }
//    Node* TreeMaximum(Node* const& r)const {
//        if (!r)
//            return nullptr;
//        if (!r->right)
//            return r;
//        return TreeMaximum(r->right);
//    }
//    Node* PredecessorLNR(int v, Node* r) {
//        if (r->left) {
//            r = r->left;
//            return TreeMaximum(r);
//        }
//        if (r->parent) {
//            r = r->parent;
//            return r;
//        }
//    }
//    bool isLeaf(Node* const& r) const {
//        return !r->right and r->right == r->left;
//    }
//    Node* deleteLeaf(Node*& r) {
//        if (r->parent and r->parent->right == r)
//            r->parent->right = nullptr;
//        if (r->parent and r->parent->left == r)
//            r->parent->left = nullptr;
//        if (r == root)
//            root = nullptr;
//        Node* temp = r->parent;
//        delete r;
//        r = nullptr;
//        return temp;
//    }
//    void Delete(Node* n, int val) {
//        if (isLeaf(n)) {
//            Node* d = deleteLeaf(n);
//            recCheck(d);
//            return;
//        }
//        if (n and !n->left and n->right) {
//            Node* p = SuccessorLNR(val, n);
//            if (p) {
//                swap(p->data, n->data);
//            }
//            Node* d = deleteLeaf(p);
//            recCheck(d);
//            return;
//        }
//        if (n and !n->right and n->left) {
//            Node* p = PredecessorLNR(val, n);
//            if (p) {
//                swap(p->data, n->data);
//            }
//            Node* d = deleteLeaf(p);
//            recCheck(d);
//            return;
//        }
//        if (n) {
//            Node* p = PredecessorLNR(val, n);
//            if (p) {
//                swap(p->data, n->data);
//            }
//            Delete(p, val);
//        }
//    }
//public:
//    AVL() :root{} {}
//    void insert(int val) {
//        Node* s = nullptr;
//        Node* r = insert(root, val, s);
//        recCheck(r);
//    }
//    void printTree() {
//        printTree(root, 40, 2, 10);
//    }
//    void Delete(int val) {
//        if (!search(root, val))
//            return;
//        Node* n = searchNodeptr(root, val);
//        Delete(n, val);
//    }
//};
