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
#define	SERVER_H

class server 
{

    using tcp = asio::ip::tcp;

public:

    server(asio::io_service& io_service, short port) :
        acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
        socket_(io_service)
    {
        do_accept();
    }

    void close() { socket_.close(); }

    int workers_count() {
        return workers.size();
    }

    void begin_mul()
      {
        int n = workers.size();


        int m = 40, z = 100, l = 50;

        matrix m1(3, 3);
        for(int i = 0; i < 9; i++)
          m1(i/3, i%3) = i;

        matrix m2(3, 3);
        for(int i = 0; i < 9; i++)
          m2(i/3, i%3) = i;
        m2.transpose();

        int ad = 0;
        for(int i = 0; i < 3; i++) {
          for(int j = 0; j < 3; j++) {
            auto row = m1.get_row(i);
            auto col = m2.get_col(j);

            workers[ad]->do_write_command_mul(row, col, i, j, 3);
            ad += 1;
            ad %= n;
          }
        }
    }

private:

    void do_accept() {
        acceptor_.async_accept(socket_,
                [this](std::error_code ec) {
                    if (!ec) {

                        auto w = std::make_shared<worker_session>(std::move(socket_));
                        workers.emplace_back(w);
                        w->start();
                    }

                    std::cout << "accept. src = \"" << 
                    socket_.remote_endpoint(ec).address().to_string() << 
                    ":" << 
                    socket_.local_endpoint(ec).port()<< 
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
