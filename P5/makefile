# JUAN LLINARES MAURI 74011239E

OPTS = -Wall -Wextra -O3 -std=c++0x

all: pow2 pow2.Msteps graphs

graphs: pow2.Msteps pow2.gpi
	gnuplot pow2.gpi

pow2: pow2.cc
	g++ ${OPTS} -o pow2 pow2.cc

pow2.Msteps: pow2
	./pow2 > pow2.Msteps
	./pow2 2> pow2.Msteps.Error

tar: pow2.cc pow2.gpi makefile
	tar cvfz 74011239E.tar.gz pow2.cc pow2.gpi makefile

clean: 
	rm -f pow2 pow2.Msteps pow2.Msteps.Error fit.log pow2*.png