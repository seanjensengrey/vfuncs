//
//  sc_rng.c - inlined versions of Marsaglias C macros [want to use in threads, so need all state per rng instance]
//
//  copyright (c) 2008 gordon anderson 
//  released under BSD licence
//  contact: justgord@gmail.com
//  
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <math.h>
#include "sc_rng.h"

/////// znew  ((z=36969*(z&65535)+(z>>16))<<16)
/////// wnew  ((w=18000*(w&65535)+(w>>16))&65535)
/////// MWC   (znew+wnew)
/////// SHR3  (jsr=(jsr=(jsr=jsr^(jsr<<17))^(jsr>>13))^(jsr<<5))

inline u32 znew(u32* pz)
{
    u32 z=*pz;
    //((z=36969*(z&65535)+(z>>16))<<16);
    z=36969*(z&65535)+(z>>16);
    *pz=z;
    return z;
}

inline u32 wnew(u32* pw)
{
    u32 w=*pw;
    //((w=18000*(w&65535)+(w>>16))&65535);
    w=18000*(w&65535)+(w>>16);
    *pw=w;
    return w;
}

inline u32 mwc(u32* pz, u32* pw)
{
    return (znew(pz)<<16) + (wnew(pw)&65535);
}
  
inline u32 shr3(u32* pjsr)
{
    u32 jsr=*pjsr;
    //     (jsr=(jsr=(jsr=jsr^(jsr<<17))^(jsr>>13))^(jsr<<5));               // compiler warning :]
                      jsr=jsr^(jsr<<17); 
                 jsr=(jsr              )^(jsr>>13);           
            jsr=(jsr                              )^(jsr<<5);  
    *pjsr = jsr; 
    return jsr;
}
  
/////// CONG  (jcong=69069*jcong+1234567)
/////// KISS  ((MWC^CONG)+SHR3)
/////// LFIB4 (t[c]=t[c]+t[c+58]+t[c+119]+t[++c+178])               // <<----- beware overflow!
/////// SWB   (t[c+237]=(x=t[c+15])-(y=t[++c]+(x<y)))               // <<----- beware overflow!

inline u32 cong(u32* pjcong)
{
    u32 jcong=*pjcong;
    (jcong=69069*jcong+1234567);
    *pjcong=jcong;
    return jcong;
}

inline u32 kiss(u32* pz, u32* pw,  
            u32* pjcong,  
            u32* pjsr)
{
    return (mwc(pz,pw)^cong(pjcong))+shr3(pjsr);
}

inline u32 lfib4(u32 t[], u8* pc)
{
    u8 c=*pc;
    u32 r=t[c]=t[c]+t[(c+58)&0xff]+t[(c+119)&0xff]+t[(++c+178)&0xff];
    *pc=c;
    return r; 
}

inline u32 swb(u32 t[], u8* pc, u32* px, u32* py)
{
    u8 c=*pc;
    u32 x=*px, y=*py;
    u32 r=(t[(c+237)&0xff]=(x=t[(c+15)&0xff])-(y=t[++c]+(x<y)));
    *pc=c;
    *px=x;
    *py=y;
    return r; 
}

///

typedef struct _rng_internal
{
    u32 z, w, jsr, jcong;
    u32 t[256];
    u32 canary;
    u32 x,y; 
    u8  c;
} rng_internal;

void rng_init(rng_state* pS, u32 s)
{
    assert(pS);
    assert(sizeof(rng_state)>=sizeof(rng_internal));
    rng_internal* pI=(rng_internal*)pS;

    pI->x=pI->y=pI->c=0;
    pI->z=s;
    pI->w=7*s;
    pI->jsr=11*s; 
    pI->jcong=19*s;

    int i; 
    for(i=0;i<256;i++)  
        pI->t[i]=kiss(&(pI->z), &(pI->w), &(pI->jcong), &(pI->jsr));
}

inline u32 rng_combined(rng_state* pS)
{
    assert(pS);
    rng_internal* pI=(rng_internal*)pS;

    assert((&(pI->t[0]))==(pI->t));
    
    u32 nkiss=kiss(&(pI->z), &(pI->w), &(pI->jcong), &(pI->jsr));
    u32 nswb=swb(pI->t, &(pI->c), &(pI->x), &(pI->y));
    u32 nlfib4=lfib4(pI->t, &(pI->c));
    return ((nkiss+nswb)^(nlfib4));
}

u32 rng_rand(rng_state* pS)
{
    return rng_combined(pS);
}

u32 rng_rand_in(rng_state* pS, u32 lo, u32 hi)
{
    return lo+rng_combined(pS)%(hi-lo); 
}

float rng_unif01(rng_state* pS)
{
    return (rng_combined(pS)*2.328306e-10);
}

//
