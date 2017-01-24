#include <vector>
#include <exception>


#ifndef MATRIX_H
#define MATRIX_H

class matrix {
public:
  matrix() {
  }

  matrix(int x, int y) : rows{x}, cols{y} {
    data.resize(x*y);
  }

  matrix(const matrix& obj) {
    data = obj.data;
    transposed = obj.transposed;
    rows = obj.rows;
    cols = obj.cols;
  }

  std::vector<char>& get_data() { return data; }

  int rows, cols;

  inline
  std::vector<char> get_row(int row)
  {
    std::vector<char> result(cols);
    std::memcpy(result.data(), &(data[row*cols]), cols);
    return result;
  }

  inline
  std::vector<char> get_col(int col)
  {
    std::vector<char> result(rows);
    for(int i = 0; i < rows; i++)
      result[i] = data[cols*i + col];

    return result;
  }

  void transpose() {
    transposed = !transposed;

    std::vector<char> result(rows*cols);

    for(int a = 0; a < rows*cols; a++) {
      int i = a % rows;
      int j = a / rows;
      result[a] = data[cols*i + j];
    }


    data = result;

    std::swap(rows, cols);
  }

  void print() {
    for(int a = 0; a < rows*cols; a++) {
      std::cout << (int)data[a] << " ";
      if((a + 1) % rows == 0) std::cout << std::endl;
    }
  }

  inline
  bool is_transposed() { return transposed; }


  inline
  char& operator() (unsigned row, unsigned col)
  {
    if (row >= rows || col >= cols)
      throw std::out_of_range("matrix out of bounds");
    return data[cols*row + col];
  }

  inline
  char operator() (unsigned row, unsigned col) const
  {
    if (row >= rows || col >= cols)
      throw std::out_of_range("const matrix subscript out of bounds");
    return data[cols*row + col];
  }

  int index_to_row(int a){
    return a / rows;
  }

  int index_to_col(int a){
    return a % rows;
  }

  int pos_to_index(int i, int j){
    return i * cols + j;
  }

private:
  bool transposed = false;
  std::vector<char> data;
};

#endif