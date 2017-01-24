#include <cstdlib>
#include <cstring>
#include <iostream>
#include <asio.hpp>
#include <thread>
#include <future>
#include <chrono>
#include <deque>
#include <array>
#include "data_model.h"
#include <utility>

#ifndef PROCESSOR_H
#define	PROCESSOR_H


class processor : public std::enable_shared_from_this<processor> {

  using tcp = asio::ip::tcp;

public:

  processor(tcp::socket socket) :
  socket_(std::move(socket))
  {
  }

  void start()
  {
    do_read_header();
  }

private:

  void do_read_header() 
  {
    auto self(shared_from_this());
    asio::async_read(socket_, asio::buffer(data_, 5),
        [this, self](std::error_code ec, std::size_t length)
        {
          if (!ec)
          {
            int data_length = get_int(data_ + 1);
            std::cout << "read len: " << data_length << std::endl;

            do_read_body(data_length);
          }
          else
          {
            std::cout << "socket closed on do_read_header " << ec.message();
            socket_.close();
          }
        });
  }

  void do_read_body(std::size_t data_length)
  {
    auto self(shared_from_this());
    asio::async_read(socket_, asio::buffer(data_ + 5, data_length),
        [this, self](std::error_code ec, std::size_t length)
        {
          if (!ec)
          {
            for(int i = 0; i < 5 + length; i++)
              std::cout << (int)data_[i] << " ";
            std::cout << std::endl;

            if(data_[0] == 3) // dot product
            {
              int size = get_int(&(data_[13]));
              std::cout << size << std::endl;
              
              std::vector<char> indexes(8);
              std::memcpy(indexes.data(), &(data_[5]), 8);

              std::vector<char> a(size);
              std::vector<char> b(size);

              std::memcpy(a.data(), &(data_[17]), size);
              std::memcpy(b.data(), &(data_[17 + size]), size);

              int sum = 0;
              for(int k = 0; k < size; k++){
                sum += a[k] * b[k];
              }

              std::cout << sum << std::endl;

              send_result(sum, indexes);
            }
          }
          else
          {
            std::cout << "socket closed on do_read_body" << ec.message();

            socket_.close();
          }
        });
  }

  void send_result(int res, std::vector<char> indexes) 
  {
    std::vector<char> b_res = get_bytes_int(res);
    indexes.insert(indexes.end(), b_res.begin(), b_res.end());
    
    data_model frame(CommandType::DotProduct, indexes);

    std::cout << "created data command:" << std::endl;
    std::vector<char> buff = frame.serialize_frame();
    for(int i = 0; i < buff.size(); i++)
        std::cout << (int)buff[i] <<  " ";
    std::cout << std::endl;

    auto self(shared_from_this());
    asio::async_write(socket_, asio::buffer(buff.data(), buff.size()),
            [this, self](std::error_code ec, std::size_t /*length*/) {
      if (!ec) {
        std::cout << "sent result" << std::endl;
        do_read_header();
      }
    });
  }

// math

  int get_int(const char* buffer)
  {
    return int((unsigned char)(buffer[3]) << 24 |
      (unsigned char)(buffer[2]) << 16 |
      (unsigned char)(buffer[1]) << 8 |
      (unsigned char)(buffer[0]));
  }

  std::vector<char> get_bytes_int(int obj)
  {
    std::vector<char> v(sizeof(int));
    for(unsigned i = 0; i < sizeof(int); ++i) {
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
  char data_[max_length];

  std::deque<data_model> process_queue;

  std::vector<std::future<void>> results;
};

#endif	/* PROCESSOR_H */
