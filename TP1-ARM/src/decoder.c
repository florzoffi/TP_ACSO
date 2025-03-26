#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "shell.h"

void decode(){
    uint32_t instruction = mem_read_32( NEXT_STATE.PC );
    uint32_t opcode_mask = 0b11111111000000000000000000000000;
    uint32_t opcode = (instruction & opcode_mask) >> 24;
}

typedef struct {
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
} partition;

typedef enum {
    INSTR_TYPE_R,
    INSTR_TYPE_I,
    INSTR_TYPE_D,
    INSTR_TYPE_B,
    INSTR_TYPE_CB,
    INSTR_TYPE_IW,
    INSTR_TYPE_UNKNOWN  // Añadir un tipo desconocido para manejar casos no identificados
} InstructionCategory;

uint16_t show( uint32_t instruction, int shift, uint32_t mask ){
    return ( instruction >> shift ) & mask;
}

void instruction_sort( InstructionCategory category, uint32_t instruction ) {
    partition parts;

    switch ( category ) {
        case INSTR_TYPE_R:
            split_r(instruction, &parts);
            break;
        case INSTR_TYPE_I:
            split_i(instruction, &parts);
            break;
        case INSTR_TYPE_D:
            split_d(instruction, &parts);
            break;
        case INSTR_TYPE_B:
            split_b(instruction, &parts);
            break;
        case INSTR_TYPE_CB:
            split_cb(instruction, &parts);
            break;
        case INSTR_TYPE_IW:
            split_iw(instruction, &parts);
            break;
        case INSTR_TYPE_UNKNOWN:
        default:
            printf("Error: instrucción desconocida (opcode = 0x%X)\n", instruction);
            break;
    }
}

void split_r( uint32_t instruction, partition* parts ){
    parts -> opcode = shift_bits( instruction, 21, 0x7FF );
    parts -> rm = shift_bits( instruction, 16, 0x1F );
    parts -> shamt = shift_bits( instruction, 10, 0x3F) ;
    parts -> rn = shift_bits( instruction, 5, 0x1F );
    parts -> rd = instruction & 0x1F;
}

void split_i(uint32_t instruction, partition* parts) {
    parts -> opcode = shift_bits(instruction, 22, 0x3FF);
    parts -> alu = shift_bits(instruction, 10, 0xFFF);
    parts -> rn = shift_bits(instruction, 5, 0x1F);
    parts -> rd = instruction & 0x1F;
}

void split_d( uint32_t instruction, partition* parts ){
    parts -> opcode = shift_bits( instruction, 21, 0x7FF );
    parts -> dt = shift_bits( instruction, 12, 0x1FF );
    parts -> op = shift_bits( instruction, 10, 0x3F) ;
    parts -> rn = shift_bits( instruction, 5, 0x1F );
    parts -> rt = instruction & 0x1f;
}

void split_b(uint32_t instruction, partition* parts) {
    parts -> opcode = shift_bits(instruction, 26, 0x3F);
    parts -> br = instruction & 0x3FFFFFF;
}

void split_cb( uint32_t instruction, partition* parts ){
    parts -> opcode = shift_bits( instruction, 21, 0x7FF );
    parts -> cond_br = shift_bits( instruction, 5, 0x7FFFF );
    parts -> rt = instruction & 0x1f;
}

void split_iw(uint32_t instruction, partition* parts) {
    parts-> opcode = shift_bits(instruction, 23, 0x1FF);
    parts-> mov = shift_bits(instruction, 5, 0x3FFFF);
    parts-> rd = instruction & 0x1F;
}