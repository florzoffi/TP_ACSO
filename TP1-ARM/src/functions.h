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
#include "sim.c"

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