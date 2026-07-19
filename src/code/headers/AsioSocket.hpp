#pragma once
#include "headers.hpp"

// Wrapper class for asio::ip::tcp::socket
// Supports connecting and being connected to
class AsioSocket : public SimpleSocket{
public:
    AsioSocket(asio::io_context&);
    virtual inline size_t write(void*, size_t) override;
    virtual inline size_t read(void*, size_t) override;
    virtual inline size_t available() override;

    virtual void WaitRead() override;
    virtual void WaitWrite() override;
    virtual void Close() override;

    void await_connection(asio::ip::tcp::acceptor&);
    void connect(asio::ip::tcp::endpoint&);
private:
    asio::ip::tcp::socket socket;
    asio::ip::tcp::endpoint receiving_endpoint;
    asio::error_code ec_w;
    asio::error_code ec_r;
};