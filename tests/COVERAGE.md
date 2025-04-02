# Helios Code Coverage System

This document explains how to use the code coverage tools in the Helios project to measure and analyze test coverage.

## Overview

The code coverage system helps you understand:
- Which parts of your code are being tested
- Which lines are not covered by tests
- Which specific tests cover particular code sections

This is useful for:
- Identifying untested code paths
- Improving test coverage
- Understanding the impact of tests

## Requirements

The coverage tools require the following:

- **lcov**: A graphical front-end for gcov (part of gcc)
  - macOS: `brew install lcov`
  - Ubuntu/Debian: `sudo apt-get install lcov`
- **gcc/g++**: With coverage support (usually included by default)
- **bc**: Basic calculator for floating-point calculations

## Basic Usage

### Running Tests with Coverage

To run all tests and generate coverage information:

```bash
./tests/run_tests_with_coverage.sh
```

This will:
1. Build Helios with coverage instrumentation
2. Run all tests
3. Generate coverage data
4. Create HTML reports in `tests/coverage/`

You can open `tests/coverage/index.html` in a web browser to view the detailed coverage report.

### Command Line Options

The coverage script supports these options:

- `-v`: Verbose mode - show detailed test output
- `-n`: Skip build - don't rebuild Helios (useful for quick reruns)
- `-t=NUM`: Run only a specific test number

Example:
```bash
# Run just test #1 with coverage
./tests/run_tests_with_coverage.sh -t=1
```

## Advanced Analysis

A separate tool is provided to analyze coverage in more detail:

```bash
./tests/coverage_analyzer.sh [options]
```

### Coverage Analyzer Options

- `-f, --file FILE`: Specify a source file to analyze
- `-l, --line NUMBER`: Specify a line number to check coverage for
- `-u, --uncovered`: List all uncovered lines in the specified file
- `-a, --all-files`: List all files with coverage information
- `-h, --help`: Show help information

### Examples

Find which tests cover a specific line:
```bash
# Find tests covering line 123 in Helios.cpp
./tests/coverage_analyzer.sh -f Helios.cpp -l 123
```

List uncovered lines in a file:
```bash
# List all uncovered lines in Button.cpp
./tests/coverage_analyzer.sh -f Button.cpp -u
```

See coverage for all files:
```bash
# Get an overview of coverage for all files
./tests/coverage_analyzer.sh -a
```

## Interpreting Results

The coverage analysis provides these metrics:

- **Line coverage**: Percentage of code lines executed during tests
- **Function coverage**: Percentage of functions called during tests
- **Branch coverage**: Percentage of branches (if/else) executed during tests

Good test coverage generally means:
- Line coverage over 80%
- All critical paths are tested
- No large blocks of uncovered code in critical components

## Tips for Improving Coverage

1. Use the analyzer to identify uncovered code
2. Focus on critical components first
3. Add tests for uncovered branches and edge cases
4. Use the line-specific analysis to understand which tests exercise certain code paths

## How It Works

The coverage system uses these tools:

1. **gcc/g++ with gcov**: Instruments the code with coverage tracking
2. **lcov**: Collects and formats the coverage data
3. **genhtml**: Generates HTML reports from coverage data

Coverage is gathered by:
1. Compiling with special flags (`-fprofile-arcs -ftest-coverage`)
2. Running tests which generate `.gcda` files (coverage data)
3. Using lcov to process this data into a readable format

## Limitations

- Coverage only identifies which lines were executed, not whether the tests properly verify the behavior
- Some lines may be difficult to reach in tests (error conditions, rare edge cases)
- Code that doesn't compile won't have coverage data (tests may not even run)

## Troubleshooting

Common issues:

1. **No coverage data**: Ensure lcov is installed and the Helios binary was built with coverage flags
2. **Missing file**: Verify the file path is correct (use relative paths from project root)
3. **Line numbers mismatch**: Ensure you're checking against the same code version that was compiled

If you experience problems:
- Check that coverage files (`*.gcno`, `*.gcda`) are being generated
- Make sure tests are running and completing successfully
- Verify lcov is properly installed and in your PATH