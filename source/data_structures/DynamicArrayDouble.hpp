#pragma once
#include<initializer_list>
using namespace std;

template <typename T>
class DynamicArrayDouble {
private:
    T* data;
    int size;
    int capacity;
public:
    // constructors - can be more than 1
    DynamicArrayDouble() : capacity{ 1 }, size{ 1 } {
        data = new T{};
    }

    DynamicArrayDouble(int size) : capacity{ size }, size{ size } {
        data = new T[capacity]{};
    }

    DynamicArrayDouble(int size, initializer_list<T> elm) : capacity{ size }, size{ size } {
        if (elm.size() > size) {
            throw "double dynamic construction error";
        }
        else {
            data = new T[capacity]{};
            for (int i = 0;i < size;i++) {
                data[i] = *(elm.begin() + i);
                if (elm.size() == i + 1)
                    break;
            }
        }
    }

    DynamicArrayDouble(initializer_list<T> elm) {
        size = elm.size();
        capacity = size;
        data = new T[capacity]{};
        for (int i = 0;i < size;i++) {
            data[i] = *(elm.begin() + i);
        }
    }

    // function for insertion      
    void push_back(T value) {
        if (size == capacity) {
            capacity *= 2;
            T* d = new T[capacity]{};
            for (int i = 0;i < size;i++) {
                d[i] = data[i];
            }
            delete[] data;
            data = d;
        }
        data[size++] = value;
    }

    // operators for update/read
    T operator[](int index) const {
        return data[index];
    }
    T& operator[](int index) {
        return data[index];
    }

    // implement helper functions: get_size, print array, etc, and other utility functions
    size_t get_size() {
        return size;
    }
    size_t get_capacity() const {
        return capacity;
    }
    void print_array()const {
        cout << "[ ";
        for (int i = 0;i < size;i++) {
            cout << data[i] << ",";
        }
        cout << "\b ]" << endl;
    }
    friend ostream& operator<<(ostream& out, const DynamicArrayDouble<T>& other) {
        // implement printing elements here
        other.print_array();
        return out;
    }

    // destructor
    ~DynamicArrayDouble() {
        delete[] data;
        data=nullptr;
    }
};

