#ifndef CLIENT_H
#define	CLIENT_H

#include <asio.hpp>
#include "processor.h"

class client {

    using tcp = asio::ip::tcp;

public:
    client(asio::io_service& io_service, std::string host, std::string port) :
    io_service_(io_service),
    resolver_(io_service),
    socket_(io_service) {
        auto endpoint_iter = resolver_.resolve({host, port});
        asio::async_connect(socket_, endpoint_iter,
            [this](std::error_code ec, tcp::resolver::iterator) {
                if (!ec) {
                    std::cout << "connected" << std::endl;
                    std::make_shared<processor>(std::move(socket_))->start();
                    socket_.close();
                }
            });
    }

private:
    asio::io_service& io_service_;
    tcp::resolver resolver_;
    tcp::socket socket_;
};

#endif	/* CLIENT_H */
