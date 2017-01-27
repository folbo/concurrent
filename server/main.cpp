#include <iostream>
#include <asio.hpp>
#include "server.h"

/*
 * executable entry
 */
int main(int argc, char **argv) {
    try {
        int m = 40, z = 100, l = 50;

        // 0 1 2
        // 3 4 5
        // 6 7 8
        matrix<int> m1(300, 200);
        for (int i = 0; i < 300*200; i++)
            m1(i / 300, i % 200) = i;
        std::cout << "m1 : \n";
        //m1.print();

        // 0 1 2
        // 3 4 5
        // 6 7 8
        matrix<int> m2(300, 200);
        for (int i = 0; i < 300*200; i++)
            m2(i / 300, i % 200) = i;
        std::cout << "m2 : \n";
        //m2.print();
        // 0 3 6
        // 1 4 7
        // 2 5 8
        m2.transpose();

        // 0 1 2    0 3 6      5    14  23
        // 3 4 5  x 1 4 7   =  14   50  86
        // 6 7 8    2 5 8      23   86  149

        matrix<int> m3(300, 300);

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
