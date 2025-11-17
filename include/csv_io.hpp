#ifndef CSV_IO_HPP
#define CSV_IO_HPP

#include "matrix.hpp"
#include <string>

namespace matmul {

class CsvIO {
public:
    // Read a matrix from CSV file
    // Returns true on success, false on error
    static bool read_matrix(const std::string& filename, Matrix& matrix);

    // Write a matrix to CSV file
    // Returns true on success, false on error
    static bool write_matrix(const std::string& filename, const Matrix& matrix);

    // Check if a file exists and is readable
    static bool file_exists(const std::string& filename);

    // Generate output filename from input filename
    // e.g., "input.csv" -> "input_output.csv"
    static std::string generate_output_filename(const std::string& input_filename);

private:
    // Trim whitespace from string
    static std::string trim(const std::string& str);
};

} // namespace matmul

#endif // CSV_IO_HPP
