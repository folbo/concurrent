#include <cstdlib>
#include <cstring>
#include <iostream>
#include <asio.hpp>
#include <thread>
#include <future>
#include <chrono>
#include "client.h"

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

std::string port = "1999";
std::string addr = "localhost";

int main(int argc, char* argv[]) {
    InputParser input(argc, argv);

    std::cout << "runnining client" << std::endl;

    const std::string &port_s = input.getCmdOption("-p");
    if (!port_s.empty()){
        port = port_s;
    }
    const std::string &addr_s = input.getCmdOption("-a");
    if (!addr_s.empty()){
        addr = addr_s;
    }

    std::cout << "IP address: " << addr << std::endl;
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
