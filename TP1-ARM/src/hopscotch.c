#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "hopscotch.h"

#define FNV_OFFSET_BASIS 2166136261U
#define FNV_PRIME 16777619U
#define INITIAL_CAPACITY 128
#define LOAD_FACTOR 0.5
#define HOPSCOTCH_BUCKETS 64

enum entry_status { EMPTY, OCCUPIED, DELETED };

struct dictionary_entry {
    char *key;
    void *value;
    enum entry_status status;
    uint32_t hop_info;
};

struct dictionary {
    struct dictionary_entry *table;
    size_t size;
    size_t capacity;
    destroy_f destroy;
};

uint32_t fnv1a_hash( const char *key ) {
    uint32_t hash = FNV_OFFSET_BASIS;
    while ( *key ) {
        hash ^= ( unsigned char )( *key++ );
        hash *= FNV_PRIME;
    }
    return hash;
}

uint32_t hash( const char *key, uint32_t capacity ) {
    return fnv1a_hash( key ) % capacity;
}

char *my_strdup( const char *s ) {
    if ( !s ) { return NULL; }
    size_t len = strlen( s ) + 1;
    char *d = ( char* )malloc( len );
    if ( !d ) { return NULL; }
    memcpy( d, s, len );
    return d;
}

dictionary_t *dictionary_create(destroy_f destroy) {
    struct dictionary *dict = ( struct dictionary* )malloc( sizeof( struct dictionary ) );
    if ( !dict ) { return NULL; }

    dict->table = calloc( INITIAL_CAPACITY, sizeof( struct dictionary_entry ) );
    if ( !dict->table ) {
        free( dict );
        return NULL;
    }

    dict->size = 0;
    dict->capacity = INITIAL_CAPACITY;
    dict->destroy = destroy;
    return dict;
}

void dictionary_free_entry( struct dictionary_entry *entry, destroy_f destroy ) {
    if ( destroy && entry->status == OCCUPIED ) {
        destroy( entry->value );
    }
    if ( entry->status != EMPTY ) {
        free( entry->key );
    }
    entry->key = NULL;
    entry->value = NULL;
    entry->status = EMPTY;
}

bool dictionary_resize( struct dictionary *dictionary ) {
    size_t new_capacity = dictionary->capacity * 2;
    struct dictionary_entry *new_table = calloc( new_capacity, sizeof( struct dictionary_entry ) );
    if ( !new_table ) { return false; }

    for ( size_t i = 0; i < dictionary->capacity; i++ ) {
        struct dictionary_entry *entry = &dictionary->table[i];
        if ( entry->status == OCCUPIED ) {
            uint32_t index1 = hash( entry->key, (uint32_t)new_capacity );

            for ( size_t j = 0; j < HOPSCOTCH_BUCKETS; j++ ) {
                uint32_t index = ( index1 + ( uint32_t ) j ) % ( uint32_t )new_capacity;
                struct dictionary_entry *new_entry = &new_table[index];
                if ( new_entry->status != OCCUPIED ) {
                    *new_entry = *entry;
                    new_entry->hop_info = 0;
                    break;
                }
            }
        }
    }

    free( dictionary->table );
    dictionary->table = new_table;
    dictionary->capacity = new_capacity;
    return true;
}

bool dictionary_put( dictionary_t *dictionary, const char *key, void *value ) {
    if ( ( double )dictionary->size >= ( double )dictionary->capacity * LOAD_FACTOR ) {
        if ( !dictionary_resize( dictionary ) ) { return false; }
    }

    size_t capacity = dictionary->capacity;
    uint32_t index1 = hash( key, ( uint32_t )capacity );
    struct dictionary_entry *base_entry = &dictionary->table[index1];

    char *duplicated_key = my_strdup( key );
    if (!duplicated_key) { return false; }

    for ( size_t i = 0; i < HOPSCOTCH_BUCKETS; i++ ) {
        uint32_t index = (index1 + ( uint32_t )i ) % ( uint32_t )capacity;
        struct dictionary_entry *entry = &dictionary->table[index];

        if ( entry->status == OCCUPIED && strcmp( entry->key, key ) == 0 ) {
            if ( dictionary->destroy ) { dictionary->destroy(entry->value); }
            entry->value = value;
            free( duplicated_key );
            return true;
        } else if ( entry->status != OCCUPIED ) {
            if ( entry->status == DELETED ) {
                free( entry->key );
                entry->key = NULL;
            }

            entry->key = duplicated_key;
            entry->value = value;
            entry->status = OCCUPIED;
            entry->hop_info = 0;
            base_entry->hop_info |= ( 1 << i );
            dictionary->size++;
            return true;
        }
    }

    free( duplicated_key );

    uint32_t free_index = ( index1 + ( uint32_t )HOPSCOTCH_BUCKETS ) % ( uint32_t )capacity;
    while ( true ) {
        uint32_t candidate_index = free_index;
        for ( size_t j = 0; j < HOPSCOTCH_BUCKETS; j++ ) {
            uint32_t candidate_hop_index = ( candidate_index + ( uint32_t )capacity - ( uint32_t )j ) % ( uint32_t )capacity;
            struct dictionary_entry *candidate_entry = &dictionary->table[candidate_hop_index];

            if ( candidate_entry->status == OCCUPIED ) {
                uint32_t candidate_base_index = hash( candidate_entry->key, ( uint32_t )capacity );
                uint32_t candidate_distance = ( candidate_hop_index + ( uint32_t )capacity - candidate_base_index ) % ( uint32_t )capacity;

                if ( candidate_distance < HOPSCOTCH_BUCKETS ) {
                    dictionary->table[free_index] = *candidate_entry;
                    dictionary->table[candidate_hop_index].status = EMPTY;
                    free_index = candidate_hop_index;
                    break;
                }
            }
        }
        return false;
    }
}

