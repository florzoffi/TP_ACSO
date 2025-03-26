#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "shell.h"
#include "decoder.h"

void decode(){
    uint32_t instruction = mem_read_32( NEXT_STATE.PC );
    uint32_t opcode_mask = 0b11111111000000000000000000000000;
    uint32_t opcode = (instruction & opcode_mask) >> 24;
}

uint16_t show( uint32_t instruction, int shift, uint32_t mask ){
    return ( instruction >> shift ) & mask;
}

void split_r( uint32_t instruction, partition_t* parts ){
    parts -> opcode = show( instruction, 21, 0x7FF );
    parts -> rm = show( instruction, 16, 0x1F );
    parts -> shamt = show( instruction, 10, 0x3F) ;
    parts -> rn = show( instruction, 5, 0x1F );
    parts -> rd = instruction & 0x1F;
}

void split_i(uint32_t instruction, partition_t* parts) {
    parts -> opcode = show(instruction, 22, 0x3FF);
    parts -> alu = show(instruction, 10, 0xFFF);
    parts -> rn = show(instruction, 5, 0x1F);
    parts -> rd = instruction & 0x1F;
}

void split_d( uint32_t instruction, partition_t* parts ){
    parts -> opcode = show( instruction, 21, 0x7FF );
    parts -> dt = show( instruction, 12, 0x1FF );
    parts -> op = show( instruction, 10, 0x3F) ;
    parts -> rn = show( instruction, 5, 0x1F );
    parts -> rt = instruction & 0x1f;
}

void split_b(uint32_t instruction, partition_t* parts) {
    parts -> opcode = show(instruction, 26, 0x3F);
    parts -> br = instruction & 0x3FFFFFF;
}

void split_cb( uint32_t instruction, partition_t* parts ){
    parts -> opcode = show( instruction, 21, 0x7FF );
    parts -> cond_br = show( instruction, 5, 0x7FFFF );
    parts -> rt = instruction & 0x1f;
}

void split_iw(uint32_t instruction, partition_t* parts) {
    parts-> opcode = show(instruction, 23, 0x1FF);
    parts-> mov = shift_bits(instruction, 5, 0x3FFFF);
    parts-> rd = instruction & 0x1F;
}