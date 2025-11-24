# Pretty-Printing JSON

- **Purpose**: To convert in-memory `json_value` trees into human-readable, formatted JSON text.
- **Core Function**: `json_stringify()` is the main public function for pretty-printing.

## Features

- **Indentation**: Nested objects are indented with four spaces.
- **Compact Arrays**: Arrays are printed on a single line.
- **Escaped Strings**: Special characters in strings are properly escaped.
- **Line Breaks**: Objects use line breaks for readability.

## Core Functions

- **`json_stringify()`**: Serializes a `json_value` tree into a pretty-printed string.
- **`print_value()`**: Internal recursive function that dispatches to helper functions based on the value type.
- **`print_string_escaped()`**: Escapes and prints strings.
- **`print_array_compact()`**: Prints arrays on a single line.
- **`print_object_buf()`**: Prints objects with indentation.

## Further Reading

- [Automated Test Suite](automated-test-suite.md)
- [JSON Serialization and Testing](README.md)
