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

## SIMD Architecture (AVX2 & SSE2)

### AVX2 (Advanced Vector Extensions 2)
- **Register Width**: 256-bit registers (`__m256i`) — processes 32 bytes per load
- **Platform Requirements**: Haswell (2013) or newer, AMD Excavator (2015) or newer
- **String Parsing**: Quote/escape scanning with `_mm256_cmpeq_epi8` (2× unrolled 32-byte chunks = 64 bytes/iteration)
- **String Validation**: Control-character detection (bytes < 0x20) via:
  - `_mm256_set1_epi8(0x80)` to create XOR bit-flip mask
  - `_mm256_xor_si256()` to shift comparison domain (converts unsigned to signed comparison)
  - `_mm256_cmpgt_epi8()` to perform `0x20_shifted > byte_shifted` check
  - Early return if `_mm256_movemask_epi8()` result is non-zero
- **Compilation**: Enabled automatically by `-march=native` on capable systems
- **Performance**: ~0.19 seconds average across 100 benchmark runs

### SSE2 (Streaming SIMD Extensions 2)
- **Register Width**: 128-bit registers (`__m128i`) — processes 16 bytes per load
- **Platform Requirements**: Pentium 4 (2000) or newer, all modern systems
- **String Parsing**: Quote/escape scanning with `_mm_cmpeq_epi8` (4× unrolled 16-byte chunks = 64 bytes/iteration)
- **String Validation**: Control-character detection (bytes < 0x20) via:
  - `_mm_set1_epi8(0x80)` to create XOR bit-flip mask
  - `_mm_xor_si128()` to shift comparison domain
  - `_mm_cmplt_epi8()` to perform `byte_shifted < 0x20_shifted` check (direct comparison direction)
  - Early return if `_mm_movemask_epi8()` result is non-zero
- **Semantics**: Equivalent to AVX2 (both reject bytes < 0x20), different comparison ops due to XOR inversion
- **Fallback When**: AVX2 unavailable; provides performance boost over scalar on all modern CPUs

### Scalar Fallback
- **When Used**: Platforms without SSE2 support (embedded ARM, MIPS, etc.) or if compiled without SIMD flags
- **Implementation**: Byte-by-byte C loop for quote/escape detection and validation
- **Performance**: ~10% slower than SIMD variants but still reasonable for non-time-critical applications

### String Validation Details
- **Compile-Time Flag**: `-DSTRING_VALIDATION` (default: enabled)
- **Purpose**: Reject JSON strings containing control characters (bytes 0x00–0x1F) per JSON specification
- **Test Coverage**: `test_validate_no_error` (test/test.c:6070–6088) validates all 32 control characters are rejected
- **Overhead**: ~8–10% performance cost when enabled vs disabled (benchmark-dependent)
- **Location in Code**: `parse_string()` function (src/json.c:220–530) performs validation at post-quote check and during quote-scan sequence

## Performance Considerations
- **Avoid Malloc**: Use memory pools (static arrays) for all JSON structures — achieves O(1) allocation with zero fragmentation
- **Lookup Tables**: Assembly-optimized character classification in `whitespace_lookup.asm` and `hex_lookup.asm`
- **SIMD Vectorization**: AVX2 (256-bit) for Haswell+, SSE2 (128-bit) fallback for Pentium 4+
- **Iterative Parsing**: `json_parse_iterative()` available for deep nesting to avoid stack overflow vs recursive `json_parse()`
- **String Validation**: Toggle via `-DSTRING_VALIDATION` compile flag (enabled by default, 8–10% overhead)
- **Profile-Guided Optimization (PGO)**: Build system supports PGO workflow for additional performance gains (~15–20% depending on workload)

## File Organization
- `src/json.h` - Public API and data structures
- `src/json.c` - Core parsing logic with SIMD validation
- `src/headers.h` - Internal macros and constants (e.g., `MIN_PRINTABLE_ASCII` = 0x20)
- `src/whitespace_lookup.asm` - Assembly lookup table for whitespace character classification
- `src/hex_lookup.asm` - Assembly lookup table for hexadecimal digit validation in `\uXXXX` escapes
- `src/json_mod.c` - Reference implementation (alternative parser, kept for comparison)
- `test/` - Unit tests with 423 comprehensive test cases
- `perf/` - Performance benchmarks (C JSON Parser vs json-c vs simdjson)
- `utils/` - Shared utilities (error strings, pool initialization)
- `docs/` - Architecture documentation and design decisions
- `coverage/` - Code coverage reports (gcov output)
- `gprof-*` - Performance profiling data from PGO workflow

