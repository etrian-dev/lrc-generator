// header file for the generator class
#include "lrc-generator.h"
// SFML headers for music playback
#include <SFML/Audio.hpp>
// standard lib headers
#include <iostream>
#include <iomanip> // for streams formatting
#include <fstream>
#include <string>
#include <cstdlib>
#include <chrono> // chrono utilities
#include <ratio>
#include <limits>  // for a stream's max size
#include <utility> // for std::move, used to move-construct a stream
// curses library
#include <ncurses.h>

// constructor taking an input and an output filenames as std::string
Lrc_generator::Lrc_generator(std::string &in_file, std::string &out_file, std::string &song_fname)
{
    // open an input and an output stream with the filenames specified
    this->input_stream = std::ifstream(in_file, std::ios_base::in);
    this->output_stream = std::ofstream(out_file, std::ios_base::out);
    if (!this->input_stream.is_open())
    {
        std::cerr << "Error opening the input stream on file \"" << in_file << "\".\n";
        exit(1);
    }
    if (!this->output_stream.is_open())
    {
        std::cerr << "Error opening the input stream on file \"" << out_file << "\".\n";
        exit(1);
    }

    this->songfile = song_fname;
}
// constructor taking an input and an output stream
Lrc_generator::Lrc_generator(std::ifstream &in_stream, std::ofstream &out_stream, std::string &song_fname)
{
    // move assign the streams to the instance variables
    this->input_stream = std::move(in_stream);
    this->output_stream = std::move(out_stream);

    this->songfile = song_fname;
}

// function to sync the lyrics to the song
void Lrc_generator::sync(void)
{
    // uses system clock to compare timepoints and print elapsed time
    std::chrono::system_clock lrc_clock;
    // variables holding the current and initial timepoints
    std::chrono::time_point<std::chrono::system_clock> this_point, start;
    // stores a duration in seconds
    std::chrono::duration<int> time_secs;

    // current and next line strings
    std::string current;
    std::string next;
    // dummy variable used to store getch()'s output
    int c;

    // load the song in a sf::Music object
    // it's a stream, so it must not be destroyed as long as it's being played
    // supported formats are those listed at https://www.sfml-dev.org/tutorials/2.5/audio-sounds.php
    sf::Music song;
    if (!song.openFromFile(this->songfile))
    {
        std::cerr << "Failed to open the song at \"" << this->songfile << "\"\n";
    }

    // stores the initial timepoint
    start = lrc_clock.now();

    // THE SONG (IF LOADED) STARTS PLAYING
    // does not loop when the end is reached by default
    song.play();

    const int hoff = 2;
    const int woff = 2;

    bool cont = true;    // until the stream contains something this flag is true
    bool refresh = true; // refresh flag
    // as the user presses enter write the corresponding time difference to the file
    while (cont)
    {
        if (refresh && this->input_stream.rdstate() != std::ios_base::eofbit)
        {
            std::getline(this->input_stream, next);

            wclear(this->lyrics);
            box(this->lyrics, 0, 0);

            // display the current line being synced and the next one on stdout
            // when the current line finishes one can either wait for the next to begin
            // or press enter and make the next appear on screen (when reproduced)
            mvwprintw(this->lyrics, hoff, woff, "Curr:\t%s", (current.empty() ? "(null)" : current.c_str()));
            mvwprintw(this->lyrics, hoff + 1, woff, "Next:\t%s", next.c_str());
            mvwprintw(this->lyrics, hoff + 2, woff, "space: pause syncing");
            mvwprintw(this->lyrics, hoff + 3, woff, "any other: advance to the next line");

            wrefresh(this->lyrics);
        }
        else if (refresh && this->input_stream.rdstate() == std::ios_base::eofbit)
        {
            cont = false;
            continue;
        }

        // gets the character to advance
        c = wgetch(this->lyrics);
        // if c is space, pause the syncing
        if (refresh && c == ' ')
        {
            song.pause();
            wclear(this->lyrics);
            box(this->lyrics, 0, 0);
            wstandout(this->lyrics);
            int h, w;
            getmaxyx(this->lyrics, h, w);
            std::string pause = "PAUSED";
            mvwaddstr(this->lyrics, h / 2, w / 2 - pause.length(), pause.c_str());
            wstandend(this->lyrics);
            wrefresh(this->lyrics);
            refresh = false;
            continue;
        }
        // exit pause state
        else if (!refresh && c == ' ')
        {
            refresh = true;
            song.play();
        }

        if (refresh)
        {
            // get the current time point
            this_point = lrc_clock.now();
            // calculate the difference from the start point
            time_secs = std::chrono::duration_cast<std::chrono::seconds>(this_point - start);
            // and write to the .lrc file a new line
            // formatted as [mm:ss.centsecond]<line text>
            this->output_stream.fill('0'); // set the fill character for unspecified fields
            this->output_stream << "["
                                << std::setw(2) << (time_secs.count() / 60) % 60 << ":"
                                << std::setw(2) << time_secs.count() % 60 << "."
                                << std::setw(2) << 0 << "]"
                                << next << "\n";

            // then update the previous line
            current = next;
        }
    }

    // sync done, the song stops
    song.stop();
    wclear(this->lyrics);
    mvwprintw(this->lyrics, 0, 0, "syncing done\n");
}

// the menu loop presented by the class to the user
void Lrc_generator::run(void)
{
    // setup the interface
    interface_setup();

    bool cont = true;
    int action;
    while (cont)
    {
        // draws the menu
        draw_menu();
        // gets a character from the menu window and triggers the action accordingly
        action = wgetch(this->menu);
        switch (action - '0')
        {
        case 0:
            sync();
            break;
        case 1:
            set_attr_dialog("title");
            break;
        case 2:
            set_attr_dialog("artist");
            break;
        case 3:
            set_attr_dialog("album");
            break;
        case 4:
            set_attr_dialog("creator");
            break;
        default:
            // quit the program
            cont = false;
        }

        // after the dialog is destroyed, clear & refresh all the windows
        wclear(this->menu);
        wclear(this->lyrics);
        wrefresh(this->menu);
        wrefresh(this->lyrics);
    }

    delwin(this->menu);
    delwin(this->lyrics);
    // refreshes the standard screen
    refresh();

    // closes the streams
    this->input_stream.close();
    this->output_stream.close();
}
