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

int BRANCH_OCCURRED = FALSE;
static dictionary_t *instruction_table = NULL;

char* uint32_to_string( uint32_t number ) {
    char* key = malloc( 11 );
    if ( key == NULL ) {
        exit( 1 );
    }
    sprintf( key, "%u", number );
    return key;
}

void ADD_INSTRUCTION ( uint32_t opcode, void  (*decode_fn )( partition_t*, uint32_t ), void ( *execute_fn )( partition_t* ), const char* name_str ) {
    instruction_info_t *info = malloc( sizeof( instruction_info_t ) );
    if ( !info ) {
        exit( 1 );
    }
    info->decode = decode_fn;
    info->execute = execute_fn;
    info->name = strdup( name_str );
    if ( !info->name ) {
       free( info );
       exit( 1 );
    }
    char* key = uint32_to_string( opcode );
    dictionary_put( instruction_table, key, info );
    free( key );
    return;
}


void init_instruction_table() {
    instruction_table = dictionary_create( free );
    if ( !instruction_table ) { 
        fprintf( stderr, "Failed to create hash table.\n" );
        exit( 1 );
    }
    ADD_INSTRUCTION( 0x558, split_r, adds_extended_register, "Adds Extended Register" );
    ADD_INSTRUCTION( 0xB1, split_i, adds_immediate, "Adds Immediate" );
    ADD_INSTRUCTION( 0x758, split_r, subs_extended_register, "Subs Extended Register" );
    ADD_INSTRUCTION( 0xF1, split_i, subs_immediate, "Subs Immediate" );
    ADD_INSTRUCTION( 0x6A2, split_iw, hlt, "HLT" );
    ADD_INSTRUCTION( 0x750, split_r, ands_shifted_register, "Ands Shifted Register" );
    ADD_INSTRUCTION( 0xCA, split_r, eor_shifted_register, "Eor Shifted Register" );
    ADD_INSTRUCTION( 0xAA, split_r, orr_shifted_register, "Orr Shifted Register" );
    ADD_INSTRUCTION( 0x5, split_b, b, "B Target" );
    ADD_INSTRUCTION( 0x6B0, split_r, br_register, "Br Register" );
    ADD_INSTRUCTION( 0x54, split_cb, b_cond, "Branch Conditional Types" ); 
    ADD_INSTRUCTION( 0x34D, split_i, lsl_lsr_immediate, "LSL and LSR Immediate" );
 
 
    ADD_INSTRUCTION( 0x7C0, split_d, stur, "STUR" );
    ADD_INSTRUCTION( 0x1C0, split_d, sturb, "STURB" );
    ADD_INSTRUCTION( 0x3C0, split_d, sturh, "STURH" );
    ADD_INSTRUCTION( 0x7C2, split_d, ldur, "LDUR" );
    ADD_INSTRUCTION( 0x3C2, split_d, ldurh, "LDURH" ); 
    ADD_INSTRUCTION( 0x1C2, split_d, ldurb, "LDURB" ); 
    ADD_INSTRUCTION( 0x694, split_iw, movz, "MOVZ" ); 
 
 
    ADD_INSTRUCTION( 0x458, split_r, add_extended_register, "Add Extended Register" );
    ADD_INSTRUCTION( 0x91,  split_i, add_immediate, "Add Immediate" );
    ADD_INSTRUCTION( 0x4D8, split_r, mul_register, "MUL" );
    ADD_INSTRUCTION( 0xB4, split_cb, cbz, "CBZ" );
    ADD_INSTRUCTION( 0xB5, split_cb, cbnz, "CBNZ" );
}

uint64_t adjust_sign( uint64_t n, size_t bits ) {
    uint64_t m = 1U << ( bits - 1 );
    return ( n ^ m ) - m;
}

void process_instruction() {
    uint32_t raw_value = 0;
    int64_t offset = 0;
    if ( instruction_table == NULL ) { init_instruction_table(); }
    uint32_t instruction = mem_read_32( CURRENT_STATE.PC );
 
    instruction_info_t *info = NULL;
    partition_t splitted;
    bool err = false;


    uint32_t opcode_b = ( instruction >> 26 ) & 0x3F;
    char* key_b = uint32_to_string( opcode_b );
    info = dictionary_get( instruction_table, key_b, &err );
    free( key_b );
    if ( !err && info ) goto dispatch;

 
    uint32_t opcode_cb = ( instruction >> 24 ) & 0xFF;
    char* key_cb = uint32_to_string( opcode_cb );
    info = dictionary_get( instruction_table, key_cb, &err );
    free( key_cb );
    if ( !err && info ) goto dispatch;


    uint32_t opcode_i = ( instruction >> 22 ) & 0x3FF;
    char* key_i = uint32_to_string( opcode_i );
    info = dictionary_get( instruction_table, key_i, &err );
    free( key_i );
    if ( !err && info ) goto dispatch;


    uint32_t opcode_r_d_iw = ( instruction >> 21 ) & 0x7FF;
    char* key_r_d_iw = uint32_to_string( opcode_r_d_iw );
    info = dictionary_get( instruction_table, key_r_d_iw, &err );
    free( key_r_d_iw );
    if ( !err && info ) goto dispatch;

 
    dispatch:
    if ( !info || !info->decode || !info->execute ) {
        if ( !BRANCH_OCCURRED ) {
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            return;
        } else { BRANCH_OCCURRED = FALSE; }
        exit( EXIT_FAILURE );
    }
    info->decode( &splitted, instruction ); 
    info->execute( &splitted );
 
    if ( !BRANCH_OCCURRED ) { NEXT_STATE.PC = CURRENT_STATE.PC + 4; }
    BRANCH_OCCURRED = FALSE;  
    return;
}