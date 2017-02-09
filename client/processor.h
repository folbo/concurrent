#ifndef PROCESSOR_H
#define    PROCESSOR_H

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <chrono>
#include <utility>
#include <deque>
#include <asio.hpp>
#include "data_model.h"

class processor : public std::enable_shared_from_this<processor> {

    using tcp = asio::ip::tcp;

public:

    processor(tcp::socket socket) :
            socket_(std::move(socket)) {
    }

    void start() {
        do_read_header();
    }

private:

    void do_read_header() {
        auto self(shared_from_this());
        asio::async_read(socket_,
                         asio::buffer(data_, 5),
                         [this, self](std::error_code ec, std::size_t length) {
                             if (!ec) {
                                 unsigned int data_length = get_uint(data_ + 1);
                                 do_read_body(data_length);
                             }
                             else {
                                 std::cout << "socket closed on do_read_result_header " << ec.message();
                                 socket_.close();
                             }
                         });
    }

    void do_read_body(std::size_t data_length) {
        auto self(shared_from_this());
        asio::async_read(socket_,
                         asio::buffer(data_ + 5, data_length),
                         [this, self](std::error_code ec, std::size_t length) {
                             if (!ec) {
                                 if (data_[0] == 3) // dot product
                                 {
                                     unsigned int size = get_uint(&(data_[13]));

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

                                     do_send_result(sum, indexes);
                                 }

                                 if (data_[0] == 4) // dot product
                                 {
                                     // number of elements in row/col
                                     unsigned int size = get_uint(&(data_[21]));
                                     std::cout << size << std::endl;

                                     std::vector<char> indexes(20);
                                     std::memcpy(indexes.data(), &(data_[5]), 20);

                                     unsigned int la = get_uint(&(data_[13]));
                                     unsigned int lb = get_uint(&(data_[17]));

                                     std::vector<int> a(size*la);
                                     std::vector<int> b(size*lb);

                                     char* a_start = &(data_[25]);
                                     char* b_start = &(data_[25 + la * size * sizeof(int)]);

                                     std::vector<int> c(la*lb);

                                     //print A
                                     //for (int a = 0; a < la * size; a++) {
                                     //    /std::cout << *( reinterpret_cast<int*>(a_start + a*sizeof(int)) )<< " ";
                                     //    if ((a + 1) % size == 0) std::cout << std::endl;
                                     //}

                                     //print B
                                     //for (int b = 0; b < lb * size; b++) {
                                     //    std::cout << *( reinterpret_cast<int*>(b_start + b*sizeof(int)) )<< " ";
                                     //    if ((b + 1) % size == 0) std::cout << std::endl;
                                     //}

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

                                     do_send_result_chunk(c, indexes);
                                 }
                             }
                             else {
                                 std::cout << "socket closed on do_read_result_body" << ec.message();
                                 socket_.close();
                             }
                         });
    }

    void do_send_result(int res, std::vector<char> indexes) {
        std::vector<char> b_res = get_bytes_int(res);
        indexes.insert(indexes.end(), b_res.begin(), b_res.end());

        data_model frame(CommandType::DotProduct, indexes);

        std::vector<char> buff = frame.serialize_frame();
        //std::cout << "created data of size: " << buff.size() << std::endl;

        auto self(shared_from_this());
        asio::async_write(socket_,
                          asio::buffer(buff.data(), buff.size()),
                          [this, self, buff](std::error_code ec, std::size_t /*length*/) {
                              if (!ec) {
                                  do_read_header();
                              }
                          });
    }

    void do_send_result_chunk(std::vector<int> matrix, std::vector<char> indexes) {
        std::vector<char> result(matrix.size()*sizeof(int) + indexes.size());

        std::memcpy(&(result[0]), &(indexes[0]), indexes.size());
        std::memcpy(&(result[indexes.size()]), matrix.data(), matrix.size()*sizeof(int));

        data_model frame(CommandType::DotProductChunk, result);

        std::vector<char> buff = frame.serialize_frame();

        auto self(shared_from_this());
        asio::async_write(socket_,
                          asio::buffer(buff.data(), buff.size()),
                          [this, self, buff](std::error_code ec, std::size_t /*length*/) {
                              if (!ec) {
                                  std::cout << "sent result: ";
                                  //for (int i = 0; i < buff.size(); i++)
                                  //    std::cout << (int) buff[i] << " ";
                                  std::cout << std::endl << "waiting for next frame...";

                                  do_read_header();
                              }
                          });
    }


// math

    int get_int(const char *buffer) {
        return int((unsigned char) (buffer[3]) << 24 |
                   (unsigned char) (buffer[2]) << 16 |
                   (unsigned char) (buffer[1]) << 8 |
                   (unsigned char) (buffer[0]));
    }

    unsigned int get_uint(const char *buffer) {
        return unsigned((unsigned char) (buffer[3]) << 24 |
                        (unsigned char) (buffer[2]) << 16 |
                        (unsigned char) (buffer[1]) << 8 |
                        (unsigned char) (buffer[0]));
    }

    std::vector<char> get_bytes_int(int obj) {
        std::vector<char> v(sizeof(int));
        for (unsigned i = 0; i < sizeof(int); ++i) {
            v[i] = obj & 0xFF;
            obj >>= 8;
        }
        return v;
    }

private:
    enum {
        max_length = 1000000
    };

    tcp::socket socket_;
    char data_[max_length];

    std::deque<data_model> process_queue;
};

#endif	/* PROCESSOR_H */
