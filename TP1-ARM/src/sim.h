#include "decoder.h"

typedef struct {
    void ( *decode )( partition_t *decoded, uint32_t instruction_code );
    void ( *execute )( partition_t *decoded );
    char *name;
} instruction_info_t;

void adds_extended_register(partition_t* p);
void init_instruction_table();
char* uint32_to_string( uint32_t number );