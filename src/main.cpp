// header file for the generator class
#include "lrc-generator.h"
// ncurses header
#include <ncurses.h>
// std lib headers
#include <iostream>
#include <string>

// functions to initialize the ncurses library and do cleanup respectively
void init_ncurses()
{
    initscr();            // start curses
    cbreak();             // disable line buffering
    noecho();             // disable input echo
    keypad(stdscr, true); // enable the keypad and fn keys

    refresh();
}
void cleanup_ncurses(void)
{
    endwin();
}

int main(int argc, char **argv)
{
    // check if arguments have been given to the program
    if (argc != 2 && argv[1] != NULL && argv[2] != NULL)
    {
        // initialize the curses library for immediate input and keypad enabled
        init_ncurses();

        // the filename of the lrc file is the same as the lyrics file
        std::string lyrics_fname = std::string(argv[1]);

        // the path to the song is the second string
        std::string song_path = std::string(argv[2]);

        std::string lrc_fname = lyrics_fname;
        lrc_fname.replace(lrc_fname.size() - 3, 3, "lrc");

        // Lrc_generator is a simple class to generate the lrc file (includes a tui)
        Lrc_generator generator(lyrics_fname, lrc_fname, song_path);
        generator.run(); // main program loop (menu)
    }
    else
    {
        std::cerr << "No lyrics provided\nUsage: lrc-generator [lyrics.txt] [path to song]\n";
    }

    // does the cleanup
    cleanup_ncurses();

    return 0;
}
