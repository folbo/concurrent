#ifndef PROJECT_CHUNK_RESPONSE_H
#define PROJECT_CHUNK_RESPONSE_H

#include <iostream>
#include <algorithm>
#include <stdio.h>
#include <memory>
#include <vector>
#include "frame.h"
#include "../matrix.h"

struct chunk_response : frame {
    matrix<int> mat;
    unsigned int row;
    unsigned int col;
    unsigned int la;
    unsigned int lb;
    unsigned int n;

    chunk_response(unsigned int r,
                   unsigned int c,
                   unsigned int a,
                   unsigned int b,
                   unsigned int nn, std::vector<int> m)
            : mat(1, 1)
    {
        type = (char)CommandType::DotProductChunked;
        data_length = 4 + 4 + 4 + 4 + 4 + (m.size() * 4);

        row = r;
        col = c;
        la = a;
        lb = b;
        n = nn;

        mat = matrix<int>(la, lb);
        std::memcpy(mat.get_data().data(), &(m[0]), la * lb * sizeof(int));
    }

    chunk_response(char* data) : mat(1, 1) {
        row = bytes_converter::get_uint(&(data[0]));
        col = bytes_converter::get_uint(&(data[4]));
        la = bytes_converter::get_uint(&(data[8]));
        lb = bytes_converter::get_uint(&(data[12]));
        n = bytes_converter::get_uint(&(data[16]));

        mat = matrix<int>(la, lb);
        std::memcpy(mat.get_data().data(), &(data[20]), la * lb * sizeof(int));
    }

    const std::vector<char> get_data()
    {
        std::vector<char> d(5 + data_length);

        // append header frame info
        d[0] = type;
        auto len = bytes_converter::get_bytes_int(data_length);
        std::memcpy(&(d[1]), len.data(), sizeof(unsigned int));

        // and here data goes
        auto x_v = bytes_converter::get_bytes_int(row); //int result row
        std::memcpy(&(d[5]), x_v.data(), sizeof(unsigned int));

        auto y_v = bytes_converter::get_bytes_int(col); //int result col
        std::memcpy(&(d[9]), y_v.data(), sizeof(unsigned int));

        auto la_v = bytes_converter::get_bytes_int(la); //int height of 1st matrix
        std::memcpy(&(d[13]), la_v.data(), sizeof(unsigned int));

        auto lb_v = bytes_converter::get_bytes_int(lb); //int width of 2nd matrix
        std::memcpy(&(d[17]), lb_v.data(), sizeof(unsigned int));

        auto n_v = bytes_converter::get_bytes_int(n); //int common length
        std::memcpy(&(d[21]), n_v.data(), sizeof(unsigned int));
        // append 2 vectors to buffer

        int bytes_of_a = mat.get_data().size()*sizeof(int);
        std::memcpy(&(d[25]), mat.get_data().data(), bytes_of_a);

        return d;
    }
};

#endif //PROJECT_CHUNK_RESPONSE_H
