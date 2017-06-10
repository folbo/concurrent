//
// Created by folbo on 2017-02-05.
//

#ifndef PROJECT_FRAME_H
#define PROJECT_FRAME_H

#include <vector>
#include <string>

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
};


#endif //PROJECT_FRAME_H
