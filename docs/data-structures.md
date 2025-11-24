# Core Data Structures

The parser's internal data structures are designed for efficiency and control over memory allocation. Instead of using dynamic arrays that require frequent reallocation, the current implementation uses linked lists for JSON arrays and objects. This approach allows for items to be added in natural sequence and preserves that order during traversal.

Memory for the list nodes is managed through a fixed-size static pool, which avoids the overhead of `malloc` for each new element. This makes parsing faster and more predictable, especially for large JSON files. Dynamic allocation remains an option for environments where a static pool is not suitable.

## Data Structure Definitions

The core of the parser revolves around a few key data structures that represent the JSON hierarchy.

```c
/**
 * @brief Enumeration of JSON value types.
 */
typedef enum {
  J_NULL = 1,    // null value
  J_BOOLEAN = 2, // boolean value (true or false)
  J_NUMBER = 3,  // number value (integer or floating-point)
  J_STRING = 4,  // string value
  J_ARRAY = 5,   // array value
  J_OBJECT = 6   // object value
} json_token;

/**
 * @brief Represents a reference to a part of the original JSON string.
 * This is used to avoid allocating new memory for strings, numbers, and booleans.
 */
typedef struct {
  const char *ptr; // Pointer to the start of the value in the source JSON string.
  size_t len;      // Length of the value.
} reference;

/* Forward declarations */
typedef struct json_value json_value;
typedef struct json_object json_object;
typedef struct json_array_node json_array_node;
typedef struct json_object_node json_object_node;

/**
 * @brief Represents a JSON value.
 * The type of the value is determined by the `type` field.
 */
typedef struct json_value {
  json_token type; // The type of the JSON value.
  union {
    reference string;  // J_STRING.
    reference boolean; // J_BOOLEAN.
    reference number;  // J_NUMBER.
    struct {
      json_array_node *last;
      json_array_node *items; // Array of JSON values.
    } array;                  // J_ARRAY.
    struct {
      json_object_node *last;
      json_object_node *items; // Array of key-value pairs.
    } object;                  // J_OBJECT.
  } u;
} json_value;

/**
 * @brief Represents a key-value pair in a JSON object.
 */
typedef struct json_object {
  reference key;     // Key of the object.
  json_value value; // Pointer to the JSON value.
} json_object;

/**
 * @brief Represents a node in a linked list of JSON values.
 */
typedef struct json_object_node {
  json_object item;
  json_object_node *next; // Pointer to the JSON value.
} json_object_node;

typedef struct json_array_node {
  json_value item;
  json_array_node *next; // Pointer to the JSON value.
} json_array_node;
```

## Memory Allocation Principles

The parser provides two strategies for memory allocation, which can be selected at compile time.

### Static Pool Allocation (Default)

By default, the parser uses a fixed-size pool of `json_array_node` and `json_object_node` structs. This pool is a static array, and when a new node is needed, it is simply taken from this pool. This is extremely fast as it avoids system calls to `malloc`. When parsing is finished, the nodes are returned to the pool. This is the "alloc-free" version of the library. The size of this pool is defined by `JSON_VALUE_POOL_SIZE`.

This approach offers several advantages:

- Predictable memory usage.
- Reduced heap fragmentation.
- Faster parsing due to the absence of dynamic memory allocation overhead for each node.

### Dynamic Allocation (Optional)

If the `USE_ALLOC` macro is defined during compilation, the parser will use `calloc` to allocate memory for each `json_array_node` and `json_object_node` on the heap. This offers more flexibility if the size of the JSON data is unknown and may exceed the static pool size. However, it comes with the performance cost associated with dynamic memory allocation.

## Internal Logic of Data Structures

The `json_value` is the central structure. Its `type` field determines what kind of data it holds. For primitive types like strings, numbers, and booleans, it uses a `reference` which is a pointer into the original JSON string, avoiding data duplication.

For complex types like arrays and objects, the structure is different:

- A `json_value` of type `J_ARRAY` contains a pointer to the head of a linked list of `json_array_node`s (`items`) and a pointer to the last node (`last`).
- Similarly, a `json_value` of type `J_OBJECT` contains a pointer to the head of a linked list of `json_object_node`s (`items`) and a pointer to the last node (`last`).

The `last` pointer is a key optimization. It allows new elements to be added to the end of an array or object in constant time, O(1). This ensures that the natural order of elements in the JSON text is preserved in the parsed data structure without needing to traverse the list on every insertion.

- `json_array_node`: This structure acts as a container in a linked list, holding a `json_value`.
- `json_object_node`: This structure is similar but holds a `json_object`, which is a key-value pair. The key is a `reference`, and the value is a `json_value`.

This linked-list based design with a tail pointer provides a good balance between memory usage and performance, especially for adding elements. It ensures that the order of elements is naturally maintained, which is a common expectation for JSON parsers.
