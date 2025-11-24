# Automated Test Suite

- **Purpose**: To ensure the correctness and robustness of the JSON parser and serializer.
- **Core Functionality**: A round-trip test that parses a JSON file, serializes it back to a string, and compares the result with the original.

## Test Workflow

1. **Load**: The test loads the `test/test.json` file.
2. **Parse**: `json_parse()` converts the JSON string to a `json_value` tree.
3. **Serialize**: `json_stringify()` converts the `json_value` tree back to a JSON string.
4. **Compare**: `utils_test_json_equal()` compares the original and serialized strings, ignoring whitespace differences.
5. **Report**: The test result is printed to the console.

## Test Framework

- A lightweight testing framework is defined in `test/test.h`.
- It provides macros like `TEST`, `ASSERT_TRUE`, and `END_TEST` for writing tests.

## Further Reading

- [Pretty-Printing JSON](pretty-printing-json.md)
- [JSON Serialization and Testing](README.md)
