# JSON Parsing and Representation

## Overview

This module provides the foundational capability to parse JSON-formatted text into a structured, in-memory representation that encompasses all standard JSON data types: null, boolean, number, string, array, and object. By transforming a textual JSON string into a tree of `json_value` objects, it enables safe, efficient access and manipulation of JSON data within the application.

The primary objective is to enable downstream components to work with JSON data programmatically without repeatedly parsing or dealing with raw text. The parsed representation supports querying, modification, and serialization back to JSON text, forming a core layer for the entire JSON processing library.

## Core Concepts

### In-Memory JSON Structure

The in-memory representation centers around the `json_value` data structure (declared in [json.h](../../src/json.h)), which uses a tagged union pattern to represent any JSON type:

- Primitive types (`null`, `boolean`, `number`, `string`) store references (`reference`) pointing directly into the original JSON source string. This approach avoids unnecessary copying of string and numeric data, improving performance and memory efficiency.
- Composite types are represented with dynamically allocated arrays:
  - **Arrays** hold an expandable array of `json_value` elements.
  - **Objects** hold an expandable array of key-value pairs, where keys are strings referenced from the source JSON, and values are pointers to `json_value` instances.

The `json_object` struct represents each key-value pair in an object, maintaining a pointer and length for the key (to the original JSON string) and a pointer to the value.

This design allows efficient traversal and manipulation of JSON trees, supports deep nesting, and preserves the original JSON text for any value.

### Recursive JSON Parsing

Parsing is implemented as a recursive descent parser operating directly on the input string. The key approach is:

- Parsing functions correspond to JSON grammar elements: values, strings, numbers, arrays, and objects.
- Each parsing function advances a pointer through the input string, consuming tokens and whitespace.
- Composite structures (arrays and objects) invoke recursive calls to `parse_value_build` to parse nested elements.
- Strings are parsed with special care to handle escape sequences and Unicode escapes, maintaining a state machine to validate and process escapes correctly.
- Numbers are parsed using the standard `strtod` function, ensuring compliance with JSON number formats.
- Literal values (`null`, `true`, `false`) are matched explicitly with helper functions.

The parser builds an in-memory tree of `json_value` objects, allocating new nodes from a memory pool and linking nested structures appropriately.

If the parser encounters invalid syntax or unexpected characters at any point, it aborts and frees allocated memory, ensuring no partial or corrupt data remains.

### Memory Management and Ownership

- The parser reuses slices of the original JSON string for strings, booleans, and numbers to avoid copying.
- Arrays and objects dynamically resize their internal buffers as elements are added.
- When setting object keys with [json_object_set_take_key](../../src/json.c), the module takes ownership of the key slice and the associated value, managing duplicate keys by replacing existing entries.
- The `json_free` function recursively frees all allocated memory for nested arrays and objects, and returns the `json_value` structures to a memory pool.

## Key Functionalities and Workflow

### Parsing Workflow

1. Entry Point: `json_parse(const char *json)` is called with the JSON text input.
2. Whitespace Skipping: Leading whitespace is skipped to find the first significant token.
3. Value Parsing: `parse_value_build` recursively parses the root JSON value based on the leading character.
4. Type-Specific Parsing:
   - **Strings**: `parse_string_value` handles quoted strings with escape sequences.
   - **Numbers**: `parse_number_value` parses numeric literals using `strtod`.
   - **Arrays**: `parse_array_value` parses comma-separated values within square brackets.
   - **Objects**: `parse_object_value` parses key-value pairs within curly braces.
   - **Literals**: `match_literal_build` matches `null`, `true`, and `false`.
5. Validation: After parsing, trailing whitespace is skipped, and the parser ensures no trailing data exists.
6. Return: The root `json_value` tree is returned, or `NULL` on failure.

### Access and Manipulation

- Objects provide [json_object_get](../../src/json.c) to retrieve values by key efficiently.
- Arrays support element addition via [json_array_push](../../src/json.c).
- Equality checks ([json_equal](recursive-json-parsing.md)) recursively compare JSON trees for structural equivalence.
- The module supports safe freeing and printing of the JSON tree.

### Serialization

