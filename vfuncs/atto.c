//
//  atto.c - attolisp bytecode machine and lexer+parser
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

const char* token_names[] =
{
    "none",
    "'('",
    "')'",
    "constant",
    "identifier",
};

const char* sym_types[]=
{
    "none",
    "cfunc",
    "func",
    "var", 
    "env",
};


// built in definitions for native C funcs that arent in math.h

typedef double (*fd0) (void);
typedef double (*fd1) (double);
typedef double (*fd2) (double, double);

double add(double a, double b) { return a+b; }
double mul(double a, double b) { return a*b; }
double sub(double a, double b) { return a-b; }
double rdiv(double a, double b) { return a/b; }
double sqr(double a, double b) { return a*a; }
//
float addf(float a, float b) { return a+b; }
float mulf(float a, float b) { return a*b; }
float subf(float a, float b) { return a-b; }
float rdivf(float a, float b) { return a/b; }
float sqrf(float a, float b) { return a*a; }
//
long double addl(long double a, long double b) { return a+b; }
long double mull(long double a, long double b) { return a*b; }
long double subl(long double a, long double b) { return a-b; }
long double rdivl(long double a, long double b) { return a/b; }
long double sqrl(long double a, long double b) { return a*a; }


sym presyms[]=                                                              //pre-defined symbols [load these into syms]
{
// native calls
    {SYM_FNAT,  "add",         2,  1,   0,   NAT_ADD},
    {SYM_FNAT,  "mul",         2,  1,   0,   NAT_MUL},
    {SYM_FNAT,  "sub",         2,  1,   0,   NAT_SUB},
    {SYM_FNAT,  "div",         2,  1,   0,   NAT_DIV},
    {SYM_FNAT,  "sqr",         1,  1,   0,   NAT_SQR},

    {SYM_FNAT,  "sqrt",        1,  1,   0,   NAT_SQRT},
    {SYM_FNAT,  "sin",         1,  1,   0,   NAT_SIN},
    {SYM_FNAT,  "norm2",       2,  1,   0,   NAT_NORM2},      

//bytecode subroutines
    {SYM_FUNC,  "subnorm",     2,  1,   10,  0},      
};

typedef struct _inbuiltinfo
{
    int     nid;        //check
    int     narg;
    int     nret;
    void*   fn_f;
    void*   fn_d;
    void*   fn_ld;
} inbuiltinfo;


inbuiltinfo inbuilts[] =
{
    {NAT_ADD,   2,1,    addf,   add,    addl},      //   +
    {NAT_MUL,   2,1,    mulf,   mul,    mull},      //   *
    {NAT_SUB,   2,1,    subf,   sub,    subl},      //   -
    {NAT_DIV,   2,1,    rdivf,  rdiv,   rdivl},     //   /
    {NAT_SQR,   1,1,    sqrf,   sqr,    sqrl},      //  a*a

    {NAT_SQRT,  1,1,    sqrtf,  sqrt,   sqrtl},
    {NAT_SIN,   1,1,    sinf,   sin,    sinl},
    {NAT_NORM2, 2,1,    hypotf, hypot,  hypotl},
};

// stack frame helpers

T       st_frame(T sp, int nargs, int nip, int nfp);    //set up stack frame, return new stack pointer
T       st_arg(T sp, int nth);                          // points to nth arg on the stack frame
T       st_res(T sp);                                   // points to result slot on the stack frame
int     st_oip(T sp);                                   // points to old ip on the frame - thats where RET will jump back to on return from a CALL
int     st_ofp(T fp);


// stack frame ///////////
//
//  push a
//  push b
//  call 2 nfunc        // adds | res nargs=2 opi=ip xx |   to stack before jumping to ip=nfunc
//
//  // st : a b | res nargs oip xx |   we keep res as a temp slot, then copy back to arg[0] so only 1 result left after return
//
//  after return 
// 



// stack frame - just after a CALL
//
//  arg 0
//  ...
//  arg n-1
//  res   slot  - used by CALLNAT not used by call
//  nargs slot  - as a check
//  oip   slot  - old ip to restore to on ret
//  ofp   slot  - old frame pointer to restore to on ret
//  ...
//  ... local stack storage for this call and its sub calls goes here ...
//  ...
//
//  stack frame after RET
//
//  arg 0 <--- replaced by the return value of the CALL   //assumes all funcs have 1 return value




