#pragma once
#include "headers.hpp"

// Wrapper class for asio::ip::tcp::socket
// Supports connecting and being connected to
class AsioSocket : public SimpleSocket{
public:
    AsioSocket(asio::io_context&, asio::ip::tcp::endpoint);
    virtual size_t write(void*, size_t) override;
    virtual size_t read(void*, size_t) override;
    virtual size_t available() override;
    void await_connection();
    void connect();
    virtual void WaitRead() override;
    virtual void WaitWrite() override;
    virtual void Close() override;
private:
    asio::ip::tcp::socket socket;
    asio::ip::tcp::acceptor acceptor;
    asio::ip::tcp::endpoint endpoint;
    asio::ip::tcp::endpoint recieving_endpoint;
    asio::error_code ec_w;
    asio::error_code ec_r;
};