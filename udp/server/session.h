//
// Created by Jacek Dziubinski on 2017-06-09.
//

#ifndef PROJECT_SESSION_H
#define PROJECT_SESSION_H

#include <asio.hpp>
#include <iostream>
#include <deque>
#include "../common/bytes_converter.h"
#include "../common/protocol/frame.h"

using namespace asio::ip;

class session {
public:
    session(asio::io_service& io_service, udp::socket& socket, udp::endpoint remote_endpoint)
            : io_service_{io_service},
              socket_{socket},
              remote_endpoint_{remote_endpoint}
    {
        //do_read_result_header();
    }

    ~session(){
        std::cout << "destroyed session" << std::endl;
    }

    void send_data(std::vector<char> data)
    {
        std::cout <<". ";
        io_service_.post([this, data](){
            std::cout <<". ." << std::endl;
            bool write_in_progress = !output_queue.empty();
            output_queue.push_back(data);

            if (!write_in_progress) {
                do_write();
            }
        });
    }

private:
    void do_write()
    {
        std::vector<char>& top = output_queue.front();
        socket_.async_send_to(asio::buffer(top.data(), top.size()),
                              remote_endpoint_,
                              [this](std::error_code ec, std::size_t length){
                                  if(!ec) {
                                      output_queue.pop_front();

                                      if (!output_queue.empty())
                                          do_write();
                                  }
                                  else {
                                      std::cout << "error: " << ec.message() << std::endl;
                                  }
                              });
    }

    void do_read_result_header()
    {
        socket_.async_receive_from(asio::buffer(data_read_, frame::header_length),
                                   last_sender_endpoint_,
                                   [this](std::error_code ec, std::size_t length){
                                       if(last_sender_endpoint_ != remote_endpoint_)
                                           return;

                                       do_read_result_body(bytes_converter::get_int(&(data_read_[1])));
                                   });
    }

    void do_read_result_body(std::size_t data_length)
    {
        socket_.async_receive_from(asio::buffer(data_read_ + 5, data_length),
                                   last_sender_endpoint_,
                                   [this](std::error_code ec, std::size_t length){
                                       if(last_sender_endpoint_ != remote_endpoint_)
                                           return;

                                       char d[length];
                                       std::memcpy(d, &(data_read_[5]), length);

                                       if(data_read_[0] == (char)CommandType::DotProduct)
                                           ;//handle_result_dotproduct(d);
                                       if(data_read_[0] == (char)CommandType::DotProductChunked)
                                           ;//handle_result_chunk(d);

                                       do_read_result_header();
                                   });
    }

private:
    asio::io_service& io_service_;
    udp::socket& socket_;
    udp::endpoint remote_endpoint_;

    udp::endpoint last_sender_endpoint_;

    std::deque<std::vector<char>> output_queue;

    char data_read_[64000];
};

#endif //PROJECT_SESSION_H
