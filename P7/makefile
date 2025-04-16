# JUAN LLINARES MAURI 74011239E

OPTS = -Wall -Wextra -O3 -std=c++0x

all: maze_greedy

maze_greedy: maze_greedy.cc
	g++ ${OPTS} -o maze_greedy maze_greedy.cc

tar: maze_greedy.cc makefile
	tar cvfz 74011239E.tar.gz maze_greedy.cc makefile

clean: 
	rm -f maze_greedy *.o *~ *.tar.gz