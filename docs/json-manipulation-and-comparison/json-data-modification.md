# JSON Data Modification

- **No In-Memory Modification**: The library does not provide an API for modifying JSON data structures in memory.

## Design Rationale

- **Performance**: The zero-copy approach, which uses `reference` pointers into the original string, makes in-place modification complex and inefficient.
- **Simplicity**: Omitting modification functions keeps the API and implementation small and easy to understand.
- **Predictable Memory Management**: In-memory modification would require a more complex memory management system, which would conflict with the library's design goals.

## Recommended Workflow

1. **Parse**: Parse the JSON string into a `json_value` tree.
2. **Extract**: Copy the data into your application's own data structures.
3. **Modify**: Modify the data within your application.
4. **Generate**: If needed, generate a new JSON string from your modified data.

## Further Reading

- [JSON Comparison](README.md)
- [Deep Equality Checks](deep-equality-checks.md)
