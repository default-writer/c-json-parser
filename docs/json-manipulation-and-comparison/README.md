# JSON Manipulation and Comparison

## Overview

The JSON Manipulation and Comparison module provides APIs and utilities to modify JSON data structures in-memory and perform deep equality checks for validation.

## Core Concepts

### Modification of JSON Data Structures

JSON data is represented as a tree of `json_value` objects. Functions enable:

- **Adding elements to arrays**: `json_array_push()` appends new JSON values to arrays
- **Setting key-value pairs**: `json_object_set_take_key()` inserts or replaces pairs in objects

### Deep Equality Checks

`json_equal()` performs recursive comparison:

- Verifies type equality
- Compares primitive types by value
- Recursively compares array elements
- Compares object key-value pairs

### Memory Management

- Allocate new `json_value` objects on the heap
- Recursively free all allocated memory
- Handle ownership transfer to prevent leaks

## API Usage

```c
json_array_push(my_array, new_item);
json_object_set_take_key(my_object, key_ptr, key_len, value);
bool are_equal = json_equal(value1, value2);
json_free(value);
```

For full documentation, visit: https://nextdocs.ai/github/default-writer/c-json-parser/80082
