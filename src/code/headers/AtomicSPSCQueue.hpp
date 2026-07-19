#pragma once
#include "headers.hpp"

// Atomic Single Producer Single Consumer Queue 
// 'to' and 'from' should be treated as 'to/from this object'
class AtomicSPSCQueue 
{
public:
    AtomicSPSCQueue(SimpleSocket* to, SimpleSocket* from, size_t buf_size, uint8_t* buf, const char* name);
    void Start();
    void WaitForStop();
    void Stop();

private:    
    inline size_t FastModulo(size_t);
    void ReadFromBuffer();
    void WriteToBuffer();

    std::thread worker_to;
    std::thread worker_from;
    SimpleSocket* source_socket; // leaving non-const for the future
    SimpleSocket* dest_socket;

    const size_t buffer_size;

    bool running_na = false;

    uint8_t* const buffer; 

    const char* name;

    std::atomic<bool> running{false};

    // Place all of the writer/reader and their variables in different cache lines
    alignas(64) std::atomic_size_t writer{0};
    alignas(64) std::atomic_size_t reader{0};
};