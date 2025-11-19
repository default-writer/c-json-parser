## TL;DR  

The current implementation stores **only the tree nodes** (`json_value`) in a tiny fixed‑size pool and keeps **all container metadata (arrays, objects) in contiguous “vector‑like” buffers** that are grown with `realloc`.  

### Alternatives

* `json_value *items` (instead of `json_value **items`) – i.e. an *inline* array of whole `json_value` structs, or  
* `json_object **items` (instead of `json_object *items`) – i.e. an array of *pointers* to individually allocated `json_object`s, or  
* a representation where an object is a `json_value **` with the first element being a *key* value  

Changing the container members to another altenatives forces the parser to allocate **one extra heap block per element** (or per key/value pair) and, in the case of the inline‑struct version, blows up the size of each element dramatically. The result is a **10‑× increase in the number of malloc/realloc calls** (the benchmark shows 4.4 M vs 52.9 M) and a **3‑× larger total heap footprint**.

### Source code

Below is a line‑by‑line walk‑through of the code that shows why the current layout is so cheap and why the alternatives are expensive.

## 1. What the current data structures *are*

```c
typedef struct json_value json_value;
typedef struct json_object json_object;

typedef struct {
    const char *ptr;
    size_t      len;
} reference;                 // “view” into the original input string

struct json_value {
    json_type type;
    union {
        reference string;    // J_STRING
        reference boolean;   // J_BOOLEAN
        reference number;    // J_NUMBER
        struct {
            json_value **items;   // <-- array of *pointers* to child nodes
            size_t      count;
            size_t      capacity;
        } array;                // J_ARRAY
        struct {
            json_object *items;   // <-- contiguous array of key/value structs
            size_t      count;
            size_t      capacity;
        } object;               // J_OBJECT
    } u;
};

struct json_object {
    reference   key;      // points directly into the source JSON
    json_value *value;    // pointer to the *single* node that holds the value
};
```

* **Only the *nodes* (`json_value`) are allocated from the pool** (`json_value_pool`).  
* **Container metadata lives in separate buffers** (`json_value **items` for arrays, `json_object *items` for objects).  
* Keys are stored as a *reference* (`reference`) into the original string – **no copy, no extra allocation**.  

Because of that:

* **Every JSON value (string, number, boolean, object, array, null) is represented by exactly **one** `json_value` struct**.  
* **The only heap blocks that grow dynamically are the “vector” buffers** for array elements and object members.  
* Those buffers are *re‑allocated* only when the capacity is exhausted, i.e. **O(log N) allocations per container**, not O(N).

## 2. Where the allocations actually happen

| Function | What it allocates | Why it is cheap |
|----------|-------------------|-----------------|
| `new_json_value()` | pulls a pre‑zeroed `json_value` from `json_value_free_pool` (a static array of 512 structs) | No `malloc` at all – just a pointer bump. |
| `json_array_push()` | `realloc` of `arr->u.array.items` when capacity is full | Grows geometrically (`*2`), so each element causes a `realloc` only once every *capacity* insertions. |
| `json_object_set_take_key()` | `realloc` of `obj->u.object.items` when capacity is full | Same geometric growth. |
| `free_json_value_contents()` | walks the tree, frees the *vector* buffers (`free(v->u.array.items)`, `free(v->u.object.items)`) and then puts the node back into the pool. | Only one free per container, not per element. |

No other `malloc`/`free` calls appear in the parser or printer. That is why the benchmark shows **≈ 4 M allocation calls** (the *realloc* calls for the vectors) versus **≈ 53 M** for a naïve design.

## 3. What would happen if we change the container members

### 3.1 `json_value **items` → `json_value *items`

```c
struct {
    json_value *items;   // <-- inline array of whole json_value structs
    size_t count;
    size_t capacity;
} array;
```

* **Size blow‑up** – a `json_value` is ~48 bytes on a 64‑bit platform (type + union).  
  An array of *pointers* (`json_value **`) is 8 bytes per element; an inline array of structs is 48 bytes per element → **6× more memory** for the same JSON.  

* **Copy‑on‑push** – `json_array_push()` would need to *copy* the whole struct into the contiguous buffer, which means:
  * a `memcpy` of a 48‑byte block for every element,
  * and **every nested object/array would be duplicated** because the copied struct still contains its own pointers to child buffers.  
  This quickly turns a shallow tree into a deep copy and leads to *exponential* memory consumption for nested structures.

