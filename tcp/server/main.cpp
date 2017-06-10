#include <iostream>
#include <asio.hpp>
#include <thread>
#include "server.h"

class InputParser{
public:
    InputParser (int &argc, char **argv){
        for (int i=1; i < argc; ++i)
            this->tokens.push_back(std::string(argv[i]));
    }
    /// @author iain
    const std::string& getCmdOption(const std::string &option) const{
        std::vector<std::string>::const_iterator itr;
        itr =  std::find(this->tokens.begin(), this->tokens.end(), option);
        if (itr != this->tokens.end() && ++itr != this->tokens.end()){
            return *itr;
        }
        return empty_string;
    }
    /// @author iain
    bool cmdOptionExists(const std::string &option) const{
        return std::find(this->tokens.begin(), this->tokens.end(), option)
               != this->tokens.end();
    }
private:
    std::vector <std::string> tokens;
    std::string empty_string;
};

/*
 * executable entry
 */


short port = 1999;
int chunks_size_a = -1;
int chunks_size_b = -1;
int m = 1000, n = 100, l = 400;

int main(int argc, char **argv) {
    InputParser input(argc, argv);

    if(input.cmdOptionExists("-h")) {
        std::cout << "-p [port] - set port server will listen on" << std::endl;
        std::cout << "-ar [size] - set number of rows of matrix A" << std::endl;
        std::cout << "-ac [size] - set number of cols of matrix A" << std::endl;
        std::cout << "-br [size] - set number of rows of matrix B" << std::endl;
        std::cout << "-bc [size] - set number of cols of matrix B" << std::endl;
        std::cout << "-ca [size] - set number of sent rows per chunk from matrix A" << std::endl;
        std::cout << "-cb [size] - set number of sent cols per chunk from matrix B" << std::endl;

        return 0;
    }

    std::cout << "runnining server" << std::endl;

    const std::string &port_s = input.getCmdOption("-p");
    if (!port_s.empty()){
        port = std::stoi(port_s);
    }

    const std::string &chunks_size_a_s = input.getCmdOption("-ca");
    if (!chunks_size_a_s.empty()){
        chunks_size_a = std::stoi(chunks_size_a_s);
    }

    const std::string &chunks_size_b_s = input.getCmdOption("-cb");
    if (!chunks_size_b_s.empty()){
        chunks_size_b = std::stoi(chunks_size_b_s);
    }

    const std::string &a_rows_s = input.getCmdOption("-ar");
    if (!a_rows_s.empty()){
        m = std::stoi(a_rows_s);
    }

    const std::string &a_cols_s = input.getCmdOption("-ac");
    if (!a_cols_s.empty()){
        l = std::stoi(a_cols_s);
    }

    const std::string &b_rows_s = input.getCmdOption("-br");
    if (!b_rows_s.empty()){
        int rows = std::stoi(b_rows_s);
        if(rows != l) {
            std::cout << "cannot perform multiplication on matrices with given dimensions" << std::endl;
            return 0;
        }
    }

    const std::string &b_cols_s = input.getCmdOption("-bc");
    if (!b_cols_s.empty()){
        n = std::stoi(b_cols_s);
    }

    std::cout << "on port: " << port << std::endl;

    std::cout << "A chunk size: " << ((chunks_size_a == -1) ? "auto" : std::to_string(chunks_size_a)) << std::endl;
    std::cout << "B chunk size: " << ((chunks_size_b == -1) ? "auto" : std::to_string(chunks_size_b)) << std::endl << std::endl;

    std::cout << "generating input matrices... " << std::endl;
    std::cout << "A [ "<< m << " x " << l << " ]" << std::endl;
    matrix<int> m1(m, l);
    for (int i = 0; i < m*l; i++)
        m1(i / l, i % l) = 1;

    std::cout << "B [ "<< l << " x " << n << " ]" << std::endl;
    matrix<int> m2(l, n);
    for (int i = 0; i < n*l; i++)
        m2(i / n, i % n) = 1;

    matrix<int> m3(m, n);

    std::cout << "done." << std::endl << std::endl;

    std::cout << "commands:" << std::endl
                << "i - print number of connected nodes" << std::endl
                << "t - start multiplication - one pair of vectors per chunk (unsafe)" << std::endl
                << "r - start multiplication - specified chunks size" << std::endl
                << "y - start multiplication - single core" << std::endl
                << "p - print result matrix" << std::endl
                << "q - quit" << std::endl << std::endl;


    try {
        asio::io_service io_service;
        server s(io_service, port, m3);

        std::thread t([&io_service]() { io_service.run(); });

        char line[2];
        std::cout << "command:: " << std::endl;

        while (std::cin.getline(line, 2)) {
            std::string input = std::string(line);

            if (input == "t") {
                std::cout << "begin calculation " << s.workers_count() << std::endl;
                s.begin_mul(m1, m2);
            }
            else if (input == "r") {
                std::cout << "distributed chunked calculation... " << s.workers_count() << std::endl;
                std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

                s.begin_mul_chunked(m1, m2, chunks_size_a, chunks_size_b);

                while(!s.check_done()) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }

                std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
                std::cout << "done in " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << " microseconds" << std::endl;
            }
            else if (input == "y") {
                std::cout << "single core calculation..." << std::endl;
                std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
                for(int i = 0; i < m; i++) {
                    for (int j = 0; j < n; j++) {
                        int sum = 0;
                        for (int k = 0; k < l; k++) {
                            sum += m1(i, k) * m2(k, j);
                        }
                        m3(i, j) = sum;
                    }
                }

                std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
                std::cout << "done in " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << " microseconds" << std::endl;
            }
            else if (input == "q")
                break;
            else if (input == "i")
                std::cout << "podpietych wezlow: " << s.workers_count() << std::endl;
            else if (input == "p") {
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
