# Iterative JSON Parsing

- An alternative to recursive descent parsing that avoids deep recursion.
- Ideal for very large or deeply nested JSON documents where a recursive approach could lead to a stack overflow.

## Core Functions

- **`json_parse_iterative()`**: An iterative parser that builds a full `json_value` tree. It mirrors the functionality of the recursive `json_parse()` but uses an explicit stack on the heap to manage nesting.
- **`json_validate()`**: A high-performance, iterative validator. It checks for well-formed JSON and returns a `json_error` code without allocating memory for a `json_value` tree, making it the fastest option for pure validation.

## Approach

- The iterative parser uses a state machine and a stack data structure to keep track of the current position and nesting level within the JSON document.
- This avoids using the program's call stack for recursion, making the parsing depth limited only by available memory, not by stack size.

## Use Cases

- **Validation**: Use `json_validate()` when you only need to confirm if a JSON string is valid without accessing its data. It's the most performant option.
- **Deeply Nested Data**: Use `json_parse_iterative()` for parsing trusted, deeply nested JSON that might otherwise cause a stack overflow with the recursive `json_parse()`.

## Further Reading

- [Recursive JSON Parsing](recursive-json-parsing.md)
- [JSON Parsing and Representation](README.md)
