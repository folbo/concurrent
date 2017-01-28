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
    DotProduct = 3,
    DotProductChunked = 4
};

class worker_session : public std::enable_shared_from_this<worker_session> {

    using tcp = asio::ip::tcp;
    using socket = tcp::socket;

    matrix<int>& output_matrix;
public:
    worker_session(tcp::socket socket, matrix<int>& output) :
            socket_(std::move(socket)),
            output_matrix{output} {
        //
    }

    void start() {
        do_read_result_header();
    }

    bool is_working = false;

    std::deque<std::vector<char>> output_deq;

    void command_mul(std::vector<int> &a, std::vector<int> &b, int x, int y, int l)
    {
        //serialize to bytes
        int data_size = a.size()*sizeof(int) + b.size()*sizeof(int) + 3 * sizeof(int);

        std::vector<char> d;

        //append header frame info
        d.push_back((unsigned char) CommandType::DotProduct);

        auto len = get_bytes_int(data_size);
        d.insert(d.end(), len.begin(), len.end());

        // body
        auto x_v = get_bytes_int(x); //int row
        d.insert(d.end(), x_v.begin(), x_v.end());

        auto y_v = get_bytes_int(y); //int col
        d.insert(d.end(), y_v.begin(), y_v.end());

        auto l_v = get_bytes_int(l); //int W
        d.insert(d.end(), l_v.begin(), l_v.end());

        // append 2 vectors to buffer

        for(int& element : a) {
            auto bytes = get_bytes_int(element);
            d.insert(d.end(), bytes.begin(), bytes.end());
        }

        for(int& element : b) {
            auto bytes = get_bytes_int(element);
            d.insert(d.end(), bytes.begin(), bytes.end());
        }

        //print
        //std::cout << "sending: ";
        //for (int i = 0; i < d.size(); i++)
        //    std::cout << (int) d[i] << " ";
        //std::cout << std::endl;

        //write

        bool write_in_progress = !output_deq.empty();
        output_deq.push_back(d);
        if (!write_in_progress) {
            do_write();
        }

        is_working = true;
    }


    void command_mul_chunked(matrix<int> &a, matrix<int> &b, int x, int y, int la, int lb, int n)
    {
        //serialize to bytes
        int data_size = a.rows()*a.cols()*sizeof(int) + b.rows()*b.cols()*sizeof(int) + 4 * sizeof(int);

        std::vector<char> d;
        d.reserve(5 + data_size);

        //append header frame info
        d.push_back((unsigned char) CommandType::DotProductChunked);

        auto len = get_bytes_int(data_size);
        d.insert(d.end(), len.begin(), len.end());

        // body 1 + 4 + 5*4 = 25
        auto x_v = get_bytes_int(x); //int result row
        d.insert(d.end(), x_v.begin(), x_v.end());

        auto y_v = get_bytes_int(y); //int result col
        d.insert(d.end(), y_v.begin(), y_v.end());

        auto la_v = get_bytes_int(la); //int height of 1st matrix
        d.insert(d.end(), la_v.begin(), la_v.end());

        auto lb_v = get_bytes_int(lb); //int width of 2nd matrix
        d.insert(d.end(), lb_v.begin(), lb_v.end());

        auto n_v = get_bytes_int(n); //int common length
        d.insert(d.end(), n_v.begin(), n_v.end());
        // append 2 vectors to buffer


        char* eeeee = reinterpret_cast<char*>(a.get_data().data());
        int bytes_of_a = a.get_data().size()*sizeof(int);
        std::memcpy(&(d[25]), eeeee, bytes_of_a);

        //transpose
        //for(int i = 0; i < lb; i++) {
        //    auto cache = b.get_col(i);
        //    char* fffff = reinterpret_cast<char*>(cache.data());
        //    int bytes_of_col_of_b = cache.size()*sizeof(int);
        //    std::memcpy(&(d[25]), eeeee, bytes_of_col_of_b);
        //}

        //print
        std::cout << "sending: ";
        for (int i = 0; i < d.size(); i++)
            std::cout << (int) d[i] << " ";
        std::cout << std::endl;

        //write

        bool write_in_progress = !output_deq.empty();
        output_deq.push_back(d);
        if (!write_in_progress) {
            do_write();
        }

        is_working = true;
    }

private:
    void do_write() {
        asio::async_write(socket_,
                          asio::buffer(output_deq.front().data(), output_deq.front().size()),
                          [this](std::error_code ec, std::size_t /*length*/) {
                              if (!ec) {
                                  //std::cout << "do_write" << std::endl;
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
                                 //std::cout << "do_read__result_header" << std::endl;
                                 //std::cout << "read: " << length <<  " bytes, [] = ";
                                 //for (int i = 0; i < 5; i++)
                                 //    std::cout << (int) data_read_[i] << " ";
                                 //std::cout << std::endl;

                                 int data_length = get_int(data_read_ + 1);
                                 //std::cout << "received result. data_length = : " << data_length << ". reading body..." << std::endl;

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
                                 //std::cout << "do_read__result_body" << std::endl;
                                 //std::cout << "received body :";
                                 //for (int i = 0; i < 5 + length; i++)
                                 //    std::cout << (int) data_read_[i] << " ";
                                 //std::cout << std::endl;

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

        //std::cout << "handle result, row=" <<  row << ", col=" << col;
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