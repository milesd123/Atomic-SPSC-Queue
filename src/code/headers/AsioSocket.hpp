#pragma once
#include "headers.hpp"

class AsioSocket : public SimpleSocket{
public:
    AsioSocket(asio::io_context&, asio::error_code, asio::ip::tcp::endpoint);
    virtual void write(void*, size_t) override;
    virtual void read(void*, size_t) override;
    virtual size_t available() override;
    void await_connection();
    void connect();
private:
    asio::ip::tcp::socket socket;
    asio::ip::tcp::acceptor acceptor;
    asio::ip::tcp::endpoint endpoint;
    asio::ip::tcp::endpoint recieving_endpoint;
    asio::error_code ec;
};