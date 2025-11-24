# Deep Equality Checks

- Verifies if two JSON data structures are semantically identical.
- Essential for testing, data validation, and synchronization.
- Ignores memory layout and object key ordering.

## `json_equal()` Functionality

- Recursively traverses and compares two `json_value` trees.
- **Primitives**: Compares content for `null`, `boolean`, `number`, and `string` types.
- **Arrays**: Checks for equal length and recursively compares each element in order.
- **Objects**: Ensures the same number of key-value pairs and that corresponding values for each key are equal. Key order is not significant.

## Workflow

1. **Type Check**: Returns `false` if the types of the two values are different.
2. **Primitive Comparison**: Uses `strncmp` for booleans, numbers, and strings.
3. **Array Comparison**: Recursively calls `json_equal()` on each element.
4. **Object Comparison**: Searches for matching keys and recursively compares their values.

## Further Reading

- [JSON Comparison](README.md)
- [JSON Data Modification](json-data-modification.md)
