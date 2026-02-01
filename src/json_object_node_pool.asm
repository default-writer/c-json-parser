/* json_object_node_pool definition for performance optimization
 * Global pool of json_object_node structures to reduce memory allocation overhead
 * Size: JSON_VALUE_POOL_SIZE (65535) * sizeof(json_object_node)
 */

.global json_object_node_pool

.data
.align 16
json_object_node_pool:
    .space 3145680  /* JSON_VALUE_POOL_SIZE (65535) * 48 bytes (actual sizeof(json_object_node)) */
