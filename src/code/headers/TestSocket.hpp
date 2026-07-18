#pragma once
#include "headers.hpp"

class TestSocket: public SimpleSocket
{
public:
    TestSocket();
    virtual size_t write(void*, size_t) override;
    virtual size_t read(void*, size_t) override;
    virtual size_t available() override;
    virtual void WaitRead() override;
    virtual void WaitWrite() override;
    virtual void Close() override;
    void SetQueueSize(size_t);
    size_t GetReadOperations();
    size_t GetWriteOperations();
    const void* GetSource();
    const void* GetDestination();
    void SetSource(void*, const void*);
    void SetDestination(void*);
    void SetStoppingFunction(std::function<void(void)>);

private:

    alignas(64) void* dest_buffer;
    size_t write_operations = 0;

    alignas(64) const void* source_buffer;
    size_t read_operations = 0;
    const void* source_buffer_end;

    // random ass numbers, yes defined in header file
    size_t current_available = 11;
    size_t rng_mult = 23;
    size_t rng_incr = 57;
    size_t mod = 1023; // same as buffer size;
    std::function<void(void)> stopping_function;
};