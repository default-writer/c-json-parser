# Recursive JSON Parsing

- The library uses a recursive descent parser to process JSON documents.
- This approach naturally handles the nested structure of JSON.

## Core Functions

- **`json_parse()`**: The entry point for the parsing process.
- **`parse_value_build()`**: The dispatch function that determines the type of the value at the current position.
- **`parse_string_value()`**: Parses strings, including escape sequences.
- **`parse_array_value()`**: Parses arrays, recursively calling `parse_value_build()` for each element.
- **`parse_object_value()`**: Parses objects, recursively calling `parse_value_build()` for each value.

## Error Handling

- If a syntax error is detected, the current parsing function returns `false`.
- The error propagates up the call stack, causing the top-level `json_parse()` to fail.
- This ensures that no partially parsed or corrupted data is returned.

## Further Reading

- [In-Memory JSON Structure](in-memory-json-structure.md)
- [JSON Parsing and Representation](README.md)
