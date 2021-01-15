#pragma once

#include <cstdlib>
#include <cstring>

#include "util/ints.hpp"

template<typename T>
class CircularQueue {
public:
    CircularQueue<T>(size_t size);

    bool enqueue(T elem);
    T dequeue(void);

    size_t size(void);
    void clear(void);

protected:
    T *start, *head, *tail, *end;
};

template<typename T>
CircularQueue<T>::CircularQueue(size_t size):
start(static_cast<T *>(malloc(sizeof(T) * size))),
head(start),
tail(start),
end(start+(sizeof(T) * size)) {}

template<typename T>
bool CircularQueue<T>::enqueue(T elem) {
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
    else return false;
}

template<typename T>
T CircularQueue<T>::dequeue(void) {
    T retval;
    if(head != tail) {
        head++;
        if(head == end) head = start;

        retval = *head;

        return retval;
    }
    else return (T)0;
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