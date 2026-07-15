#pragma once
#include "headers.hpp"

// T - the real socket we intend to use in write() and read();
class SimpleSocket
{
public:
    SimpleSocket() = default;
    virtual void write(void*, size_t) = 0;
    virtual void read(void*, size_t) = 0;
    virtual size_t available() = 0;
};