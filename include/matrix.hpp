#ifndef MATRIX_HPP
#define MATRIX_HPP

#include <vector>
#include <memory>
#include <random>

namespace matmul {

// Result structure for matrix comparison with detailed diagnostics
struct ComparisonResult {
    bool all_close;              // Overall pass/fail
    double max_abs_error;        // Maximum absolute difference
    double mean_abs_error;       // Average absolute difference
    double max_rel_error;        // Maximum relative error
    double mean_rel_error;       // Average relative error
    double rms_error;            // Root mean square error
    int num_elements;            // Total elements compared
    int num_failures;            // Elements exceeding tolerance
    double failure_rate;         // Percentage of failures

    // Location and values of worst error (for debugging)
    int worst_row;
    int worst_col;
    double worst_value_this;
    double worst_value_other;

    // Tolerances used for comparison
    double abs_tolerance;
    double rel_tolerance;
};

class Matrix {
public:
    // Constructors
    Matrix();
    Matrix(int size);
    Matrix(int rows, int cols);
    Matrix(const Matrix& other);
    Matrix(Matrix&& other) noexcept;
    ~Matrix();

    // Assignment operators
    Matrix& operator=(const Matrix& other);
    Matrix& operator=(Matrix&& other) noexcept;

    // Element access
    double& operator()(int row, int col);
    const double& operator()(int row, int col) const;
    double* data();
    const double* data() const;

    // Dimension queries
    int rows() const { return rows_; }
    int cols() const { return cols_; }
    int size() const { return rows_; }  // For square matrices
    bool is_square() const { return rows_ == cols_; }

    // Matrix operations
    void fill(double value);
    void randomize(double min = 0.0, double max = 1.0);
    void zero();
    void identity();

    // Submatrix operations (for Strassen)
    Matrix submatrix(int row_start, int col_start, int row_end, int col_end) const;
    void set_submatrix(int row_start, int col_start, const Matrix& sub);

    // Addition and subtraction (for Strassen)
    Matrix operator+(const Matrix& other) const;
    Matrix operator-(const Matrix& other) const;
    Matrix& operator+=(const Matrix& other);
    Matrix& operator-=(const Matrix& other);

    // Utility
    void resize(int rows, int cols);
    void print(int max_display = 10) const;
    bool equals(const Matrix& other, double epsilon = 1e-9) const;

    // Advanced comparison with detailed diagnostics
    // Uses combined absolute + relative error tolerance (NumPy allclose convention)
    ComparisonResult compare(const Matrix& other,
                             double abs_tol = 1e-8,
                             double rel_tol = 1e-5) const;

private:
    int rows_;
    int cols_;
    std::vector<double> data_;

    // Cache-friendly storage (row-major)
    inline int index(int row, int col) const {
        return row * cols_ + col;
    }
};

} // namespace matmul

#endif // MATRIX_HPP
