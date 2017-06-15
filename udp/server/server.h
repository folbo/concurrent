//
// Created by Jacek Dziubinski on 2017-06-08.
//

#ifndef PROJECT_SERVER_H
#define PROJECT_SERVER_H

#include <iostream>
#include <vector>
#include <map>
#include <memory>
#include <deque>
#include <asio.hpp>
#include "../common/protocol/frame.h"
#include "../common/bytes_converter.h"
#include "../common/matrix.h"
#include "../common/protocol/chunk_frame.h"
#include "session.h"
#include "result.h"

using namespace asio::ip;

class server {
public:
    server(asio::io_service & io_service, short port, matrix<int>& m)
        : io_service_{io_service},
          socket_{io_service, udp::endpoint(udp::v4(), port)},
          output_matrix{m}
    {
        listen_hello();
    }

    ~server(){
        std::cout << "destroyed server" << std::endl;
    }

    void close()
    {
    }

    void begin_mul_chunked(matrix<int>& m1, matrix<int>& m2, int chunk_size_a, int chunk_size_b) {
        int n = sessions_.size();
        int ad = 0;

        // auto
        if(chunk_size_a == -1){
            chunk_size_a = m1.rows() / n;
            std::cout << "auto-sizing A chunks: sending chunks of " << chunk_size_a << " rows from A." << std::endl;
        }

        if(chunk_size_b == -1){
            chunk_size_b = m1.cols() / n;
            std::cout << "auto-sizing B chunks: sending chunks of " << chunk_size_b << " cols from B." << std::endl;
        }


        try {
            int i = 0;
            for (; i < m2.cols() / chunk_size_b; i++) {
                int col = i * chunk_size_b; //col + chunk_size_b - ostatnia kolumna chunka
                auto chunk2 = m2.get_cols(col, chunk_size_b);

                int j = 0;
                for (; j < m1.rows() / chunk_size_a; j++) {
                    int row = j * chunk_size_a;
                    auto chunk1 = m1.get_rows(row, chunk_size_a);

                    chunk_frame dto(row, col, chunk_size_a, chunk_size_b, m1.cols(), chunk1, chunk2);
                    sessions_[ad]->send_data(dto);

                    ad++;
                    ad %= n;
                    //std::this_thread::sleep_for(std::chrono::seconds(10));
                }

                //last row chunk
                int last_chunk_size_a = m1.rows() % chunk_size_a;
                int row = j * chunk_size_a;
                auto chunk1 = m1.get_rows(row, last_chunk_size_a);

                chunk_frame dto(row, col, chunk_size_a, chunk_size_b, m1.cols(), chunk1, chunk2);
                sessions_[ad]->send_data(dto);

            }

            //last column chunk
            int last_chunk_size_b = m2.cols() % chunk_size_b;
            int col = i * chunk_size_b; //col + chunk_size_b - ostatnia kolumna chunka
            auto chunk2 = m2.get_cols(col, last_chunk_size_b);

            int j = 0;
            for (; j < m1.rows() / chunk_size_a; j++) {
                int last_chunk_size_a = m1.rows() % chunk_size_a;
                int row = j * chunk_size_a;
                auto chunk1 = m1.get_rows(row, chunk_size_a);

                chunk_frame dto(row, col, chunk_size_a, chunk_size_b, m1.cols(), chunk1, chunk2);
                sessions_[ad]->send_data(dto);

                ad++;
                ad %= n;
            }
            int last_chunk_size_a = m1.rows() % chunk_size_a;
            int row = j * chunk_size_a;
            auto chunk1 = m1.get_rows(row, last_chunk_size_a);

            chunk_frame dto(row, col, chunk_size_a, chunk_size_b, m1.cols(), chunk1, chunk2);
            sessions_[ad]->send_data(dto);

        }
        catch(std::exception &e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
    }

    bool check_done(){
        //if (std::any_of(sessions_.cbegin(), sessions_.cend(),
                        //[](const std::shared_ptr<session> &worker) { return worker->check_done() == false; }))
            return false;
        return true;
    }

private:
    void listen_hello()
    {
        socket_.async_receive_from(asio::buffer(buffer_, 8192),
                                   last_remote_endpoint_,
                                   [this](std::error_code ec, std::size_t length) {
            if(ec) {
                std::cout << "listening for ping end with error: " << ec.message();
            }
            if(buffer_[0] == (char)CommandType::Hello)
            {
                auto s = std::make_shared<session>(io_service_,
                                                  socket_,
                                                  last_remote_endpoint_,
                                                  output_matrix);
                sessions_.emplace_back(std::move(s));

                std::cout << "accepted connection from "
                          << last_remote_endpoint_.address().to_string()
                          << ":"
                          << last_remote_endpoint_.port()
                          << std::endl;

                socket_.send_to(asio::buffer(buffer_, 5), last_remote_endpoint_);
            }
            else {
                char d[length-5];
                std::memcpy(d, &(buffer_[5]), length);

                if(buffer_[0] == (char)CommandType::DotProduct)
                    ;//handle_result_dotproduct(d);
                if(buffer_[0] == (char)CommandType::DotProductChunked)
                    handle_result_chunk(d, last_remote_endpoint_);
            }

            listen_hello();
        });
    }

    void handle_result_chunk(char* data, udp::endpoint endpoint){
        try {
            chunk_response dto(data);

            output_matrix.patch(*dto.mat.get(), dto.row, dto.col);
            std::cout << "received response.\n";
        }
        catch(...){
            std::cout <<"mam dziada 143" <<std::endl;
        }
    }

private:
    std::vector<std::shared_ptr<session>> sessions_;

    udp::endpoint last_remote_endpoint_;
    asio::io_service & io_service_;
    udp::socket socket_;

    char buffer_[8192];

    matrix<int>& output_matrix;
};

#endif //PROJECT_SERVER_H