// stack frame helpers

T st_frame(T sp, int nargs, int nip, int nfp)       //set up stack frame, return new stack pointer
{
    T p=sp+1; 
    p->t=ti32;  
    p->I32=nargs;   //store nargs
    
    p++;
    p->t=ti32;  
    p->I32=nip;     //store oip

    p++;
    p->t=ti32;  
    p->I32=nfp;     //store ofp

    return sp+4;
}


T st_arg(T sp, int nth)         // points to nth arg on the stack frame
{
    int nargs=sp[-3].I32;
    assert(nth<nargs);
    return sp+nth-4-nargs;
}

T st_res(T sp)                  // points to result slot on the stack frame
{
    return sp-4;
}

int st_oip(T sp)                  // points to old ip on the frame - thats where RET will jump back to on return from a CALL
{
    T t=sp-2;
    return t->I32;
}
int st_ofp(T fp)
{
    T t=fp-1;
    return t->I32;
}

///

void trace_ts(T t, T te)
{
    for(;t<te;t++)
    {
        switch (t->t)
        {
            case ti32:  printf("TT ti32 : %d\n", t->I32); break;
            case td:    printf("TT td   : %2.3f\n", t->d); break;
            default:    printf("TT      :\n"); break;
        }
    }
    //printf("\n");
}

int bc_run(int* bytes, int nbytes, T env, int nenv, sym* syms, int nsyms)         
{
    //run the bytecode machine over the bytes passed in, using environment

    TT st[1024];                            // stack
    memset(st, 0, sizeof(st));
    T sp=st;                                //stack pointer
    T fp=sp;                                //frame pointer [the sp on entry to the current call]

    int* ip=bytes;
    int* ipe=bytes+nbytes;
    while(ip<ipe)
    {
        switch (*ip++)
        {
            case  BC_PUSHFRAME :  
                {
                    printf("BC_PUSHFRAME\n");
                    int n=*ip++;            // arg is which frame slot
                    *sp=*st_arg(fp, n);     // push frame[n] onto stack
                    assert(sp->t==st_arg(fp, n)->t);
                    assert(sp->t==td);
                    sp++;
                }
                break;
            case BC_POPFRAME :  
                {
                    printf("BC_POPFRAME\n");
                    int n=*ip++;            // arg is which frame slot
                    sp--;
                    *st_arg(fp, n)=*sp;     // pop stack into frame[n] 
                }
                break;
            case BC_PUSHENV :  
                {
                    printf("BC_PUSHENV\n");
                    int ne=*ip++;           // arg is which env slot
                    *sp=env[ne];          // push env onto stack
                    assert(sp->t==env[ne].t);
                    assert(sp->t==td);
                    sp++;
                }
                break;
            case BC_POPENV :  
                {
                    printf("BC_POPENV\n");
                    int ne=*ip++;           // arg is which env slot
                    env[ne]=*--sp;          // pop stack into env 
                }
                break;
            case BC_CALL :  
                {
                    int narg=*ip++;
                    int nfunc=*ip++;             
                    assert(narg==syms[nfunc].nargs);   
                    assert(SYM_FUNC==syms[nfunc].symtype);

                    printf("BC_CALL  : %s [curr ip=%d] \n", syms[nfunc].szname, ip-bytes);

                    sp=st_frame(sp, narg, ip-bytes, fp-st);             //set up stack frame, store current ip [for later RET]

                    fp=sp;                                              //set new frame pointer
                    ip=bytes+syms[nfunc].nbc;                          //set new ip - jump to SUB at absolute offset nbc
                }
                break;
            case BC_RET : 
                {
                    int cip=ip-bytes;

                    int noip=st_oip(fp);                                //lookup stored previous oip on frame
                    ip=bytes+noip;                                      //jump back there
                    int nofp=st_ofp(fp);
                    
                    printf("BC_RET   : curr ip=%d returning to %d\n", cip, noip);

                    sp=st_arg(fp,0);
                    sp++;                                               //leave return value on stack

                    fp=st+nofp;                                         //point to the previous stack frame [of the caller]
                }
                break;
            case BC_CALLNAT :  
                {
                    int narg=*ip++;
                    int nfunc=*ip++;             
                    printf("BC_CALLNAT %s\n", syms[nfunc].szname);
                    assert(SYM_FNAT==syms[nfunc].symtype);
        
                    sp=st_frame(sp, narg, ip-bytes, fp-st);
 
                    inbuiltinfo info=inbuilts[nfunc]; 
                    assert(nfunc==info.nid);
                    assert(info.narg==narg);            //should be called with same no of params we expect

                    //TODO - only supports double args ... 

                    double res;
                    if (info.narg==2)
                    {
                        T pd1=st_arg(sp, 0);
                        T pd2=st_arg(sp, 1);
                        assert(pd1->t==td);
                        assert(pd2->t==td);

                        fd2 ff=(fd2)info.fn_d;

                        res=ff(pd1->d, pd2->d);
                    }
                    else if (info.narg==1)
                    {
                        T pd1=st_arg(sp, 0);
                        assert(pd1->t==td);

                        fd1 ff=(fd1)info.fn_d;
                        res=ff(pd1->d);
                    }
                    else if (info.narg==0)
                    {
                        fd0 ff=(fd0)info.fn_d;
                        res=ff();
                    }
                    T pres=st_res(sp);
                    pres->d=res;
                    pres->t=td;

                    // have to do an implicit ret ...
                    // ip is ok, put res at end of sp, just beyond where it was on entry

                    sp=st_arg(sp,0);
                    *sp++=*pres;                //copy it into the return slot
                }
                break;
            case BC_STOP :  
                printf("BC_STOP\n");
                assert(sp==st);    
                return 0;
            default : 
                printf("ERROR: unknown bytecode\n");
                return -1;
        }
        //trace_ts(st, sp);
    } 
    return 0;
}

