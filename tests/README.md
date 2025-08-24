# Helios Engine Testing

## Testing

The Helios Engine project includes a comprehensive test suite to ensure firmware reliability and functionality. The tests are designed to cover various aspects of the system, including pattern generation, color management, and user input handling.

### Test Structure

Tests are located in the `tests/` directory and are organized as individual `.test` files. Each test file contains:

- Input commands
- A brief description of the test
- Any specific arguments needed for the test
- Expected output

### Running Tests

To run the tests:

1. Navigate to the `tests/` directory.
2. Run the `run_tests.sh` script:

```bash
./run_tests.sh
```

This script will execute all available tests and report the results.

### Test Options

The `run_tests.sh` script supports several options:

- `-v`: Verbose mode. Provides more detailed output during test execution.
- `-n`: No-make mode. Skips rebuilding the Helios executable before running tests.
- `-f`: Run tests with Valgrind for memory leak detection.
- `-a`: Audit mode. Runs tests in verbose mode without Valgrind.
- `-t=<number>`: Run a specific test number.

Example usage:

```bash
./run_tests.sh -v -t=5
```

This command runs test number 5 in verbose mode.

### Creating New Tests

To create a new test:

1. Use the `create_test.sh` script:
2. Follow the prompts to input test commands, description, and any necessary arguments.
3. The script will generate a new `.test` file in the `tests/` directory.

### Writing Tests

Writing effective tests is crucial for maintaining the reliability of the Helios Engine firmware. Here's a guide on how to write tests:

#### Test File Structure

Each test file (`.test`) follows this structure:

```bash
Input=<input commands>
Brief=<brief description of the test>
Args=<any additional arguments>
--------------------------------------------------------------------------------
<expected output>
```

#### Input Commands

Use the following commands to simulate user interactions:

- `c`: Short click
- `l`: Long click
- `p`: Press and hold
- `r`: Release
- `t`: Toggle press state
- `w`: Wait for one tick
- `q`: Quit the simulation

You can chain these commands together. For example:

```bash
Input=300wcw300wcp1500wr300wq
```

This input waits 300 ticks, performs a short click, waits 300 ticks, presses for 1500 ticks, releases, waits 300 ticks, and quits.

#### Writing the Test Description

Provide a clear, concise description of what the test is checking. For example:

```bash
Brief=Verify that a long press activates the menu
```

#### Specifying Arguments

If your test requires specific arguments to be passed to the Helios CLI, include them in the `Args` field. For example:

```bash
Args=--pattern 1 --colorset "red,green,blue"
```

#### Expected Output

After the separator line, include the expected output from the Helios CLI. This should match exactly what the CLI would produce given the input commands and arguments.

#### Example Test File

Here's an example of a complete test file:

```bash
Input=300wcw300wcp1500wr300wq
Brief=Verify menu activation and color change
Args=--pattern 1 --colorset "red,green,blue"
--------------------------------------------------------------------------------
FF0000
00FF00
0000FF
MENU
```

### Best Practices for Writing Tests

1. **Cover edge cases**: Write tests for both normal usage and edge cases.
2. **One test, one purpose**: Each test should verify a specific behavior or feature.
3. **Use descriptive names**: Name your test files clearly to indicate what they're testing.
4. **Keep tests independent**: Each test should be able to run independently of others.
5. **Update tests with changes**: When you modify the firmware, update or add tests accordingly.
6. **Comment complex tests**: If a test is particularly complex, add comments to explain its purpose and structure.

By following these guidelines, you can create a robust test suite that helps maintain the quality and reliability of the Helios Engine firmware.

### Recording Tests

The project includes scripts for recording and replaying test scenarios:

- `record_test.sh`: Records a single test case.
- `record_tests.sh`: Records multiple test cases.

#### Recording a Single Test

To record a single test:

```bash
./record_test.sh <test_file> [validate] [total_tests] [num_files] [quiet]
```

This script will prompt you to enter test commands, a description, and any necessary arguments.

#### Recording Multiple Tests

To record multiple tests:

1. Navigate to the `tests/` directory.
2. Run the `record_tests.sh` script:

```bash
./record_tests.sh [options]
```

The `record_tests.sh` script supports the following options:

- `-q`: Quiet mode. Reduces output verbosity.
- `-v`: Validate mode. Performs additional validation checks.
- `-t=<number>`: Specifies a particular test number to record.

Example usage:

```bash
./record_tests.sh -v -t=10
```

This command will record test number 10 with validation enabled.

When running `record_tests.sh`:

1. The script will prompt you to enter test commands for each test case.
2. After entering the commands, you'll be asked to provide a brief description of the test.
3. You can specify any necessary arguments for the test.
4. The script will generate a new `.test` file for each recorded test in the `tests/` directory.

Using `record_tests.sh` is particularly useful when you need to create multiple related tests or when you want to quickly generate a series of test cases for a new feature or bug fix.

### Exporting and Importing Tests

You can export and import tests using CSV format:

- To export tests: `./export_tests.sh`
- To import tests: `./import_tests.sh <csv_file>`

These scripts facilitate sharing test cases and maintaining a consistent test suite across different environments.

### Continuous Integration

The project uses GitHub Actions for continuous integration. The workflow defined in `.github/workflows/build.yml` automatically builds the project and runs the test suite on every push and pull request to the main branch.

Thank you for your interest in the Helios Engine project!
