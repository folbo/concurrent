#include <cstdlib>
#include <cstring>
#include <iostream>
#include <chrono>
#include <asio.hpp>
#include "client.h"

std::string port = "1999";
std::string addr = "localhost";

int main(int argc, char* argv[]) {
    std::cout << "runnining client" << std::endl;

    std::cout << "server IP address: " << addr << std::endl;
    std::cout << "on port: " << port << std::endl;

    try {
        asio::io_service io_service;

        client cl(io_service, addr, port);
        io_service.run();
    } catch (std::exception& e) {
        std::cout << "Exception: " << e.what() << "\n";
    }

    return 0;
}
