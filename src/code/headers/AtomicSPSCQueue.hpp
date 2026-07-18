#pragma once
#include "headers.hpp"

#define FastModulo(x) (x & 1023)

// Atomic Single Producer Single Consumer Queue 
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
        running.store(true, std::memory_order_release);
        worker_from = std::thread(&AtomicSPSCQueue::ReadFromBuffer, this);
        worker_to = std::thread(&AtomicSPSCQueue::WriteToBuffer, this);
    }

    void WaitForStop()
    {
        if(running.load() == false) return;
        worker_from.join();
        worker_to.join();
    }

    void Stop()
    {
        running.store(false);
    }

private:    
    void ReadFromBuffer() noexcept
    {
        // assuming that nothing is added inbetween these variables(and 64-byte cache lines) 
        // they should be on the same cache page;
        alignas(64) size_t writer_ = 0;
        size_t reader_ = 0;
        size_t space_until_end = 0;
        size_t read_index = 0;
        size_t read_amount = 0;

        while (running.load(std::memory_order_relaxed))
        {
            reader_ = reader.load(std::memory_order_relaxed);
            writer_ = writer.load(std::memory_order_acquire);
            read_index = FastModulo(reader_);

            read_amount = writer_ - reader_;

            space_until_end = buffer_size - read_index;

            if(read_amount >= space_until_end) read_amount = space_until_end;

            if(read_amount == 0) continue;

            // Increment by the amount read returned by the reader (may differ from the amount_w)
            reader.store(
                reader_ + socket_to->write(buffer + read_index, read_amount),
                std::memory_order_release
            );
        }
    }

    // Write to the buffer from the socket
    void WriteToBuffer() noexcept
    {

        alignas(64) size_t writer_ = 0;
        size_t reader_ = 0;
        size_t space_until_end = 0;
        size_t writer_index = 0;
        size_t write_amount = 0;

        while (running.load(std::memory_order_relaxed))
        {
            writer_ = writer.load(std::memory_order_relaxed);
            writer_index = FastModulo(writer_);
            reader_ = reader.load(std::memory_order_acquire);

            write_amount = buffer_size - (writer_ - reader_);

            space_until_end = buffer_size - writer_index;

            if(write_amount >= space_until_end) write_amount = space_until_end;

            if(write_amount == 0) continue; // TODO: Possibly remove me

            // Increment by the amount read returned by the reader (may differ from the amount_w)
            writer.store(
                writer_ + socket_from->read(buffer + writer_index, write_amount),
                std::memory_order_release
            );
        }
    }

    const size_t buffer_size;
    BufferType* const buffer; 
    SimpleSocket* socket_to; // leaving non-const for the future
    SimpleSocket* socket_from;
    std::thread worker_to;
    std::thread worker_from;
    std::atomic<bool> running{false};

    // Place all of the writer/reader and their variables in different cache lines
    alignas(64) std::atomic_size_t writer{0};
    alignas(64) std::atomic_size_t reader{0};
};