// pa_eval(" (sqrt (sum (sqr a) (sqr b)) )");
//
//  attolisp grammar -
//
//    lexer patterns -
//      lettr: [a..z]
//      alpha: {lettr}+
//      dig  : [0..9]
//      digs : {dig}+
//
//    lexer tokens -
//      left : (
//      right: )
//      const: {[+-]}digs{.digs{{[+-]}digs}}         // 123 0.123  +0.34e-34 
//      ident: alpha{alpha|_|-|dig}*          //inbuilt and defun'd functions in the symbol table
//
//    nonterminals -
//      
//      expr : left func args right 
//      args : arg args | empty
//      arg  : ident | const | expr
//      func : ident 
//
//  expression evaluator [just nested known funcs... 
//      TODO    extend -  (defun funcname ( a b c ) ( ... ))
//      TODO    extend let - curry/replace args from environment via let func


inline char* pa_skipws(char* s)
{
    while(isspace(*s))
        s++;
    return s;
}

void trace_tok(token* t)
{
    assert(t);
    switch (t->ntype)
    {
        case TOK_LEFT   : printf("TOK_LEFT\n");  break;
        case TOK_RIGHT  : printf("TOK_RIGHT\n"); break;
        case TOK_CONST  : printf("TOK_CONST : %3.6e\n", t->d); break;
        case TOK_IDENT  : printf("TOK_IDENT : [%s]\n", t->szname); break;
        default:         printf("TOK unknown\n");
    }
}

inline char* pa_token_const(char* s, token* t)
{
    t->ntype=TOK_NONE;
    t->szname=0;

//  const: {[+-]}digs{.digs} {{[+-]}digs}       // 123 0.123  +0.34e-34 

    char* sb=s;
    char* se=s; 
    errno=0;
    double val=strtod(sb, &se); 
    if (errno!=0 || sb==se)
    {
        printf("ERROR lexer reading const\n");
        exit(0);
    }
   
    t->ntype=TOK_CONST; 
    t->d=val;
    return se;
}

inline int pa_isidchar(char c)
{
    return (isalnum(c) || c=='_' || c=='-');
}

inline char* pa_token_ident(char* s, token* t)
{
//  ident: alpha{alpha|_|-|dig}*          //inbuilt and defun'd functions in the symbol table

    t->ntype=TOK_NONE;
    t->szname=0;

    assert(isalpha(*s));

    char* se=s;
    se++;
    while(pa_isidchar(*se))
        se++;
    
    t->ntype=TOK_IDENT; 
    t->szname=strndup(s, se-s);
    return se;
}

