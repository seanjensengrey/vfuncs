//
//  attotest.c - attolisp test code
//
//
#define _GNU_SOURCE 
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <math.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "sc_rng.h"
#include "variant.h"
#include "atto.h"

// hand crafted bytecode test runs

int bc_hypot[]=
{
    BC_PUSHENV, 0, 
    BC_PUSHENV, 1,            
    BC_CALLNAT, 2, NAT_NORM2,
    BC_POPENV,  2,          
    BC_STOP,
};

int bc_norm2[]=
{
    BC_PUSHENV, 0,
    BC_CALLNAT, 1, NAT_SQR,
    BC_PUSHENV, 1,
    BC_CALLNAT, 1, NAT_SQR,
    BC_CALLNAT, 2, NAT_ADD, 
    BC_CALLNAT, 1, NAT_SQRT,
    BC_POPENV,  2,
    BC_STOP,
};

int bc_test[]=
{
    BC_PUSHENV, 0,
    BC_CALLNAT, 1, NAT_SQR,
    BC_POPENV,  2,
    BC_STOP,
};

int bc_norm2subcall[]= 
{
    BC_PUSHENV,     0, 
    BC_PUSHENV,     1,            
    BC_CALL,        2, SUB_NORM,        //lookup the sub in the symbol table [pos SUB_NORM]
    BC_POPENV,      2,          
    BC_STOP,
                                        //eval(" (sqrt (sum (sqr a) (sqr b)) )");
///
    BC_PUSHFRAME,   0,                  //get a and sqr it
    BC_CALLNAT,     1, NAT_SQR,
    BC_PUSHFRAME,   1,                  //get b and sqr it
    BC_CALLNAT,     1, NAT_SQR,
    BC_CALLNAT,     2, NAT_ADD, 
    BC_CALLNAT,     1, NAT_SQRT,
    BC_POPFRAME,    0,                  //put result of combined computations into the result slot on the frame
    BC_RET,
    BC_STOP,
};

int main(void)
{
    PA* pa;

/*
    printf("bytecode machine tests >>>\n");

    TT env[4];
    pa=pa_init("");     //dummy setup for sym table

    memset(env, 0, sizeof(env));
    env[0].t=td;
    env[1].t=td;
    env[0].d=3.0;
    env[1].d=4.0;

    bc_run(bc_hypot, sizeof(bc_hypot),  env, sizeof(env), pa->syms, pa->nsym);
    printf("ENV -\n");
    trace_ts(&env[0], &env[3]);
    printf("\n");

    memset(env, 0, sizeof(env));
    env[0].t=td;
    env[1].t=td;
    env[0].d=3.0;
    env[1].d=4.0;
    bc_run(bc_norm2, sizeof(bc_norm2),  env, sizeof(env), pa->syms, pa->nsym);     
    printf("ENV -\n");
    trace_ts(&env[0], &env[3]);
    printf("\n");
    
    memset(env, 0, sizeof(env));
    env[0].t=td;
    env[1].t=td;
    env[0].d=3.0;
    env[1].d=4.0;
    bc_run(bc_norm2subcall, sizeof(bc_norm2subcall),  env, sizeof(env), pa->syms, pa->nsym);     
    printf("ENV -\n");
    trace_ts(&env[0], &env[3]);
    printf("\n");

    printf("tokenizer / lexer tests >>>\n");

    pa_tokenize(" ( sqrt (sum (sqr a) (sqr b)) )  ");    
    pa_tokenize(" ( sqrt (sum (sqr 3.1415926) (sqr .3e-5)) )  ");    
    pa_tokenize(" 0 1 1234567890 -1 -10 -10.345 +10.345 -1e-30 +1e+20 0.00002 442242422442.4 .3e-5 ");    

    pa_free(pa);
    printf("\n");

    printf("parser tests >>>\n");


    pa=pa_init(" ( sqrt (add (sqr a) (sqr b)) )  ");
    pa_parse(pa);
    pa_free(pa);

    pa=pa_init(" ( sqrt (add (sqr 3.0) (sqr (mul 2.0 2.0))) )  ");
    pa_parse(pa);
    pa_free(pa);
*/

    char* tests[]=
    {
        "(sqr 3.1415926)",
        "(sqrt (add (sqr 3.0) (sqr 4.0)) )  ",
        "(sqrt (add (sqr 3.0) (sqr (mul 2.0 2.0))) )  ",
//TODO
//      "(defun normL2 (a b) (sqrt (add (sqr a) (sqr b))) )  ",             // defun to enter new functions we can CALL
//      "(defun triple (a)   (let (b 3) (mul a b)))",                       // let closure
//      "(defun op-by (fnx a b) (fnx a b))",                                // pass fn as argument and invoke variable as function
    };
    int i;
    for (i=0;i<_count_of(tests);i++)
    {
        pa=pa_init(tests[i]);

        pa_parse(pa);

        pa_run(pa);

        pa_free(pa);
    }

    return 0;
}
