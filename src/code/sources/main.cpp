#include "../headers/headers.hpp"

#define BUFFERSIZE 1024

alignas(64)     uint8_t server_buffer[BUFFERSIZE];
alignas(64)     uint8_t client_buffer[BUFFERSIZE];

int main(){
    std::vector<asio::ip::tcp::endpoint> endpoints;
    asio::ip::tcp::endpoint localhost(asio::ip::make_address_v4("127.0.0.1"), asio::ip::port_type(25565));
    asio::io_context io_context_;
    asio::error_code ec;

    // Get hypixel endpoints
    ProxyUtils::resolve_endpoints("mc.hypixel.net", "25565", endpoints, io_context_, ec);

    if(endpoints.empty()){
        std::cout << "No Endpoints Found. Aborting..." << std::endl;
        return 1;
    }

    AsioSocket ClientSocket(io_context_, ec, localhost);
    AsioSocket ServerSocket(io_context_, ec, endpoints[0]);

    AtomicSPSCQueue<uint8_t> ClientToServerQueue(&ClientSocket, &ServerSocket, BUFFERSIZE, server_buffer);
    AtomicSPSCQueue<uint8_t> ServerToClientQueue(&ServerSocket, &ClientSocket, BUFFERSIZE, client_buffer);

    ClientSocket.await_connection();
    ServerSocket.connect();

    ClientToServerQueue.Start();
    ServerToClientQueue.Start();

    for(;;); // stall forever

    return 0;
}