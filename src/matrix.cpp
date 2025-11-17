#include "matrix.hpp"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <stdexcept>
#include <algorithm>

namespace matmul {

// Constructors
Matrix::Matrix() : rows_(0), cols_(0) {}

Matrix::Matrix(int size) : rows_(size), cols_(size), data_(size * size, 0.0) {}

Matrix::Matrix(int rows, int cols) : rows_(rows), cols_(cols), data_(rows * cols, 0.0) {}

Matrix::Matrix(const Matrix& other)
    : rows_(other.rows_), cols_(other.cols_), data_(other.data_) {}

Matrix::Matrix(Matrix&& other) noexcept
    : rows_(other.rows_), cols_(other.cols_), data_(std::move(other.data_)) {
    other.rows_ = 0;
    other.cols_ = 0;
}

Matrix::~Matrix() = default;

// Assignment operators
Matrix& Matrix::operator=(const Matrix& other) {
    if (this != &other) {
        rows_ = other.rows_;
        cols_ = other.cols_;
        data_ = other.data_;
    }
    return *this;
}

Matrix& Matrix::operator=(Matrix&& other) noexcept {
    if (this != &other) {
        rows_ = other.rows_;
        cols_ = other.cols_;
        data_ = std::move(other.data_);
        other.rows_ = 0;
        other.cols_ = 0;
    }
    return *this;
}

// Element access
double& Matrix::operator()(int row, int col) {
    return data_[index(row, col)];
}

const double& Matrix::operator()(int row, int col) const {
    return data_[index(row, col)];
}

double* Matrix::data() {
    return data_.data();
}

const double* Matrix::data() const {
    return data_.data();
}

// Matrix operations
void Matrix::fill(double value) {
    std::fill(data_.begin(), data_.end(), value);
}

void Matrix::randomize(double min, double max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dis(min, max);

    for (auto& val : data_) {
        val = dis(gen);
    }
}

void Matrix::zero() {
    fill(0.0);
}

void Matrix::identity() {
    if (!is_square()) {
        throw std::runtime_error("Identity matrix must be square");
    }
    zero();
    for (int i = 0; i < rows_; ++i) {
        (*this)(i, i) = 1.0;
    }
}

// Submatrix operations
Matrix Matrix::submatrix(int row_start, int col_start, int row_end, int col_end) const {
    int sub_rows = row_end - row_start;
    int sub_cols = col_end - col_start;
    Matrix result(sub_rows, sub_cols);

    for (int i = 0; i < sub_rows; ++i) {
        for (int j = 0; j < sub_cols; ++j) {
            result(i, j) = (*this)(row_start + i, col_start + j);
        }
    }

    return result;
}

void Matrix::set_submatrix(int row_start, int col_start, const Matrix& sub) {
    for (int i = 0; i < sub.rows_; ++i) {
        for (int j = 0; j < sub.cols_; ++j) {
            (*this)(row_start + i, col_start + j) = sub(i, j);
        }
    }
}

// Addition and subtraction
Matrix Matrix::operator+(const Matrix& other) const {
    if (rows_ != other.rows_ || cols_ != other.cols_) {
        throw std::runtime_error("Matrix dimensions must match for addition");
    }

    Matrix result(rows_, cols_);
    for (size_t i = 0; i < data_.size(); ++i) {
        result.data_[i] = data_[i] + other.data_[i];
    }
    return result;
}

Matrix Matrix::operator-(const Matrix& other) const {
    if (rows_ != other.rows_ || cols_ != other.cols_) {
        throw std::runtime_error("Matrix dimensions must match for subtraction");
    }

    Matrix result(rows_, cols_);
    for (size_t i = 0; i < data_.size(); ++i) {
        result.data_[i] = data_[i] - other.data_[i];
    }
    return result;
}

Matrix& Matrix::operator+=(const Matrix& other) {
    if (rows_ != other.rows_ || cols_ != other.cols_) {
        throw std::runtime_error("Matrix dimensions must match for addition");
    }

    for (size_t i = 0; i < data_.size(); ++i) {
        data_[i] += other.data_[i];
    }
    return *this;
}

Matrix& Matrix::operator-=(const Matrix& other) {
    if (rows_ != other.rows_ || cols_ != other.cols_) {
        throw std::runtime_error("Matrix dimensions must match for subtraction");
    }

    for (size_t i = 0; i < data_.size(); ++i) {
        data_[i] -= other.data_[i];
    }
    return *this;
}

// Utility
void Matrix::resize(int rows, int cols) {
    rows_ = rows;
    cols_ = cols;
    data_.resize(rows * cols);
}

void Matrix::print(int max_display) const {
    std::cout << "Matrix " << rows_ << "x" << cols_ << ":\n";

    int display_rows = std::min(rows_, max_display);
    int display_cols = std::min(cols_, max_display);

    for (int i = 0; i < display_rows; ++i) {
        for (int j = 0; j < display_cols; ++j) {
            std::cout << std::setw(10) << std::setprecision(4) << (*this)(i, j) << " ";
        }
        if (cols_ > max_display) {
            std::cout << "...";
        }
        std::cout << "\n";
    }

    if (rows_ > max_display) {
        std::cout << "...\n";
    }
}

bool Matrix::equals(const Matrix& other, double epsilon) const {
    if (rows_ != other.rows_ || cols_ != other.cols_) {
        return false;
    }

    for (size_t i = 0; i < data_.size(); ++i) {
        if (std::abs(data_[i] - other.data_[i]) > epsilon) {
            return false;
        }
    }

    return true;
}

ComparisonResult Matrix::compare(const Matrix& other, double abs_tol, double rel_tol) const {
    ComparisonResult result;
    result.abs_tolerance = abs_tol;
    result.rel_tolerance = rel_tol;
    result.num_elements = 0;
    result.num_failures = 0;
    result.max_abs_error = 0.0;
    result.mean_abs_error = 0.0;
    result.max_rel_error = 0.0;
    result.mean_rel_error = 0.0;
    result.rms_error = 0.0;
    result.worst_row = -1;
    result.worst_col = -1;
    result.worst_value_this = 0.0;
    result.worst_value_other = 0.0;
    result.all_close = true;

    // Check dimensions match
    if (rows_ != other.rows_ || cols_ != other.cols_) {
        result.all_close = false;
        return result;
    }

    result.num_elements = data_.size();

    double sum_abs_error = 0.0;
    double sum_rel_error = 0.0;
    double sum_squared_error = 0.0;

    for (int i = 0; i < rows_; ++i) {
        for (int j = 0; j < cols_; ++j) {
            double val_this = (*this)(i, j);
            double val_other = other(i, j);

            // Compute absolute error
            double abs_error = std::abs(val_this - val_other);

            // Compute relative error
            double max_val = std::max(std::abs(val_this), std::abs(val_other));
            double rel_error = (max_val > 0.0) ? (abs_error / max_val) : 0.0;

            // Combined absolute + relative tolerance check (NumPy allclose)
            double tolerance = std::max(abs_tol, rel_tol * max_val);
            if (abs_error > tolerance) {
                result.all_close = false;
                result.num_failures++;
            }

            // Accumulate statistics
            sum_abs_error += abs_error;
            sum_rel_error += rel_error;
            sum_squared_error += abs_error * abs_error;

            // Track maximum errors
            if (abs_error > result.max_abs_error) {
                result.max_abs_error = abs_error;
                result.worst_row = i;
                result.worst_col = j;
                result.worst_value_this = val_this;
                result.worst_value_other = val_other;
            }

            if (rel_error > result.max_rel_error) {
                result.max_rel_error = rel_error;
            }
        }
    }

    // Compute mean errors
    if (result.num_elements > 0) {
        result.mean_abs_error = sum_abs_error / result.num_elements;
        result.mean_rel_error = sum_rel_error / result.num_elements;
        result.rms_error = std::sqrt(sum_squared_error / result.num_elements);
        result.failure_rate = (100.0 * result.num_failures) / result.num_elements;
    }

    return result;
}

} // namespace matmul
