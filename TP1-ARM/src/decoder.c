#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "shell.h"

void decode(){
    uint32_t instruction = mem_read_32( NEXT_STATE.PC );
    uint32_t opcode_mask = 0b11111111000000000000000000000000;
    uint32_t opcode = (instruction & opcode_mask) >> 24;
    
}
