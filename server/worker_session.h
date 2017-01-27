#include <asio.hpp>
#include <iostream>
#include <vector>
#include <deque>
#include <memory>
#include "matrix.h"

#ifndef WORKER_SESSION_H
#define    WORKER_SESSION_H

enum class CommandType {
    Hello = 1,
    Please = 2,
    DotProduct = 3
};

class worker_session : public std::enable_shared_from_this<worker_session> {

    using tcp = asio::ip::tcp;
    using socket = tcp::socket;

    matrix& output_matrix;
public:
    worker_session(tcp::socket socket, matrix& output) :
            socket_(std::move(socket)),
            output_matrix{output} {
        //
    }

    void start() {
    }

    bool is_working = false;

    std::deque<std::vector<char>> output_deq;

    void command_mul(std::vector<char> &a, std::vector<char> &b, int x, int y, int l)
    {
        int data_size = a.size() + b.size() + 3 * sizeof(int);

        std::vector<char> d;

        //append header frame info
        d.push_back((unsigned char) CommandType::DotProduct);

        auto len = get_bytes_int(data_size);
        d.insert(d.end(), len.begin(), len.end());

        // body
        auto x_v = get_bytes_int(x); //int row, int col, int W, int N
        d.insert(d.end(), x_v.begin(), x_v.end());

        auto y_v = get_bytes_int(y); //int row, int col, int W, int N
        d.insert(d.end(), y_v.begin(), y_v.end());

        auto l_v = get_bytes_int(l); //int row, int col, int W, int N
        d.insert(d.end(), l_v.begin(), l_v.end());

        // append 3 vectors to buffer
        d.insert(d.end(), a.begin(), a.end());
        d.insert(d.end(), b.begin(), b.end());

        bool write_in_progress = !output_deq.empty();
        output_deq.push_back(d);
        if (!write_in_progress) {
            do_write();
        }

        do_read_result_header();
        is_working = true;
    }

private:
    void do_write() {
        asio::async_write(socket_,
                          asio::buffer(output_deq.front().data(), output_deq.front().size()),
                          [this](std::error_code ec, std::size_t /*length*/) {
                              if (!ec) {
                                  std::cout << "do_write" << std::endl;
                                  output_deq.pop_front();

                                  //immediately wait for response

                                  if(!output_deq.empty())
                                      do_write();
                              }
                              else {
                                  std::cout << "socket closed on do_write" << ec.message() << std::endl;
                                  socket_.close();
                              }
                          });
    }

    void do_read_result_header() {
        data_read_[0] = 111;
        asio::async_read(socket_,
                         asio::buffer(data_read_, 5),
                         [this](std::error_code ec, std::size_t length) {
                             if (!ec) {
                                 std::cout << "do_read_header" << std::endl;
                                 std::cout << "read: " << length <<  " bytes, [] = "
                                 << (int)data_read_[0] << " "
                                 << (int)data_read_[1] << " "
                                 << (int)data_read_[2] << " "
                                 << (int)data_read_[3] << " "
                                 << (int)data_read_[4] << std::endl;

                                 int data_length = get_int(data_read_ + 1);
                                 std::cout << "received result. data_length = : " << data_length << ". reading body..." << std::endl;

                                 do_read_result_body(data_length);
                             }
                             else {
                                 std::cout << "socket closed on do_read_result_header " << ec.message();
                                 socket_.close();
                             }
                         });
    }

    void do_read_result_body(std::size_t data_length) {
        asio::async_read(socket_,
                         asio::buffer(data_read_ + 5, data_length),
                         [this](std::error_code ec, std::size_t length) {
                             if (!ec) {
                                 std::cout << "received body :";
                                 for (int i = 0; i < 5 + length; i++)
                                     std::cout << (int) data_read_[i] << " ";
                                 std::cout << std::endl;

                                 char d[length];
                                 std::memcpy(d, &(data_read_[5]), length);
                                 handle_result(d);

                                 do_read_result_header();
                             }
                             else {
                                 std::cout << "socket closed on do_read_result_body" << ec.message() << std::endl;
                                 socket_.close();
                             }
                         });
    }

    void handle_result(char* data) {
        int row = get_int(&(data[0]));
        int col = get_int(&(data[4]));

        std::cout << "handle result, row=" <<  row << ", col=" << col;
        output_matrix(row, col) = get_int(&(data[8]));
    }

    int get_int(const char *buffer) {
        return int((unsigned char) (buffer[3]) << 24 |
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
        max_length = 4096
    };

    tcp::socket socket_;
    char data_read_[max_length];
    char data_write_[max_length];
    int max_threads;
};

#endif	/* WORKER_SESSION_H */