void *dictionary_get( dictionary_t *dictionary, const char *key, bool *err ) {
    size_t capacity = dictionary->capacity;
    uint32_t index1 = hash( key, ( uint32_t )capacity );

    for (size_t i = 0; i < HOPSCOTCH_BUCKETS; i++) {
        uint32_t index = ( index1 + ( uint32_t )i ) % ( uint32_t )capacity;
        struct dictionary_entry *entry = &dictionary->table[index];

        if ( entry->status == OCCUPIED && strcmp( entry->key, key ) == 0 ) {
            if ( err ) *err = false;
            return entry->value;
        } else if ( entry->status == EMPTY ) {
            break;
        }
    }

    if ( err ) *err = true;
    return NULL;
}

bool dictionary_delete( dictionary_t *dictionary, const char *key ) {
    size_t capacity = dictionary->capacity;
    uint32_t index1 = hash( key, ( uint32_t )capacity );

    for ( size_t i = 0; i < HOPSCOTCH_BUCKETS; i++ ) {
        uint32_t index = ( index1 + ( uint32_t )i ) % ( uint32_t )capacity;
        struct dictionary_entry *entry = &dictionary->table[index];

        if ( entry->status == OCCUPIED && strcmp( entry->key, key ) == 0 ) {
            if ( dictionary->destroy ) dictionary->destroy( entry->value );
            free( entry->key );
            entry->key = NULL;
            entry->status = DELETED;
            dictionary->size--;
            return true;
        } else if ( entry->status == EMPTY ) {
            break;
        }
    }

    return false;
}

void *dictionary_pop( dictionary_t *dictionary, const char *key, bool *err ) {
    size_t capacity = dictionary->capacity;
    uint32_t index1 = hash(key, ( uint32_t )capacity );

    for ( size_t i = 0; i < HOPSCOTCH_BUCKETS; i++ ) {
        uint32_t index = ( index1 + ( uint32_t )i ) % ( uint32_t )capacity;
        struct dictionary_entry *entry = &dictionary->table[index];

        if ( entry->status == OCCUPIED && strcmp( entry->key, key ) == 0 ) {
            void *value = entry->value;
            free( entry->key );
            entry->key = NULL;
            entry->value = NULL;
            entry->status = DELETED;
            dictionary->size--;
            if ( err ) *err = false;
            return value;
        } else if ( entry->status == EMPTY ) {
            break;
        }
    }

    if ( err ) *err = true;
    return NULL;
}

bool dictionary_contains( dictionary_t *dictionary, const char *key ) {
    size_t capacity = dictionary->capacity;
    uint32_t index1 = hash( key, ( uint32_t )capacity );

    for ( size_t i = 0; i < HOPSCOTCH_BUCKETS; i++ ) {
        uint32_t index = ( index1 + ( uint32_t )i ) % ( uint32_t )capacity;
        struct dictionary_entry *entry = &dictionary->table[index];

        if ( entry->status == OCCUPIED && strcmp( entry->key, key ) == 0 ) {
            return true;
        } else if ( entry->status == EMPTY ) {
            break;
        }
    }

    return false;
}

size_t dictionary_size( dictionary_t *dictionary ) {
    return dictionary->size;
}

void dictionary_destroy( dictionary_t *dictionary ) {
    for ( size_t i = 0; i < dictionary->capacity; i++ ) {
        dictionary_free_entry( &dictionary->table[i], dictionary->destroy );
    }
    free( dictionary->table );
    free( dictionary );
}