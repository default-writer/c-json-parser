/* json_array_node_pool definition for performance optimization
 * Global pool of json_array_node structures to reduce memory allocation overhead
 * Size: JSON_VALUE_POOL_SIZE (65535) * sizeof(json_array_node)
 */

.global json_array_node_pool

.data
.align 16
json_array_node_pool:
    .space 2097120  /* JSON_VALUE_POOL_SIZE (65535) * 32 bytes (actual sizeof(json_array_node)) */
