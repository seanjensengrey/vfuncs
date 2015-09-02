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
