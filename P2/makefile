# JUAN LLINARES MAURI 74011239E

OPTS = -Wall -Wextra -O3 -std=c++0x

FIT_LOG = fit.log
FIT_HISTORY = fit.history

all: qs-vs-hs qs-vs-hs.Msteps graphs append_log

graphs: qs-vs-hs.Msteps qs-vs-hs.gpi
	gnuplot qs-vs-hs.gpi >> $(FIT_LOG) 2>> $(FIT_LOG)

append_log:
	@cat $(FIT_HISTORY) >> $(FIT_LOG)
	@> $(FIT_HISTORY)
	rm -f $(FIT_HISTORY)

qs-vs-hs: qs-vs-hs.cc
	g++ ${OPTS} -o qs-vs-hs qs-vs-hs.cc

qs-vs-hs.Msteps: qs-vs-hs
	./qs-vs-hs > qs-vs-hs.Msteps
	./qs-vs-hs 2> qs-vs-hs.Msteps.Error

tar: qs-vs-hs.cc qs-vs-hs.gpi makefile
	tar cvfz 74011239E.tar.gz qs-vs-hs.cc qs-vs-hs.gpi makefile

clean: 
	rm -f qs-vs-hs qs-vs-hs.Msteps qs-vs-hs.Msteps.Error fit.log qs-vs-hs*.png quickSort.png heapSort.png *~ *.history