char* pa_token(char* s, token* t)
{
    //printf("pa_token : [%s]\n", s);
    assert(s);
    assert(t);

    t->ntype=TOK_NONE;
    t->szname=0;

    s=pa_skipws(s);    
    if (*s=='(')
    {
        t->ntype=TOK_LEFT;
        t->szname=0;
        return ++s; 
    }
    if (*s==')')
    {
        t->ntype=TOK_RIGHT;
        t->szname=0;
        return ++s; 
    }
    if (isalpha(*s))
        return pa_token_ident(s,t);

    if (!*s)
        return s;

    return pa_token_const(s,t);
}

void pa_freetoks(token* t, int n)
{
    int i;
    for (i=0;i<n;i++)
        free(t[i].szname);
}


void pa_tokenize(char* sz)          //TEST
{
    token toks[512];
    memset(toks, 0, sizeof(toks));
    char* sze=sz+strlen(sz);
    int i=0;
    while (sz<sze) 
    {
        sz=pa_token(sz, &toks[i]);
        if (sz<sze)
            trace_tok(&toks[i]);
        i++;
    }

    pa_freetoks(toks, 512);

    printf("\n\n");
}


//

PA* pa_init(char* sz)
{
                                            //TODO - make free blocklists instead of fixed arrays...
    const int nts=512;
    const int nsyms=512;
    const int nenvs=256;
    const int nbcs=128;

    printf("pa_init : [%s]\n", sz);

    assert(sz);
    PA* pa=calloc(1,sizeof(PA));
    if(!pa)
        goto bad;
    pa->sb=pa->s=pa->sprev=sz;
  
    token* ts=calloc(nts, sizeof(token)); 
    if(!ts)
        goto bad;
    pa->toks=ts;
    pa->nt=0;
    pa->ntmax=nts;

    int ns=nsyms+_count_of(presyms);                  //always fit in the predefined symbols
    sym* y=calloc(ns, sizeof(sym));
    if(!y)
        goto bad;
    pa->syms=y;
    memcpy(y,presyms, sizeof(presyms));        //copy in predefined symbols [native C funcs etc]
    pa->nsym=_count_of(presyms);
    pa->nsymmax=ns; 

    T e=calloc(nenvs, sizeof(TT));
    if(!e)
        goto bad;
    pa->envs=e;
    pa->nenv=0;
    pa->nenvmax=nenvs; 

    int* pbc=calloc(nbcs, sizeof(int));
    if (!pbc)
        goto bad;
    pa->bcs=pbc;
    pa->nbc=0;
    pa->nbcmax=nbcs;

    return pa;

bad:
    if (pa)
    {
        free(pa->toks);
        free(pa->syms);    
        free(pa->envs);    
        free(pa->bcs);    
        free(pa);
    }
    assert(0);
    return NULL; 
}

void pa_free(PA* pa)
{
    //printf("pa_free\n");
    printf("\n\n");
    assert(pa);

    pa_freetoks(pa->toks, pa->nt);
    free(pa->toks);
    free(pa->syms);    
    free(pa->envs);    
    free(pa->bcs);    

    free(pa);
}

sym* sym_lookup(PA* pa, char* s)
{
    //printf("sym_lookup(%s)\n", s);

    sym* p=pa->syms;
    int i;
    for(i=0;i<pa->nsym;i++,p++)
    {
        //printf("  compare [%s] to [%s]\n", p->szname, s);
        if (0==strcmp(p->szname, s))
            return p;
    }
    return NULL;
}

inline int pa_symindex(PA* pa, sym* p)
{
    return p-pa->syms;
}

inline token* pa_currtok(PA* pa)
{
    if (pa->nt==0)
        return NULL;
    return &pa->toks[pa->nt-1];
    
}
inline int pa_currtype(PA* pa)
{
    if (pa->nt==0)
        return TOK_NONE;
    return pa->toks[pa->nt-1].ntype;
}

int pa_next(PA* pa, int nexpect)
{
    assert(pa->nt<pa->ntmax);
    pa->sprev=pa->s;
    pa->s=pa_token(pa->s, &pa->toks[pa->nt]);
    pa->nt++;

    if (nexpect==TOK_ANY)
        return 0;
    if (nexpect!=pa_currtype(pa))
    {
        printf("PARSE ERROR : expected %s got %s\n", token_names[nexpect], token_names[pa_currtype(pa)]);
        return -1;
    }
    return 0;
}

void pa_poptoken(PA* pa)
{
    if (pa->nt==0)
        return;

    free(pa_currtok(pa)->szname);
    memset(pa_currtok(pa), 0, sizeof(token));

    pa->s=pa->sprev;
    pa->nt--;
}

