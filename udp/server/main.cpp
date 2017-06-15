#include <iostream>
#include <asio.hpp>
#include <thread>
#include "server.h"

/*
 * executable entry
 */

short port = 1999;

int chunks_size_a = -1;
int chunks_size_b = -1;
int m = 800, n = 800, l = 800;

int main(int argc, char **argv) {
    std::cout << "runnining server" << std::endl;
    std::cout << "on port: " << port << std::endl;

    std::cout << "A chunk size: " << ((chunks_size_a == -1) ? "auto" : std::to_string(chunks_size_a)) << std::endl;
    std::cout << "B chunk size: " << ((chunks_size_b == -1) ? "auto" : std::to_string(chunks_size_b)) << std::endl << std::endl;

    std::cout << "generating input matrices... " << std::endl;
    std::cout << "A [ "<< m << " x " << l << " ]" << std::endl;
    matrix<int> m1(m, l);
    for (int i = 0; i < m*l; i++)
        m1(i / l, i % l) = i;

    std::cout << "B [ "<< l << " x " << n << " ]" << std::endl;
    matrix<int> m2(l, n);
    for (int i = 0; i < n*l; i++)
        m2(i / n, i % n) = i;

    matrix<int> m3(m, n);
    std::cout << "done." << std::endl << std::endl;
    std::cout << "commands:\n"
              << "i - print number of connected nodes\n"
              << "t - start multiplication - one pair of vectors per chunk (unsafe)\n"
              << "r - start multiplication - specified chunks size\n"
              << "y - start multiplication - single core\n"
              << "p - print result matrix\n"
              << "q - quit\n\n";



    try {
        asio::io_service io_service;

        server s(io_service, port, m3);

        std::thread t([&io_service]() {
            try {
                std::error_code ec;
                io_service.run(ec);

                std::cout << "error: " << ec << std::endl;
            }
            catch(std::exception &e){
                std::cout << "Exception in io_service: " << e.what() << std::endl;
            }
        });

        char line[2];
        std::cout << "command:: " << std::endl;

        while (std::cin.getline(line, 2)) {
            std::string input = std::string(line);

            if (input == "t") {
            }
            else if (input == "r") {

                std::cout << "distributed chunked calculation... "
                          << std::endl;
                auto begin_time = std::chrono::steady_clock::now();

                s.begin_mul_chunked(m1, m2, chunks_size_a, chunks_size_b);

                while(!s.check_done()) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }

                auto end_time = std::chrono::steady_clock::now();
                std::cout << "done in "
                          << std::chrono::duration_cast<std::chrono::microseconds>(end_time - begin_time).count()
                          << " microseconds"
                          << std::endl;
            }
            else if (input == "y") {
            }
            else if (input == "q")
                break;
            else if (input == "i")
                std::cout << "podpietych wezlow: " << /*s.workers_count() <<*/ std::endl;
            else if (input == "p") {
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
