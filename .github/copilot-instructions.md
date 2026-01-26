# C JSON Parser - AI Coding Guidelines

## Project Overview

This is a **zero-allocation, high-performance JSON parser** written in C, optimized for speed with support for RFC 8259. The parser avoids dynamic memory allocation during parsing by using static memory pools and reference pointers to the original JSON string.

**Key characteristics**: 2.8 nanoseconds per parse operation, ~10x faster than json-c, competitive with simdjson, C89 compatible yet C17 ready.

## Architecture & Data Structures

### Memory Management Philosophy: "Reference-Based" Design

The parser does **not copy data** for primitives. Instead of storing string values, it stores `reference` structs—pointers to slices of the original JSON input:

```c
typedef struct reference {
  const char *ptr;   // Points into the original JSON string
  size_t len;        // Length of the slice
} reference;
```

- **Strings, numbers, booleans**: Stored as `reference` (zero-copy)
- **Arrays & objects**: Use linked lists of dynamically allocated nodes
- **Memory allocation**: Default is **static pool** (`JSON_ARRAY_NODE_POOL`, `JSON_OBJECT_NODE_POOL`); optional dynamic via `USE_ALLOC` macro

### JSON Value Representation

[json_value](src/json.h#L105-L118) is the core union type representing any JSON element:

```c
typedef struct json_value {
  json_token type;     // J_NULL, J_BOOLEAN, J_NUMBER, J_STRING, J_ARRAY, J_OBJECT
  union {
    reference string;
    reference boolean;
    reference number;
    struct { json_array_node *last, *items; } array;   // Linked list + O(1) tail insertion
    struct { json_object_node *last, *items; } object;
  } u;
} json_value;
```

### Parsing Flow

1. [json_parse()](src/json.h#L148) — Recursive descent parser entry point
2. [parse_value_build()](src/json.c#L300+) — Dispatches to type-specific parsers
3. Type-specific parsers: `parse_number()`, `parse_string()`, `parse_array_value()`, `parse_object_value()`
4. Strings escape validation (controlled by `STRING_VALIDATION` macro, default enabled)
5. Returns tree of `json_value` objects; caller manages memory via `json_free()`

## Build System & Compilation

### Build Tool: Ninja

Uses **Ninja** build system with platform-specific files:
- `build.linux.ninja` — Linux with SSE2, clang, lld
- `build.osx.ninja` — macOS
- `build.windows.ninja` — Windows

### Build Targets

```bash
ninja -f build.linux.ninja main              # Debug build
ninja -f build.linux.ninja perf-c-json-parser # Optimized build (-O3, LTO, march=native)
ninja -t clean                               # Clean
```

### Compilation Flags

**Debug** (`cflags`): `-msse2 -Wall -Wextra -std=c89 -g -DSTRING_VALIDATION`

**Performance** (`cflags_perf`): `-msse2 -Wall -Wextra -std=c17 -O3 -march=native -flto -fomit-frame-pointer -DSTRING_VALIDATION -DNDEBUG`

**Key macros**:
- `STRING_VALIDATION` — Enable escape sequence validation (slower but stricter)
- `USE_ALLOC` — Use dynamic allocation instead of static pool
- `SSE2` — SIMD optimizations (auto-detected)

## Testing & Validation

### Test Execution

```bash
./test.sh                    # Comprehensive test suite
./test-main                  # Quick sanity check
```

### Test Files Location

- [test/test.c](test/test.c) — Main test harness (~6700 lines, uses custom test macros)
- [test/test.h](test/test.h) — TEST() macro definitions
- Test utilities: [utils/utils.c](utils/utils.c) — JSON comparison, file loading, timing

### Performance Benchmarking

```bash
./perf.sh                         # Interactive perf runner
./perf.sh perf-c-json-parser      # 100K/1M run benchmarks
./perf.sh perf-c-json-parser-no-string-validation  # Without escape validation
./gprof.sh                        # Profiling with gprof
```

Benchmark code: [perf/test_c_json_parser.c](perf/test_c_json_parser.c)

## Key Coding Patterns

### Pattern 1: Reference Handling

Never allocate when referencing original string content:

```c
// ✓ DO: Create reference (zero-copy)
json_value v;
v.type = J_STRING;
v.u.string.ptr = json_ptr;
v.u.string.len = length;

// ✗ AVOID: Copying strings (defeats performance goal)
char *copy = malloc(len + 1);
strcpy(copy, ptr);
```

### Pattern 2: Linked List Insertion (O(1) Tail)

Arrays and objects use `.last` pointer for constant-time append:

```c
// Creating array with items: use last pointer, not traversal
json_value *arr = ...;
json_array_node *new_node = new_array_node();
new_node->item = item;
if (arr->u.array.last == NULL) {
  arr->u.array.items = new_node;
} else {
  arr->u.array.last->next = new_node;
}
arr->u.array.last = new_node;
```

### Pattern 3: String Escape Validation

[print_string_escaped()](src/json.c#L200+) validates and re-encodes escapes. Controlled by `STRING_VALIDATION` macro.

### Pattern 4: Error Handling

Use `json_error` enum return codes; no exceptions:

```c
json_error err = json_validate(&s);
if (err != E_NO_ERROR) {
  // Handle specific error code (E_INVALID_JSON, E_STACK_OVERFLOW_OBJECT, etc.)
}
```

### Pattern 5: Memory Lifecycle

1. **Parsing**: `json_value root; json_parse(input, &root);` — Stack allocation
2. **Cleanup**: `json_free(&root);` — Optional; resets pool if using static allocation
3. **Reset between parses**: `json_reset();` — Resets pool counters

## Cross-File Integration

### Core Dependencies

- [src/json.h](src/json.h) — Public API (types, function declarations)
- [src/json.c](src/json.c) — Parser implementation (1353 lines, internal functions)
- [src/headers.h](src/headers.h) — Common includes, platform detection (`_WIN32`, `SSE2`)
- [utils/utils.c](utils/utils.c) — Helpers: JSON comparison, file I/O, timing

### Documentation

- [docs/data-structures.md](docs/data-structures.md) — Memory model deep dive
- [docs/json-parsing-and-representation/](docs/json-parsing-and-representation/) — Parsing algorithms
- [README.md](README.md) — Performance benchmarks, build instructions

## Development Workflow

1. **Modify parser logic** → Edit [src/json.c](src/json.c)
2. **Change public API** → Update [src/json.h](src/json.h) and bump function docs
3. **Add test cases** → Extend [test/test.c](test/test.c) using TEST() macro
4. **Build & test**: `ninja -f build.linux.ninja main && ./test-main`
5. **Performance regression check**: `./perf.sh perf-c-json-parser` (compare vs. baseline)
6. **Profiling**: `./gprof.sh` for hotspot analysis

## Platform Compatibility

- **C Standard**: C89 compatible (primary), C17 with optimizations
- **SIMD**: SSE2 auto-detected; falls back to scalar code if unavailable
- **Platforms**: Linux (Ninja default), macOS, Windows with platform-specific build files
- **Compiler**: Clang (primary), GCC supported

## Common Pitfalls to Avoid

1. **Copying strings unnecessarily** — Use `reference` pointers; defeats zero-allocation goal
2. **Manual linked list traversal** — Always use `.last` pointer for array/object insertion
3. **Forgetting json_reset() after reusing pools** — Can lead to pool exhaustion
4. **Ignoring escape validation** — STRING_VALIDATION should remain enabled unless benchmarking
5. **Modifying original JSON string after parsing** — References point into it; mutations corrupt data
6. **Mixing stack & dynamic allocation modes** — Stick to either `USE_ALLOC` or static pool, not both
