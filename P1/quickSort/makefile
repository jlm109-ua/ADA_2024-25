# JUAN LLINARES MAURI 74011239E

OPTS = -Wall -Wextra -O3 -std=c++0x

all: quickSort quickSort.CPUtimes graphs

graphs: quickSort.CPUtimes quickSort.gpi
	gnuplot quickSort.gpi

quickSort: quickSort.cc
	g++ ${OPTS} -o quickSort quickSort.cc

quickSort.CPUtimes: quickSort
	./quickSort > quickSort.CPUtimes
	./quickSort 2> quickSort.CPUtimes.Error

tar: quickSort.cc quickSort.gpi makefile
	tar cvfz 74011239E.tar.gz quickSort.cc quickSort.gpi makefile

clean: 
	rm -f quickSort quickSort.CPUtimes quickSort.CPUtimes.Error fit.log quickSort*.png *~