int pa_env_set(PA* pa, double val)
{
    assert(pa->nenv<pa->nenvmax);
    T t=&pa->envs[pa->nenv];
    t->t=td;
    t->d=val;

    return pa->nenv++;
}

void pa_emit(PA* pa, int bc)
{
    //append to BC bytes in pa

    assert(pa->nbc<pa->nbcmax);

    pa->bcs[pa->nbc++]=bc;
}

void pa_emit_pushenv(PA* pa)
{
    int nenv=pa_env_set(pa, pa_currtok(pa)->d);

    pa_emit(pa, BC_PUSHENV);
    pa_emit(pa, nenv);

    printf("BC_PUSHENV,    %d,           // %2.3e\n", nenv, pa_currtok(pa)->d);
}

void pa_emit_function(PA* pa, sym* y, char* fn)
{
    if (y->symtype==SYM_FNAT)
    {
        pa_emit(pa, BC_CALLNAT);
        printf("BC_CALLNAT,    %d,    %d,     // %s\n", y->nargs, pa_symindex(pa, y), fn);
    }
    else 
    {
        assert(y->symtype==SYM_FUNC);
        pa_emit(pa, BC_CALL);
        printf("BC_CALL,       %d,    %d,     // %s\n", y->nargs, pa_symindex(pa, y), fn);
    }
    pa_emit(pa, y->nargs);
    pa_emit(pa, pa_symindex(pa, y));
}

void pa_emit_popenv(PA* pa, int nenv)
{
    // emit bc to pop the stack, and put that into the env at slot nenv

    printf("BC_POPENV %d\n",nenv);
    pa_emit(pa, BC_POPENV);
    pa_emit(pa, nenv);
}


int pa_expr(PA* pa)
{
    //printf("pa_expr >> [%s]\n",pa->s);

    if (pa_next(pa, TOK_LEFT))
        goto bad;

    if (pa_next(pa, TOK_IDENT))
        goto bad;
    char* fn=pa_currtok(pa)->szname;
    sym* y=sym_lookup(pa, fn);
    if (!y)
    {
        printf("PARSE ERROR : unknown function [%s]\n", fn);
        goto bad;
    }
    //printf("func [%s] a %s\n", fn, sym_types[y->symtype]); 

    if (pa_next(pa, TOK_ANY))
        goto bad;
    int narg=0;
    while(pa_currtype(pa)!=TOK_RIGHT)
    {
        // we can hit ident | const | expr 
        // or opening ( for a sub expr
        // or ) to close current expr

        switch (pa_currtype(pa))
        {
            case TOK_CONST : 
                    pa_emit_pushenv(pa);
                    break;

            case TOK_IDENT : 
                    //printf("arg %d : variable [%s]\n", narg, pa_currtok(pa)->szname); 
                    printf("BC_PUSHENV ident\n");
                    break;

            case TOK_LEFT : 
                    pa_poptoken(pa);                            //undo the parse of left, so expr can chew it
                    if(pa_expr(pa))
                        goto bad;
                    break;

            default : 
                    printf("PARSE ERROR: bad arg in expression : arg %d\n", narg);
                    goto bad;
        }

        narg++;
        if (pa_next(pa, TOK_ANY))
            goto bad;
    }

    //weve swallowed the right ) exiting the loop

    if (y->nargs!=narg)
    {
        printf("PARSE ERROR: function %s expects %d args [got %d args]\n", fn, y->nargs, narg);
        goto bad;
    }
    pa_emit_function(pa, y, fn);

    return 0;

bad:
    printf("pa_expr: PARSE ERROR\n");
    return -1;
}

int pa_parse(PA* pa)
{
    int res=pa_expr(pa);
    
    int nenv=pa_env_set(pa, 0);         //set a place to hold the result at runtime, at the end of env
    pa_emit_popenv(pa, nenv);

    printf("ENV :\n");
    trace_ts(pa->envs, pa->envs+pa->nenv);        
    printf("\n");

    return res;
}

void pa_run(PA* pa)
{
    printf("pa_run\n");

    bc_run(pa->bcs, pa->nbc, pa->envs, pa->nenv, pa->syms, pa->nsym);     
    printf("ENV -\n");
    trace_ts(pa->envs, pa->envs+pa->nenv);
    printf("\n");
}

///
