//
// Created by folbo on 2017-06-10.
//

#ifndef PROJECT_UDP_TOO_BIG_EXCEPTION_H
#define PROJECT_UDP_TOO_BIG_EXCEPTION_H

#include <stdexcept>

class udp_too_big_exception : public std::runtime_error {
public:
    udp_too_big_exception() : std::runtime_error("Single chunk size is too big. Consider splitting data to smaller chunks.")
    {
    }
};


#endif //PROJECT_UDP_TOO_BIG_EXCEPTION_H
