# In-Memory JSON Structure

## Purpose

This document describes how parsed JSON data is represented in memory. It details the data structures for storing JSON values like strings, numbers, booleans, arrays, and objects. The design prioritizes minimal memory overhead and safe, efficient manipulation of parsed JSON.

The in-memory representation supports all JSON types, references the original JSON text to avoid copying, and manages dynamic arrays and objects. This provides a consistent and memory-efficient model for other parts of the system, like manipulation, comparison, and serialization.

## Functionality

### Data Structure Overview

The core data structure is the `json_value` struct, which represents any JSON value. It includes a `json_type` enum indicating the value type and a union holding data specific to that type:

- **Primitive Types (String, Number, Boolean):** Primitives are stored as `reference` structs, which point to the original JSON text. This avoids memory duplication and allows for zero-copy access.

- **Complex Types (Array, Object):** Arrays and objects store their children dynamically. Arrays use a contiguous block of `json_value` items, with count and capacity for resizing. Objects store key-value pairs, where keys reference the source string. Capacity and count fields manage resizing for both.

### Memory Optimization and Safety

- **Minimal Allocation:** Referencing the original JSON for primitives minimizes allocations to only what's needed for array and object containers, reducing memory overhead.

- **Fixed-Size Memory Pool:** To further reduce allocation overhead, `json_value` structures are allocated from a fixed-size memory pool (`JSON_VALUE_POOL_SIZE` of 0x200 items). This pool is managed by two static arrays: one holding the `json_value` objects themselves, and another holding pointers to free objects. This approach avoids frequent calls to `malloc` for small `json_value` allocations, improving performance and reducing memory fragmentation.

- **Safe Manipulation:** Explicit count and capacity fields in arrays and objects ensure safe dynamic growth and prevent buffer overruns.

- **Pointer-Based Navigation:** Object keys are pointers to the original string, allowing for fast comparisons and minimal memory use.

### Interaction with Parsing and Other Subtopics

- The parser, detailed in [`Recursive JSON Parsing`](/80084), populates these structures, building nested arrays and objects.

- Manipulation APIs like [json_array_push()](https://nextdocs.ai/github/default-writer/c-json-parser/80075) or [json_object_set_take_key()](https://nextdocs.ai/github/default-writer/c-json-parser/80075) (covered in [`JSON Manipulation and Comparison`](80082)) operate on these structures to safely modify JSON data.

- Serialization ([json_stringify()](https://nextdocs.ai/github/default-writer/c-json-parser/80075)) and printing ([json_print()](https://nextdocs.ai/github/default-writer/c-json-parser/80075)) routines traverse these structures to generate JSON text.

### Illustrative Code Snippet

```c
typedef struct json_value {
  json_type type; // The type of the JSON value.
  union {
    reference string;  // J_STRING.
    reference boolean; // J_BOOLEAN.
    reference number;  // J_NUMBER.
    struct {
      json_value **items; // Array of JSON values.
      size_t count;       // Number of items in the array.
      size_t capacity;    // Allocated capacity of the array.
    } array;              // J_ARRAY.
    struct {
      json_object *items; // Array of key-value pairs.
      size_t count;       // Number of items in the object.
      size_t capacity;    // Allocated capacity of the object.
    } object;             // J_OBJECT.
  } u;
} json_value;
```

This `json_value` struct can represent any JSON type.

## Integration

The in-memory JSON structure is central to parsing and representation. The parser recursively builds these structures, as detailed in the [`Recursive JSON Parsing`](/80084) subtopic.

Other subtopics depend on these structures:

- [`JSON Manipulation and Comparison`](80082) uses these data models for safe mutation and equality checks.
- [`JSON Serialization and Testing`](80083) relies on them to traverse and convert JSON data back to text and validate correctness.

This provides a clean and memory-efficient representation for parsing, manipulation, and serialization.

```mermaid
```

This class diagram visually emphasizes how `json_value` encapsulates the various JSON types through unions and pointers, highlighting relationships between arrays, objects, and primitive references.
