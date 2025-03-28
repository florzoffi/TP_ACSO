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
int FLAG = FALSE;


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
    ADD_INSTRUCTION( 0x558, split_r, adds_extended_register, "Adds Extended Register" );
    ADD_INSTRUCTION( 0xB1, split_i, adds_immediate, "Adds Immediate" );

    ADD_INSTRUCTION( 0x758, split_r, subs_extended_register, "Subs Extended Register" );
    ADD_INSTRUCTION( 0xF1, split_i, subs_immediate, "Subs Immediate" );

    ADD_INSTRUCTION( 0x6a2, split_iw, hlt, "HLT" ); // NO SE SI LA CORRI O NO

    ADD_INSTRUCTION( 0x750, split_r, ands_shifted_register, "Ands Shifted Register" );

    ADD_INSTRUCTION( 0xCA, split_r, eor_shifted_register, "Eor Shifted Register" );

    ADD_INSTRUCTION( 0xAA, split_r, orr_shifted_register, "Orr Shifted Register" ); // No la testee todavía

    ADD_INSTRUCTION( 0x54, split_cb, b_cond, "Branch Conditional Types" ); // No la testee todavia

   // ADD_INSTRUCTION( 0x34D, split_i, lsl_lsr_immediate, "LSL and LSR Immediate" ); // No la testee todavia

}

int32_t adjust_sign(uint32_t value, int bits) {
    int32_t sign_bit = 1 << (bits - 1);
    return (value ^ sign_bit) - sign_bit;
}

// ----------------------------------- Funciones de Instrucciones ---------------------------------------------------------

void adds_extended_register( partition_t *split_data ) {
    uint64_t operation = CURRENT_STATE.REGS[split_data->rn] + CURRENT_STATE.REGS[split_data->rm];
    NEXT_STATE.FLAG_N = operation >> 63;
    NEXT_STATE.FLAG_Z = operation == 0;
    NEXT_STATE.REGS[split_data->rd] = operation;
}

void adds_immediate(partition_t *split_data) {
    uint64_t imm = split_data->alu;
    if (split_data->shamt == 0x1) {
        imm <<= 12;
    }
    uint64_t operation = CURRENT_STATE.REGS[split_data->rn] + imm;
    NEXT_STATE.FLAG_Z = (operation == 0);
    NEXT_STATE.FLAG_N = (operation < 0);

    NEXT_STATE.REGS[split_data->rd] = operation;
}

void subs_extended_register(partition_t *split_data) {
    uint64_t operation = CURRENT_STATE.REGS[split_data->rn] - CURRENT_STATE.REGS[split_data->rm];
    NEXT_STATE.FLAG_N = (operation >> 63) & 1;
    NEXT_STATE.FLAG_Z = (operation == 0);
    NEXT_STATE.REGS[split_data->rd] = operation;
    if (split_data->rd != 31) {
        NEXT_STATE.REGS[split_data->rd] = operation;
    }
}

void subs_immediate(partition_t *split_data) {
    uint64_t imm = split_data->alu;
    if (split_data->shamt == 0x1) {
        imm <<= 12;
    }
    uint64_t operation = CURRENT_STATE.REGS[split_data->rn] - imm;
    NEXT_STATE.FLAG_N = (operation >> 63) & 1;
    NEXT_STATE.FLAG_Z = (operation == 0);
    NEXT_STATE.REGS[split_data->rd] = operation;
    if (split_data->rd != 31) {
        NEXT_STATE.REGS[split_data->rd] = operation;
    }
}

void hlt(partition_t *split_data) {
    RUN_BIT = 0;
}

void ands_shifted_register(partition_t *split_data) {
    uint64_t operation = CURRENT_STATE.REGS[split_data->rn] & CURRENT_STATE.REGS[split_data->rm];
    NEXT_STATE.FLAG_N = operation >> 63;
    NEXT_STATE.FLAG_Z = operation == 0;
}

void eor_shifted_register(partition_t *split_data) {
    uint64_t operation = CURRENT_STATE.REGS[split_data->rn] ^ CURRENT_STATE.REGS[split_data->rm];
    NEXT_STATE.REGS[split_data->rd] = operation;
}

void orr_shifted_register(partition_t *split_data) {
    uint64_t operation = CURRENT_STATE.REGS[split_data->rn] | CURRENT_STATE.REGS[split_data->rm];
    NEXT_STATE.REGS[split_data->rd] = operation;
}

void b(partition_t *split_data) {
    NEXT_STATE.PC = CURRENT_STATE.PC + adjust_sign(split_data->br << 2, 28);
    FLAG = TRUE;
}

void br_register(partition_t *split_data) {
    CURRENT_STATE.PC = CURRENT_STATE.REGS[split_data->rn];
}

void b_cond(partition_t *split_data) {
    int64_t offset = adjust_sign(split_data->cond_br << 2, 21);
    bool branch_allowed = false;

    // Asumiendo que split_data->rt es el código de condición
    switch (split_data->rt) {  
        case 0:  // BEQ
            branch_allowed = (CURRENT_STATE.FLAG_Z == 1);
            break;
        case 1:  // BNE
            branch_allowed = (CURRENT_STATE.FLAG_Z == 0);
            break;
        case 11: // BLT
            branch_allowed = (CURRENT_STATE.FLAG_N == 1);
            break;
        case 12: // BGT
            branch_allowed = (!CURRENT_STATE.FLAG_N && CURRENT_STATE.FLAG_Z == 0);
            break;
        case 10: // BGE
            branch_allowed = (CURRENT_STATE.FLAG_N == 0);
            break;
        case 13: // BLE
            branch_allowed = (CURRENT_STATE.FLAG_N == 1 || CURRENT_STATE.FLAG_Z == 1);
            break;
        default:
            break;
    }

    if (branch_allowed) {
        NEXT_STATE.PC += offset;  // Actualizar PC con el offset correctamente calculado
    }
}


/*
void lsl_lsr_immediate(partition_t *split_data) {
    uint64_t immr = split_data->alu >> 6;
    uint64_t imms = split_data->alu & 0x3f;
    if(imms != 0x3F){
        NEXT_STATE.REGS[split_data->rd] = NEXT_STATE.REGS[split_data->rn] << ( 64 - immr );
    }
    else{
        NEXT_STATE.REGS[split_data->rd] = NEXT_STATE.REGS[split_data->rn] >> immr;
    }
}

*/


// --------------------------------------------------------------------------------------------

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