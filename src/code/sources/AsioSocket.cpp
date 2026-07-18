#include "../headers/AsioSocket.hpp"

AsioSocket::AsioSocket
(
    asio::io_context& ctx, asio::ip::tcp::endpoint ep
)
:   acceptor(ctx), endpoint(ep), socket(ctx) {}

// write to the socket from a buffer
size_t AsioSocket::write(void* buffer, size_t amount)
{

    // return socket.write_some(asio::mutable_buffer(buffer, amount), ec_w);

    size_t write = socket.write_some(asio::mutable_buffer(buffer, amount), ec_w);
    static volatile int flag = 1;
    if(ec_w && flag)
    {
        std::cout << "[W]"<<std::this_thread::get_id();
        flag = 0;
    }
    return write;
}

// read from the socket into a buffer
size_t AsioSocket::read(void* buffer, size_t amount)
{
    // return socket.read_some(asio::mutable_buffer(buffer, amount), ec_r);

    // return asio::read(socket, asio::mutable_buffer(buffer, amount), ec_r);
    size_t read = socket.read_some(asio::mutable_buffer(buffer, amount), ec_r);
    static volatile int flag = 1;
    if(ec_r && flag)
    {
        std::cout << "[R]"<<std::this_thread::get_id();
        flag = 0;
    }
    return read;
}

// return the available bytes in the socket
size_t AsioSocket::available()
{
    return socket.available();
}

// bind to the endpoint, open on ipv4, listen and accept
void AsioSocket::await_connection()
{

    acceptor.open(endpoint.protocol()); // create OS socket descriptor
    acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));

    acceptor.bind(endpoint);
    acceptor.listen();

    std::cout << "["<< endpoint.address().to_string() <<":"<< endpoint.port()<< "] Awaiting connection..." << std::endl;
    acceptor.accept(socket, recieving_endpoint);

    std::cout << "Accepted Connection from: " << recieving_endpoint.address() << std::endl;
}

// connect to the endpoint
void AsioSocket::connect()
{
    socket.connect(endpoint, ec_w);

    if(ec_w){
        std::cout << "Error connecting to the endpoint " << endpoint <<": "<<ec_w.message()<<std::endl;
    } else{
        std::cout << "Connected to " << endpoint.address() << std::endl;
    }
}

// block until ready to read
void AsioSocket::WaitRead()
{
    socket.wait(asio::ip::tcp::socket::wait_read);
    // while(!socket.is_open());
}


// block until ready to write
void AsioSocket::WaitWrite()
{
    socket.wait(asio::ip::tcp::socket::wait_write);
    // while(!socket.is_open());
}

void AsioSocket::Close()
{
    asio::error_code ec;
    socket.cancel(ec);
    socket.close(ec);
}

// private:
//     asio::ip::tcp::socket socket;
//     asio::ip::tcp::acceptor acceptor;
//     asio::ip::tcp::endpoint endpoint;