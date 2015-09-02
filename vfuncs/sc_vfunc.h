//
//  sc_vfunc - vector array definitions and verbs/funcs 
//
//  copyright (c) 2008 gordon anderson 
//  released under BSD licence
//  contact: justgord@gmail.com
//

typedef struct _I           // array of int
{
    int*    p;
    int     n;
} I;

typedef struct _V           // array of double
{
    double* p;
    int     n;
} V;


I inew(int n);                              // alloc
void idel(I x);                             // free
I inewbin(V draws, V cdf);                  // new I with draws' bins in cdf  [ie. which bin each draw falls into within cdf]
I inewcountof(V draws, V vals);             // new counts in draws of each unq elt - the spectrum histogram
void itrace(I v, const char* sz);           // print each item

V vnew(int n);                              // alloc
void vdel(V v);                             // free
V vnewcopy(V v);                            // new copy of v
V vnewseq(double a, double dx, int n);      // new sequence [a, a+dx, ... a+ndx ]  - returns V with n+1 elements
V vnewrand(int n);                          // new V drawn uniformly from reals [0, 1]
V vnewpasc(int n);                          // new pascals triangle eg. [1 4 6 4 1] nb: n+1 elts
V vnewspecdraw(V spec, V vals, int n);      // new V drawn from discrete spectrum 
V vnewuniq(V v);                            // new list of unique items occurring in v

double vlast(V x);                          // last element
double vsum(V x);                           // sum
double vavg(V x);                           // average
double vdev(V x);                           // std deviation
double vwavg(V w, V x);                     // w weighted average of v - dot product or expectation

V vfill(V v, double s);                     // fill v with all s's
V vsums(V v);                               // replace v with partial sums, making it cumulative monotonic
V vmul(V v, double s);                      // replace v[i] with s*v[i]
V vnmlz(V v);                               // normalize - scale so sum is 1.0

V vhead(V x, int n);                        // subset of first n elts
V vtail(V x, int n);                        // subset of last n elts

V vrand(V r, int n);                        // fill V with n random rs from uniform draw of [0,1]
V vpasc(V v, int n);                        // fill v with nth pascals triangle - eg. 1,4,6,4,1 if n=4
V vidx(V r, I ix, V v);                     // return the values indexed by each ix into v
V vsort(V v);                               // sort contents

int vbsearch(V x, double v);                // binary search for v in x - X must be sorted! - returns value in that bin if not exact

void vtrace(V v, const char* sz);           // print each item

///

// TODO verbs - map() reduce()  convolve()

// convolution support ...

typedef double (*fmix2)(V va, V vb);        // function called with each window [as they slide along A and B]

V vnewslide2(int nwin,                      // mix 2 arrays - slide window from one past a window of the other
            V vA, int dA,                   // array A - increment window by dA as it slides across vA [left if dA -ve] 
            V vB, int dB,                   // array B and its directional increment for each step
            fmix2 fn);                      // in each position, call fn with wa & wB sliding windows each of width nwindow

V vnewfilter(V spec, V data);               // apply the spectrum filter against the data, returning the filtered stream


                
