#include <iostream>
#include <asio.hpp>
#include "server.h"

/*
 * executable entry
 */
int main(int argc, char **argv) {
    try {
        int m = 40, z = 100, l = 50;

        matrix m1(3, 3);
        for (int i = 0; i < 9; i++)
            m1(i / 3, i % 3) = i;

        matrix m2(3, 3);
        for (int i = 0; i < 9; i++)
            m2(i / 3, i % 3) = i;
        m2.transpose();

        matrix m3(3, 3);

        asio::io_service io_service;
        server s(io_service, 1999, m3);

        std::thread t([&io_service]() { io_service.run(); });

        char line[2];
        std::cout << "komenda: " << std::endl;



        while (std::cin.getline(line, 2)) {
            std::string input = std::string(line);
            if (input == "t") {
                std::cout << "begin calculation " << s.workers_count() << std::endl;
                s.begin_mul(m1, m2);
            }
            if (input == "q")
                break;
            if (input == "i")
                std::cout << "podpietych wezlow: " << s.workers_count() << std::endl;
            if (input == "p") {
                std::cout << "output m:" << std::endl;
                m3.print();
            }
        }

        s.close();
        t.join();
    } catch (std::exception &e) {
        std::cout << "Exception: " << e.what() << "\n";
    }

    return 0;
}
