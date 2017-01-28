#include <iostream>
#include <asio.hpp>
#include "server.h"

/*
 * executable entry
 */
int main(int argc, char **argv) {
    try {
        int m = 400, n = 400, l = 400;
        // 0 1 2 3 4 5 6 7 8 9
        // 10 11 12 13 14 15 16 17 18 19
        // 20 21 22 23 24 25 26 27 28 29



        // 0 1 2
        // 3 4 5
        // 6 7 8
        matrix<int> m1(m, l);
        for (int i = 0; i < m*l; i++)
            m1(i / l, i % l) = 1;
        //std::cout << "m1 : \n";
        //m1.print();

        // 0 1 2
        // 3 4 5
        // 6 7 8
        matrix<int> m2(n, l);
        for (int i = 0; i < n*l; i++)
            m2(i / l, i % l) = 1;
        //std::cout << "m2 : \n";
        // 0 3 6
        // 1 4 7
        // 2 5 8
        m2.transpose();
        //m2.print();
        // 0 1 2    0 3 6      5    14  23
        // 3 4 5  x 1 4 7   =  14   50  86
        // 6 7 8    2 5 8      23   86  149

        //int nn = 3;
        //matrix<int> m3(nn, nn);
        //for (int i = 0; i < nn*nn; i++)
        //    m3(i / nn, i % nn) = i;
        //m3.print();

        //m1.patch(m3, 2,2);
        //m1.print();

        //std::cout << "\n\n\n";

        //matrix<int> test = m1.get_cols(0, 2);
        //test.print();

        //test = m1.get_rows(0, 2);
        //test.print();


        matrix<int> m3(m, n);

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
            if (input == "r") {
                std::cout << "begin chunked calculation " << s.workers_count() << std::endl;
                s.begin_mul_chunked(m1, m2, 5, 5);
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
