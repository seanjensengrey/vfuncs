//
//  main.c  -  generate a few million draws from a binomial spectrum, then compare actual distribution
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
#include "sc_rng.h"
#include "sc_vfunc.h"


void test_spectrum()
{
    printf("test spectrum....\n");

    static const int n=4000000;
    static const int ns=10;      

    assert(ns<=800);                            // doubles can handle pascals triangle up to around ns=800
    V pas = vnmlz(vpasc(vnew(ns+1),ns));        // normalized binomial distribution with ns elements
    vtrace(pas, "pas");

    V val = vnewseq(0.0, 1.0, ns);              // val = 0,1,2...n 

    V draws = vnewspecdraw(pas, val, n);        // draw n values from discrete spectrum V
    I cnt = inewcountof(draws, val);            // how many of each unique element - the spectrum histogram

    vtrace(val, "val");                         // for each val
    itrace(cnt, "cnt");                         // show its count  [the actual spectrum for this draw]

    double avg = vavg(draws);                   // average of draws from the distribution
    double dev = vdev(draws);                   // std deviation over draws
    double exp = vwavg(pas, val);               // expected value
    printf("stats : avg=%2.4f [sd=%2.4f] exp=%2.4f over %d draws\n", avg, dev, exp, n);

    idel(cnt);
    vdel(draws);
    vdel(val);
    vdel(pas);
}

void test_filter()
{
    printf("test filter....\n");

    static const int n=6000000;                 // # draws
    static const int ns=15;                     // spectrum size for generating draws
    static const int nk=11;                     // filter kernel size

    V val = vnewseq(10.0,0.1, ns);              // val = 10.0 10.1 10.2 ... 
    V pas = vnmlz(vnewpasc(ns));                // normalized binomial distribution with ns elements
    V draws = vnewspecdraw(pas, val, n);        // draw n values from discrete spectrum V
    vdel(pas);
    vdel(val);

    V filt = vnmlz(vnewpasc(nk));               // smoothing kernel eg. [1,4,6,4,1] / 16
    vtrace(filt, "smoothing filter");

    V out  = vnewfilter(filt, draws);           // apply the filter to the data using sliding windows
    V out2 = vnewfilter(filt, out);             // apply the filter again 
    
    vtrace(vhead(draws,15), "raw data");        // show first 15 elements of data for comparison...
    vtrace(vhead(out,15),   "smoothed");        // windows overlap fully so filtering shortens by filt length
    vtrace(vhead(out2,15),  "smoother");        // so each filtering tends to ignore the ends progressively
                                                
    printf("avgs - raw data = %2.3f smoothed = %2.3f smoother = %2.3f [%d draws]\n", vavg(draws), vavg(out), vavg(out2), n );

    vdel(out2);
    vdel(out);
    vdel(draws);
    vdel(filt);
}

int main(void)
{
    //test_spectrum();

    test_filter(); 

    return 0;
}

///
