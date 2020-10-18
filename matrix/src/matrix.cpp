#include "matrix.h"
#include <algorithm>
#include <cmath>
#include <iostream>

namespace task {
using namespace std;

Matrix::Matrix() {
  rows_ = 1;
  cols_ = 1;

  data_ = new MatrixRow[1];

  const double one[1] = {1.0};
  data_[0].FillWithData(1, one);
}

Matrix::Matrix(size_t rows, size_t cols) {
  rows_ = rows;
  cols_ = cols;

  data_ = new MatrixRow[rows_];
  for (size_t i = 0; i < rows_; ++i) {
    data_[i].FillWithZeros(cols_);
  }

  for (size_t k = 0; k < min(rows, cols); ++k) {
    data_[k][k] = 1.0;
  }
}

Matrix::Matrix(const Matrix& other) { Assign(other); }

Matrix& Matrix::operator=(const Matrix& other) {
  if (&other == this) {
    return *this;
  }
  Clear();
  Assign(other);
  return *this;
}

Matrix::~Matrix() { Clear(); }

void Matrix::Assign(const Matrix& other) {
  rows_ = other.rows_;
  cols_ = other.cols_;

  data_ = new MatrixRow[rows_];
  for (size_t i = 0; i < rows_; ++i) {
    data_[i].FillWithData(cols_, other.data_[i].data_);
  }
}

void Matrix::Clear() {
  for (size_t i = 0; i < rows_; ++i) {
    data_[i].Clear();
  }
  delete[] data_;
}

double& Matrix::get(size_t row, size_t col) {
  if (row >= rows_ || col >= cols_) {
    throw OutOfBoundsException();
  }
  return data_[row][col];
}

const double& Matrix::get(size_t row, size_t col) const {
  if (row >= rows_ || col >= cols_) {
    throw OutOfBoundsException();
  }
  return data_[row][col];
}

void Matrix::set(size_t row, size_t col, const double& value) {
  if (row >= rows_ || col >= cols_) {
    throw OutOfBoundsException();
  }
  data_[row][col] = value;
}

void Matrix::resize(size_t new_rows, size_t new_cols) {
  if (rows_ == new_rows && cols_ == new_cols) {
    return;
  }

  auto tmp = new MatrixRow[new_rows];
  for (size_t i = 0; i < new_rows; ++i) {
    tmp[i].FillWithZeros(new_cols);
  }

  size_t rows_min = min(rows_, new_rows);
  size_t cols_min = min(rows_, new_cols);
  for (size_t i = 0; i < rows_min; ++i) {
    for (size_t j = 0; j < cols_min; ++j) {
      tmp[i][j] = data_[i][j];
    }
  }

  Clear();
  data_ = tmp;

  rows_ = new_rows;
  cols_ = new_cols;
}

Matrix::MatrixRow& Matrix::operator[](size_t row) {
  if (row >= rows_) {
    throw OutOfBoundsException();
  }
  return data_[row];
}

Matrix::MatrixRow& Matrix::operator[](size_t row) const {
  if (row >= rows_) {
    throw OutOfBoundsException();
  }
  return data_[row];
}

Matrix& Matrix::operator+=(const Matrix& other) {
  if (rows_ != other.rows_ || cols_ != other.cols_) {
    throw SizeMismatchException();
  }
  for (size_t i = 0; i < rows_; ++i) {
    for (size_t j = 0; j < cols_; ++j) {
      data_[i][j] += other.data_[i][j];
      if (fabs(data_[i][j]) < EPS) {
        data_[i][j] = 0;
      }
    }
  }
  return *this;
}

Matrix& Matrix::operator-=(const Matrix& other) {
  if (rows_ != other.rows_ || cols_ != other.cols_) {
    throw SizeMismatchException();
  }
  for (size_t i = 0; i < rows_; ++i) {
    for (size_t j = 0; j < cols_; ++j) {
      data_[i][j] -= other.data_[i][j];
      if (fabs(data_[i][j]) < EPS) {
        data_[i][j] = 0;
      }
    }
  }
  return *this;
}

Matrix& Matrix::operator*=(const Matrix& other) {
  if (cols_ != other.rows_) {
    throw SizeMismatchException();
  }

  auto tmp = new MatrixRow[rows_];
  for (size_t i = 0; i < rows_; ++i) {
    tmp[i].FillWithZeros(other.cols_);
  }

  for (size_t i = 0; i < rows_; ++i) {
    for (size_t j = 0; j < other.cols_; ++j) {
      double& tk = tmp[i][j];
      for (size_t k = 0; k < cols_; ++k) {
        tk += data_[i][k] * other.data_[k][j];
      }
      if (fabs(tmp[i][j]) < EPS) {
        tmp[i][j] = 0;
      }
    }
  }

  Clear();
  data_ = tmp;

  cols_ = other.cols_;

  return *this;
}

Matrix& Matrix::operator*=(const double& number) {
  for (size_t i = 0; i < rows_; ++i) {
    for (size_t j = 0; j < cols_; ++j) {
      data_[i][j] *= number;
      if (fabs(data_[i][j]) < EPS) {
        data_[i][j] = 0;
      }
    }
  }
  return *this;
}

Matrix Matrix::operator+(const Matrix& other) const {
  if (rows_ != other.rows_ || cols_ != other.cols_) {
    throw SizeMismatchException();
  }
  return Matrix(*this) += other;
}

Matrix Matrix::operator-(const Matrix& other) const {
  if (rows_ != other.rows_ || cols_ != other.cols_) {
    throw SizeMismatchException();
  }
  return Matrix(*this) -= other;
}

Matrix Matrix::operator*(const Matrix& other) const {
  if (cols_ != other.rows_) {
    throw SizeMismatchException();
  }
  return Matrix(*this) *= other;
}

Matrix Matrix::operator*(const double& number) const {
  return Matrix(*this) *= number;
}

Matrix Matrix::operator-() const { return Matrix(*this) *= (-1.0); }

Matrix Matrix::operator+() const { return Matrix(*this); }

double Matrix::det() const {
  if (rows_ != cols_) {
    throw SizeMismatchException();
  }

  auto tmp = new MatrixRow[rows_];
  for (size_t i = 0; i < rows_; ++i) {
    tmp[i].FillWithData(rows_, data_[i].data_);
  }

  double result = 1.0;
  for (size_t i = 0; i < rows_; ++i) {
    size_t k = i;

    for (size_t j = i + 1; j < rows_; ++j) {
      if (fabs(tmp[j][i]) > fabs(tmp[k][i])) {
        k = j;
      }
    }

    if (fabs(tmp[k][i]) < EPS) {
      result = 0;
      break;
    }

    for (size_t j = 0; j < rows_; ++j) {
      swap(tmp[k], tmp[i]);
    }

    if (i != k) {
      result = -result;
    }

    result *= tmp[i][i];

    for (size_t j = i + 1; j < rows_; ++j) {
      tmp[i][j] /= tmp[i][i];
      if (fabs(tmp[i][j]) < EPS) {
        tmp[i][j] = 0.0;
      }
    }

    for (size_t j = 0; j < rows_; ++j) {
      if (j != i && fabs(tmp[j][i]) > EPS) {
        for (size_t k = i + 1; k < rows_; ++k) {
          tmp[j][k] -= tmp[i][k] * tmp[j][i];
          if (fabs(tmp[j][k]) < EPS) {
            tmp[j][k] = 0.0;
          }
        }
      }
    }
  }

  return result;

  for (size_t i = 0; i < rows_; ++i) {
    tmp[i].Clear();
  }
  delete[] tmp;

  return result;
}

void Matrix::transpose() {
  auto tmp = new MatrixRow[cols_];
  for (size_t i = 0; i < cols_; ++i) {
    tmp[i].FillWithZeros(rows_);
  }

  for (size_t i = 0; i < rows_; ++i) {
    for (size_t j = 0; j < cols_; ++j) {
      tmp[j][i] = data_[i][j];
    }
  }

  Clear();
  data_ = tmp;

  swap(rows_, cols_);
}

Matrix Matrix::transposed() const {
  Matrix result(*this);
  result.transpose();
  return result;
}

double Matrix::trace() const {
  if (rows_ != cols_) {
    throw SizeMismatchException();
  }

  double result = 0.0;
  for (size_t k = 0; k < rows_; ++k) {
    result += data_[k][k];
  }
  return result;
}

vector<double> Matrix::getRow(size_t row) {
  vector<double> result(cols_);
  for (size_t i = 0; i < cols_; ++i) {
    result[i] = data_[row][i];
  }
  return result;
}

vector<double> Matrix::getColumn(size_t column) {
  vector<double> result(rows_);
  for (size_t i = 0; i < rows_; ++i) {
    result[i] = data_[i][column];
  }
  return result;
}

bool Matrix::operator==(const Matrix& other) const {
  if (rows_ != other.rows_ || cols_ != other.cols_) {
    return false;
  }
  for (size_t i = 0; i < rows_; ++i) {
    for (size_t j = 0; j < cols_; ++j) {
      if (fabs(data_[i][j] - other.data_[i][j]) >= EPS) {
        return false;
      }
    }
  }
  return true;
}

bool Matrix::operator!=(const Matrix& other) const {
  if (rows_ != other.rows_ || cols_ != other.cols_) {
    return true;
  }
  for (size_t i = 0; i < rows_; ++i) {
    for (size_t j = 0; j < cols_; ++j) {
      if (fabs(data_[i][j] - other.data_[i][j]) >= EPS) {
        return true;
      }
    }
  }
  return false;
}

Matrix operator*(const double& a, const Matrix& b) { return b * a; }

std::ostream& operator<<(std::ostream& output, const Matrix& matrix) {
  for (size_t i = 0; i < matrix.rows(); ++i) {
    for (size_t j = 0; j < matrix.cols(); ++j) {
      output << matrix.get(i, j) << " ";
    }
    output << "\n";
  }
  return output;
}

std::istream& operator>>(std::istream& input, Matrix& matrix) {
  size_t rows, cols;
  input >> rows >> cols;
  matrix.resize(rows, cols);
  for (size_t i = 0; i < matrix.rows(); ++i) {
    for (size_t j = 0; j < matrix.cols(); ++j) {
      double elem;
      input >> elem;
      matrix.set(i, j, elem);
    }
  }
  return input;
}

}  // namespace task
