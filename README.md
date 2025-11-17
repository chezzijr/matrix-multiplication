# Matrix Multiplication - Parallel Computing Assignment

A comprehensive matrix multiplication implementation with multiple algorithms and parallelization strategies.

## Features

### Algorithms
- **Naive Matrix Multiplication**: Standard O(n³) implementation
- **Strassen Algorithm**: Divide-and-conquer O(n^2.807) implementation
- **OpenBLAS**: Reference implementation for comparison

### Parallelization Modes
- **Sequential**: Single-threaded execution
- **OpenMP**: Shared-memory parallelization
- **MPI**: Distributed-memory parallelization
- **Hybrid**: Combined MPI + OpenMP

### Optimizations
- Cache-friendly blocked multiplication
- Configurable block sizes
- Support for large matrices (up to 10000x10000)

### Interactive CLI
- Modern cross-platform CLI with arrow key navigation
- Easy selection of algorithms, modes, and parameters
- Support for CSV input/output
- No external dependencies (POSIX/Windows native)

### Verification & Validation
- Validate results against OpenBLAS reference implementation
- Multi-algorithm verification mode with pairwise comparisons
- Detailed error statistics (max, mean, RMS errors)
- Combined absolute + relative error tolerance (NumPy allclose convention)
- Comprehensive comparison reports with color-coded PASS/FAIL

## Prerequisites

Install the required dependencies:

```bash
# On Arch Linux
sudo pacman -S cmake openmpi openblas

# On Ubuntu/Debian
sudo apt-get install cmake libopenmpi-dev libopenblas-dev

# On Fedora/RHEL
sudo dnf install cmake openmpi-devel openblas-devel

# On Windows (via vcpkg or MSYS2)
# vcpkg install openmpi openblas
# Or use Microsoft MPI + OpenBLAS from official sources
```

## Building

```bash
# Create build directory
mkdir -p build
cd build

# Configure with CMake
cmake ..

# Build
make

# The executable will be at: build/matmul
```

**Note**: Building creates a 107KB executable with all algorithms and parallelization modes.

### Build Types

```bash
# Debug build (with symbols and warnings)
cmake -DCMAKE_BUILD_TYPE=Debug ..
make

# Release build (with optimizations)
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

## Running

The program supports two modes: **Interactive** and **Command-line**.

### Interactive Mode (No Arguments)

```bash
# Launch interactive menu (for development/testing)
./matmul
```

This will launch the interactive CLI menu where you can:
1. Select algorithm (Naive, Strassen, or OpenBLAS)
2. Choose execution mode
3. Set number of threads (for OpenMP/Hybrid)
4. Configure optimizations
5. Set matrix size
6. Specify input/output files (optional)

**Note:** Interactive mode does NOT work with `mpirun`. Use command-line arguments for MPI.

### Command-Line Mode (With Arguments)

For batch jobs, automation, and MPI execution, use command-line arguments:

```bash
# Show help
./matmul --help

# Basic usage
./matmul --algorithm naive --mode seq --size 1000

# Short form
./matmul -a naive -m seq -s 1000
```

**Available options:**
- `-a, --algorithm <type>` : naive, strassen, openblas
- `-m, --mode <type>` : seq, omp, mpi, hybrid
- `-s, --size <N>` : Matrix size NxN
- `-t, --threads <N>` : Number of OpenMP threads
- `-o, --optimize` : Enable cache-friendly blocking
- `-b, --block-size <N>` : Block size for optimization
- `-i, --input <file>` : Input CSV file
- `--validate` : Validate against OpenBLAS
- `--verify` : Verification mode (compare algorithms)
- `-h, --help` : Show help message

### Examples

**Sequential Execution:**
```bash
# Naive algorithm, 1000x1000 matrix
./matmul -a naive -m seq -s 1000

# Strassen with cache optimization
./matmul -a strassen -m seq -s 2000 --optimize
```

**OpenMP Execution:**
```bash
# Naive with 8 threads
./matmul -a naive -m omp -t 8 -s 1000

# Strassen with custom block size
./matmul -a strassen -m omp -t 4 -s 2000 -b 128
```

**MPI Execution (REQUIRED: Must use command-line args):**
```bash
# Run with 4 MPI processes
mpirun -np 4 ./matmul -a naive -m mpi -s 1000

