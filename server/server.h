#include "ThreadPool.h"
#include <thread>
#include <future>
#include <chrono>
#include <iostream>
#include <asio.hpp>
#include <deque>
#include "worker_session.h"
#include "matrix.h"

#ifndef SERVER_H
#define    SERVER_H

class server {

    using tcp = asio::ip::tcp;
    matrix<int>& output_matrix;
public:

    server(asio::io_service &io_service, short port, matrix<int>& m) :
            acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
            socket_(io_service),
            output_matrix{m} {
        do_accept();
    }

    void close() { socket_.close(); }

    int workers_count() {
        return workers.size();
    }

    void begin_mul(matrix<int>& m1, matrix<int>& m2) {
        int n = workers.size();
        int ad = 0;

        for (int i = 0; i < m1.rows(); i++) {
            for (int j = 0; j < m2.cols(); j++) {
                auto row = m1.get_row(i);
                auto col = m2.get_col(j);

                workers[ad]->command_mul(row, col, i, j, m1.cols());
                ad++;
                ad %= n;
                //std::cout << "node number: " << ad << std::endl;

                //std::this_thread::sleep_for(std::chrono::seconds(10));
            }
        }
    }

private:

    void do_accept() {
        acceptor_.async_accept(socket_,
                               [this](std::error_code ec) {
                                   if (!ec) {
                                       auto w = std::make_shared<worker_session>(std::move(socket_), output_matrix);
                                       workers.emplace_back(w);
                                       w->start();
                                   }

                                   std::cout << "accept. src = \"" <<
                                   socket_.remote_endpoint(ec).address().to_string() <<
                                   ":" <<
                                   socket_.local_endpoint(ec).port() <<
                                   "\"" <<
                                   std::endl;

                                   do_accept();
                               });
    }

    tcp::acceptor acceptor_;
    tcp::socket socket_;

    std::vector<std::shared_ptr<worker_session>> workers;
};

#endif	/* SERVER_H */
