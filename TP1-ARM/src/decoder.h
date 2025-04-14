#ifndef DECODER_H
#define DECODER_H

#include <stdint.h>

typedef struct{
    uint32_t opcode;
    uint32_t rm;
    uint32_t shamt;
    uint32_t rn;
    uint32_t rd;
    uint32_t alu;
    uint32_t br;
    uint32_t dt;
    uint32_t op;
    uint32_t rt;
    uint32_t cond_br;
    uint32_t mov;
}partition_t;

typedef struct {
    void ( *decode )( partition_t *decoded, uint32_t instruction_code );
    void ( *execute )( partition_t *decoded );
    char *name;
} instruction_info_t;

void init_instruction_table();
char* uint32_to_string( uint32_t number );
void ADD_INSTRUCTION ( uint32_t opcode, void  (*decode_fn )( partition_t*, uint32_t ), void ( *execute_fn )( partition_t* ), const char* name_str );
uint64_t adjust_sign( uint64_t n, size_t bits );
void process_instruction();

void decode(void);
void split_r( partition_t* parts, uint32_t instruction );
void split_i( partition_t* parts, uint32_t instruction );
void split_d( partition_t* parts, uint32_t instruction );
void split_b( partition_t* parts, uint32_t instruction );
void split_cb( partition_t* parts, uint32_t instruction );
void split_iw( partition_t* parts, uint32_t instruction );
uint32_t show(uint32_t instruction, int shift, uint32_t mask);

#endif