//
//  atto.h - attolisp bytecode machine and lexer+parser
//
//

// symbol table defs

enum 
{   
    SYM_NONE,
    SYM_FNAT,       //native function
    SYM_FUNC,       //function subroutine
    SYM_VAR,        //variable
    SYM_ENV,        //environment variable
};

typedef struct _tsym
{
    int             symtype;                    // type of symbol
    const char*     szname;                     // name

    int             nargs;                      // how many arguments?
    int             nret;                       // returns value or void?

    int             nbc;                        // user defined - absolute bytecode offset [ip to jump to]
    
    int             ninbuilt;                   //which inbuilt - index into inbuilts 

} sym;    



//bytecodes

enum
{
    BC_STOP,

    BC_PUSH,        // used?
    BC_POP,

    BC_PUSHENV,     // push the nth env variable onto the stack - arg is which env slot
    BC_POPENV,      // pop stack, set the nth env variable to that - arg is which env slot

    BC_PUSHFRAME,   // push the frame arg onto the stack - arg is which frame slot      - copy around the stack
    BC_POPFRAME,    // pop stack and put on the call frame                              

    BC_CALL,        // call to bytecode section subroutine  - CALL    nargs nbc_offset 
    BC_CALLNAT,     // call to native C function or unbuilt - CALLNAT nargs nfunc_offset
    BC_RET,
};

enum 
{
                    //convenience aliases
    NAT_ADD,        
    NAT_MUL,
    NAT_SUB,
    NAT_DIV,        
    NAT_SQR,
                    //math.h functions
    NAT_SQRT,
    NAT_SIN,
    NAT_NORM2,
                    // bytecode subroutines
    SUB_NORM,
};


///

void trace_ts(T t, T te);

int bc_run(int* bytes, int nbytes, T env, int nenv, sym* syms, int nsyms);  //run bytecode on the bytecode machine


// token defs

enum
{
    TOK_NONE,
    TOK_LEFT,
    TOK_RIGHT,
    TOK_CONST,
    TOK_IDENT,
    TOK_ANY,                        //flag for accepting any token 
};

typedef struct _token
{
    int     ntype;                  // type of token - TOK_LEFT || TOK_IDENT etc
    char*   szname;                 // copy of the name [using strdup, we must free]
    double  d;                      // value of the constant
} token;

void    trace_tok(token* t);

void    pa_tokenize(char* sz);      //for testing


// parser defs

typedef struct _PA
{
    char *      sb;                 //begin of parse
    char *      s;                  //current place in string
    char *      sprev;              //previous position [before current token was parsed]

    token*      toks;               //array of tokens
    int         nt;                 //number of tokens used
    int         ntmax;              //max humber of tokens available on toks

    sym*        syms;               //symbol table
    int         nsym;               //num of used sym entries
    int         nsymmax;            //num sym slots available

    T           envs;               //environment variables, constants etc
    int         nenv;
    int         nenvmax;

    int*        bcs;                //byte code     //TODO s16
    int         nbc;
    int         nbcmax;
} PA;


PA*     pa_init(char* sz);
int     pa_parse(PA* pa);
void    pa_run(PA* pa);
double  pa_result(PA* ps);
void    pa_free(PA* pa);

///
