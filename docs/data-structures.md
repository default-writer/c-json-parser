# Data Structures

- Data structures designed for efficiency and memory control.
- Linked lists are used for JSON arrays and objects to preserve order.
- Memory is managed through a static pool to avoid `malloc` overhead.

## Core Definitions

- `json_token`: Enum for JSON value types (`J_NULL`, `J_BOOLEAN`, `J_NUMBER`, `J_STRING`, `J_ARRAY`, `J_OBJECT`).
- `reference`: A pointer to a slice of the original JSON string (`ptr`, `len`). Avoids allocations for primitive types.
- `json_value`: Represents any JSON value. Contains a `type` field and a `union` for the specific value.
- `json_object`: Represents a key-value pair, where the key is a `reference` and the value is a `json_value`.
- `json_array_node`: A node in a linked list for array elements.
- `json_object_node`: A node in a linked list for object members.

## Memory Allocation

- **Static Pool (Default)**: A fixed-size pool of nodes is used for fast, "alloc-free" parsing.
  - Predictable memory usage.
  - Reduced heap fragmentation.
  - Faster parsing.
- **Dynamic Allocation (Optional)**: If `USE_ALLOC` is defined, `calloc` is used for node allocation.

## Design

- **Primitives**: `json_value` uses a `reference` to point to the original string, avoiding data copies.
- **Arrays/Objects**: `json_value` points to a linked list of nodes (`json_array_node` or `json_object_node`).
- **O(1) Insertion**: A `last` pointer in the `json_value` for arrays and objects allows for constant-time element insertion at the end of the list.