# Strassen with optimization
mpirun -np 8 ./matmul -a strassen -m mpi -s 2000 --optimize

# On specific hosts
mpirun -np 8 --hostfile hosts.txt ./matmul -a naive -m mpi -s 5000
```

**Hybrid Mode (MPI + OpenMP):**
```bash
# 2 MPI processes, each with 4 OpenMP threads
mpirun -np 2 ./matmul -a naive -m hybrid -t 4 -s 1000

# Explicit thread count per process
export OMP_NUM_THREADS=8
mpirun -np 4 ./matmul -a strassen -m hybrid -s 2000
```

**Validation and Verification:**
```bash
# Validate against OpenBLAS
./matmul -a naive -m seq -s 1000 --validate

# Verify multiple algorithms (compare correctness)
./matmul -m seq -s 1000 --verify
```

**Automation and Scripting:**
```bash
# Parameter sweep
for size in 100 500 1000 2000; do
    mpirun -np 4 ./matmul -a naive -m mpi -s $size --optimize
done

# Batch job (no terminal required)
sbatch --nodes=4 --ntasks=16 << EOF
#!/bin/bash
mpirun ./matmul -a strassen -m mpi -s 5000 --optimize
EOF
```

## Verification and Validation

The application includes comprehensive verification features to ensure correctness of different algorithm implementations.

### Validation Mode (Single Algorithm)

Validate a single algorithm's results against OpenBLAS reference:

```bash
./matmul
# Select: Normal Execution
# Select your algorithm (e.g., Naive)
# Select your mode (e.g., OpenMP)
# ... configure parameters ...
# Select: Validate against OpenBLAS → YES
```

The application will:
1. Compute result using your selected algorithm
2. Compute reference result using OpenBLAS
3. Compare results using combined absolute + relative error tolerance
4. Display detailed comparison report with:
   - Overall PASS/FAIL status
   - Maximum and mean absolute errors
   - Maximum and mean relative errors
   - RMS error
   - Location and values of worst error
   - Tolerance levels used

**Example Output:**
```
========================================
      Comparison Report
========================================
Algorithm 1:     Naive
Algorithm 2:     OpenBLAS
----------------------------------------
Status:          PASSED ✓
----------------------------------------
Error Statistics:
  Max Absolute:  2.345678e-10
  Mean Absolute: 1.234567e-12
  RMS Error:     5.678901e-12
  Max Relative:  0.000001%
  Mean Relative: 0.000000%
```

### Verification Mode (Multiple Algorithms)

Compare multiple algorithms against each other:

```bash
./matmul
# Select: Verification Mode
# Select algorithms: [X] Naive [X] Strassen [X] OpenBLAS (use SPACE to toggle)
# Select: Sequential or OpenMP mode
# ... configure parameters ...
```

The application will:
1. Run all selected algorithms with same inputs
2. Compare all algorithm pairs
3. Display execution times for each
4. Show detailed comparison reports for each pair
5. Provide overall PASS/FAIL summary

**Features:**
- Pairwise comparison of all selected algorithms
- Cross-validation to detect implementation errors
- Performance comparison in addition to correctness
- Comprehensive error statistics

**Note:** Verification mode is restricted to Sequential and OpenMP modes (MPI requires all processes to run the same algorithm).

### Comparison Methodology

The comparison uses **combined absolute + relative error tolerance** (NumPy `allclose` convention):

```
|a - b| ≤ max(abs_tol, rel_tol × max(|a|, |b|))
```

**Default Tolerances:**
- Absolute tolerance: `1e-8`
- Relative tolerance: `1e-5` (0.001%)

These tolerances account for:
- Floating-point rounding errors
- Different operation orders in algorithms
- Numerical stability differences
- Accumulation effects in large matrices

### Why Results Differ Between Algorithms

Different algorithms produce slightly different floating-point results due to:

1. **Operation Order**: Floating-point addition is not associative
   - Naive: Sequential accumulation
   - Strassen: 7 recursive multiplications + 18 additions
   - OpenBLAS: Optimized accumulation with compensated summation

2. **Cache Blocking**: Different memory access patterns affect rounding

3. **Parallel Decomposition**: Different thread/process splits change accumulation order

These differences are **expected and normal**. The verification system helps ensure they remain within acceptable numerical bounds.

## CSV File Format

Input CSV files should contain comma-separated floating-point values:

```csv
1.0,2.0,3.0
4.0,5.0,6.0
7.0,8.0,9.0
```

- All rows must have the same number of columns
- Whitespace is automatically trimmed
- Output files are automatically named with `_output` suffix

## Performance Testing

Example workflow for benchmarking:

```bash
# Small matrix (100x100)
./matmul
# Select: Naive → Sequential → No optimization → 100

