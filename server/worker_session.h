#include <asio.hpp>
#include <iostream>
#include <vector>
#include <deque>

#ifndef WORKER_SESSION_H
#define	WORKER_SESSION_H

enum class CommandType {
  Hello = 1,
  Please = 2,
  DotProduct = 3
};

class worker_session : public std::enable_shared_from_this<worker_session> {

  using tcp = asio::ip::tcp;

public:

  using socket = tcp::socket;

  worker_session(tcp::socket socket) :
  socket_(std::move(socket))
  {
  }

  void start() {
  }

  bool is_working = false;

  std::deque<std::vector<char>> output_deq;

  void do_write_command_mul(std::vector<char>& a, std::vector<char>& b, int x, int y, int l) 
  {

    int data_size = a.size() + b.size() + 3 * sizeof(int);

    std::vector<char> d;

            //append header frame info
    d.push_back((unsigned char)CommandType::DotProduct);

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

    //for(auto&& byte : d){
    //  std::cout << (int)byte << " ";
    //}
    //std::cout << std::endl;


    bool write_in_progress = !output_deq.empty();
    output_deq.push_back(d);
    if (!write_in_progress)
    {
      do_write();
    }


    is_working = true;
  }

private:

  void do_read_header() 
  {

    asio::async_read(socket_, asio::buffer(data_, 5),
        [this](std::error_code ec, std::size_t length)
        {
          if (!ec)
          {
            std::cout << "asdasdf" << length << "fdsadsa" << std::endl;


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
    asio::async_read(socket_, asio::buffer(data_ + 5, data_length),
        [this](std::error_code ec, std::size_t length)
        {
          if (!ec)
          {
            std::cout  << "received result :";
            for(int i = 0; i < 5 + length; i++)
              std::cout << (int)data_[i] << " ";
            std::cout << std::endl;

            do_read_header();
          }
          else
          {
            std::cout << "socket closed on do_read_body" << ec.message() << std::endl;

            socket_.close();
          }
        });
  }

  void do_write() 
  {
    asio::async_write(socket_, 
      asio::buffer(output_deq.front().data(), output_deq.front().size()),
      [this](std::error_code ec, std::size_t /*length*/) {
        if (!ec)
        {
          output_deq.pop_front();

          //immediately wait for response
          do_read_header();
        }
        else
        {
          std::cout << "socket closed on do_write" << ec.message() << std::endl;
          socket_.close();
        }
      });
  }

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

  int max_threads;
};


#endif	/* WORKER_SESSION_H */