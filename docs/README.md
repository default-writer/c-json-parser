# C JSON Parser Documentation

- A lightweight and efficient C library for JSON processing.

## Core Features

- **Parsing**: Converts JSON text into an in-memory tree.
- **Comparison**: Performs deep equality checks on JSON values.
- **Serialization**: Converts the in-memory structure back to a JSON string.
- **Testing**: Includes a comprehensive test suite for robustness.

## API

- `json_parse()`: Parses a JSON string into a `json_value` tree.
- `json_equal()`: Compares two JSON values for equality.
- `json_stringify()`: Converts a `json_value` to a JSON string.
- `json_free()`: Frees the memory of a `json_value` tree.

## Data Structures

- `json_token`: Enum for JSON value types (`J_NULL`, `J_BOOLEAN`, `J_NUMBER`, `J_STRING`, `J_ARRAY`, `J_OBJECT`).
- `reference`: A struct that points to a part of the original JSON string, containing a `ptr` and `len`.
- `json_value`: A struct representing a JSON value, with a `type` and a `union` of value types.
- `json_object`: A struct representing a key-value pair in a JSON object.
- `json_object_node`: A linked-list node for JSON object members.
- `json_array_node`: A linked-list node for JSON array elements.

## Architecture

- **Core**: `src/json.c`, `src/json.h`
- **Tests**: `test/main.c`, `test/test.c`, `test/test.h`
- **Build**: Ninja files, `install.sh`, `install.ps1`

## Contribution

- **Tests**: Add new test cases to `test/test.c` or create new test files.
- **Features**: Start with a failing test, then implement the feature.
- **Pull Requests**: Ensure all tests pass before submission.
- **Core Logic**: Modify `src/json.c` and `src/json.h` for core changes.

## Further Reading

- [JSON Manipulation and Comparison](json-manipulation-and-comparison/README.md)
- [JSON Parsing and Representation](json-parsing-and-representation/README.md)
- [JSON Serialization and Testing](json-serialization-and-testing/README.md)
