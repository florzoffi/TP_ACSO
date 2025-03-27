#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

#include "shell.h"
#include "hopscotch.h"
#include "decoder.h"

int BRANCH_OCCURRED = FALSE;

#define XZR 0x1f

static dictionary_t *instruction_table = NULL;

char* uint32_to_string( uint32_t number ) {
    char* key = malloc( 11 );
    if ( key == NULL ) {
        fprintf( stderr, "Failed to allocate memory for key\n" );
        exit( 1 );
    }
    sprintf( key, "%u", number );
    return key;
}

void ADD_INSTRUCTION( uint32_t opcode, void (*decode_fn)(partition_t*, uint32_t), void (*execute_fn)(partition_t*), const char* name_str ){ 
    instruction_info_t *info = malloc( sizeof( instruction_info_t ) ); 
    if ( !info ) {
        fprintf( stderr, "Failed to allocate memory for instruction_info\n" );
        exit( 1 );
    }
    info->decode = decode_fn;
    info->execute = execute_fn;
    info->name = strdup( name_str );
    if ( !info->name ) {
        free( info );
        fprintf( stderr, "Failed to duplicate name string\n" );
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
        fprintf( stderr, "Error al crear el diccionario de instrucciones\n" );
        exit( 1 );
    }
    ADD_INSTRUCTION( 0x22C, split_r, adds_extended_register, "Adds Extended Register" );
    ADD_INSTRUCTION( 0xB1, split_i, adds_immediate, "Adds Immediate" );

    //ADD_INSTRUCTION( 0xB1, split_i, subs_extended_register, "Subs Extended Register" );
    // ADD_INSTRUCTION( 0xB1, split_i, subs_immediate, "Subs Immediate" );
    //ADD_INSTRUCTION( 0x6a2, split_i, hlt, "HLT" );

    // ADD_INSTRUCTION( 0xB1, split_i, adds_immediate, "Adds Immediate" );
    // ADD_INSTRUCTION( 0xB1, split_i, adds_immediate, "Adds Immediate" );
    // ADD_INSTRUCTION( 0xB1, split_i, adds_immediate, "Adds Immediate" );
    // ADD_INSTRUCTION( 0xB1, split_i, adds_immediate, "Adds Immediate" );
    // ADD_INSTRUCTION( 0xB1, split_i, adds_immediate, "Adds Immediate" );
    // ADD_INSTRUCTION( 0xB1, split_i, adds_immediate, "Adds Immediate" );
}

void adds_extended_register( partition_t *split_data ) {
    uint64_t operation = CURRENT_STATE.REGS[split_data->rn] + CURRENT_STATE.REGS[split_data->rm];
    NEXT_STATE.FLAG_N = operation >> 63;
    NEXT_STATE.FLAG_Z = operation == 0;
}

void adds_immediate(partition_t *instruction_data) {
    uint64_t operand1 = CURRENT_STATE.REGS[instruction_data->rn];
    uint64_t imm = instruction_data->alu;
    if (instruction_data->shamt == 0x1) {
        imm <<= 12;
    }
    uint64_t result = operand1 + imm;
    NEXT_STATE.FLAG_N = result >> 63;
    NEXT_STATE.FLAG_Z = result == 0;
    if (instruction_data->rd != XZR) {
        NEXT_STATE.REGS[instruction_data->rd] = result;
    }
}

void subs_extended_register(partition_t *instruction_data) {
}














void lazy_innit_hash() {
    printf( "entro a la funcion process_intstruction" );
    if ( instruction_table == NULL ) {
        printf( "hay que crear tabla" );
        init_instruction_table();
        printf( "se creó la tabla" );
    }
}

void process_instruction() {
    lazy_innit_hash();
    
    uint32_t instruction = mem_read_32( CURRENT_STATE.PC );
    printf( "se leyó la instrucción" );
    printf("\nInstruction: 0x%08x\n", instruction);

    // Inicializo un puntero que va a almacenar lo que me devuelva el hash
    instruction_info_t *info = NULL;

    // Inicializo un puntero que va a almacenar el output de split, por ende el input de execute
    partition_t splitted; //esto seria partitions_t

    // Inicializo una variable booleana para saber si el diccionario encontró o no la clave.
    bool err = false;
    printf( "inicializacion de variables" );

    // Tipo B (bits 31:26, 6 bits), el primer tipo de instrucción que probamos
    uint32_t opcode_b = ( instruction >> 26 ) & 0x3F;
    char* key_b = uint32_to_string( opcode_b );
    info = dictionary_get( instruction_table, key_b, &err );
    free( key_b );
    if ( !err && info ) goto dispatch;
    printf( "opcode 1 done" );

    // Tipo CB (bits 31:24, 8 bits)
    uint32_t opcode_cb = ( instruction >> 24 ) & 0xFF;
    char* key_cb = uint32_to_string( opcode_cb );
    info = dictionary_get( instruction_table, key_cb, &err );
    free( key_cb );
    if ( !err && info ) goto dispatch;
    printf( "opcode 2 done" );

    // Tipo I (bits 31:22, 10 bits)
    uint32_t opcode_i = ( instruction >> 22 ) & 0x3FF;
    char* key_i = uint32_to_string( opcode_i );
    info = dictionary_get( instruction_table, key_i, &err );
    free( key_i );
    if ( !err && info ) goto dispatch;
    printf( "opcode 3 done" );

    // Tipo R / D / IW (bits 31:21, 11 bits)
    uint32_t opcode_r_d_iw = ( instruction >> 21 ) & 0x7FF;
    char* key_r_d_iw = uint32_to_string( opcode_r_d_iw );
    info = dictionary_get( instruction_table, key_r_d_iw, &err );
    free( key_r_d_iw );
    if ( !err && info ) goto dispatch;
    printf( "opcode 4 done" );

    // Handle que no reconoció la instrucción 
    //printf("Instrucción no reconocida: 0x%08x", instruction);
    //        SIM_RUNNING = false; // no se que sería eso, yo creería que aca llamo a halt (hlt)
    //        return;

    // Ejecutar con la estructura decodificada
    dispatch:
    if (!info || !info->decode || !info->execute) {
        printf("ERROR: info o sus punteros están en NULL\n");
        exit(1);
    }
    printf( "empieza el split" );
    info->decode( &splitted, instruction );  
    printf( "empueza el execute" ); 
    info->execute( &splitted );              

    if ( !BRANCH_OCCURRED ) {
        NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    }
    BRANCH_OCCURRED = false;    
    return;
}