#include "../headers/headers.hpp"

#define BUFFERSIZE 1024

struct Benchmark{
    size_t reads = 0;
    size_t writes = 0;
    size_t buffer_size = 0;
    size_t socket_item_count_mbs = 0;
    std::chrono::duration<double> seconds;
};

void MinrcraftProxy(const char*, const char*);
void SocketTest(int n, Benchmark* b);


int main(int argc, char* argv[]){
    try{
        if(argc == 2)
        {
            MinrcraftProxy(argv[1], "25565");
        }else{
            MinrcraftProxy("na.hypixel.net", "25565");
        }

        // MinrcraftProxy("minemen.club", "25565");
    } catch(std::system_error error)
    {
        std::cout << error.what() << std::endl;
    }
    // SocketTest(atoi(argv[1]));

    // Benchmark benchmark{0, 0, BUFFERSIZE, 1000, std::chrono::duration<double>{}};

    // for(size_t i = 0; i < 1000; i++)
    // {
    //     // 1 GB * 256
    //     SocketTest(1, &benchmark);
    // }

    // std::cout << "Atomic Queue Size: " << BUFFERSIZE << std::endl;
    // std::cout << "Transferred 1 GB in " <<  benchmark.seconds.count() << std::endl;
    // std::cout << benchmark.reads / benchmark.seconds.count() << " Reads/Second" << std::endl;
    // std::cout << benchmark.writes / benchmark.seconds.count() << " Writes/Second" << std::endl;

    return 0;
}

void MinrcraftProxy(const char* hostname, const char* port)
{
    alignas(64)     uint8_t server_buffer[BUFFERSIZE];
    alignas(64)     uint8_t client_buffer[BUFFERSIZE];

    std::vector<asio::ip::tcp::endpoint> endpoints;
    asio::ip::tcp::endpoint localhost(asio::ip::make_address_v4("127.0.0.1"), 25565);
    asio::io_context io_context_;
    asio::error_code ec;

    // Get hypixel endpoints
    ProxyUtils::resolve_endpoints(hostname, port, endpoints, io_context_, ec);
    
    for(const auto& ep : endpoints)
    {
        std::cout << "Endpoint: " << ep.address().to_string() << std::endl;
    }

    if(endpoints.empty())
    {
        std::cout << "No Endpoints Found. Aborting..." << std::endl;
        return;
    }

    int i = 0;
    while(true){
        std::cout << "iteration " << i++ << std::endl;
        AsioSocket ClientSocket(io_context_, localhost);
        AsioSocket ServerSocket(io_context_, endpoints[0]);
        
        AtomicSPSCQueue<uint8_t> ClientToServerQueue(&ClientSocket, &ServerSocket, BUFFERSIZE, server_buffer, "C->S");
        AtomicSPSCQueue<uint8_t> ServerToClientQueue(&ServerSocket, &ClientSocket, BUFFERSIZE, client_buffer, "S->C");
        
        ClientSocket.await_connection();
        ServerSocket.connect();
        
        ClientToServerQueue.Start();
        ServerToClientQueue.Start();
        
        ClientToServerQueue.WaitForStop();
        ServerToClientQueue.WaitForStop();
    }
}

void SocketTest(int n, Benchmark* b)
{
    // create test buffers and queue buffer
    const size_t test_buffer_size = n * 1048576 ; // n MBs
    alignas(64) uint8_t* source_buffer = new uint8_t[test_buffer_size];
    alignas(64) uint8_t* dest_buffer = new uint8_t[test_buffer_size];
    alignas(64) uint8_t queue_buffer[BUFFERSIZE];
    
    // fill the source buffer with random values

    TestSocket sock;

    // initiate the "socket"
    sock.SetQueueSize(1024);
    sock.SetDestination(dest_buffer);
    sock.SetSource(source_buffer, source_buffer + test_buffer_size);

    // create our worker
    AtomicSPSCQueue<uint8_t> queue(&sock, &sock, BUFFERSIZE, queue_buffer, "A");

    sock.SetStoppingFunction([&](){queue.Stop();});


    // Start timer
    auto start = std::chrono::steady_clock::now();

    queue.Start(); 

    queue.WaitForStop();

    // end timer after queue has finished
    auto end = std::chrono::steady_clock::now();
    

    // performance
    size_t reads = sock.GetReadOperations();
    size_t writes = sock.GetWriteOperations();
    std::chrono::duration<double> seconds = end - start;

    // std::cout << "Atomic Queue Size: " << BUFFERSIZE << std::endl;
    // std::cout << "Transferred " << test_buffer_size << " bytes in " <<  seconds.count() << " seconds("<<(test_buffer_size / 1e6) / seconds.count() <<" Mbps)"<< std::endl;
    // std::cout << reads / seconds.count() << " Reads/Second" << std::endl;
    // std::cout << writes / seconds.count() << " Writes/Second" << std::endl;

    b->reads += reads;
    b->writes += writes;
    b->seconds += seconds;

    delete[] source_buffer;
    delete[] dest_buffer;
}