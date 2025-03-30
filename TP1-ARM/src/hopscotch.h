#ifndef HOPSCOTCH_H
#define HOPSCOTCH_H

#include <stdbool.h>
#include <stddef.h>

struct dictionary;
struct dictionary_entry;

typedef struct dictionary dictionary_t;
typedef void (*destroy_f)(void *);

dictionary_t *dictionary_create(destroy_f destroy);
bool dictionary_put(dictionary_t *dictionary, const char *key, void *value);
void *dictionary_get(dictionary_t *dictionary, const char *key, bool *err);
bool dictionary_delete(dictionary_t *dictionary, const char *key);
void *dictionary_pop(dictionary_t* dictionary, const char *key, bool *err);
bool dictionary_contains(dictionary_t *dictionary, const char *key);
size_t dictionary_size(dictionary_t *dictionary);
void dictionary_destroy(dictionary_t *dictionary);
void dictionary_free_entry( struct dictionary_entry *entry, destroy_f destroy );
bool dictionary_resize( struct dictionary *dictionary );

#endif