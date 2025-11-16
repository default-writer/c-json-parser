import json
import time
import os
import sys

# ===============================================================================
# Test Harness Globals & Configuration
# ===============================================================================
tests_run = 0
tests_passed = 0
LINE_WIDTH = 80
TEST_COUNT = 100000

# Enable colors only if stdout is a TTY
if sys.stdout.isatty():
    GREEN = "\033[0;32m"
    RED = "\033[0;31m"
    RESET = "\033[0m"
else:
    GREEN = ""
    RED = ""
    RESET = ""

# ===============================================================================
# Helper Functions
# ===============================================================================
def format_time(seconds):
    """Formats seconds into hh:mm:ss.ms"""
    seconds_int = int(seconds)
    millis = int((seconds - seconds_int) * 1000)
    mins, secs = divmod(seconds_int, 60)
    hours, mins = divmod(mins, 60)
    return f"{hours:02d}:{mins:02d}:{secs:02d}.{millis:03d}"

# ===============================================================================
# Test Harness Implementation
# ===============================================================================
def initialize_tests():
    print("=" * LINE_WIDTH)
    print("running unit tests")
    print("=" * LINE_WIDTH)

def finalize_tests():
    print("=" * LINE_WIDTH)
    print(f"tests run: {tests_run}")
    print(f"tests passed: {tests_passed}")
    print("=" * LINE_WIDTH)
    if tests_run == tests_passed:
        print(f"all tests {GREEN}PASSED{RESET}")
    else:
        print(f"some tests {RED}FAILED{RESET}")

def run_test(name, func, *args):
    """Runs a single test function and prints its status."""
    global tests_run, tests_passed
    tests_run += 1
    
    # Aligned to match the C test output
    left_string = f"running test: {name: >56}"
    try:
        func(*args)
        print(f"{left_string} ...{GREEN}PASSED{RESET}")
        tests_passed += 1
        return True
    except Exception as e:
        print(f"{left_string} ...{RED}FAILED{RESET}")
        print(f"  ERROR: {e}")
        return False

# ===============================================================================
# Test Case Definitions
# ===============================================================================
def parse_json_file(file_name):
    """Parses a JSON file once to check for correctness."""
    file_path = os.path.join(os.getcwd(), 'test', file_name)
    with open(file_path, 'rb') as f:
        json_bytes = f.read()
    json.loads(json_bytes)

def run_perf_test(file_name, test_count):
    """Runs a performance test by parsing a file multiple times."""
    file_path = os.path.join(os.getcwd(), 'test', file_name)
    with open(file_path, 'rb') as f:
        json_bytes = f.read()
    
    for _ in range(test_count):
        json.loads(json_bytes)

# ===============================================================================
# Main Execution
# ===============================================================================
if __name__ == "__main__":
    initialize_tests()

    # Run standard unit tests
    run_test("test_simple_json_parsing", parse_json_file, 'test-simple.json')
    run_test("test_json_parsing", parse_json_file, 'test.json')

    print("=" * LINE_WIDTH)
    print("running performance tests")
    print("=" * LINE_WIDTH)

    # Run performance test and print its specific output format
    perf_start_time = time.perf_counter()
    try:
        run_perf_test('test.json', TEST_COUNT)
        perf_end_time = time.perf_counter()
        print(f"test_json_parsing execution time: {format_time(perf_end_time - perf_start_time)}")
        
        # This test is just for display to match the desired output format
        run_test("test_json_perf_test", lambda: None)
    except Exception as e:
        print(f"Performance test failed: {e}")
        # Log a failed test for the summary
        run_test("test_json_perf_test", lambda: (_ for _ in ()).throw(Exception("Perf test failed")))


    finalize_tests()