## Public API Signatures

### Core Parsing Functions
```c
// Parse JSON from string with specified length
// Returns: true on success, false on error; check json_get_error() for error code
bool json_parse(const char *json_string, size_t length, json_value *out_value, 
                json_error *out_error);

// Validate JSON without parsing (checks syntax without building data structures)
// Parameters: json_string (input), length (input length in bytes)
// Returns: JSON_OK on valid syntax, json_error enum on error
json_error json_validate(const char *json_string, size_t length);

// Reset memory pools for reuse (allows parsing multiple JSONs with same pool)
// Clears all previously allocated values but keeps memory allocated
void json_reset(void);

// Zero-fill and reset memory pools (secure teardown, overwritespool contents)
// Use when parsing sensitive data (passwords, tokens) to avoid information leakage
void json_cleanup(void);

// Get descriptive error message string for error code
// Parameters: error (json_error enum value)
// Returns: Pointer to static string (e.g., "Unexpected token" for JSON_ERROR_UNEXPECTED_TOKEN)
const char *json_error_string(json_error error);

// Get last error from most recent parse operation
// Returns: json_error enum value (JSON_OK if no error)
json_error json_get_error(void);

// Iterative parser for deeply nested JSON (avoids stack overflow)
// Use instead of json_parse() when depth > 128 nesting levels
bool json_parse_iterative(const char *json_string, size_t length, 
                          json_value *out_value, json_error *out_error);
```

### Data Structure ABI (Binary-Compatible Layout)

**`json_reference` (Zero-Copy String/Number Reference)**
```c
typedef struct {
    const char *ptr;     // Pointer to start of string in original input (offset 0)
    size_t len;          // Length in bytes (offset: pointer_size)
} json_reference;
// Total size: pointer_size + size_t (typically 16 bytes on 64-bit systems)
```

**`json_value` (Tagged Union with Type Discriminator)**
```c
typedef struct json_value {
    json_type type;      // Enum: JSON_TYPE_NULL, JSON_TYPE_BOOLEAN, JSON_TYPE_NUMBER, 
                         //       JSON_TYPE_STRING, JSON_TYPE_ARRAY, JSON_TYPE_OBJECT
    union {
        int boolean;                           // For JSON_TYPE_BOOLEAN (0 or 1)
        json_reference number;                 // For JSON_TYPE_NUMBER (reference to numeric string)
        json_reference string;                 // For JSON_TYPE_STRING (reference to unescaped content)
        struct json_array {
            struct json_value *first;          // Points to first array element (or NULL if empty)
            struct json_value *last;           // Points to last array element for O(1) append
            size_t length;                     // Element count
        } array;
        struct json_object {
            struct json_object_member *first;  // Points to first key-value pair
            struct json_object_member *last;   // Points to last pair for O(1) insert
            size_t length;                     // Member count
        } object;
    } u;
} json_value;
// Typical size: 8 bytes (type) + 24 bytes (union) = 32 bytes on 64-bit systems
```

**`json_array` (Linked List Node)**
```c
struct {
    json_value *first;   // Head of singly-linked list
    json_value *last;    // Tail for O(1) append, points to last->next_array_element
    size_t length;       // Total elements in linked list
};
// Iteration: for (json_value *elem = array.first; elem; elem = elem->next_array_element)
```

**`json_object_member` (Key-Value Pair)**
```c
typedef struct json_object_member {
    json_reference key;              // Key as zero-copy reference
    json_value value;                // Value (can be any json_type)
    struct json_object_member *next; // Next member in linked list
} json_object_member;
// Total size: sizeof(json_reference) + sizeof(json_value) + pointer_size
```

