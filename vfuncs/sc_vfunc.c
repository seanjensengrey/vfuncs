//
//  sc_vfunc - vector array definitions and verbs/funcs 
//
//  copyright (c) 2008 gordon anderson 
//  released under BSD licence
//  contact: justgord@gmail.com
//
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <math.h>
#include "sc_rng.h"
#include "sc_vfunc.h"

V vnewcopy(V v)
{
    V c = vnew(v.n);

    int i;
    for(i=0;i<v.n;i++)
        c.p[i]=v.p[i];

    assert(c.n==v.n);
    return c;
}

#define USE_VSPECDRAW_V2 1

#ifdef USE_VSPECDRAW_V2
//
// v2 - optimized version - less readable, uses less ram by avoiding creating another large array
//
V vnewspecdraw(V spec, V vals, int n)   // draw n values from discrete spectrum V
{
    assert(vals.n>=spec.n);

    V m=vsums(vnmlz(vnewcopy(spec)));   // make partial sums - m is now a monotonic cdf for spec pdf

    rng_state rng;
    rng_init(&rng, 0x331155ab);

    V r=vnew(n);                    
    int i;
    for (i=0;i<n;i++)
    {
        double f=rng_unif01(&rng);      // draw f in [0, 1]

        int idx=vbsearch(m, f);         // find where f falls into a bin of cdf m
        r.p[i]=vals.p[idx+1];           // we want the val for that bin
    }       

    vdel(m); 
    return r;
}
#else
//
// v1 - original  version - more readable, but makes extra array so uses more RAM
//
V vnewspecdraw(V spec, V vals, int n)   //draw n values from discrete spectrum V
{
    assert(vals.n>=spec.n);

    V m=vsums(vnmlz(vnewcopy(spec)));   // make partial sums - m is now a monotonic cdf for spec pdf

    V r=vnewrand(n);                    // n draws from uniform RNG
    I ix=inewbin(r, m);                 // return bin index of each r in cdf m - ie. where would it fall?
    vidx(r, ix, vals);                  // set each r to the rth val

    vdel(m);
    idel(ix);
    return r;
}
#endif

V vfill(V v, double s)
{
    int i;
    for (i=0;i<v.n;i++)
        v.p[i]=s;
    return v;
}

V vpasc(V v, int n)
{
    int i,j;
   
    assert(v.n>n);          //need n+1 for pas n
     
    vfill(v, 0.0);
    v.p[0]=1.0;
        
    for (i=0;i<n;i++)
        for (j=i;j>=0;j--)
            v.p[j+1]+=v.p[j];

    return v;    
}

V vnew(int n)
{
    //printf("vnew : allocated %d elts\n",n);

    V v;
    v.n=n;
    v.p=calloc(n, sizeof(double));          //TODO get from a free list  //TODO refcount //TODO iterators for chunks
    return v;
}

V vnewseq(double a, double dx, int n)
{
    // new sequence [a, a+dx, ... a+ndx ]  - returns V with n+1 elements

    V x = vnew(n+1);    

    int i;
    for (i=0;i<x.n;i++)
    {
        x.p[i]=a;
        a+=dx;
    }
    return x;
}

V vnewpasc(int n)
{
    return vpasc(vnew(n+1),n);
}

V vnewuniq(V v)
{
    // new list of unique items occurring in v - 2 passes : first count uniq, then store uniq

    V u;
    u.p=NULL;
    u.n=0;   
    if (v.n<=0)
        return u;

    int n=1;
    double d=v.p[0];
    int i;
    for (i=1;i<v.n;i++)
    {
        if (d!=v.p[i])                  //count how many unique elts
        {
            n++;
            d=v.p[i];
        }
    }

    u=vnew(n);
    d=u.p[0]=v.p[0]; 
    int j=1;
    for (i=1;i<v.n;i++)
    {
        if (d!=v.p[i]) 
        {
            d=u.p[j]=v.p[i]; 
            j++;                        //move to next slot of u
        }
    }
    return u;
}

I inewcountof(V draws, V vals)
{
    // new counts in draws of each unq elt - the spectrum histogram - assume vals sorted

    I nvs=inew(vals.n);

    int i;
    for (i=0;i<draws.n;i++)
    {
        double f=draws.p[i];
        int idx=vbsearch(vals, f);               //binary search to find f in cdf
        if (idx>=0)
            nvs.p[idx]++;
    }

    return nvs;
}


void vdel(V v)
{
    free(v.p);
    v.p=NULL;
    v.n=0;
}

double vlast(V x)
{
    return x.p[x.n-1];
}

double vavg(V x)
{
    return vsum(x)/x.n;
}

double vsum(V x)
{
    double s=0.0;
    int i;
    for (i=0;i<x.n;i++)
        s+=x.p[i];     
    return s;
}

double vdev(V x)
{
    double mu=vavg(x);
    double s=0.0;
    int i;
    for (i=0;i<x.n;i++)
    {
        double d=x.p[i]-mu;
        s+=d*d;
    }
    return sqrt(s/x.n);
}

