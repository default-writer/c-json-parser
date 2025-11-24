# JSON Serialization and Testing

- **Serialization**: Converts in-memory `json_value` trees back into pretty-printed JSON text.
- **Testing**: A comprehensive test suite validates the correctness of the parser and serializer.

## Pretty-Printing JSON

- **Purpose**: To produce well-formatted, human-readable JSON output.
- **Core Function**: `json_stringify()` converts a `json_value` tree into a JSON string.
- **Features**:
  - Well-indented objects.
  - Compact, single-line arrays.
  - Correctly escapes special characters in strings.

## Automated Test Suite

- **Purpose**: To ensure the correctness and robustness of the library.
- **Location**: The test suite is located in the `test/` directory.
- **Workflow**:
  1. Load `test/test.json`.
  2. Parse the JSON into a `json_value` tree.
  3. Stringify the tree back into a JSON string.
  4. Compare the original and stringified versions for semantic equality.
  5. Report the test result.

## Further Reading

- [Automated Test Suite](automated-test-suite.md)
- [Pretty-Printing JSON](pretty-printing-json.md)
