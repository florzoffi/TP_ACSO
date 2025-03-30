#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

#include "shell.h"
#include "hopscotch.h"
#include "decoder.h"
#include "functions.h"
extern int BRANCH_OCCURRED;

void adds_extended_register( partition_t *split_data ) {
   uint64_t operation = CURRENT_STATE.REGS[split_data->rn] + CURRENT_STATE.REGS[split_data->rm];
   NEXT_STATE.FLAG_N = operation >> 63;
   NEXT_STATE.FLAG_Z = operation == 0;
   NEXT_STATE.REGS[split_data->rd] = operation;

   if ( split_data->rd != 31 ) { NEXT_STATE.REGS[split_data->rd] = operation; }
}

void adds_immediate( partition_t *split_data ) {
   uint64_t imm = split_data->alu;
   if ( split_data->shamt == 0x1 ) { imm <<= 12; }
   uint64_t operation = CURRENT_STATE.REGS[split_data->rn] + imm;

   NEXT_STATE.FLAG_Z = ( operation == 0 );
   NEXT_STATE.FLAG_N = ( operation < 0 );
   NEXT_STATE.REGS[split_data->rd] = operation;

   if ( split_data->rd != 31 ) { NEXT_STATE.REGS[split_data->rd] = operation; }
}

void subs_extended_register( partition_t *split_data ) {
   uint64_t operation = CURRENT_STATE.REGS[split_data->rn] - CURRENT_STATE.REGS[split_data->rm];
   NEXT_STATE.FLAG_N = ( operation >> 63 ) & 1;
   NEXT_STATE.FLAG_Z = ( operation == 0 );

   if ( split_data->rd != 31 ) { NEXT_STATE.REGS[split_data->rd] = operation; }
}

void subs_immediate( partition_t *split_data ) {
   uint64_t imm = split_data->alu;
   if ( split_data->shamt == 0x1 ) { imm <<= 12; }
   uint64_t operation = CURRENT_STATE.REGS[split_data->rn] - imm;

   NEXT_STATE.FLAG_N = ( operation >> 63 ) & 1;
   NEXT_STATE.FLAG_Z = ( operation == 0 );

   if ( split_data->rd != 31 ) { NEXT_STATE.REGS[split_data->rd] = operation; }
}

void hlt( partition_t *split_data ) {
   RUN_BIT = 0;
   printf( "Simulator halted\n\n" );
}

void ands_shifted_register( partition_t *split_data ) {
   uint64_t operation = CURRENT_STATE.REGS[split_data->rn] & CURRENT_STATE.REGS[split_data->rm];
   NEXT_STATE.FLAG_N = operation >> 63;
   NEXT_STATE.FLAG_Z = operation == 0;

   if ( split_data->rd != 31 ) { NEXT_STATE.REGS[split_data->rd] = operation; }
}

void eor_shifted_register( partition_t *split_data ) {
   uint64_t operation = CURRENT_STATE.REGS[split_data->rn] ^ CURRENT_STATE.REGS[split_data->rm];
   NEXT_STATE.REGS[split_data->rd] = operation;
}

void orr_shifted_register( partition_t *split_data ) {
   uint64_t operation = CURRENT_STATE.REGS[split_data->rn] | CURRENT_STATE.REGS[split_data->rm];
   NEXT_STATE.REGS[split_data->rd] = operation;
}


void b( partition_t *split_data ) {
   NEXT_STATE.PC = CURRENT_STATE.PC + adjust_sign( split_data->br << 2, 28 );
   BRANCH_OCCURRED = TRUE;
}


void br_register( partition_t *split_data ) {
   CURRENT_STATE.PC = CURRENT_STATE.REGS[split_data->rn];
   BRANCH_OCCURRED = TRUE;
}

void extra( bool cond, int64_t offset ) {
    if ( cond ) {
        NEXT_STATE.PC = CURRENT_STATE.PC + offset;
        BRANCH_OCCURRED = TRUE;
    }
}

void b_cond( partition_t *split_data ) {
    uint32_t raw_value = split_data->cond_br;
    int64_t offset = adjust_sign( split_data->cond_br, 19 ) << 2;

    switch ( split_data->rt ) { 
        case 0:
            extra( CURRENT_STATE.FLAG_Z, offset );
            break;
        case 1:
            extra( !CURRENT_STATE.FLAG_Z, offset );
            break;
        case 10:
            extra( CURRENT_STATE.FLAG_N == 0, offset );
            break;
        case 11:
            extra( CURRENT_STATE.FLAG_N, offset );
            break;
        case 12:
            extra( !CURRENT_STATE.FLAG_N, offset );   
            break;
        case 13:
            extra( CURRENT_STATE.FLAG_N == 1 || CURRENT_STATE.FLAG_Z == 1, offset );
            break;
        default:
            break;
   }
}

