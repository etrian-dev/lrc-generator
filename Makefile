.POSIX:
.PREFIX:
#compiler macro
CC = g++
#compilation flags for the stable build
#turns on optimization level 2
#compilation only, produces object files
CFLAGS = -O2 -Wall -pedantic -c
#linking flags: sfml and ncurses
# sfml-audio depends on sfml-system, so it must come before it in the list
LDFLAGS = -lsfml-audio -lsfml-system -lncurses
#debug flags
DBFLAGS = -g -D DEBUG_MODE

#source files path (relative to Makefile)
SRC = src/
#object files path (relative to Makefile)
#OBJ = obj/

clean:
	rm -f stable/lrc-generator debug/lrc-generator
stable: $(SRC)*.cpp
	$(CC) -o stable/lrc-generator $(SRC)*.cpp $(LDFLAGS)
debug: $(SRC)*.cpp
	$(CC) -o debug/lrc-generator $(DBFLAGS) $(SRC)*.cpp $(LDFLAGS)
