#ifndef DECODER_H
#define DECODER_H

#include <stdint.h>

typedef struct partition_t{
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
};

void decode(void);
void split_r(uint32_t instruction, partition_t* parts);
void split_i(uint32_t instruction, partition_t* parts);
void split_d(uint32_t instruction, partition_t* parts);
void split_b(uint32_t instruction, partition_t* parts);
void split_cb(uint32_t instruction, partition_t* parts);
void split_iw(uint32_t instruction, partition_t* parts);
uint16_t show(uint32_t instruction, int shift, uint32_t mask);

#endif