* **No pool reuse** – the pool only supplies the *node* structs. If the container holds the structs themselves, the pool can’t be used for them, so each element must be allocated from the *heap* (e.g. via `malloc` or `realloc`). The number of allocation calls jumps from *log N* per container to *N* per container.

* **Fragmentation** – each `realloc` now has to move a huge block (potentially many kilobytes) because the element size is large, causing more page faults and higher latency.

### 3.2 `json_object *items` → `json_object **items`

```c
struct {
    json_object **items;   // <-- array of pointers to individually allocated json_object structs
    size_t count;
    size_t capacity;
} object;
```

* **One extra heap block per member** – every key/value pair would require a separate `malloc` for the `json_object` struct (which itself holds a `reference` and a `json_value *`).  

* **Allocation pattern** – when an object grows, `json_object_set_take_key()` would have to:
  1. `realloc` the pointer array (`json_object **`),
  2. `malloc` a new `json_object` for the new entry,
  3. store the pointer.  

  This means **two allocations per insertion** (the pointer array and the struct) instead of one (just the pointer array). With 1 M object members you end up with ~2 M heap blocks, which is exactly the kind of explosion seen in the benchmark.

* **Cache unfriendly** – the pointer array no longer contains the actual key/value data, so a lookup (`json_object_get`) has to chase an extra pointer indirection for every member, hurting branch prediction and L1 cache hit rates.

### 3.3 Represent an object as `json_value **` where the first element is the key

```c
/* pseudo‑layout
   json_value **obj;   // obj[0] = key (string value)
                       // obj[1] = value (any json_value)
                       // obj[2] = next key …
*/
```

* **Two `json_value` structs per member** – one for the *key* (a string) and one for the *value*. In the current design the key is just a `reference` (two pointers) stored *inside* the `json_object` struct, which lives in the same contiguous buffer as the value pointer. Using a full `json_value` for the key forces:
  * a **separate allocation** from the pool for the key (or a copy of the string if the pool is exhausted),
  * an extra **pointer indirection** (`obj[i]` points to a heap‑allocated struct).

* **Loss of the “take‑key” optimisation** – the parser currently does `json_object_set_take_key(obj, ref.ptr, ref.len, value)`. The key pointer is *borrowed* from the source string, so no extra node is created. If the key were a `json_value`, the parser would have to allocate a new node (or copy the string) for every key, inflating the allocation count by the number of object members.

* **Memory layout becomes irregular** – the object’s “vector” now stores *pointers to heterogeneous structs* (key nodes and value nodes) rather than a compact array of homogeneous `json_object` structs. This leads to more `malloc`/`free` calls and a larger overall heap size.

## 4. How the current design avoids the pitfalls

| Feature | Implementation detail | Effect on allocations / memory |
|---------|----------------------|--------------------------------|
| **Node pool** | `static json_value json_value_pool[JSON_VALUE_POOL_SIZE];` + `json_value_free_pool[]` | Zero‑cost node creation (no `malloc`). |
| **Reference‑only keys** | `reference key;` points directly into the original JSON string | No string copy, no extra `json_value`. |
| **Vector‑like containers** | `json_value **items` (array of *pointers*) and `json_object *items` (array of *structs*) | One allocation per capacity increase (geometric growth). |
| **Geometric growth** | `capacity ? capacity*2 : DICTIONARY_SIZE` | `log₂(N)` reallocs per container → ~4 M allocations for a large document. |
| **Single free per container** | `free(v->u.array.items);` / `free(v->u.object.items);` | Keeps free‑call count low. |
| **No per‑element malloc** | Elements are stored *by value* (pointers) inside the vector, not as separate heap blocks. | Allocation count stays low, memory fragmentation stays low. |

Because of these choices the parser can handle a 1‑GB JSON document with **≈ 4 M allocation calls** and a **≈ 1.3 GB** heap footprint (most of that is the original JSON string itself, which is *borrowed*).

## 5. Quantitative illustration

Assume an object with **100 000 members** (a typical “large map”).

| Design | # of heap blocks | Approx. memory for container metadata* |
|--------|------------------|----------------------------------------|
| **Current** (`json_object *items`) | 1 (the contiguous `json_object` array) | `100 000 * sizeof(json_object)` ≈ 2 MiB |
| **Pointer‑to‑object** (`json_object **items`) | 1 + 100 000 (one for each `json_object`) | 1 × ≈ 8 MiB (pointer array) + 100 000 × ≈ 24 MiB (structs) ≈ 32 MiB |
| **Inline `json_value` array** (`json_value *items`) | 1 (the contiguous `json_value` array) | `100 000 * sizeof(json_value)` ≈ 5 MiB **plus** each `json_value` contains its own union, so nested objects/arrays cause deep copies. |
| **Object as `json_value **` (key + value)** | 1 + 200 000 (key node + value node) | 1 × ≈ 8 MiB + 200 000 × ≈ 48 MiB ≈ 96 MiB |