# Medium matrix (1000x1000) with cache optimization
./matmul
# Select: Naive → OpenMP → Cache-friendly → 1000

# Large matrix (5000x5000) with MPI
mpirun -np 8 ./matmul
# Select: Strassen → MPI → Cache-friendly → 5000

# Compare with OpenBLAS
./matmul
# Select: OpenBLAS → (any mode) → 1000
```

## Project Structure

```
ma/
├── CMakeLists.txt           # Build configuration
├── README.md                # This file
├── include/                 # Header files
│   ├── algorithms.hpp       # Algorithm interfaces
│   ├── ansi_codes.hpp       # ANSI escape sequences
│   ├── cli_menu.hpp         # Interactive CLI menu system
│   ├── cli_prompts.hpp      # Modern CLI prompt components
│   ├── config.hpp           # Configuration structures
│   ├── csv_io.hpp           # CSV file handling
│   ├── matrix.hpp           # Matrix class
│   ├── terminal.hpp         # Cross-platform terminal abstraction
│   └── timer.hpp            # Timing utilities
├── src/                     # Source implementations
│   ├── cli_menu.cpp         # Menu flow and configuration
│   ├── cli_prompts.cpp      # Inline prompts (select, input, etc.)
│   ├── csv_io.cpp
│   ├── main.cpp             # Main application
│   ├── matrix.cpp
│   ├── terminal.cpp         # Platform-specific terminal I/O
│   └── timer.cpp
└── algo/                    # Algorithm implementations
    ├── naive_seq.cpp        # Naive sequential
    ├── naive_omp.cpp        # Naive OpenMP
    ├── naive_mpi.cpp        # Naive MPI
    ├── naive_hybrid.cpp     # Naive Hybrid
    ├── strassen_seq.cpp     # Strassen sequential
    ├── strassen_omp.cpp     # Strassen OpenMP
    ├── strassen_mpi.cpp     # Strassen MPI
    ├── strassen_hybrid.cpp  # Strassen Hybrid
    └── openblas_wrapper.cpp # OpenBLAS reference
```

## Implementation Notes

### Naive Algorithm
- Standard triple-nested loop
- Cache-friendly blocking available
- Parallelized across outer loops

### Strassen Algorithm
- Switches to naive for matrices smaller than threshold (64)
- Automatically pads non-power-of-2 sizes
- Requires square matrices

### MPI Distribution
- Row-wise distribution of matrix A
- Matrix B broadcast to all processes
- Results gathered using MPI_Allgatherv

### OpenMP Parallelization
- Collapse directive for nested loops
- Dynamic scheduling for load balancing
- Configurable thread count

## Troubleshooting

### Build Issues

**Problem**: Cannot find OpenBLAS
```bash
# Specify BLAS library path
cmake -DBLAS_LIBRARIES=/usr/lib/libopenblas.so ..
```

**Problem**: MPI not found
```bash
# Load MPI module (on clusters)
module load openmpi
cmake ..
```

### Runtime Issues

**Problem**: Terminal display issues or arrow keys not working
```bash
# Reset terminal state
reset
# Ensure TERM variable is set
echo $TERM
# Try again
./matmul
```

**Problem**: ANSI colors not displaying on Windows
```bash
# Ensure you're using Windows 10+ with modern terminal
# Use Windows Terminal or PowerShell 7+ for best compatibility
# Enable VT100 processing if needed
```

**Problem**: MPI processes hang
```bash
# Ensure all processes can communicate
# Check firewall/network settings
# Use fewer processes
mpirun -np 2 ./matmul
```

## Performance Tips

1. **Use Release build** for accurate benchmarking
2. **Match thread count** to available cores
3. **Enable cache optimization** for large matrices
4. **Use power-of-2 sizes** for best Strassen performance
5. **Benchmark OpenBLAS** as baseline reference

## License

Educational project for Parallel Computing Assignment.
