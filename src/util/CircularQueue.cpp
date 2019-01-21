#include "CircularQueue.hpp"

#include <cstring>

#include "ints.hpp"

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
    head = tail = arr;
}