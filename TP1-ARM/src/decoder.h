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


void adds_extended_register(partition_t* p);
void adds_immediate(partition_t* p);
void subs_extended_register(partition_t* p);
void subs_immediate(partition_t* p);
void hlt(partition_t* p);
void ands_shifted_register(partition_t* p);
void eor_shifted_register(partition_t* p);
void orr_shifted_register(partition_t* p);
void b(partition_t* p);
void br_register(partition_t* p);
void b_cond(partition_t* p);
void lsl_lsr_immediate(partition_t* p);

void stur(partition_t* p);
void sturb(partition_t* p);
void sturh(partition_t* p);
void ldur(partition_t* p);
void ldurb(partition_t* p);
void ldurh(partition_t* p);
void movz(partition_t* p);

void add_immediate(partition_t* p);
void add_extended_register(partition_t* p);
void mul_register(partition_t* p);
void cbz(partition_t* p);
void cbnz(partition_t* p);

void init_instruction_table();
char* uint32_to_string( uint32_t number );

void decode(void);
void split_r( partition_t* parts, uint32_t instruction );
void split_i( partition_t* parts, uint32_t instruction );
void split_d( partition_t* parts, uint32_t instruction );
void split_b( partition_t* parts, uint32_t instruction );
void split_cb( partition_t* parts, uint32_t instruction );
void split_iw( partition_t* parts, uint32_t instruction );
uint16_t show(uint32_t instruction, int shift, uint32_t mask);

#endif