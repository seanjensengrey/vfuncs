//
//  variant.h - variant type T and W vectors v2 [demand loadable from file, accessors]
//
//      T is the var type datum, W is the managed array [of Ts]
//

typedef enum { 
    tvoid,        
                                                // singles...
    tany,        
    ti8, ti16, ti32, ti64, tc=ti8, tb=ti8, 
    tsz, 
    tf, td, tld, tdt=td,
    tfunc,      
                                                // arrays ...
    taany,     
    tai8, tai16, tai32, tai64, tac=tai8, 
    tasz, 
    taf, tad, tald, tadt=tad, 
    tafunc   
} vtype;


typedef struct _T               // variant type [16bytes]
{
    i16     t;                  // data type [one of vtype]
    i16     ref;                // refcount
    union 
    {
        // integer

        char            c;
        i8              I8; 
        i16             I16;
        i32             I32;
        i64             I64;

        // real

        float           f;
        double          d;
        long double     ld;         // 12b

        //array & string
       
        struct                      // arrays of any kind - length and pointer
        {
            int         n;          // count of items if an array [or bytes if a buffer such as string]

            struct _T*  t;          // array elements are always Ts, [could be multitype later]
            char*       z;          // C string, buffer has n allocated, should be padded to 8 or 16bytes??
        };

        //function                  // functions must be data..so we can pass a func into another func
/*
        struct
        {
            int         f;          //func
            //int       ifn;        // function - offset in symbol table   //TODO - eg. FMUL etc
            int*        psl;        //slot indexes
            int         nsl;        //num slot indexes
        };
*/
    };
} *T, TT;


//accessors 

T       vtnew(vtype typ);                       // single elt [from say a freelist]
T       vtnewn(vtype typ, int n);               // an array of n elts
void    vtdel(T);
    

typedef struct _chunk                           //internal impl only
{
    void*       p;                              //pointer to buffer if loaded
    int         n;                              //size of chunk in bytes
    int         nelts;                          //num items [may be variable length], used for skipping when indexing

    double      tlastrw;                        //last read or write timestamp [sec since 2000.01.01.000]
    int         ndirty;                         //dirty bytes
} chunk;

int         vchload(chunk* pc, int noff, int fd);       //load the chunk in from file, into the chunk buffer
int         vchsave(chunk* pc, int noff, int fd);       //save the chunk to the file - in a single write from the chunk buffer
int         vchunload(chunk* pc);                       //free up the chunk memory


// memory cached array type W [array of Vs]

typedef struct _W
{
                            // absolute - [na ... na+np .... na+n)  relative to window - [0 ... np....n)
    int         np;         // current pos - [absolute pos is na+np]
    int         n;          // size
    int         na;         // start pos - where window starts [normally 0]

    chunk*      pch;        // array of chunk headers with info for each chunk [p, min, max, end] p==NULL if not loaded
    int         nch;        // chunk size
    int         fd;         // file descriptor [or null]
} *W;


W           vinit(int n);                           // not backed by disk - null fd [can later associate with file?]
W           vopen(int n, const char* nm, int fl);   // open file, and load in from file [or create file depending on flags]
int         vsave(W w, const char* nm, int fl);     // explicitly save whole file - sync all chunks to disk
void        vdone(W w);                             // deref and if refcount goes to 0, sync, close, unload and free

void        vmov(W w);                              // move right 1 pos - update npos
void        vmovn(W w, int n);                      // move window right by n - for clone sub indexing windows - update npos

// get / set item

T           vr(W w, int pos);                       // get value at pos [relative to npos]
void        vw(W w, int pos, T val);                // set value at pos to val  [relative to npos]

i32         vri(W w, int pos);                      // read an int
void        vwi(W w, int pos, i32 val);             // write an int
double      vrd(W w, int pos);                      // read a double
void        vwd(W w, int pos, double d);            // write a double

void        vrai(W w, int pos, int* pd, int n);     // read an array of i32  
void        vwai(W w, int pos, int* pd, int n);     // write an array of i32  
void        vrad(W w, int pos, double* pd, int n);  // read an array of doubles  
void        vwad(W w, int pos, double* pd, int n);  // write an array of doubles  



