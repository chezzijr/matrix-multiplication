#include "csv_io.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <sys/stat.h>

namespace matmul {

bool CsvIO::read_matrix(const std::string& filename, Matrix& matrix) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file '" << filename << "'\n";
        return false;
    }

    std::vector<std::vector<double>> rows;
    std::string line;

    // Read all rows
    while (std::getline(file, line)) {
        if (line.empty()) continue;

        std::vector<double> row;
        std::stringstream ss(line);
        std::string value;

        while (std::getline(ss, value, ',')) {
            try {
                row.push_back(std::stod(trim(value)));
            } catch (const std::exception& e) {
                std::cerr << "Error: Invalid number in CSV: '" << value << "'\n";
                return false;
            }
        }

        if (!row.empty()) {
            rows.push_back(row);
        }
    }

    if (rows.empty()) {
        std::cerr << "Error: CSV file is empty\n";
        return false;
    }

    // Check that all rows have the same number of columns
    size_t cols = rows[0].size();
    for (const auto& row : rows) {
        if (row.size() != cols) {
            std::cerr << "Error: CSV rows have inconsistent column counts\n";
            return false;
        }
    }

    // Create matrix and fill it
    matrix.resize(rows.size(), cols);
    for (size_t i = 0; i < rows.size(); ++i) {
        for (size_t j = 0; j < cols; ++j) {
            matrix(i, j) = rows[i][j];
        }
    }

    return true;
}

bool CsvIO::write_matrix(const std::string& filename, const Matrix& matrix) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not create file '" << filename << "'\n";
        return false;
    }

    for (int i = 0; i < matrix.rows(); ++i) {
        for (int j = 0; j < matrix.cols(); ++j) {
            file << matrix(i, j);
            if (j < matrix.cols() - 1) {
                file << ",";
            }
        }
        file << "\n";
    }

    return true;
}

bool CsvIO::file_exists(const std::string& filename) {
    struct stat buffer;
    return (stat(filename.c_str(), &buffer) == 0);
}

std::string CsvIO::generate_output_filename(const std::string& input_filename) {
    // Find the last dot for extension
    size_t dot_pos = input_filename.find_last_of('.');

    if (dot_pos == std::string::npos) {
        // No extension
        return input_filename + "_output.csv";
    }

    // Insert "_output" before the extension
    std::string base = input_filename.substr(0, dot_pos);
    std::string ext = input_filename.substr(dot_pos);

    // Make sure extension is .csv
    if (ext != ".csv") {
        ext = ".csv";
    }

    return base + "_output" + ext;
}

std::string CsvIO::trim(const std::string& str) {
    const char* whitespace = " \t\n\r\f\v";
    size_t start = str.find_first_not_of(whitespace);
    if (start == std::string::npos) {
        return "";
    }
    size_t end = str.find_last_not_of(whitespace);
    return str.substr(start, end - start + 1);
}

} // namespace matmul
