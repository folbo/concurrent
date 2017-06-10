//
// Created by Jacek Dziubinski on 2017-06-09.
//

#ifndef PROJECT_HELLO_FRAME_H
#define PROJECT_HELLO_FRAME_H

#include <wchar.h>
#include <cstring>
#include "frame.h"
#include "../bytes_converter.h"

struct hello_frame : frame {
public:
    hello_frame(){
        type = (char)CommandType::Hello;
        data_length = 0;
    }

    std::vector<char> serialize()
    {
        std::vector<char> result(5 + data_length);
        result[0] = type;
        auto len = bytes_converter::get_bytes_int(data_length);
        std::memcpy(&(result[1]), len.data(), sizeof(unsigned int));

        return result;
    }
};

#endif //PROJECT_HELLO_FRAME_H
