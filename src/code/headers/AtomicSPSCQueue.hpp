#pragma once
#include "headers.hpp"

#define FastModulo(x) (x & 1023)

// 'to' and 'from' should be treated as 'to/from this object'
template<typename BufferType>
class AtomicSPSCQueue 
{
public:
    AtomicSPSCQueue
    (
        SimpleSocket* to, SimpleSocket* from, size_t buf_size, BufferType* buf
    )
    :   socket_to(to), socket_from(from), buffer(buf), buffer_size(buf_size) {}

    void Start()
    {
        // No Signal for stopping, for now...
        worker_from = std::thread(&AtomicSPSCQueue::ReadFromBuffer, this);
        worker_to = std::thread(&AtomicSPSCQueue::WriteToBuffer, this);
    }

    void Stop()
    {
        // Does nothing for now
    }

private:    
    void ReadFromBuffer() noexcept
    {
        // assuming that nothing is added inbetween these variables(and 64-byte cache lines) 
        // they should be on the same cache page;
        alignas(64) size_t start_inclusive_r;
        size_t end_exclusive_r;
        size_t amount_r;
        while (1)
        {
            // get our indexes
            start_inclusive_r = FastModulo(reader.load(std::memory_order_relaxed));
            end_exclusive_r = FastModulo(writer.load(std::memory_order_acquire));

            // determine where to stop reading
            if(start_inclusive_r > end_exclusive_r) end_exclusive_r = buffer_size;
            amount_r = end_exclusive_r - start_inclusive_r;

            // write to socket
            socket_to->write(buffer + start_inclusive_r, amount_r);

            // atomically increment read_in_index
            reader.store(start_inclusive_r + amount_r, std::memory_order_release);
        }
    }

    void WriteToBuffer() noexcept
    {
        alignas(64) size_t start_inclusive_w;
        size_t end_exclusive_w;
        size_t available_bytes_w;
        size_t amount_w;
        while (1)
        {
            start_inclusive_w = FastModulo(writer.load(std::memory_order_relaxed));
            end_exclusive_w = FastModulo(reader.load(std::memory_order_acquire));

            if(start_inclusive_w > end_exclusive_w) end_exclusive_w = buffer_size;

            amount_w = end_exclusive_w - start_inclusive_w;

            // determine how many bytes we CAN read
            available_bytes_w = socket_from->available();

            // compare with amount of space we have
            if(amount_w > available_bytes_w) amount_w = available_bytes_w;

            socket_from->read(buffer + start_inclusive_w, amount_w);

            writer.store(start_inclusive_w + amount_w);
        }
    }
    const size_t buffer_size;
    BufferType*  const buffer; 
    SimpleSocket* socket_to; // leaving non-const for the future
    SimpleSocket* socket_from;
    std::thread worker_to;
    std::thread worker_from;

    // Place all of the writer/reader and their variables in the same cache line
    alignas(64) std::atomic_size_t writer;
    alignas(64) std::atomic_size_t reader;
};