# In-Memory JSON Structure

## Purpose

The **In-Memory JSON Structure** subtopic addresses the challenge of representing parsed JSON data within a program’s memory efficiently and safely. While the parent topic [`JSON Parsing and Representation`](80084) focuses on converting JSON text into a structured format, this subtopic zeroes in on the concrete data structures used to store JSON values such as strings, numbers, booleans, arrays, and objects. The goal is to minimize memory allocation overhead while enabling safe, flexible, and performant manipulation of JSON content once parsed.

This subtopic exists to provide a foundational representation that supports all JSON types, preserves references to the original JSON text to avoid unnecessary copying, and manages dynamic collections (arrays and objects) with capacity management. It enables the rest of the system—such as manipulation, comparison, and serialization—to operate on a consistent, memory-optimized model.

## Functionality

### Data Structure Overview

The core data structure is the `json_value` struct, which represents any JSON value. It includes a `json_type` enum indicating the value type and a union holding data specific to that type:

- **Primitive Types (String, Number, Boolean):**  
  Instead of copying the actual string or numeric value, these are stored as `reference` structs. Each `reference` holds a pointer and length referring back to the original JSON input text. This avoids memory duplication and allocation costs, ensuring zero-copy access to primitive data.

- **Complex Types (Array, Object):**  
  Arrays and objects store their children dynamically using pointers:
  - Arrays hold a pointer to a contiguous block of `json_value` items, with fields tracking the count and capacity to allow efficient resizing.
  - Objects hold an array of `json_object` key-value pairs, where each key is a reference into the source JSON string and the value is a pointer to a `json_value`. Capacity and count fields similarly manage resizing.

### Memory Optimization and Safety

- **Minimal Allocation:**  
  By referencing original JSON text for primitive values, allocations are only needed for array and object containers and their children, reducing heap fragmentation and memory overhead.

- **Safe Manipulation:**  
  The design uses explicit count and capacity fields to safely manage dynamic growth of arrays and objects, preventing buffer overruns and enabling controlled reallocation.

- **Pointer-Based Navigation:**  
  Objects store keys as raw pointers into the original string rather than duplicated strings, enabling fast key comparisons and minimal memory usage.

### Interaction with Parsing and Other Subtopics

- The parser implemented in the [`Recursive JSON Parsing`](/80084) subtopic populates these structures as it processes JSON text, assigning references and building nested arrays and objects.

- Manipulation APIs like [json_array_push()](https://nextdocs.ai/github/default-writer/376/80073) or [json_object_set_take_key()](https://nextdocs.ai/github/default-writer/376/80073) (covered in [`JSON Manipulation and Comparison`](80082)) operate on these in-memory structures to modify JSON data safely.

- Serialization ([json_stringify()](https://nextdocs.ai/github/default-writer/376/80073)) and printing ([json_print()](https://nextdocs.ai/github/default-writer/376/80073)) routines traverse these structures to output JSON text.

### Illustrative Code Snippet

```c
typedef struct json_value {
  json_type type; /**< The type of the JSON value. */
  union {
    reference string;  /**< Used when type is J_STRING. */
    reference boolean; /**< Used when type is J_BOOLEAN. */
    reference number;  /**< Used when type is J_NUMBER. */
    struct {
      json_value **items; /**< Array of JSON values. */
      size_t count;       /**< Number of items in the array. */
      size_t capacity;    /**< Allocated capacity of the array. */
    } array;              /**< Used when type is J_ARRAY. */
    struct {
      json_object *items; /**< Array of key-value pairs. */
      size_t count;       /**< Number of items in the object. */
      size_t capacity;    /**< Allocated capacity of the object. */
    } object;             /**< Used when type is J_OBJECT. */
  } u;
} json_value;
```

This union structure allows a single `json_value` to embody any JSON type while sharing the same memory footprint for type and data pointers.

## Integration

The **In-Memory JSON Structure** subtopic is the backbone of the broader [`JSON Parsing and Representation`](80084) topic. The parser builds and populates these structures recursively, as detailed in the [`Recursive JSON Parsing`] subtopic, linking parsed tokens to their memory references and constructing arrays and objects with dynamic capacity management.

Other subtopics depend on these structures:

- [`JSON Manipulation and Comparison`](80082) uses these data models for safe mutation and equality checks.
- [`JSON Serialization and Testing`](80083) relies on them to traverse and convert JSON data back to text and validate correctness.

Thus, this subtopic provides a clean, extensible, and memory-conscious representation that interlocks with parsing, manipulation, and serialization components of the system.

```mermaid
```

This class diagram visually emphasizes how `json_value` encapsulates the various JSON types through unions and pointers, highlighting relationships between arrays, objects, and primitive references.
