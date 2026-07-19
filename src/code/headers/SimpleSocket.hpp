#pragma once
#include "headers.hpp"

// T - the real socket we intend to use in write() and read();
class SimpleSocket
{
public:
    SimpleSocket() = default;
    virtual inline size_t write(void*, size_t) = 0;
    virtual inline size_t read(void*, size_t) = 0;
    virtual inline size_t available() = 0;
    virtual void WaitRead() = 0;
    virtual void WaitWrite() = 0;
    virtual void Close() = 0;
};