void lsl_lsr_immediate( partition_t *split_data ) {
    uint64_t immr = split_data->alu >> 6;
    uint64_t imms = split_data->alu & 0x3F;

    if ( imms != 0x3F ) { NEXT_STATE.REGS[split_data->rd] = NEXT_STATE.REGS[split_data->rn] << ( 64 - immr ); }
    else { NEXT_STATE.REGS[split_data->rd] = NEXT_STATE.REGS[split_data->rn] >> immr; }
}

void stur( partition_t *split_data ) {
   uint64_t address = CURRENT_STATE.REGS[split_data->rn] + adjust_sign( split_data->dt, 9 );
   uint64_t value = CURRENT_STATE.REGS[split_data->rt];

   mem_write_32( address, value & 0xFFFFFFFF );
   mem_write_32( address + 4, value >> 32 );
}

void sturb( partition_t *split_data ) {
   uint64_t address = CURRENT_STATE.REGS[split_data->rn] + adjust_sign( split_data->dt, 9 );
   uint32_t value = CURRENT_STATE.REGS[split_data->rt] & 0xFF;
   uint32_t mem = mem_read_32( value ) & 0xFFFFFF00;

   value = value | mem;
   mem_write_32( address, value );
}

void sturh( partition_t *split_data ) {
   uint64_t address = CURRENT_STATE.REGS[split_data->rn] + adjust_sign( split_data->dt, 9 );
   uint64_t value = CURRENT_STATE.REGS[split_data->rt] & 0xFFFF;
   uint64_t mem = mem_read_32( address ) & 0xFFFF0000;
   value = value | mem;
   mem_write_32( address, value );
}

void ldur( partition_t *split_data ) {
   uint64_t address = CURRENT_STATE.REGS[split_data->rn] + adjust_sign( split_data->dt, 9 );
   uint32_t lower = mem_read_32( address );     
   uint32_t upper = mem_read_32( address + 4 );
   uint64_t value;

   value = ( uint64_t ) upper;
   value = ( value << 32 ) | lower;
   NEXT_STATE.REGS[split_data->rt] = value;
}

void ldurh( partition_t *split_data ) {
   uint64_t address = CURRENT_STATE.REGS[split_data->rn] + adjust_sign( split_data->dt, 9 );
   uint32_t value = mem_read_32( address ) & 0xFFFF;

   NEXT_STATE.REGS[split_data->rt] = value;
}

void ldurb( partition_t *split_data ) {
   uint64_t address = CURRENT_STATE.REGS[split_data->rn] + adjust_sign( split_data->dt, 9 );
   uint32_t value = mem_read_32( address ) & 0xFF;

   NEXT_STATE.REGS[split_data->rt] = value;
}

void movz( partition_t *split_data ) {
   uint64_t value = split_data->mov;
   if ( split_data->rd != 31 ) { NEXT_STATE.REGS[split_data->rd] = value; }
}

void add_immediate( partition_t *split_data ) {
   uint64_t imm = split_data->alu;
   if ( split_data->shamt == 0x1 ) { imm <<= 12; }

   uint64_t operation = CURRENT_STATE.REGS[split_data->rn] + imm;
   if ( split_data->rd != 31 ) { NEXT_STATE.REGS[split_data->rd] = operation; }
}

void add_extended_register( partition_t *split_data ) {
   uint64_t operation = CURRENT_STATE.REGS[split_data->rn] + CURRENT_STATE.REGS[split_data->rm];
   if ( split_data->rd != 31 ) { NEXT_STATE.REGS[split_data->rd] = operation; }
}

void mul_register( partition_t *split_data ) {
   uint64_t result = CURRENT_STATE.REGS[split_data->rn] * CURRENT_STATE.REGS[split_data->rm];
   if (split_data->rd != 31) { NEXT_STATE.REGS[split_data->rd] = result; }
}

void cbz( partition_t *split_data ) {
    if ( CURRENT_STATE.REGS[split_data->rt] == 0 ) {
        int64_t offset = adjust_sign( split_data->cond_br << 2, 21 );
        NEXT_STATE.PC = CURRENT_STATE.PC + offset;
        BRANCH_OCCURRED = TRUE;
    }
}

void cbnz( partition_t *split_data ) {
    if ( CURRENT_STATE.REGS[split_data->rt] != 0 ) {
        int64_t offset = adjust_sign( split_data->cond_br << 2, 21 );
        NEXT_STATE.PC = CURRENT_STATE.PC + offset;
        BRANCH_OCCURRED = TRUE;
    }
} 