cmake_minimum_required(VERSION 3.12)
project(MatrixMultiplication VERSION 1.0 LANGUAGES CXX)

# Set C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Optimization flags
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra")

# Find required packages
find_package(OpenMP REQUIRED)
find_package(MPI REQUIRED)
find_package(BLAS REQUIRED)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${MPI_CXX_INCLUDE_DIRS})

# Source files
set(SOURCES
    src/main.cpp
    src/matrix.cpp
    src/csv_io.cpp
    src/timer.cpp
    src/cli_menu.cpp
    src/verification.cpp
    src/terminal.cpp
    src/cli_prompts.cpp
)

# Algorithm implementations
set(ALGO_SOURCES
    algo/naive_seq.cpp
    algo/naive_omp.cpp
    algo/naive_mpi.cpp
    algo/naive_hybrid.cpp
    algo/strassen_seq.cpp
    algo/strassen_omp.cpp
    algo/strassen_mpi.cpp
    algo/strassen_hybrid.cpp
    algo/openblas_wrapper.cpp
)

# Create executable
add_executable(matmul ${SOURCES} ${ALGO_SOURCES})

# Link libraries
target_link_libraries(matmul
    OpenMP::OpenMP_CXX
    MPI::MPI_CXX
    ${BLAS_LIBRARIES}
)

# Compiler definitions
target_compile_definitions(matmul PRIVATE
    $<$<CONFIG:Release>:NDEBUG>
)

# Installation
install(TARGETS matmul DESTINATION bin)

# Print configuration
message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "OpenMP Found: ${OpenMP_FOUND}")
message(STATUS "MPI Found: ${MPI_FOUND}")
message(STATUS "BLAS Libraries: ${BLAS_LIBRARIES}")
