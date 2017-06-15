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
#include "../common/protocol/chunk_response.h"

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

        do_send_hello();
    }

    ~client(){
        std::cout << "destroyed client" << std::endl;
    }

private:
    void do_send_hello()
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

    void do_read_header()
    {
        socket_.async_receive_from(asio::buffer(data_, 8192),
                                   last_sender_endpoint_,
                                   [this](std::error_code ec, std::size_t length) {
            if(!ec){
                if(last_sender_endpoint_ != server_endpoint_)
                    return;

                unsigned int data_length = bytes_converter::get_uint(data_ + 1);

                std::cout << "received frame. expected data length=" << data_length << std::endl;


                // //

                // //

                if (data_[0] == (char)CommandType::DotProduct) // dot product
                {
                    unsigned int size = bytes_converter::get_uint(&(data_[13]));

                    std::vector<char> indexes(8);
                    std::memcpy(indexes.data(), &(data_[5]), 8);

                    std::vector<int> a(size);
                    std::vector<int> b(size);

                    std::memcpy(a.data(), &(data_[17]), size * sizeof(int));
                    std::memcpy(b.data(), &(data_[17 + (size * sizeof(int))]), size * sizeof(int));

                    int sum = 0;
                    for (int k = 0; k < size; k++) {
                        sum += a[k] * b[k];
                    }

                    //do_send_result(sum, indexes);
                }

                if (data_[0] == (char)CommandType::DotProductChunked) // dot product chunked
                {

                    unsigned int row = bytes_converter::get_uint(&(data_[5]));
                    unsigned int col = bytes_converter::get_uint(&(data_[9]));
                    unsigned int la = bytes_converter::get_uint(&(data_[13]));
                    unsigned int lb = bytes_converter::get_uint(&(data_[17]));
                    // number of elements in row/col
                    unsigned int size = bytes_converter::get_uint(&(data_[21]));

                    std::cout << "row: " << row << ", col: " << col << ", length: " << size << std::endl;

                    std::vector<char> indexes(20);
                    std::memcpy(indexes.data(), &(data_[5]), 20);

                    std::vector<int> a(size*la);
                    std::vector<int> b(size*lb);

                    char* a_start = &(data_[25]);
                    char* b_start = &(data_[25 + la * size * sizeof(int)]);

                    std::vector<int> c(la*lb);


                    for(unsigned int i = 0; i < la; i++){ //row
                        for(unsigned int j = 0; j < lb; j++){ //col
                            int* ptr_a = reinterpret_cast<int*>(a_start + i * size * sizeof(int));
                            int* ptr_b = reinterpret_cast<int*>(b_start + j * size * sizeof(int));

                            int sum = 0;
                            for(int k = 0; k < size; k++){ //col
                                sum += ptr_a[k] * ptr_b[k];
                            }

                            c[i * lb + j] = sum;
                        }
                    }

                    do_send_result_chunk(c, row, col, la, lb, size);
                }

                // //
                // //
            }
        });
    }

    void do_send_result_chunk(std::vector<int> matrix,
                              unsigned int row,
                              unsigned int col,
                              unsigned int la,
                              unsigned int lb,
                              unsigned int n)
    {
        std::vector<char> result(20 + matrix.size()*sizeof(int));

        chunk_response dto(row, col, la, lb, n, matrix);
        std::vector<char> serialized_frame = dto.get_data();

        socket_.async_send_to(asio::buffer(serialized_frame.data(), serialized_frame.size()),
                              server_endpoint_,
                              [this](std::error_code ec, std::size_t length) {
            if (!ec) {
                std::cout << "sent result";
                //for (int i = 0; i < buff.size(); i++)
                //    std::cout << (int) buff[i] << " ";
                std::cout << std::endl << "waiting for next frame..." << std::endl;
                do_read_header();
            }
        });
    }

private:
    asio::io_service& io_service_;
    udp::socket socket_;
    udp::endpoint server_endpoint_;
    udp::endpoint last_sender_endpoint_;

    char data_[8192];
};

#endif //PROJECT_CLIENT_H
