//
// Created by folbo on 2017-06-14.
//

#ifndef PROJECT_RESULT_H
#define PROJECT_RESULT_H

#include <functional>
#include <chrono>
#include <future>
#include <cstdio>


struct result {
    result(int r, int c, std::vector<char> v) :
            row{r},
            col{c},
            frame{v}
    {
    }
    std::vector<char> frame;
    int row;
    int col;
    bool calculated;
};

#endif //PROJECT_RESULT_H
