#pragma once
#include "headers.hpp"

namespace ProxyUtils
{
    void resolve_endpoints(const char*, const char*, std::vector<asio::ip::tcp::endpoint>&, asio::io_context&, asio::error_code&);
}