\*Numbers are rough (64‑bit platform, `sizeof(json_object)=24`, `sizeof(json_value)=48`). The *allocation calls* follow the same pattern: one per container vs one per element.

---

## 6. Why the benchmark numbers make sense

| Metric | c‑json‑parser (current) | “naïve” variant |
|--------|------------------------|-----------------|
| Execution time | 1.04 s | 4.20 s |
| Allocation calls | 4.4 M | 52.9 M |
| Heap usage | 1.29 GB | 4.18 GB |

* The naïve variant does a `malloc` for **every key/value pair** (≈ 50 M calls for a ~5 MB JSON).  
* Each call incurs lock contention inside the allocator, cache misses, and bookkeeping overhead → 4× slower.  
* The extra allocations also increase fragmentation and the overall amount of memory that the allocator must manage → ~3× larger RSS.

## 7. Bottom line – design guidelines for a low‑allocation JSON parser

1. **Allocate only the logical nodes** (`json_value`) from a pre‑allocated pool or a bump allocator.  
2. **Store container members in contiguous buffers** (`json_value **` for arrays, `json_object *` for objects) and grow them geometrically.  
3. **Never copy keys** – keep a `reference` that points back into the original input string.  
4. **Avoid per‑element heap blocks** (no `json_object **`, no `json_value *` arrays).  
5. **Reuse the same node pool** across multiple parses (reset with `json_pool_reset()`).  
6. **Keep the union inside `json_value` small** (just two pointers for a reference) – the larger the union, the more memory you waste for each node that lives in the pool.

If you follow those rules you will end up with the same allocation profile as the *c‑json‑parser* shown in the table: a handful of megabytes of heap, a few million `realloc` calls, and sub‑second parse times for multi‑gigabyte inputs.

### Quick code sketch for the “bad” alternatives (what not to do)

```c
/* BAD: array stores whole json_value structs */
typedef struct {
    json_value *items;   // <-- WRONG
    size_t count, capacity;
} json_array_bad;

/* BAD: object stores pointers to individually allocated json_object */
typedef struct {
    json_object **items; // <-- WRONG
    size_t count, capacity;
} json_object_bad;

/* BAD: object as json_value ** (key/value as separate nodes) */
typedef struct {
    json_value **kv;     // kv[0] = key (string json_value)
                         // kv[1] = value (any json_value)
    size_t count;        // count = 2 * number_of_members
    size_t capacity;
} json_object_as_valueptrs;
```

Each of those forces a `malloc` per element (or per pair) and destroys the O(1)‑per‑container‑growth property that keeps the parser fast and memory‑efficient.

**The benchmark’s speed isn’t a hidden compiler trick – it’s all about the data‑structure choice.**  

| What the current design does | Why it’s fast |
|------------------------------|----------------|
| **One pool‑allocated `json_value` node per JSON value** | No per‑element `malloc`; nodes come from a static pool. |
| **One contiguous vector (dynamic array) per container** | Only a *logarithmic* number of `realloc` calls as the vector grows. |

### What happens when you change the layout  

- **`json_value **items` → `json_value *items`**  
  * Stores whole `json_value` structs inline.  
  * Each element becomes **~6× larger** (pointer → struct) and must be **copied** on every push, blowing up memory and CPU usage.  

- **`json_object *items` → `json_object **items`**  
  * Introduces **one extra heap allocation per key/value pair** (the `json_object` itself).  
  * Allocation pattern changes from *one per capacity increase* to *two per insertion* → massive increase in allocation calls.  

- **Object as `json_value **` with the first entry being a key**  
  * Forces a full `json_value` node for every key (instead of a cheap `reference`).  
  * Results in **two nodes per member** (key + value) and extra pointer indirections, turning the previously *logarithmic* allocation behaviour into a *linear* one.

### Conclusion  

- The current layout gives **O(log N)** allocation work per container.  
- The alternatives turn it into **O(N)** allocations and dramatically increase the heap footprint, causing the parser to **exhaust memory** and **slow down**.  
- Keep the pool‑allocated node per value and a single contiguous vector per container to stay fast and memory‑efficient.
