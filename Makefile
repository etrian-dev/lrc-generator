#compiler macro
CC = g++
#compilation flags for the stable build
#turns on optimization level 2
#compilation only, produces object files
CPPFLAGS =  -std=c++17 -O2 -Wall -pedantic
#linking flags: sfml and ncurses
# sfml-audio depends on sfml-system, so it must come before it in the list
LDFLAGS = -lsfml-audio -lsfml-system -lncurses
#debug flags
DBFLAGS =  -std=c++17 -Wall -pedantic -g -D DEBUG_MODE

#directory containing sources
SRC = $(wildcard src/*.cpp)

.PHONY: all debug clean

stable: $(SRC)
	$(CC) -o stable/lrc-generator $(CPPFLAGS) $(SRC) $(LDFLAGS)
debug: $(SRC)
	$(CC) -o debug/lrc-generator $(DBFLAGS) $(SRC) $(LDFLAGS)
clean:
	rm -f stable/lrc-generator debug/lrc-generator
gentags:
	geany -g -P geany.cpp.tags $(wildcard src/*.cpp) $(wildcard headers/*.h)
