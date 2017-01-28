#include <vector>
#include <exception>

#ifndef MATRIX_H
#define MATRIX_H

template <typename T>
class matrix {
public:
    matrix(int x, int y) : rows_{x}, cols_{y} {
        data.resize(x * y);
    }

    //matrix(const matrix<T> &obj) {
        //data = obj.data;
        //transposed = obj.transposed;
        //rows_ = obj.rows_;
        //cols_ = obj.cols_;
    //}

    // transposes the matrix
    void transpose() {
        transposed = !transposed;

        std::vector<T> result(rows_ * cols_);
        for (int a = 0; a < rows_ * cols_; a++) {
            int i = a % rows_;
            int j = a / rows_;
            result[a] = data[cols_ * i + j];
        }
        data = result;

        std::swap(rows_, cols_);
    }

    inline
    T &operator()(unsigned row, unsigned col) {
        if (row >= rows_ || col >= cols_)
            throw std::out_of_range("matrix out of bounds");
        return data[cols_ * row + col];
    }

    inline
    T operator()(unsigned row, unsigned col) const {
        if (row >= rows_ || col >= cols_)
            throw std::out_of_range("const matrix subscript out of bounds");
        return data[cols_ * row + col];
    }

    inline
    std::vector<T> &get_data() { return data; }

    inline
    std::vector<T> get_row(int row) {
        std::vector<T> result(cols_);
        std::memcpy(result.data(), &(data[row * cols_]), cols_ * sizeof(T));
        return result;
    }

    inline
    std::vector<T> get_col(int col) {
        std::vector<T> result(rows_);
        for (int i = 0; i < rows_; i++)
            result[i] = data[cols_ * i + col];

        return result;
    }

    inline
    matrix<T> get_rows(int row, int n) {
        matrix<T> result(n, cols_);
        std::memcpy(result.get_data().data(), &(data[row * cols_]), cols_ * n * sizeof(T));
        return result;
    }

    inline
    matrix<T> get_cols(int col, int n) {
        matrix<T> result(rows_, n);
        for (int i = 0; i < rows_; i++)
            std::memcpy(result.get_data().data() + n*i, &(data[cols_ * i + col]), n * sizeof(T));

        return result;
    }

    inline
    int index_to_row(int a) { return a / rows_; }

    inline
    int index_to_col(int a) { return a % rows_; }

    inline
    int pos_to_index(int i, int j) { return i * cols_ + j; }

    inline
    bool is_transposed() { return transposed; }

    inline
    int rows() { return rows_; }

    inline
    int cols() { return cols_; }

    //prints matrix in console
    void print() {
        for (int a = 0; a < rows_ * cols_; a++) {
            std::cout << (int) data[a] << " ";
            if ((a + 1) % cols_ == 0) std::cout << std::endl;
        }
    }

private:
    bool transposed = false;
    std::vector<T> data;

    int rows_, cols_;
};

#endif