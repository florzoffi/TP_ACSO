#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "decoder.h"

void adds_extended_register( partition_t* p );
void adds_immediate( partition_t* p );
void subs_extended_register( partition_t* p );
void subs_immediate( partition_t* p );
void hlt( partition_t* p );
void ands_shifted_register( partition_t* p );
void eor_shifted_register( partition_t* p );
void orr_shifted_register( partition_t* p );
void b( partition_t* p );
void br_register( partition_t* p );
void b_cond( partition_t* p );
void lsl_lsr_immediate( partition_t* p );
void stur( partition_t* p );
void sturb( partition_t* p );
void sturh( partition_t* p );
void ldur( partition_t* p );
void ldurb( partition_t* p );
void ldurh( partition_t* p );
void movz( partition_t* p );
void add_immediate( partition_t* p );
void add_extended_register( partition_t* p );
void mul_register( partition_t* p );
void cbz( partition_t* p );
void cbnz( partition_t* p );

#endif