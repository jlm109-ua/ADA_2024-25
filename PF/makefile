# JUAN LLINARES MAURI 74011239E

OPTS = -Wall -Wextra -O3 -std=c++0x

all: maze_bb

bfs: bfs.cc
	g++ ${OPTS} -o bfs bfs.cc

maze_bb: maze_bb.cc
	g++ ${OPTS} -o maze_bb maze_bb.cc
	
tar: maze_bb.cc makefile memoria.pdf
	tar cvfz 74011239E.tar.gz maze_bb.cc makefile memoria.pdf

clean: 
	rm -f maze_bb *.o *~ *.tar.gz