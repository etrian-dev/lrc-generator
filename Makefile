.POSIX:
.PREFIX:
#compiler macro
CC = g++
#compiler flags for the stable build, turns on optimization level 2 and turn off linking
CFLAGS = -O2 -Wall -pedantic -c
#linking flags: this application uses the audio module, which depends on the
#system module, so the latter must appear last in the list
LDFLAGS = -lsfml-audio -lsfml-system
#debug flags, turn on additional console warning and messages on the status
DBFLAGS = -g -D DEBUG_MODE

#source files required for compiling
SRC = main.cpp lrc-generator.cpp
#object files required for linking
OBJ = main.o lrc-generator.o

all: stable
clean:
	rm -f stable/lrc-generator debug/lrc-generator
	rm -f $(OBJ)
stable: $(OBJ)
	$(CC) -o stable/lrc-generator $(OBJ) $(LDFLAGS)
debug: $(SRC)
	$(CC) -o debug/lrc-generator $(DBFLAGS) $(SRC) $(LDFLAGS)
src: $(SRC)
	$(CC) $(CFLAGS) $(SRC)
