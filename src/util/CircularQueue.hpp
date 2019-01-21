#pragma once

#include "ints.hpp"

template<typename T>
class CircularQueue {
public:
    CircularQueue<T>(size_t size);

    bool enqueue(T elem);
    T dequeue(void);

    size_t size(void);
    void clear(void);

private:
    T *start, *head, *tail, *end;
};