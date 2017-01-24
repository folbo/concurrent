#include <iostream>
#include <asio.hpp>
#include "server.h"
#include <thread>
#include <string>
#include <exception>
/*
 * executable entry
 */
int main(int argc, char** argv) {
    try {
        asio::io_service io_service;
        server s(io_service, 1999);

        std::thread t([&io_service](){ io_service.run(); });

        char line[2];
        std::cout << "komenda: " << std::endl;

        while (std::cin.getline(line, 2))
        {
          std::string input = std::string(line);
          if(input == "t")
            s.begin_mul();
          if(input == "q")
            break;
          if(input == "i")
            std::cout << "podpietych wezlow: " << s.workers_count() << std::endl;
        }

        s.close();
        t.join();
    } catch (std::exception& e) {
        std::cout << "Exception: " << e.what() << "\n";
    }

    return 0;
}
