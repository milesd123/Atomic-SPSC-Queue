#include "../headers/AsioSocket.hpp"

AsioSocket::AsioSocket
(
    asio::io_context& ctx, asio::error_code ec, asio::ip::tcp::endpoint ep
)
:   acceptor(ctx), endpoint(ep), socket(ctx), ec(ec) {}

// write to the socket from a buffer
void AsioSocket::write(void* buffer, size_t amount)
{
    asio::write(socket, asio::mutable_buffer(buffer, amount));
}

// read from the socket into a buffer
void AsioSocket::read(void* buffer, size_t amount)
{
    asio::read(socket, asio::mutable_buffer(buffer, amount));
}

// return the available bytes in the socket
size_t AsioSocket::available()
{
    return socket.available();
}

// bind to the endpoint, open on ipv4, listen and accept
void AsioSocket::await_connection()
{
    acceptor.bind(endpoint);
    acceptor.open(asio::ip::tcp::v4());
    acceptor.listen();
    acceptor.accept(socket, recieving_endpoint);

    std::cout << "Accepted Connection from: " << recieving_endpoint.address() << std::endl;
}

// connect to the endpoint
void AsioSocket::connect()
{
    socket.connect(endpoint, ec);

    if(ec){
        std::cout << "Error connecting to the endpoint " << endpoint <<": "<<ec.message()<<std::endl;
    } else{
        std::cout << "Connected to " << endpoint.address() << std::endl;
    }
}

// private:
//     asio::ip::tcp::socket socket;
//     asio::ip::tcp::acceptor acceptor;
//     asio::ip::tcp::endpoint endpoint;