#ifndef BOUNDED_BLOCKING_QUEUE_HPP
#define BOUNDED_BLOCKING_QUEUE_HPP

#include <pthread.h>
#include <iostream>

template<typename T>
class BoundedBlockingQueue {
private:
    T* buffer;
    int capacity;
    int size;
    int front;
    int rear;
    
    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
    
public:
    BoundedBlockingQueue(int cap) : capacity(cap), size(0), front(0), rear(0) {
        buffer = new T[capacity];
        pthread_mutex_init(&mutex, nullptr);
        pthread_cond_init(&not_empty, nullptr);
        pthread_cond_init(&not_full, nullptr);
    }
    
    ~BoundedBlockingQueue() {
        delete[] buffer;
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&not_empty);
        pthread_cond_destroy(&not_full);
    }
    
    void enqueue(const T& item) {
        pthread_mutex_lock(&mutex);
        
        while (size == capacity) {
            pthread_cond_wait(&not_full, &mutex);
        }
        
        buffer[rear] = item;
        rear = (rear + 1) % capacity;
        size++;
        
        pthread_cond_signal(&not_empty);
        pthread_mutex_unlock(&mutex);
    }
    
    T dequeue() {
        pthread_mutex_lock(&mutex);
        
        while (size == 0) {
            pthread_cond_wait(&not_empty, &mutex);
        }
        
        T item = buffer[front];
        front = (front + 1) % capacity;
        size--;
        
        pthread_cond_signal(&not_full);
        pthread_mutex_unlock(&mutex);
        
        return item;
    }
    
    bool try_dequeue(T& item) {
        pthread_mutex_lock(&mutex);
        
        if (size == 0) {
            pthread_mutex_unlock(&mutex);
            return false;
        }
        
        item = buffer[front];
        front = (front + 1) % capacity;
        size--;
        
        pthread_cond_signal(&not_full);
        pthread_mutex_unlock(&mutex);
        
        return true;
    }
    
    int get_size() {
        pthread_mutex_lock(&mutex);
        int current_size = size;
        pthread_mutex_unlock(&mutex);
        return current_size;
    }
    
    bool is_empty() {
        pthread_mutex_lock(&mutex);
        bool empty = (size == 0);
        pthread_mutex_unlock(&mutex);
        return empty;
    }
    
    bool is_full() {
        pthread_mutex_lock(&mutex);
        bool full = (size == capacity);
        pthread_mutex_unlock(&mutex);
        return full;
    }
};

#endif