//
// Created by Jacek Dziubinski on 2017-06-09.
//

#ifndef PROJECT_SESSION_H
#define PROJECT_SESSION_H

#include <asio.hpp>
#include <atomic>
#include <iostream>
#include <deque>
#include "../common/bytes_converter.h"
#include "../common/protocol/frame.h"
#include "../common/protocol/chunk_response.h"
#include "result.h"

using namespace asio::ip;

class later
{
public:
    template <class callable, class... arguments>
    later(int after, bool async, callable&& f, arguments&&... args)
    {
        std::function<typename std::result_of<callable(arguments...)>::type()> task(std::bind(std::forward<callable>(f), std::forward<arguments>(args)...));

        if (async)
        {
            std::thread([after, task]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(after));
                task();
            }).detach();
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(after));
            task();
        }
    }

};


class session : public std::enable_shared_from_this<session> {
public:
    session(asio::io_service & io_service, udp::socket& socket, udp::endpoint remote_endpoint, matrix<int>& output)
            : io_service_{io_service},
              socket_{socket},
              remote_endpoint_{remote_endpoint},
              output_matrix{output}
    {
    }

    ~session(){
        std::cout << "destroyed session" << std::endl;
    }

    void send_data(const chunk_frame data)
    {
        //std::cout <<". ";

        auto b = data.get_data();
        int row = data.row();
        int col = data.col();

        std::shared_ptr<session> self = this->shared_from_this();
        io_service_.post([this, self, b, row, col](){
            //std::cout << ". ." << std::endl;
            bool write_in_progress = !output_queue.empty();
            output_queue.push_back(b);

            results.emplace_back(result(row, col, b));

            if (!write_in_progress) {
                do_write();
            }
        });
    }

    udp::endpoint& Endpoint()
    {
        return remote_endpoint_;
    }

    std::list<result>& Results()
    {
        return results;
    }


    std::atomic<int> received_count;

    bool check_done()
    {
        return !std::any_of(results.cbegin(), results.cend(),
                            [](const result &res) { return !res.calculated; });
    }

private:
    void do_write()
    {
        std::shared_ptr<session> self = this->shared_from_this();
        io_service_.post([this, self]() {
            std::shared_ptr<session> self = this->shared_from_this();
            std::vector<char> &top = output_queue.front();
            socket_.async_send_to(asio::buffer(top.data(), top.size()),
                                  remote_endpoint_,
                                  [this, self](std::error_code ec, std::size_t length) {
                if (!ec) {
                    //std::cout << "sent " << length << " bytes" << std::endl;
                    output_queue.pop_front();

                    if (!output_queue.empty()) {
                        do_write();
                    }
                } else {
                    std::cout << "error: " << ec.message() << std::endl;
                }
            });
        });
    }

private:
    asio::io_service& io_service_;
    udp::socket& socket_;
    udp::endpoint remote_endpoint_;

    std::deque<std::vector<char>> output_queue;

    matrix<int>& output_matrix;

    std::list<result> results;
};

#endif //PROJECT_SESSION_H
