//
//  attotest.c - attolisp test code
//
//      usage $> ./attotest "(sqrt (add (sqr 3) (sqr 4)))"
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

int test_list()
{
    char* tests[]=
    {
        "(sqr 3.1415926)",
        "(sqrt (add (sqr 3.0) (sqr 4.0)) )  ",
        "(sqrt (add (sqr 3.0) (sqr (mul 2.0 2.0))) )  ",

//TODO
        //"(defun normL2 (a b) (sqrt (add (sqr a) (sqr b))) )  ",   // defun to enter new functions we can CALL
        //"(defun triple (a)   (let (b 3) (mul a b)))",             // let closure
        //"(defun op-by (fnx a b) (fnx a b))",                      // pass fn as argument and invoke variable as function
    };

    int i;
    for (i=0;i<_count_of(tests);i++)
    {
        PA* pa=pa_init(tests[i]);
        if(0==pa_parse(pa))
        {
            pa_run(pa);
            printf("RESULT : [%3.6g]\n", pa_result(pa));         // show the result, at end of env
        } 
        pa_free(pa);
    }

    return 0;
}


int main(int argc, char* argv[])
{
    if (argc<=1)
        return test_list();                     //no argument, just run some tests

    char* sz=argv[1];                           // eval the expression on the command line

    PA* pa=pa_init(sz);
    int ret=pa_parse(pa);
    if (ret)
    {
        pa_free(pa);
        return ret;
    }

    pa_run(pa);
    printf("RESULT : [%3.6g]\n", pa_result(pa));         // show the result, at end of env

    pa_free(pa);
    return 0;
}

