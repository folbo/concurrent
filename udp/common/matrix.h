#ifndef MATRIX_H
#define MATRIX_H

#include <vector>
#include <exception>



template <typename T>
class matrix {
public:
    matrix(int x, int y) : rows_{x}, cols_{y} {
        data.resize(x * y);
    }

    T& operator()(unsigned row, unsigned col);
    T operator()(unsigned row, unsigned col) const;

    void transpose();
    void patch(matrix<T> &part, int row, int col);

    std::vector<T>& get_data() { return data; }

    std::vector<T> get_row(int row);
    std::vector<T> get_col(int col);
    matrix<T> get_rows(int row, int n);
    matrix<T> get_cols(int col, int n);

    int index_to_row(int a) { return a / cols_; }
    int index_to_col(int a) { return a % cols_; }
    int pos_to_index(int i, int j) { return i * cols_ + j; }

    bool is_transeposed() { return transposed; }

    int rows() { return rows_; }
    int cols() { return cols_; }

    void print();
private:
    bool transposed = false;
    std::vector<T> data;

    int rows_, cols_;
};


template <typename T>
void matrix<T>::transpose() {
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

template <typename T>
T matrix<T>::operator()(unsigned row, unsigned col) const {
    if (row >= rows_ || col >= cols_)
        throw std::out_of_range("const matrix subscript out of bounds");
    return data[cols_ * row + col];
}

template <typename T>
T &matrix<T>::operator()(unsigned row, unsigned col) {
    if (row >= rows_ || col >= cols_)
        throw std::out_of_range("matrix out of bounds");
    return data[cols_ * row + col];
}

template <typename T>
std::vector<T> matrix<T>::get_row(int row) {
    std::vector<T> result(cols_);
    std::memcpy(result.data(), &(data[row * cols_]), cols_ * sizeof(T));
    return result;
}

template <typename T>
std::vector<T> matrix<T>::get_col(int col) {
    std::vector<T> result(rows_);
    for (int i = 0; i < rows_; i++)
        result[i] = data[cols_ * i + col];

    return result;
}

template <typename T>
matrix<T> matrix<T>::get_rows(int row, int n) {
    matrix<T> result(n, cols_);
    std::memcpy(result.get_data().data(),
                &(data[row * cols_]),
                cols_ * n * sizeof(T));
    return result;
}

template <typename T>
matrix<T> matrix<T>::get_cols(int col, int n) {
    matrix<T> result(rows_, n);
    for (int i = 0; i < rows_; i++)
        std::memcpy(result.get_data().data() + n*i,
                    &(data[cols_ * i + col]),
                    n * sizeof(T));

    return result;
}

template <typename T>
void matrix<T>::patch(matrix<T> &part, int row, int col) {
    //std::cout << "patching:" << std::endl;
    //part.print();
    //std::cout << "into:" << std::endl;
    //print();
        for (int i = 0; i < part.rows(); i++) {
            //std::cout << "row of out:  " << i + row;
            //std::cout << "col of out" << col;
            std::memcpy(&(data[(i + row) * cols_ + col]), &(part.get_row(i)[0]), part.cols() * sizeof(T));
        }
    //std::cout << "result: " << std::endl;
    //print();
}

template <typename T>
void matrix<T>::print() {
    for (int a = 0; a < rows_ * cols_; a++) {
        std::cout << (int) data[a] << " ";
        if ((a + 1) % cols_ == 0) std::cout << std::endl;
    }
}


#endif