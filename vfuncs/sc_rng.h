//
//  sc_rng.h - random number generator [based on George Marsaglias RNG code snippets]
//
//  copyright (c) 2008 gordon anderson 
//  released under BSD licence
//  contact: justgord@gmail.com
//

typedef uint64_t        u64;
typedef int64_t         i64;
typedef uint32_t        u32;
typedef int32_t         i32;
typedef uint16_t        u16;
typedef int16_t         i16;
typedef uint8_t         u8;
typedef int8_t          i8;

#define _count_of(rg)  ((sizeof(rg))/(sizeof(rg[0])))

#define MIN(a,b)       ( (a<b) ? a : b )
#define MAX(a,b)       ( (a>b) ? a : b )

typedef struct _rng_state
{
    u32  state[256+16];                             // opaque storage for private random state
} rng_state;

void rng_init(rng_state* pS, u32 seed);             // initialised pS internal state

u32 rng_rand(rng_state* pS);                        // random unsigned 32bit int
u32 rng_rand_in(rng_state* pS, u32 lo, u32 hi);     // random unsigned within range

float rng_unif01(rng_state* pS);                    // random float uniformly in [0.0, 1.0]

///
