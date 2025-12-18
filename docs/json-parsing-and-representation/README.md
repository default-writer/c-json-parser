# JSON Parsing and Representation

- Converts a JSON string into an in-memory tree of `json_value` objects.
- The library offers multiple parsing strategies, including recursive and iterative approaches.

## In-Memory Structure

- **`json_value`**: A tagged union representing any JSON type.
- **Primitives**: Represented by a `reference` struct (pointer and length) to avoid data copies.
- **Arrays/Objects**: Represented as linked lists of nodes.
  - **Array**: A linked list of `json_array_node`s.
  - **Object**: A linked list of `json_object_node`s.

## Parsing Functions

- **`json_parse()`**: The original recursive descent parser. Simple and effective for most use cases.
- **`json_parse_iterative()`**: An iterative parser that avoids deep recursion, making it suitable for highly nested JSON structures that might otherwise cause a stack overflow.
- **`json_validate()`**: A fast, iterative validator that checks for well-formed JSON and returns a detailed `json_error` code without building a full in-memory tree.

## Error Handling

- The `json_parse` and `json_parse_iterative` functions return `false` on a syntax error.
- The `json_validate` function returns a `json_error` enum, providing more specific information about the parsing failure.
- All parsers perform strict validation of string contents, disallowing unescaped control characters as per RFC 8259.

## Further Reading

- [In-Memory JSON Structure](in-memory-json-structure.md)
- [Recursive JSON Parsing](recursive-json-parsing.md)
- [Iterative JSON Parsing](iterative-json-parsing.md)
