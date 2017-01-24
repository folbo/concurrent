#include <cstdlib>
#include <cstring>
#include <iostream>
#include <asio.hpp>
#include <thread>
#include <future>
#include <chrono>
#include "client.h"

int main(int argc, char* argv[]) {
    try {
        asio::io_service io_service;

        client cl(io_service, "localhost", "1999");
        io_service.run();


        //cl.close();
        //.join();
    } catch (std::exception& e) {
        std::cout << "Exception: " << e.what() << "\n";
    }

    return 0;
}
