#pragma once
#include "IQ.h"

const int cap = 1000;

template <typename T>
class SimpleArrayQueue : public IQueue<T> {
    int size;
    T arr[cap]{};
    void shiftleft() {
        for (int i = 0;i + 1 < size;i++) {
            arr[i] = arr[i + 1];
        }
        size--;
    }
public:
    SimpleArrayQueue() : size{ 0 } {}

    void enqueue(const T& value) {
        if (try_enqueue(value)) {
            arr[size++] = value;
            return;
        }
        throw "Array is full";
    }
    bool try_enqueue(const T& value) {
        return size != cap;
    }
    T dequeue() {
        T val;
        if (try_dequeue(val)) {
            shiftleft();
            return val;
        }
        throw "Array is empty";
    }
    bool try_dequeue(T& out) {
        if (size != 0) {
            out = arr[0];
            return true;
        }
        return false;
    }
    size_t Size() const {
        return size;
    }
    bool empty() const {
        return size == 0;
    }
    size_t capacity() const {
        return cap;
    }
    void display()const {
         //for(int i=0;i<size;i++){
         //    cout<<arr[i]<<" ";
         //}
         //cout<<endl;
    }
};

