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
    ADD_INSTRUCTION( 0x5, split_r, adds_extended_register, "Adds Extended Register" );
}

void adds_extended_register( partition_t *instruction_data ) {
    uint64_t x1 = CURRENT_STATE.REGS[instruction_data->rn];
    uint64_t num = CURRENT_STATE.REGS[instruction_data->rm];
    uint64_t x0 = x1 + num;

    // Actualización de las banderas de estado
    NEXT_STATE.FLAG_N = x0 >> 63;  // Extrae el bit más significativo
    NEXT_STATE.FLAG_Z = ( x0 == 0 ) ? 1 : 0;

    // Actualiza el registro destino con el resultado
    NEXT_STATE.REGS[instruction_data->rd] = x0;
}

void process_instruction() {
    if ( instruction_table == NULL ) {
        init_instruction_table();
    }

    // Leemos la instrucción a realizar.
    uint32_t instruction = mem_read_32( CURRENT_STATE.PC );

    // Inicializo un puntero que va a almacenar lo que me devuelva el hash
    instruction_info_t *info = NULL;

    // Inicializo un puntero que va a almacenar el output de split, por ende el input de execute
    partition_t splitted; //esto seria partitions_t

    // Inicializo una variable booleana para saber si el diccionario encontró o no la clave.
    bool err = false;
    
    // Tipo B (bits 31:26, 6 bits), el primer tipo de instrucción que probamos
    uint32_t opcode_b = ( instruction >> 26 ) & 0x3F;
    char* key_b = uint32_to_string( opcode_b );
    info = dictionary_get( instruction_table, key_b, &err );
    free( key_b );
    if ( !err && info ) goto dispatch;

    // Tipo CB (bits 31:24, 8 bits)
    uint32_t opcode_cb = ( instruction >> 24 ) & 0xFF;
    char* key_cb = uint32_to_string( opcode_cb );
    info = dictionary_get( instruction_table, key_cb, &err );
    free( key_cb );
    if ( !err && info ) goto dispatch;

    // Tipo I (bits 31:22, 10 bits)
    uint32_t opcode_i = ( instruction >> 22 ) & 0x3FF;
    char* key_i = uint32_to_string( opcode_i );
    info = dictionary_get( instruction_table, key_i, &err );
    free( key_i );
    if ( !err && info ) goto dispatch;

    // Tipo R / D / IW (bits 31:21, 11 bits)
    uint32_t opcode_r_d_iw = ( instruction >> 21 ) & 0x7FF;
    char* key_r_d_iw = uint32_to_string( opcode_r_d_iw );
    info = dictionary_get( instruction_table, key_r_d_iw, &err );
    free( key_r_d_iw );
    if ( !err && info ) goto dispatch;

    // Handle que no reconoció la instrucción 
    //printf("Instrucción no reconocida: 0x%08x", instruction);
    //        SIM_RUNNING = false; // no se que sería eso, yo creería que aca llamo a halt (hlt)
    //        return;

    // Ejecutar con la estructura decodificada
    dispatch:
    info->decode( &splitted, instruction );   // split
    info->execute( &splitted );              // ejecución

    if ( !BRANCH_OCCURRED ) {
        NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    }
    BRANCH_OCCURRED = false;    
    return;
}