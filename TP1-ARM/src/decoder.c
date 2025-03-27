#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "shell.h"
#include "decoder.h"

uint16_t show( uint32_t instruction, int shift, uint32_t mask ){
    return ( instruction >> shift ) & mask;
}

void split_r(partition_t* parts, uint32_t instruction) {
    parts->opcode = (instruction >> 21) & 0x7FF;  // Extrae bits 31-21
    parts->rm = (instruction >> 16) & 0x1F;       // Extrae bits 20-16
    parts->shamt = (instruction >> 10) & 0x3F;    // Extrae bits 15-10
    parts->rn = (instruction >> 5) & 0x1F;        // Extrae bits 9-5
    parts->rd = instruction & 0x1F;               // Extrae bits 4-0
}


void split_i( partition_t* parts, uint32_t instruction ){
    parts -> opcode = show( instruction, 22, 0x3FF );
    parts -> alu = show( instruction, 10, 0xFFF );
    parts -> rn = show( instruction, 5, 0x1F );
    parts -> rd = instruction & 0x1F;
}

void split_d( partition_t* parts, uint32_t instruction ){
    parts -> opcode = show( instruction, 21, 0x7FF );
    parts -> dt = show( instruction, 12, 0x1FF );
    parts -> op = show( instruction, 10, 0x3F) ;
    parts -> rn = show( instruction, 5, 0x1F );
    parts -> rt = instruction & 0x1f;
}

void split_b( partition_t* parts, uint32_t instruction ){
    parts -> opcode = show( instruction, 26, 0x3F );
    parts -> br = instruction & 0x3FFFFFF;
}

void split_cb( partition_t* parts, uint32_t instruction ){
    parts -> opcode = show( instruction, 21, 0xFF );
    parts -> cond_br = show( instruction, 5, 0x7FFFF );
    parts -> rt = instruction & 0x1f;
}

void split_iw( partition_t* parts, uint32_t instruction ){
    parts-> opcode = show( instruction, 23, 0x7FF );
    parts-> mov = show( instruction, 5, 0x3FFFF );
    parts-> rd = instruction & 0x1F;
}