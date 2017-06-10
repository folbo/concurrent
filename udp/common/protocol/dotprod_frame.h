#ifndef PROJECT_DOTPROD_FRAME_H
#define PROJECT_DOTPROD_FRAME_H

#include "frame.h"

struct dotprod_frame : public frame {
    dotprod_frame(unsigned int row, unsigned int col, unsigned int l, std::vector<int>& a, std::vector<int>& b) :
            row_{row}, col_{col}, l_{l}, a_{a}, b_{b}
    {
        type = (unsigned char) CommandType::DotProduct;
        data_length = 4 + //sizeof(row)
                      4 + //sizeof(col)
                      4 + //sizeof(l)
                      a.size() * 4 + //a.size() * sizeof(int)
                      b.size() * 4;  //b.size()*sizeof(int);
    }

    std::vector<char> get_data() {
        std::vector<char> d(5 + data_length);

        //append header frame info
        d[0] = type;
        auto len = get_bytes_int(data_length);
        d.insert(d.end(), len.begin(), len.end());

        // body
        auto x_v = get_bytes_int(row_); //int row
        d.insert(d.end(), x_v.begin(), x_v.end());

        auto y_v = get_bytes_int(col_); //int col
        d.insert(d.end(), y_v.begin(), y_v.end());

        auto l_v = get_bytes_int(l_); //int W
        d.insert(d.end(), l_v.begin(), l_v.end());

        // append 2 vectors to buffer

        for(int& element : a_) {
            auto bytes = get_bytes_int(element);
            d.insert(d.end(), bytes.begin(), bytes.end());
        }

        for(int& element : b_) {
            auto bytes = get_bytes_int(element);
            d.insert(d.end(), bytes.begin(), bytes.end());
        }

        return d;
    }

private :
    // result row
    unsigned int row_;
    // result col
    unsigned int col_;

    // common length
    unsigned int l_;

    std::vector<int>& a_;
    std::vector<int>& b_;
};


#endif //PROJECT_DOTPROD_FRAME_H
