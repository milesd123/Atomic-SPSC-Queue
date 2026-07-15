#include "../headers/ProxyUtils.hpp"

void ProxyUtils::resolve_endpoints(const char* host, const char* port, std::vector<asio::ip::tcp::endpoint>& endpoints, asio::io_context& ctx, asio::error_code& ec)
{
    // create a resolver
    asio::ip::tcp::resolver resolver(ctx);

    // resolve hypixel into an ip address (may be v4 or v6)
    asio::ip::basic_resolver_results<asio::ip::tcp> res = resolver.resolve(
        asio::ip::tcp::v4(), host, port, ec
    );

    if(ec){
        std::cout << "Resolver Error: " << ec.message() << std::endl;
        return;
    }

    asio::ip::basic_resolver_iterator<asio::ip::tcp> iter = res.begin();

    auto end = res.end();

    while(iter != end)
    {
        endpoints.push_back(*iter++);
    }


}
