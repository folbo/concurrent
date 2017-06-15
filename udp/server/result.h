//
// Created by folbo on 2017-06-14.
//

#ifndef PROJECT_RESULT_H
#define PROJECT_RESULT_H

struct result {
    result(int r, int c) :
            row{r},
            col{c},
            calculated{false}
    {
    }

    int row;
    int col;
    bool calculated;
};

#endif //PROJECT_RESULT_H
