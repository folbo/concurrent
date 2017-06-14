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
#include "../common/protocol/chunk_response.h"

using namespace asio::ip;

class session {
public:
    session(asio::io_service& io_service, udp::socket& socket, udp::endpoint remote_endpoint, matrix<int>& output)
            : io_service_{io_service},
              socket_{socket},
              remote_endpoint_{remote_endpoint},
              output_matrix{output}
    {
        do_read_result_header();
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
                    std::cout <<"sent " << length << " bytes" << std::endl;
                    output_queue.pop_front();

                    if (!output_queue.empty()) {
                        do_write();

                        //socket_.receive_from(asio::buffer(top.data(), top.size()), last_sender_endpoint_);
                        //if(last_sender_endpoint_ != remote_endpoint_)
                        //    return;
                    }
            }
            else {
                std::cout << "error: " << ec.message() << std::endl;
            }
        });
    }

    void do_read_result_header()
    {
        socket_.async_receive_from(asio::buffer(data_read_, 8192),
                                   last_sender_endpoint_,
                                   [this](std::error_code ec, std::size_t length){
            if(last_sender_endpoint_ != remote_endpoint_)
                return;

            char d[length-5];
            std::memcpy(d, &(data_read_[5]), length);

            if(data_read_[0] == (char)CommandType::DotProduct)
                ;//handle_result_dotproduct(d);
            if(data_read_[0] == (char)CommandType::DotProductChunked)
                handle_result_chunk(d);

            do_read_result_header();
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
                handle_result_chunk(d);

             do_read_result_header();
        });
    }

    void handle_result_chunk(char* data){
        chunk_response dto(data);

        output_matrix.patch(*dto.mat.get(), dto.row, dto.col);
        std::cout << "received response.\n";
/*
        auto res_it = std::find_if(results.begin(),
                                   results.end(),
                                   [&response](const result& res)
                                   {
                                       return res.row == response.row && res.col == response.col;
                                   });

        if(res_it == results.end()){
            exit(-4);
        }

        res_it->calculated = true;
        */
    }

private:
    asio::io_service& io_service_;
    udp::socket& socket_;
    udp::endpoint remote_endpoint_;

    udp::endpoint last_sender_endpoint_;

    std::deque<std::vector<char>> output_queue;

    char data_read_[64000];

    matrix<int>& output_matrix;
};

#endif //PROJECT_SESSION_H
