#ifndef SINGLY_LINKED_LIST_HPP
#define SINGLY_LINKED_LIST_HPP

#include <functional>
#include <iterator>

template <typename T>
class SinglyLinkedList {
private:
    struct Node {
        T data;
        Node* next;
        Node(const T& d) : data(d), next(nullptr) {}
    };

    Node* head;
    Node* tail;
    size_t count;

public:
    // ============================================================
    // Iterator
    // ============================================================
    class Iterator {
        Node* current;
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using reference = T&;
        using pointer = T*;

        Iterator(Node* n = nullptr) : current(n) {}
        T& operator*() { return current->data; }
        T* operator->() { return &current->data; }
        Iterator& operator++() { current = current->next; return *this; }
        bool operator!=(const Iterator& other) const { return current != other.current; }
        bool operator==(const Iterator& other) const { return current == other.current; }
        Node* get_node() const { return current; }
    };

    // ============================================================
    // Constructors / Destructors
    // ============================================================
    SinglyLinkedList() : head(nullptr), tail(nullptr), count(0) {}
    ~SinglyLinkedList() { clear(); }

    Iterator begin() const { return Iterator(head); }
    Iterator end() const { return Iterator(nullptr); }

    bool empty() const { return count == 0; }
    size_t size() const { return count; }

    // ============================================================
    // Push element at end
    // ============================================================
    void push_back(const T& value) {
        Node* node = new Node(value);
        if (!head)
            head = tail = node;
        else {
            tail->next = node;
            tail = node;
        }
        count++;
    }

    // ============================================================
    // Find element (by predicate)
    // ============================================================
    template <typename Predicate>
    Iterator find_if(Predicate pred) const {
        Node* cur = head;
        while (cur) {
            if (pred(cur->data))
                return Iterator(cur);
            cur = cur->next;
        }
        return end();
    }

    // ============================================================
    // Erase first element matching predicate
    // ============================================================
    template <typename Predicate>
    bool erase_if(Predicate pred) {
        Node* cur = head;
        Node* prev = nullptr;

        while (cur) {
            if (pred(cur->data)) {
                if (prev)
                    prev->next = cur->next;
                else
                    head = cur->next;

                if (cur == tail)
                    tail = prev;

                delete cur;
                count--;
                return true;
            }
            prev = cur;
            cur = cur->next;
        }
        return false;
    }

    // ============================================================
    // Clear entire list
    // ============================================================
    void clear() {
        Node* cur = head;
        while (cur) {
            Node* next = cur->next;
            delete cur;
            cur = next;
        }
        head = tail = nullptr;
        count = 0;
    }
};

#endif // SINGLY_LINKED_LIST_HPP
