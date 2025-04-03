# JUAN LLINARES MAURI 74011239E

OPTS = -Wall -Wextra -O3 -std=c++0x

all: maze

maze: maze.cc
	g++ ${OPTS} -o maze maze.cc

tar: maze.cc makefile
	tar cvfz 74011239E.tar.gz maze.cc makefile

clean: 
	rm -f maze *.o *~ *.tar.gz