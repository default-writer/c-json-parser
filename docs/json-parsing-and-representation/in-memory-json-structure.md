# In-Memory JSON Structure

- In-memory representation of parsed JSON data.
- Designed for minimal memory overhead and efficient parsing.

## Data Structures

- **`reference`**: A struct with a pointer and length, used for zero-copy representation of primitives.
- **`json_value`**: A tagged union representing any JSON value.
- **`json_array_node`**: A node in a linked list for array elements.
- **`json_object_node`**: A node in a linked list for object members.

## Memory Optimizations

- **Zero-Copy Primitives**: `reference` structs point to the original JSON string, avoiding data duplication.
- **Static Memory Pool**: Nodes for arrays and objects are allocated from a fixed-size pool to avoid `malloc` overhead. This is the default behavior.

## Key Design Points

- **Arrays and Objects**: Implemented as linked lists to preserve element order.
- **O(1) Appends**: `last` pointers in array and object `json_value`s allow for constant-time insertion at the end of the list.

## Further Reading

- [Recursive JSON Parsing](recursive-json-parsing.md)
- [JSON Parsing and Representation](README.md)