While serialization is primarily covered in the `[JSON Serialization and Testing](../json-serialization-and-testing/README.md)` topic, this module provides functions to print JSON values back to text, preserving formatting and escaping.

## Interaction with Other Modules

- The parser outputs a tree of `json_value` objects used by other modules such as `[JSON Manipulation and Comparison](../json-manipulation-and-comparison/README.md)` to modify or compare JSON data.
- The serialized output generated by this module serves as input for `[JSON Serialization and Testing](../json-serialization-and-testing/README.md)` for formatting and validation.
- The testing framework invokes [json_parse](../../src/json.c) to verify parser correctness against various JSON inputs.

## Illustration of the JSON Parsing Process

```mermaid
flowchart TD
A[JSON Text Input] --> B[Skip Whitespace]
B --> C{Next Token?}
C -->|String|" --> D["Parse String (parse_string_value)"]
C -->|Number|digit --> E["Parse Number (parse_number_value)"]
C -->|Array| "["" --> F[Parse Array (parse_array_value)"]
C -->|Object| "{" --> G["Parse Object (parse_object_value)"]
C -->|Literal|null,true,false --> H["Match Literal (match_literal_build)"]
D --> I[Return json_value]
E --> I
F --> I
G --> I
H --> I
I --> J[Skip Trailing Whitespace]
J --> K{End of Input?}
K -->|Yes| L[Return Parsed Tree]
K -->|No| M[Parsing Error: Unexpected Data]
M --> N[Free Allocated Memory]
```

## Overview of In-Memory JSON Data Structures

```mermaid
classDiagram
class json_value {
+json_type type
+union {
reference string
reference boolean
reference number
array {
json_value* items
size_t count
size_t capacity
}
object {
json_object* items
size_t count
size_t capacity
}
}
}
class json_object {
+const char* ptr
+size_t len
+json_value* value
}
json_value "1" o-- "*" json_object : contains
json_value "1" o-- "*" json_value : array items
```

## Important Concepts

- Pointer-Based String Storage: Rather than copying string data, the parser stores pointers and lengths referencing the original JSON input. This reduces memory overhead but requires the input string to remain valid during the lifetime of the parsed tree.
- Dynamic Resizing: Arrays and objects start with a small capacity and double their capacity when needed, avoiding frequent reallocations.
- Recursive Descent: The parsing functions call each other recursively to handle nested structures, simplifying the parsing logic and mapping directly to JSON grammar.
- State Machine for String Parsing: Parsing JSON strings involves a state machine to correctly interpret escape sequences including Unicode escapes.
- Ownership Semantics: The module carefully manages ownership of allocated memory, transferring it where necessary and freeing on errors to prevent leaks.

## Summary of Main Functions (in [src/json.c](https://nextdocs.ai/github/default-writer/c-json-parser/80075))

- [json_parse](../../src/json.c) — Entry point for parsing a JSON string.
- `parse_value_build` — Dispatches to type-specific parse functions.
- `parse_string_value` — Parses JSON strings with escape handling.
- `parse_number_value` — Parses numeric literals.
- `parse_array_value` — Parses arrays recursively.
- `parse_object_value` — Parses objects recursively.
- [json_object_set_take_key](../../src/json.c) — Inserts or updates object key-value pairs.
- [json_object_get](../../src/json.c) — Retrieves values by key.
- [json_equal](recursive-json-parsing.md) — Recursively compares two JSON trees.
- [json_free](../../src/json.c) — Recursively frees JSON tree memory.
- [json_print](../../src/json.c) — Pretty-prints JSON tree to output.

This module forms the essential first step in the full JSON processing pipeline, enabling all subsequent manipulation, comparison, and serialization operations by providing a robust, memory-efficient in-memory JSON representation. For details on how to manipulate and compare these structures, see `[JSON Manipulation and Comparison](../json-manipulation-and-comparison/README.md)`, and for serialization, see `[JSON Serialization and Testing](../json-serialization-and-testing/README.md)`.

## Further Reading

- [JSON Manipulation and Comparison](../json-manipulation-and-comparison/README.md)
- [JSON Serialization and Testing](../json-serialization-and-testing/README.md)
