#ifndef WORKER_SESSION_H
#define    WORKER_SESSION_H

#include <asio.hpp>
#include <iostream>
#include <vector>
#include <deque>
#include <memory>
#include <cstdint>
#include <list>
#include "matrix.h"
#include "protocol/chunk_frame.h"
#include "protocol/dotprod_frame.h"
#include "protocol/chunk_response.h"

struct result {
    result(int r, int c) :
            row{r},
            col{c},
            calculated{false} {
        //
    }

    int row;
    int col;
    bool calculated;
};


class worker_session : public std::enable_shared_from_this<worker_session> {

    using tcp = asio::ip::tcp;
    using socket = tcp::socket;

    matrix<int>& output_matrix;
public:
    worker_session(asio::io_service& io_service, tcp::socket socket, matrix<int>& output) :
            socket_(std::move(socket)),
            output_matrix{output},
            io_service_{io_service} {
        //
    }

    void start() {
        do_read_result_header();
    }

    bool is_working = false;

    std::deque<std::vector<char>> output_deq;

    void command_mul(std::vector<int> &a, std::vector<int> &b, int x, int y, int l)
    {
        auto d = dotprod_frame(x, y, l, a, b).get_data();

        io_service_.post(
                [this, d]() {
                    //std::cout << "sending" << std::endl;

                    //write
                    bool write_in_progress = !output_deq.empty();
                    output_deq.push_back(d);

                    if (!write_in_progress) {
                        do_write();
                    }
                });
    }

    void command_mul_chunked(matrix<int> a, matrix<int> b, unsigned int x, unsigned int y, unsigned int la, unsigned int lb, unsigned int n)
    {
        auto chunk_bytes = chunk_frame(x, y, la, lb, n, a, b).get_data();

        io_service_.post(
                [this, chunk_bytes, x, y]() {
                    //std::cout << "sending row: " << x << ", col: " << y << std::endl;

                    bool write_in_progress = !output_deq.empty();
                    output_deq.emplace_back(chunk_bytes);

                    if (!write_in_progress) {
                        do_write();
                    }

                    results.emplace_back(result(x, y));
                });
    }

    bool check_done(){
        return !std::any_of(results.cbegin(), results.cend(),
                            [](const result &res) { return !res.calculated; });
    }

private:
    void do_write() {
        asio::async_write(socket_,
                          asio::buffer(output_deq.front().data(), output_deq.front().size()),
                          [this](std::error_code ec, std::size_t /*length*/) {
                              if (!ec) {
                                  output_deq.pop_front();

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
        asio::async_read(socket_,
                         asio::buffer(data_read_, frame<int>::header_length),
                         [this](std::error_code ec, std::size_t) {
                             if (!ec) {
                                 int data_length = get_int(&(data_read_[1]));

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
                                 char d[length];
                                 std::memcpy(d, &(data_read_[5]), length);

                                 if(data_read_[0] == (char)CommandType::DotProduct)
                                     handle_result_dotproduct(d);
                                 if(data_read_[0] == (char)CommandType::DotProductChunked)
                                     handle_result_chunk(d);

                                 do_read_result_header();
                             }
                             else {
                                 std::cout << "socket closed on do_read_result_body" << ec.message() << std::endl;
                                 socket_.close();
                             }
                         });
    }

    void handle_result_dotproduct(char* data) {
        int row = get_int(&(data[0]));
        int col = get_int(&(data[4]));

        //std::cout << "handle result, row=" <<  row << ", col=" << col;
        output_matrix(row, col) = get_int(&(data[8]));
    }

    void handle_result_chunk(char* data) {

        chunk_response response(data);

        //std::cout << "patching: row=" << response.row << ", col=" << response.col << std::endl;

        output_matrix.patch(*response.result.get(), response.row, response.col);

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
    }



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

private:
    enum {
        max_length = 1000000
    };

    tcp::socket socket_;
    asio::io_service& io_service_;

    char data_read_[max_length];


    std::list<result> results;
};

#endif	/* WORKER_SESSION_H */