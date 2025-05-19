# JUAN LLINARES MAURI 74011239E

OPTS = -Wall -Wextra -O3 -std=c++0x

all: maze_bt

maze_bt: maze_bt.cc
	g++ ${OPTS} -o maze_bt maze_bt.cc

iteracio: iteracio.cc
	g++ ${OPTS} -o iteracio iteracio.cc

tar: maze_bt.cc makefile
	tar cvfz 74011239E.tar.gz maze_bt.cc makefile

clean: 
	rm -f maze_bt *.o *~ *.tar.gz