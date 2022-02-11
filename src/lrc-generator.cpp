// header file for the generator class
#include "../headers/lrc-generator.h"
// SFML headers for music playback
#include <SFML/Audio.hpp>
// curses library
#include <ncurses.h>
// standard lib headers
#include <iostream>
#include <iomanip> // streams formatting functions
#include <fstream>
#include <string>
#include <limits>  // to obtain a stream's max size
#include <utility> // to use std::move, used to move-construct a stream
#include <filesystem>
#include <chrono>
#include <ratio>

namespace ns = std::filesystem;

// define more practical names for std::chrono things
using Clock = std::chrono::system_clock;
using TimeSecs = std::chrono::duration<int>;

// constructor taking an input and an output filenames as std::string
Lrc_generator::Lrc_generator(fs::path &in_file, fs::path &out_file, fs::path &song_path)
{
    // open an input and an output stream with the filenames specified
    std::ifstream input_stream = std::ifstream(in_file, std::ios_base::in);
    this->output_stream = std::ofstream(out_file, std::ios_base::out);
    if (!input_stream.is_open())
    {
        std::cerr << "Error opening the input stream on file \"" << in_file << "\".\n";
        exit(1);
    }
    if (!this->output_stream.is_open())
    {
        std::cerr << "Error opening the input stream on file \"" << out_file << "\".\n";
        exit(1);
    }

    while(input_stream.good()) {
        std::string line;
        std::getline(input_stream, line);
        this->lyrics.push_back(std::move(line));
    }

    this->songfile = song_path;

    if(input_stream.eof()) {
        input_stream.close();
    }
}

Lrc_generator::~Lrc_generator() {
    size_t i;
    for(i = 0; i < this->lyrics.size(); i++) {
        // first append the time point
        this->output_stream << this->lrc_text[i] << this->lyrics[i] << '\n';
    }
    this->output_stream.close();
}

// function to sync the lyrics to the song
void Lrc_generator::sync(void)
{
    // uses the system clock to compare timepoints and print elapsed time
    Clock lrc_clock;
    // variables holding the current and initial timepoints
    std::chrono::time_point<Clock> start, current_tp, tmp;
    // the total pause is initialized as zero
    std::chrono::time_point<Clock> pause_total = std::chrono::time_point<Clock>();
    // current and next line strings
    std::string current;
    std::string next;
    // dummy variable used to store getch()'s output
    int c;

    // load the song in a sf::Music object
    // it's a stream, so it must not be destroyed as long as it's being played
    // supported formats are those listed at
    // https://www.sfml-dev.org/tutorials/2.5/audio-sounds.php
    sf::Music song;
    if (!song.openFromFile(this->songfile.string()))
    {
        std::cerr << "Failed to open the song file at \"" << this->songfile << "\"\n";
    }

    // stores the initial timepoint
    start = lrc_clock.now();

    // THE SONG (IF LOADED) STARTS PLAYING
    // does not loop when the end is reached by default
    song.play();

    int height, width;
    getmaxyx(this->lyrics_win, height, width);

    const int hoff = 2;
    const int woff = 2;

    unsigned int dim = this->lyrics.size();
    unsigned int idx = 0;
    next = this->lyrics[0]; // to initialize the loop correctly
    // as the user presses enter write the corresponding time difference to the file
    while(idx < dim) {
        current = next;
        if(idx + 1 < dim) {
            next = this->lyrics[idx + 1];
        }
        else {
            next.erase();
        }

        wclear(this->lyrics_win);
        box(this->lyrics_win, 0, 0);
        // display the current line being synced and the next one on stdout
        // or print "(null)" if the line is empty
        mvwprintw(this->lyrics_win, hoff, woff, "Curr:\t%s", (current.empty() ? "(null)" : current.c_str()));
        mvwprintw(this->lyrics_win, hoff + 1, woff, "Next:\t%s", (next.empty() ? "(null)" : next.c_str()));
        mvwaddstr(this->lyrics_win, height - 4, woff, "space: pause syncing");
        mvwaddstr(this->lyrics_win, height - 3, woff, "'s': restart syncing");
        mvwaddstr(this->lyrics_win, height - 2, woff, "any other key: advance to the next line");
        wrefresh(this->lyrics_win);

        // gets the character to advance
        c = wgetch(this->lyrics_win);
        // if c is space, pause the syncing
        if (c == ' ') {
          // record the time point at the start of the pause
          std::chrono::time_point<Clock> pause_start = lrc_clock.now();
          // pause the song being played and print a PAUSE string at the window's center
          song.pause();
          wclear(this->lyrics_win);
          box(this->lyrics_win, 0, 0);
          wstandout(this->lyrics_win);
          std::string_view pause = "PAUSED";
          std::string_view resume = "(press any key to resume)";
          mvwaddstr(this->lyrics_win, height / 2, width / 2 - pause.length()/2, pause.data());
          wstandend(this->lyrics_win);
          mvwaddstr(this->lyrics_win, height / 2 + 1, width / 2 - resume.length()/2, resume.data());
          wrefresh(this->lyrics_win);
          // wait for a key to be pressed to resume
          c = wgetch(this->lyrics_win);
          // record duration of the pause
          pause_total += lrc_clock.now() - pause_start;
          continue;
        }
        // if the user pressed 's' the syncing is restarted
        if(c == 's') {
            // stop the song, clear the output and reset the index and line
            song.stop();
            this->lrc_text.clear();
            start = lrc_clock.now();
            idx = 0;
            next = this->lyrics[0];
            continue; // skip the rest of the iteration
        }

        // Another key has been pressed: register the time point of this line on
        // the output vector and then advance the index in the input vector

        // get the current time point
        current_tp = lrc_clock.now();
        // calculate the difference from the start point
        // FIXME: the operator '-' creates a duration, which can't be subtracted from a time point
        TimeSecs tm_diff = std::chrono::duration_cast<TimeSecs>(current_tp - start);
        TimeSecs tm_pause = std::chrono::duration_cast<TimeSecs>(
            pause_total - std::chrono::time_point<Clock>());
        TimeSecs elapsed_tm = tm_diff - tm_pause;
        // and write to the .lrc file a new line
        // formatted as [mm:ss.centsecond]<line text>
        std::string str_timestamp = "["
            + std::to_string((elapsed_tm.count() / 60) % 60) + ":"
            + std::to_string(elapsed_tm.count() % 60) + ".00]";
        this->lrc_text.push_back(str_timestamp);
        // get the next line
        idx++;
    }

    // sync done, the song stops
    song.stop();
    wclear(this->lyrics_win);
    mvwprintw(this->lyrics_win, 0, 0, "syncing done\n");
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
        wclear(this->lyrics_win);
        wrefresh(this->menu);
        wrefresh(this->lyrics_win);
    }

    delwin(this->menu);
    delwin(this->lyrics_win);
    // refreshes the standard screen
    refresh();
}
