#ifndef PROJECT_CHUNK_RESPONSE_H
#define PROJECT_CHUNK_RESPONSE_H

#include <iostream>
#include <algorithm>
#include <stdio.h>
#include <memory>
#include "frame.h"
#include "../matrix.h"

struct chunk_response : frame<int> {
    std::unique_ptr<matrix<int>> result;
    unsigned int row;
    unsigned int col;
    unsigned int la;
    unsigned int lb;
    unsigned int n;

    chunk_response(char* data) {
        row = get_uint(&(data[0]));
        col = get_uint(&(data[4]));
        la = get_uint(&(data[8]));
        lb = get_uint(&(data[12]));
        n = get_uint(&(data[16]));

        result = std::unique_ptr<matrix<int>>(new matrix<int>(la, lb));

        std::memcpy(result->get_data().data(), &(data[20]), la * lb * sizeof(int));
    }
};

#endif //PROJECT_CHUNK_RESPONSE_H
