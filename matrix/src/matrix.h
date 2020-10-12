#pragma once

#include <vector>
#include <iostream>


namespace task {
using namespace std;

const double EPS = 1e-6;

class OutOfBoundsException : public exception {};
class SizeMismatchException : public exception {};

class Matrix {

    class MatrixRow {
    friend class Matrix;
    private:
        size_t size_;
        double* data_;

        void FillWithZeros(size_t size) {
            size_ = size;
            data_ = new double[size_];
            for (size_t i = 0; i < size_; ++i) {
                data_[i] = 0.0;
            }            
        }

        void FillWithData(size_t size, const double* data) {
            if (data_ != nullptr) {
                delete [] data_;
            }
            size_ = size;
            data_ = new double[size_];
            for (size_t i = 0; i < size_; ++i) {
                data_[i] = data[i];
            }
        }

        void Clear() {
            if (data_ != nullptr) {
                delete [] data_;
            }
            data_ = nullptr;
        }

    public:
        MatrixRow() : size_(0), data_(nullptr) { }

        ~MatrixRow() { }

        constexpr double& operator[] (size_t col) {
            return data_[col];
        }

        constexpr const double& operator[] (size_t col) const {
            return data_[col];
        }
    };

    size_t rows_ = 0;
    size_t cols_ = 0;
    MatrixRow* data_ = nullptr;

    void Assign(const Matrix& other);
    void Clear();

public:

    Matrix();
    Matrix(size_t rows, size_t cols);
    Matrix(const Matrix& copy);
    Matrix& operator=(const Matrix& a);
    ~Matrix();

    constexpr size_t rows() const { return rows_; }
    constexpr size_t cols() const { return cols_; }

    double& get(size_t row, size_t col);
    const double& get(size_t row, size_t col) const;
    void set(size_t row, size_t col, const double& value);
    void resize(size_t new_rows, size_t new_cols);

    MatrixRow& operator[](size_t row);
    MatrixRow& operator[](size_t row) const;

    Matrix& operator+=(const Matrix& a);
    Matrix& operator-=(const Matrix& a);
    Matrix& operator*=(const Matrix& a);
    Matrix& operator*=(const double& number);

    Matrix operator+(const Matrix& a) const;
    Matrix operator-(const Matrix& a) const;
    Matrix operator*(const Matrix& a) const;
    Matrix operator*(const double& a) const;

    Matrix operator-() const;
    Matrix operator+() const;


    double det() const;
    void transpose();
    Matrix transposed() const;
    double trace() const;

    std::vector<double> getRow(size_t row);
    std::vector<double> getColumn(size_t column);

    bool operator==(const Matrix& a) const;
    bool operator!=(const Matrix& a) const;
};

Matrix operator*(const double& a, const Matrix& b);

std::ostream& operator<<(std::ostream& output, const Matrix& matrix);
std::istream& operator>>(std::istream& input, Matrix& matrix);

}  // namespace task
