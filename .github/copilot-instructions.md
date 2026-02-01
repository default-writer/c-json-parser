# C JSON Parser - AI Coding Guidelines

## Architecture Overview
This is a high-performance, zero-allocation JSON parser written in C. Key design principles:
- **Memory Pool Allocation**: Uses static pools (`JSON_VALUE_POOL_SIZE`, `JSON_STACK_SIZE`) for O(1) allocation instead of `malloc`
- **Zero-Copy Parsing**: Primitives use `reference` structs pointing to original input string
- **Linked Lists**: Arrays and objects stored as singly-linked lists with `last` pointers for O(1) append
- **Assembly Optimizations**: Lookup tables in assembly for fast character classification (`whitespace_lookup.asm`, `hex_lookup.asm`)

## Build System
- **Ninja-based**: Platform-specific build files (`build.linux.ninja`, `build.osx.ninja`, `build.windows.ninja`)
- **Clang + LLD**: Compiler and linker for performance
- **Dual Modes**: Debug (C89, `-g`) and performance (C17, `-O3 -march=native -flto`)
- **Key Scripts**:
  - `./build-c-json-parser.sh [target]` - Build specific target (default: `perf-c-json-parser`)
  - `./test.sh [target]` - Build and run tests (default: `main`)
  - `./perf.sh [variant]` - Performance testing variants

## Code Patterns
- **Tagged Union**: `json_value` uses `type` field to determine active union member
- **Reference Structs**: `{const char *ptr; size_t len;}` for zero-copy strings/numbers
- **Error Handling**: `json_error` enum with descriptive strings via `json_error_string()`
- **Memory Management**: `json_reset()` for pool reuse, `json_cleanup()` for zero-fill reset
- **Assembly Integration**: Include `.asm` files in build, call as C functions

## Testing Conventions
- **Unity Framework**: Test files in `test/` directory using `TEST()` macro
- **Coverage-Driven**: Extensive test cases for edge cases and error paths
- **Performance Variants**: Test both string-validation and no-validation builds
- **Fuzzing-Ready**: Random JSON generation for comprehensive testing

## Performance Considerations
- **Avoid Malloc**: Use memory pools for all JSON structures
- **Lookup Tables**: Assembly-optimized character classification
- **SSE2**: Vectorized operations where beneficial
- **Iterative Parsing**: `json_parse_iterative()` for deep nesting vs recursive `json_parse()`
- **String Validation**: Optional compile-time flag affecting performance trade-offs

## File Organization
- `src/json.h` - Public API and data structures
- `src/json.c` - Core parsing logic
- `src/*.asm` - Assembly lookup tables
- `test/` - Unit tests with coverage goals
- `perf/` - Performance benchmarks
- `utils/` - Shared utilities
- `docs/` - Architecture documentation

## Development Workflow
1. Edit source in `src/`
2. Run `./test.sh` for validation
3. Run `./perf.sh` for performance checks
4. Use `./coverage.sh` for test coverage analysis
5. Assembly optimizations: Add `.asm` files and update ninja rules

## Key Files to Reference
- `src/json.h` - API contracts and data structures
- `docs/data-structures.md` - Architecture decisions
- `build.linux.ninja` - Build configuration patterns
- `test/test.c` - Testing patterns and edge cases</content>
<parameter name="filePath">/workspaces/c-json-parser/.github/copilot-instructions.md
