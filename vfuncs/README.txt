
    vfunc - vector array definitions and verbs/funcs in C

    Introduction

        Some functions to allow higher level verb-like programming with simple arrays

        see sc_func.h for the simple api.

        The main.c shows generation of random draws from a binomial spectrum,
        [ie. pascals triangle, but any real discrete spectrum could be used of course]
        then compares the actual draws to the supplied spectrum.
        Also shows stats - mean & std deviation of actual draws vs expected value

        The idea is to think in verbs on arrays and thus write more readable code 
        [even in C] thus avoiding many repeated for loops etc.  

        An experiment inspired by functional languages and stream databases.

    Build

        $> make sc
        $> ./sc

    Performance

        To time it and check memory allocations use, eg : 

        $> echo "install valgrind first" 
        $> make sc && valgrind ./sc && time ./sc
        test spectrum....
        pas : 0.0010 0.0098 0.0439 0.1172 0.2051 0.2461 0.2051 0.1172 0.0439 0.0098 0.0010 
        val : 0.0000 1.0000 2.0000 3.0000 4.0000 5.0000 6.0000 7.0000 8.0000 9.0000 10.0000 
        cnt :   3891  39107 175770 469774 820652 983638 820822 468013 175657  38811   3865 
        stats : avg=4.9987 [sd=1.5808] exp=5.0000 over 4000000 draws
        real    0m0.565s
        user    0m0.532s
        sys     0m0.032s

        ie. < 1 sec including allocating a large array of about 32Mb for 4mil draws
        nb: valgrind is a (seperate) superb debugging and performance tuning tool

    Platform

        linux only so far [but should be ansi C compliant and posix friendly]

    Legal

        copyright (c) 2008 gordon anderson 
        released under BSD licence

        Use at your own risk, no warranty of fitness for any purpose given or implied

    Contact

        email: justgord@gmail.com [gord]
        blog: quantblog.wordpress.com 

        Email me if you have comments, bugs, suggestions or consulting offers :)


