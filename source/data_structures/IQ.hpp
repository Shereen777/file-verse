

#include<iostream>
#include<initializer_list>
#include<memory>
using namespace std;

template < typename T >
class IQueue {

public:
    virtual ~IQueue() = default;
    virtual void enqueue(const T& value) = 0; // throws or block
    virtual bool try_enqueue(const T& value) = 0; // non -blocking , returns false if full
    virtual T dequeue() = 0; // throws or blocks if empty
    virtual bool try_dequeue(T& out) = 0; // non -blocking
    virtual size_t Size() const = 0;
    virtual bool empty() const = 0;
    virtual size_t capacity() const = 0;
    virtual void display()const = 0;
};
