#pragma once

#include <cstdlib>
#include <cstring>

#include <nowide/iostream.hpp>

#include "primitives.hpp"

template<typename T>
class CircularQueue {
public:
    CircularQueue<T>(size_t size);
    ~CircularQueue<T>();

    bool enqueue(T const& elem);
    bool dequeue(T &elem);
    bool dequeue();

    size_t size(void);
    void clear(void);

    T const& at(size_t idx);
    void replace(size_t idx, T const& elem);

protected:
    T *start, *head, *tail, *end;

    T *_get_pos(size_t idx);
};

template<typename T>
T *CircularQueue<T>::_get_pos(size_t idx) {
    if(size() <= idx) {
        return nullptr;
    }

    T *pos = head + idx;
    if(pos >= end) {
        pos -= (end - start);
    }

    return pos;
}

template<typename T>
CircularQueue<T>::CircularQueue(size_t size):
start(new T[size + 1]),
head(start),
tail(start),
end(start+size+1) {}

template<typename T>
CircularQueue<T>::~CircularQueue() {
    delete[] start;
}

template<typename T>
bool CircularQueue<T>::enqueue(T const& elem) {
    if((tail+1) != head) {
        tail++;

        if(tail == end) {
            if(head == start) {
                tail--;
                return false;
            }
            else tail = start;
        }

        *tail = elem;
        return true;
    }
    else {
        return false;
    }
}

template<typename T>
bool CircularQueue<T>::dequeue(T &elem) {
    if(head != tail) {
        head++;
        if(head == end) head = start;

        elem = *head;
        return true;
    }

    return false;
}

template<typename T>
bool CircularQueue<T>::dequeue() {
    if(head != tail) {
        head++;
        if(head == end) head = start;

        return true;
    }

    return false;
}

template<typename T>
size_t CircularQueue<T>::size(void) {
    if      (head <  tail) return tail - head;
    else if (head >  tail) return (end-start)-(head-tail);
    else                   return 0;
}

template<typename T>
void CircularQueue<T>::clear(void) {
    head = tail = start;
}

template<typename T>
T const& CircularQueue<T>::at(size_t idx) {
    if(size() <= idx) {
        throw std::invalid_argument("idx must be less than size");
    }
    return *_get_pos(idx);
}

template<typename T>
void CircularQueue<T>::replace(size_t idx, T const& elem) {
    if(size() <= idx) {
        throw std::invalid_argument("idx must be less than size");
    }
    T* pos = _get_pos(idx);

    *pos = elem;
}