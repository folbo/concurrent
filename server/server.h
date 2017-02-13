#ifndef SERVER_H
#define    SERVER_H

#include <iostream>
#include <chrono>
#include <deque>
#include <asio.hpp>
#include "worker_session.h"
#include "matrix.h"

class server {

    using tcp = asio::ip::tcp;
    matrix<int>& output_matrix;
public:

    server(asio::io_service &io_service, short port, matrix<int>& m) :
            acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
            socket_(io_service),
            output_matrix{m},
            io_service_{io_service} {
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

    void begin_mul_chunked(matrix<int>& m1, matrix<int>& m2, int chunk_size_a, int chunk_size_b) {
        int n = workers.size();
        int ad = 0;

        // auto
        if(chunk_size_a == -1){
            chunk_size_a = m1.rows() / n;
            std::cout << "auto-sizing A chunks: sending chunks of" << chunk_size_a << " rows from A." << std::endl;
        }

        if(chunk_size_b == -1){
            chunk_size_b = m1.cols() / n;
            std::cout << "auto-sizing B chunks: sending chunks of" << chunk_size_b << " cols from B." << std::endl;
        }


        int i = 0;
        for(; i < m2.cols() / chunk_size_b; i++){
            int col = i * chunk_size_b; //col + chunk_size_b - ostatnia kolumna chunka
            auto chunk2 = m2.get_cols(col, chunk_size_b);

            int j = 0;
            for(; j < m1.rows() / chunk_size_a; j++){
                int row = j * chunk_size_a;
                auto chunk1 = m1.get_rows(row, chunk_size_a);

                workers[ad]->command_mul_chunked(chunk1, chunk2, row, col, chunk_size_a, chunk_size_b, m1.cols());

                ad++;
                ad %= n;
                //std::this_thread::sleep_for(std::chrono::seconds(10));
            }

            //last row chunk
            int last_chunk_size_a = m1.rows() % chunk_size_a;
            int row = j * chunk_size_a;
            auto chunk1 = m1.get_rows(row, last_chunk_size_a);

            workers[ad]->command_mul_chunked(chunk1, chunk2, row, col, last_chunk_size_a, chunk_size_b, m1.cols());
        }

        //last column chunk
        int last_chunk_size_b = m2.cols() % chunk_size_b;
        int col = i * chunk_size_b; //col + chunk_size_b - ostatnia kolumna chunka
        auto chunk2 = m2.get_cols(col, last_chunk_size_b);

        int j = 0;
        for( ; j < m1.rows() / chunk_size_a; j++){
            int last_chunk_size_a = m1.rows() % chunk_size_a;
            int row = j * chunk_size_a;
            auto chunk1 = m1.get_rows(row, chunk_size_a);

            workers[ad]->command_mul_chunked(chunk1, chunk2, row, col, chunk_size_a, last_chunk_size_b, m1.cols());

            ad++;
            ad %= n;
        }
        int last_chunk_size_a = m1.rows() % chunk_size_a;
        int row = j * chunk_size_a;
        auto chunk1 = m1.get_rows(row, last_chunk_size_a);

        workers[ad]->command_mul_chunked(chunk1, chunk2, row, col, last_chunk_size_a, last_chunk_size_b, m1.cols());
    }

    bool check_done(){
        if (std::any_of(workers.cbegin(), workers.cend(),
                        [](const std::shared_ptr<worker_session> &worker) { return worker->check_done() == false; }))
            return false;
        return true;
    }


private:

    void do_accept() {
        acceptor_.async_accept(socket_,
                               [this](std::error_code ec) {
                                   if (!ec) {
                                       auto w = std::make_shared<worker_session>(io_service_, std::move(socket_), output_matrix);
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
    asio::io_service& io_service_;

    std::vector<std::shared_ptr<worker_session>> workers;
};

#endif	/* SERVER_H */
