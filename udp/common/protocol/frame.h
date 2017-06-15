//
// Created by folbo on 2017-02-05.
//

#ifndef PROJECT_FRAME_H
#define PROJECT_FRAME_H

#include <vector>
#include <string>
#include "../udp_too_big_exception.h"

enum class CommandType {
    Hello = 1,
    Please = 2,
    DotProduct = 3,
    DotProductChunked = 4
};

struct frame {
    const static int header_length = 5;

    char type;
    unsigned int data_length;

    virtual const std::vector<char> get_data() const
    {
    }
};


#endif //PROJECT_FRAME_H