**`json_error` (Error Codes)**
```c
typedef enum {
    JSON_OK = 0,                          // No error, parsing successful
    JSON_ERROR_UNEXPECTED_TOKEN,          // Invalid character or malformed syntax
    JSON_ERROR_INVALID_NUMBER,            // Number parsing failed
    JSON_ERROR_INVALID_STRING,            // Unterminated string or invalid escape
    JSON_ERROR_INVALID_ESCAPE_SEQUENCE,   // Malformed \uXXXX or invalid escape char
    JSON_ERROR_CONTROL_CHARACTER,         // String contains control char < 0x20
    JSON_ERROR_INVALID_UTF8,              // (Reserved for future UTF-8 validation)
    JSON_ERROR_MEMORY_POOL_EXHAUSTED,     // Pool limit exceeded (JSON_VALUE_POOL_SIZE)
    JSON_ERROR_STACK_OVERFLOW,            // Nesting depth exceeded (JSON_STACK_SIZE)
} json_error;
```

## Compilation Configuration

### Compile-Time Flags
```bash
# Enable control-character validation (default: ON)
# Adds ~8–10% overhead but required for JSON spec compliance
-DSTRING_VALIDATION

# Use malloc/free instead of static pools (default: OFF)
# Reduces memory footprint for single-parse scenarios but adds allocation overhead
-DUSE_ALLOC

# CPU feature detection (automatic with -march=native, can be explicit)
-D__AVX2__        # Enable AVX2 SIMD (256-bit registers)
-D__SSE2__        # Enable SSE2 SIMD (128-bit registers)
# If neither defined, scalar C implementation used
```

### Build Options
```bash
# Standard C17 with maximum optimization and link-time optimization (LTO)
clang -std=c17 -O3 -march=native -flto -fuse-ld=lld

# Enable code coverage (generates .gcda files)
clang -fprofile-instr-generate -fcoverage-mapping -O0 -g

# Enable Profile-Guided Optimization (PGO) - two-pass build
clang -fprofile-generate=./profdata <sources>  # Pass 1: instrumented build
# Run tests to generate profdata
clang -fprofile-use=./profdata <sources>       # Pass 2: optimized with profile data
```

### Memory Pool Sizing
```c
// In src/json.h (tunable constants)
#define JSON_VALUE_POOL_SIZE    65536      // Maximum json_value structures (default)
#define JSON_STACK_SIZE         256        // Maximum nesting depth
#define JSON_STRING_POOL_SIZE   1048576    // Character buffer for parsed strings

// Typical memory usage: 32 * 65536 = 2 MB for value pool
```

## Platform Support

| Architecture | Variant | Support Level | Notes |
|:-------------|:-----------|:--------------|:------|
| x86-64 (Intel/AMD) | AVX2 | ✅ Full (Haswell 2013+) | 256-bit registers, 64-byte loop unroll, ~0.19s benchmark |
| x86-64 (Intel/AMD) | SSE2 | ✅ Full (Pentium 4 2000+) | 128-bit registers, 64-byte loop unroll, ~7% slower than AVX2 |
| x86-64 (Intel/AMD) | Scalar | ✅ Fallback | Pure C, no SIMD, ~10% slower than SSE2 |
| ARM (64-bit) | NEON | ⏳ Planned | Similar to SSE2 width (128-bit) |
| ARM (32-bit) | NEON | ⏳ Research | 128-bit, architecture dependent |
| MIPS | Scalar | ✅ Fallback | Pure C loop |
| RISC-V | Scalar | ✅ Fallback | Pure C loop, optional `V` extension support TBD |

## Build System Details

### Ninja Build Configuration
- **Linux**: `build.linux.ninja` — Default target: `perf-c-json-parser`
- **macOS**: `build.osx.ninja` — Includes Xcode integration
- **Windows**: `build.windows.ninja` — MSVC/Clang-CL compatibility (experimental)

### Build Targets
```bash
ninja -f build.linux.ninja main              # Debug: O0, symbols, string validation
ninja -f build.linux.ninja perf-c-json-parser  # Performance: O3, LTO, PGO-optimized
ninja -f build.linux.ninja perf-c-json-parser-no-validation  # No control-char validation
ninja -f build.linux.ninja test-perf-memory    # Memory usage benchmark
```

### Key Build Rules
1. **Compile `.asm` files**: Assembly lookup tables compiled from `*.asm` → object files
2. **Link with LLD**: All binaries linked with `-fuse-ld=lld` for speed and size optimization
3. **PGO Workflow**: Instrumented build → test execution → profile data collection → final optimized build
4. **Coverage Build**: Separate configuration with `-fprofile-instr-generate` for `-gcov` reports

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
