#ifndef List_HPP
#define List_HPP

#include <vector>
#include <utility>
#include <functional>
#include <stdexcept>
#include <algorithm>

using namespace std;


template <typename T>
class LinkedList {
private:
    struct Node {
        T data;
        Node* next;
        Node* prev;
        
        Node(const T& d) : data(d), next(nullptr), prev(nullptr) {}
    };
    
    Node* head;
    Node* tail;
    size_t length;
    
public:
    class Iterator {
    private:
        Node* current;
        
    public:
        Iterator(Node* node = nullptr) : current(node) {}
        
        T& operator*() {
            return current->data;
        }
        
        T* operator->() {
            return &current->data;
        }
        
        Iterator& operator++() {
            current = current->next;
            return *this;
        }
        
        Iterator operator++(int) {
            Iterator temp = *this;
            current = current->next;
            return temp;
        }
        
        Iterator& operator--() {
            current = current->prev;
            return *this;
        }
        
        bool operator==(const Iterator& other) const {
            return current == other.current;
        }
        
        bool operator!=(const Iterator& other) const {
            return current != other.current;
        }
        
        friend class LinkedList;
    };
    
    class ConstIterator {
    private:
        const Node* current;
        
    public:
        ConstIterator(const Node* node = nullptr) : current(node) {}
        
        const T& operator*() const {
            return current->data;
        }
        
        const T* operator->() const {
            return &current->data;
        }
        
        ConstIterator& operator++() {
            current = current->next;
            return *this;
        }
        
        ConstIterator operator++(int) {
            ConstIterator temp = *this;
            current = current->next;
            return temp;
        }
        
        ConstIterator& operator--() {
            current = current->prev;
            return *this;
        }
        
        bool operator==(const ConstIterator& other) const {
            return current == other.current;
        }
        
        bool operator!=(const ConstIterator& other) const {
            return current != other.current;
        }
    };
    
    LinkedList() : head(nullptr), tail(nullptr), length(0) {}
    
    ~LinkedList() {
        clear();
    }
    
    LinkedList(const LinkedList&) = delete;
    LinkedList& operator=(const LinkedList&) = delete;
    
    void push_back(const T& data) {
        Node* new_node = new Node(data);
        
        if (tail == nullptr) {
            head = tail = new_node;
        } else {
            new_node->prev = tail;
            tail->next = new_node;
            tail = new_node;
        }
        length++;
    }
    
    template <typename... Args>
    void emplace_back(Args&&... args) {
        T data(forward<Args>(args)...);
        push_back(data);
    }
    
    void push_front(const T& data) {
        Node* new_node = new Node(data);
        
        if (head == nullptr) {
            head = tail = new_node;
        } else {
            new_node->next = head;
            head->prev = new_node;
            head = new_node;
        }
        length++;
    }
    
    Iterator erase(Iterator it) {
        if (it.current == nullptr) return end();
        
        Node* node = it.current;
        Iterator next_it(node->next);
        
        if (node->prev) {
            node->prev->next = node->next;
        } else {
            head = node->next;
        }
        
        if (node->next) {
            node->next->prev = node->prev;
        } else {
            tail = node->prev;
        }
        
        delete node;
        length--;
        
        return next_it;
    }
    
    T& front() {
        return head->data;
    }
    
    const T& front() const {
        return head->data;
    }
    
    T& back() {
        return tail->data;
    }
    
    const T& back() const {
        return tail->data;
    }
    
    bool empty() const {
        return length == 0;
    }
    
    size_t size() const {
        return length;
    }
    
    void clear() {
        Node* current = head;
        while (current) {
            Node* next = current->next;
            delete current;
            current = next;
        }
        head = tail = nullptr;
        length = 0;
    }
    
    Iterator begin() {
        return Iterator(head);
    }
    
    ConstIterator begin() const {
        return ConstIterator(head);
    }
    
    Iterator end() {
        return Iterator(nullptr);
    }
    
    ConstIterator end() const {
        return ConstIterator(nullptr);
    }
};

#endif
