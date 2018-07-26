# CombDepth
CombDepth calculates the length of the genomic intersection of an arbitrary number of samples at different pre-specified depths. It is intended evaluate the number of genomic positions that are appropriate to carry out phylogenetic estimation on multi-sample datasets. It parses the result of _samtools depth_ for each sample, which can be a regular or gzipped tsv file. 

I initially developed this program as a Perl script for personal use. Due to lack of performance I decided to migrate it to C99 and then to share it as an stand-alone program. This program does not do anything novel, it is just quick and convenient. 

# Dependencies
This program requires [libbsd](https://libbsd.freedesktop.org/wiki/) for its compilation in non-BSD systems.

# Installation
A simple make should do it if your system meets all the requirements. See [Dependencies](#dependencies).

# Usage
TBA
In the meantime, execute the program without any arguments (or with -h) for a short description on how to use it.