double vwavg(V w, V x)
{
    // w weighted average of v - dot product or expectation
    
    int n=MIN(w.n, x.n);

    double s=0.0;
    int i;
    for (i=0;i<n;i++)
        s+=w.p[i]*x.p[i];       // summa w[i]*x[i]
    return s;
}


V vnewrand(int n)
{
    V v = vnew(n);
    vrand(v, n);
    return v;
}

V vsums(V v)
{
    // replace v with partial sums, making it cumulative monotonic
    
    double s=0.0;
    int i;
    for (i=0;i<v.n;i++)
    {
        s+=v.p[i];
        v.p[i]=s;
    }
    return v;
}
    
V vmul(V v, double s)
{
    // replace v[i] with s*v[i]
    int i;
    for (i=0;i<v.n;i++)
        v.p[i]*=s;
    return v;
}

V vnmlz(V v)
{
    double s = vsum(v);
    vmul(v, 1.0/s);      
    return v;
}

V vrand(V r, int n)
{
    assert(r.n>=n);

    rng_state rng;
    rng_init(&rng, 0x331155ab);

    int i;
    for (i=0;i<n;i++)
        r.p[i]=rng_unif01(&rng);

    return r;
}

V vidx(V r, I ix, V v)
{
    // set each r to the v[ix]

    int n=MIN(ix.n, r.n);

    int i;
    for (i=0;i<n;i++)
    {
        assert(ix.p[i]<v.n);

        r.p[i]=v.p[ix.p[i]];        // r[i] = v[ix[i]]
    }
    return r;
}

void itrace(I v, const char* sz)
{
    if (sz)
        printf("%s : ", sz);

    int i;
    for (i=0;i<v.n;i++)
        printf("%6d ", v.p[i]);
    printf("\n");
}

void vtrace(V v, const char* sz)
{
    if (sz)
        printf("%s : ", sz);

    int i;
    for (i=0;i<v.n;i++)
        printf("%2.4f ", v.p[i]);
    printf("\n");
}

inline int vcompd(const void* pa, const void* pb)
{
    return (*(double*)pa>*(double*)pb);
}

V vsort(V v)
{
    qsort(v.p, v.n, sizeof(double), vcompd);
    return v;
}

V vhead(V x, int n)                         // subset of first n elts - nb: dont vdel the range returned
{
    x.n=MIN(n,x.n);                         
    return x;
}

V vtail(V x, int n)                        // subset of last n elts nb: dont vdel the range returned
{
    n=MIN(n,x.n);
    x.p+=x.n-n;
    x.n=n;
    return x;
}

///


I inew(int n)
{
    //printf("inew : allocated %d elts\n",n);

    I x;
    x.n=n;
    x.p=calloc(n, sizeof(int));          //TODO get from a free list  //TODO refcount //TODO iterators for chunks
    return x;
}

void idel(I x)
{
    free(x.p);
    x.p=NULL;
    x.n=0;
}

int vbsearch(V x, double v)                 // binary search for v in x - X must be sorted!
{
    int a=0;
    int b=x.n-1;
    double* p = x.p;

    if (p[a]>v || v>p[b])
        return -1;                          //not here

    while (1)
    {
        assert(a<=b);
        //printf("a=%d b=%d p[a]=%2.4f p[b]=%2.4f : v=%2.4f\n",a,b,p[a],p[b], v);
        assert(p[a]<=v && v<=p[b]);

        if (b==a)
            return a;

        if ((b-a)==1)
            return (v==p[b]) ? b : a;

        int m=(b+a)/2;
        double vm=p[m];
        if (v<vm)
            b=m;
        else 
            a=m;
    }
    assert(0);
    return 0;
}

I inewbin(V draws, V cdf)
{
    I ix=inew(draws.n);  

    int i;
    for (i=0;i<draws.n;i++)
    {
        double f=draws.p[i];
        int idx=vbsearch(cdf, f);               //binary search to find f in cdf
        ix.p[i]=idx+1;
    }

    return ix;
}

V vnewslide2(int nw,
            V vA, int dA,
            V vB, int dB,
            fmix2 fn)
{
    assert(dA>=0 && dB>=0);                 //TODO do sliding left

    V wa;
    wa.n=nw;
    wa.p=vA.p;
    V wb;
    wb.n=nw;
    wb.p=vB.p;

    int n;
    if (dA==0 && dB==0)
        n=1;
    else if (dA==0)
        n=(vB.n-nw)/dB;
    else if (dB==0)
        n=(vA.n-nw)/dA;
    else
        n=MIN((vA.n-nw)/dA, (vB.n-nw)/dB);

    V out=vnew(n);

    int i;
    for (i=0;i<n;i++)
    {
        out.p[i]=fn(wa, wb);

        wa.p+=dA;
        wb.p+=dB;
    }
    return out;
}

V vnewfilter(V filt, V data)                // apply the spectrum filter against the data, returning the filtered stream
{
    int n=MIN(filt.n, data.n);
    V v=vnewslide2(n, filt,0, data,1, vwavg);    // slide the data past the static filter window, gathering dot product
    return v;
}

///
