# target: [pre]
# [TAB]  command
all: hf read2e 2e basis.o overlap.o common.o overlap_main kinetic nuclear_elect ints.o

common.o: common.c common.h
	gcc common.c -c -Wall -g -lgsl -lm -o common.o

hf: hf.c common.o
	gcc hf.c common.o -Wall -g  -lgsl -lm -o hf

read2e: read2e.f90
	ifort read2e.f90 -o read2e

2e: read2e
	./read2e > 2e

ints.o: ints.c
	gcc ints.c -lm -Wall -g -c -o ints.o

basis.o: basis.c basis.h
	gcc basis.c -c -Wall -g -lm -o basis.o

overlap.o: overlap.c overlap.h basis.o common.o
	gcc overlap.c basis.o common.o -c -Wall -g -lm -lgsl -o overlap.o

overlap_main:common.o basis.o overlap.o overlap_main.c ints.o
	gcc overlap_main.c overlap.o basis.o common.o ints.o -Wall -g -lgsl -lm -o overlap_main

kinetic: kinetic.c overlap.o basis.o common.o ints.o
	gcc kinetic.c overlap.o basis.o common.o ints.o -Wall -g -lgsl -lm -o kinetic

coord: coord.c basis.o overlap.o common.o
	gcc coord.c basis.o overlap.o common.o -Wall -g -lgsl -lm -o coord

nuclear_elect: nuclear_elect.c basis.o overlap.o common.o ints.o
	gcc nuclear_elect.c basis.o overlap.o common.o ints.o -Wall -g -lgsl -lm -o nuclear_elect

clean:
	rm -rf hf read2e *.o 2e *.mod overlap_main kinetic coord ints nuclear_elect
