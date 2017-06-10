//
// Created by Jacek Dziubinski on 2017-06-08.
//

#ifndef PROJECT_CLIENT_H
#define PROJECT_CLIENT_H

#include <iostream>
#include <chrono>
#include <asio.hpp>
#include "../common/bytes_converter.h"
#include "../common/protocol/hello_frame.h"

using namespace asio::ip;

class client {
public:
    client(asio::io_service& io_service,  std::string server_ip, std::string server_port)
        : io_service_{io_service},
          socket_{io_service, udp::endpoint(udp::v4(), 0)}
    {
        udp::resolver resolver(io_service);
        udp::resolver::query query(udp::v4(), server_ip, server_port);
        server_endpoint_ = *resolver.resolve(query);

        send_hello();
    }

    ~client(){
        std::cout << "destroyed client" << std::endl;
    }

private:
    void send_hello()
    {
        hello_frame frame;
        std::vector<char> serialized_frame = frame.serialize();
        socket_.async_send_to(asio::buffer(serialized_frame.data(), serialized_frame.size()),
                              server_endpoint_,
                              [](std::error_code ec, std::size_t length){
            if(!ec)
                std::cout << "sent ping\n";
            else
                std::cout <<"problem with ping"; });

        char data[1000];
        socket_.async_receive_from(asio::buffer(data, 5),
                                   last_sender_endpoint_,
                                   [this](std::error_code ec, std::size_t length){
            if(!ec){
                if(last_sender_endpoint_ != server_endpoint_)
                    return;

                std::cout << "received pong\n";
                do_read_header();
            }
            else
                std::cout <<"problem with pong";
        });
    }

    void do_read_header() {

        socket_.async_receive_from(asio::buffer(data_, 5),
                                   last_sender_endpoint_,
                                   [this](std::error_code ec, std::size_t length){
            if(last_sender_endpoint_ != server_endpoint_)
                return;

            unsigned int data_length = bytes_converter::get_uint(data_ + 1);

            std::cout << "received frame \n";
            //do_read_body(data_length);
        });
    }

private:
    asio::io_service& io_service_;
    udp::socket socket_;
    udp::endpoint server_endpoint_;
    udp::endpoint last_sender_endpoint_;

    char data_[1000000];
};

#endif //PROJECT_CLIENT_H
