#include <string.h>
#include "map.h"

Map *new_map() {
  Map *map = malloc(sizeof(Map));
  map->keys = new_vector();
  map->vals = new_vector();
  return map;
}

void map_put(Map *map, char *key, void *val) {
  vec_push(map->keys, key);
  vec_push(map->vals, val);
}

void map_put_safe(Map *map, char *key, void *val) {
  void *v = map_get(map, key);
  if (v != NULL || v != val)
    error("same key putted to a map");
  vec_push(map->keys, key);
  vec_push(map->vals, val);
}

void *map_get(Map *map, char *key) {
  for (int i = map->keys->len - 1; 0 <= i; i--) {
    if (strcmp(map->keys->data[i], key) == 0) {
      return map->vals->data[i];
    }
  }
  return NULL;
}

int map_size(Map *map) {
  return map->keys->len;
}
