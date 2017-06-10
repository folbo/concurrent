#ifndef PROJECT_CHUNK_FRAME_H
#define PROJECT_CHUNK_FRAME_H

#include "frame.h"
#include "../matrix.h"

struct chunk_frame : public frame {
    chunk_frame(unsigned int row, unsigned int col, unsigned int h, unsigned int w, unsigned int l, matrix<int>& a, matrix<int>& b) :
            row_{row}, col_{col}, height1{h}, width2{w}, l_{l}, a_{a}, b_{b}
    {
        type = (unsigned char) CommandType::DotProductChunked;
        data_length = 4 + //sizeof(row)
                      4 + //sizeof(col)
                      4 + //sizeof(height1)
                      4 + //sizeof(width2)
                      4 + //sizeof(l)
                      a.rows()*a.cols() * 4 + //a.rows()*a.cols() * sizeof(int)
                      b.rows()*b.cols() * 4;  //b.rows()*b.cols() * sizeof(int);
    }

    std::vector<char> get_data() {
        std::vector<char> d(5 + data_length);

        //append header frame info
        d[0] = type;
        auto len = bytes_converter::get_bytes_int(data_length);
        std::memcpy(&(d[1]), len.data(), sizeof(unsigned int));

        // body 1 + 4 + 5*4 = 25
        auto x_v = bytes_converter::get_bytes_int(row_); //int result row
        std::memcpy(&(d[5]), x_v.data(), sizeof(unsigned int));

        auto y_v = bytes_converter::get_bytes_int(col_); //int result col
        std::memcpy(&(d[9]), y_v.data(), sizeof(unsigned int));

        auto la_v = bytes_converter::get_bytes_int(height1); //int height of 1st matrix
        std::memcpy(&(d[13]), la_v.data(), sizeof(unsigned int));

        auto lb_v = bytes_converter::get_bytes_int(width2); //int width of 2nd matrix
        std::memcpy(&(d[17]), lb_v.data(), sizeof(unsigned int));

        auto n_v = bytes_converter::get_bytes_int(l_); //int common length
        std::memcpy(&(d[21]), n_v.data(), sizeof(unsigned int));
        // append 2 vectors to buffer

        int bytes_of_a = a_.get_data().size()*sizeof(int);
        std::memcpy(&(d[25]), a_.get_data().data(), bytes_of_a);

        matrix<int> transposed(b_);
        transposed.transpose();

        int bytes_of_b = transposed.get_data().size()*sizeof(int);
        std::memcpy(&(d[25 + bytes_of_a]), transposed.get_data().data(), bytes_of_b);

        return d;
    }

private :
    // result row
    unsigned int row_;
    // result col
    unsigned int col_;

    // height of 1st matrix
    unsigned int height1;
    // width of 2nd matrix
    unsigned int width2;
    // common length
    unsigned int l_;

    matrix<int>& a_;
    matrix<int>& b_;
};

#endif //PROJECT_CHUNK_FRAME_H
