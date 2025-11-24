# JSON Parsing and Representation

- Converts a JSON string into an in-memory tree of `json_value` objects.
- The core function for parsing is `json_parse()`.

## In-Memory Structure

- **`json_value`**: A tagged union representing any JSON type.
- **Primitives**: Represented by a `reference` struct (pointer and length) to avoid data copies.
- **Arrays/Objects**: Represented as linked lists of nodes.
  - **Array**: A linked list of `json_array_node`s.
  - **Object**: A linked list of `json_object_node`s.

## Parsing

- **Type**: Recursive descent parser.
- **Error Handling**: Stops on syntax errors and returns `false`.
- **Data Access**: The library does not provide a public API for data access. Traverse the `json_value` tree manually.

## Further Reading

- [In-Memory JSON Structure](in-memory-json-structure.md)
- [Recursive JSON Parsing](recursive-json-parsing.md)
