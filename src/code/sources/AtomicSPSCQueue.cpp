#include "../headers/AsioSocket.hpp"

// Atomic Single Producer Single Consumer Queue 
// 'to' and 'from' should be treated as 'to/from this object'

AtomicSPSCQueue::AtomicSPSCQueue
(
    SimpleSocket* to, SimpleSocket* from, size_t buf_size, uint8_t* buf, const char* name
)
:   source_socket(to), dest_socket(from), buffer(buf), buffer_size(buf_size), name(name) 
{
    // assert buffer is a power of 2 for FastModulo() to work correctly
    assert(buf_size & (buf_size - 1) == 0);
}

void AtomicSPSCQueue::Start()
{
    running.store(true, std::memory_order_release);
    running_na = true;
    worker_from = std::thread(&AtomicSPSCQueue::ReadFromBuffer, this);
    worker_to = std::thread(&AtomicSPSCQueue::WriteToBuffer, this);
}

void AtomicSPSCQueue::WaitForStop()
{
    if(!running_na) return;
    running_na = false;
    if(worker_from.joinable()) worker_from.join();
    if(worker_to.joinable()) worker_to.join();
}

void AtomicSPSCQueue::Stop()
{
    if(!running_na) return;
    running_na = false;
    running.store(false);
    if(worker_from.joinable()) worker_from.join();   // <-- called FROM worker_from itself
    if(worker_to.joinable()) worker_to.join();
    std::cout << name << " Stopped." << std::endl;
}


void AtomicSPSCQueue::ReadFromBuffer()
{
    std::cout <<name<< " ReadFromBuffer Started: " << std::this_thread::get_id() <<std::endl;
    // assuming that nothing is added inbetween these variables(and 64-byte cache lines) 
    // they should be on the same cache page;
    alignas(64) size_t writer_ = 0;
    size_t reader_ = 0;
    size_t space_until_end = 0;
    size_t read_index = 0;
    size_t read_amount = 0;
    size_t avail = 0;

    while (running.load(std::memory_order_relaxed))
    {
        reader_ = reader.load(std::memory_order_relaxed);
        writer_ = writer.load(std::memory_order_acquire);
        read_index = FastModulo(reader_);

        read_amount = writer_ - reader_;

        space_until_end = buffer_size - read_index;

        if(read_amount >= space_until_end) read_amount = space_until_end;

        if(read_amount == 0) continue;

        size_t done = dest_socket->write(buffer + read_index, read_amount);

        if(done == 0) {
            source_socket->Close();
            dest_socket->Close();
            running.store(false);
        }
        std::cout <<"\t\t\t\t"<<name <<" |Consumer Size:" << read_amount << ". Written to sock: " << done << std::endl;

        // Increment by the amount read returned by the reader (may differ from the amount_w)
        reader.store(
            reader_ + done,
            std::memory_order_release
        );
    }
    std::cout << name << " ReadFromBuffer Stopped: " << std::this_thread::get_id() <<std::endl;
}

// Write to the buffer from the socket
void AtomicSPSCQueue::WriteToBuffer()
{
    std::cout << name << " WriteToBuffer Started: " << std::this_thread::get_id() <<std::endl;

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

        size_t done = source_socket->read(buffer + writer_index, write_amount);

        if(done == 0) {
            source_socket->Close();
            dest_socket->Close();
            running.store(false);
        }

        std::cout <<name<<" Producer write_amount:" << write_amount << ". Written to buf: " << done << std::endl;

        // Increment by the amount read returned by the reader (may differ from the amount_w)
        writer.store(
            writer_ + done,
            std::memory_order_release
        );
    }
    std::cout << name << " WriteToBuffer Stopped: " << std::this_thread::get_id() <<std::endl;

}

inline size_t AtomicSPSCQueue::FastModulo(size_t n)
{
    return n & (buffer_size - 1);
}
