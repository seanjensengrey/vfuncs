vfuncs is a library for semi-functional style coding with
vector verbs in ansi C. It includes functions to allow higher
level verb-like programming with simple in memory arrays.

The code could be useful for prototyping and development of
signal analyzing time series data [say, in finance,
bioinformatics, stats], and was motivated largely as an
experiment to write readable C code which has reasonably good
performance - currently does gaussian filter across 6-million
points per second.

It includes for example implementations of Marsaglias random
number generators and a simple digital filter implementation.

Inspired partly by K, Q and other functional languages, and
the open source tool R.

imported into git from http://code.google.com/p/vfuncs/ the
code.google repo contains no code, but does contain hosted
binaries:

```
dec66e6c6cac026e834c9aa37e92de5ab8ec9b87  vfuncs-0.6.1.tar.gz
6722cc78c1afe43600250781e1ce449a1a5ac441  vfuncs-0.6.5.tar.gz
e978c6bacf6d3961abee4974d924151b335d4944  vfuncs-0.6.7.tar.gz
```
