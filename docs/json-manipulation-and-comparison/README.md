# JSON Comparison

- The library provides deep, structural equality checks for JSON values.
- The core function for comparison is `json_equal()`.
- **Note**: The library does not support in-memory modification of JSON data.

## `json_equal()`

- Recursively compares two `json_value` trees.
- **Types**: Verifies that the types of both values are identical.
- **Primitives**: Compares the content of strings, numbers, booleans, and nulls.
- **Arrays**: Ensures arrays have the same length and that corresponding elements are equal.
- **Objects**: Verifies that both objects have the same number of key-value pairs and that for each key, the corresponding values are equal. The order of keys does not affect equality.

## Memory Management

- Call `json_free()` to release the memory of `json_value` trees after use.

## Further Reading

- [Deep Equality Checks](deep-equality-checks.md)
- [JSON Data Modification](json-data-modification.md)
