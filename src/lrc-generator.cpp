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
using MilliSecs = std::chrono::milliseconds;

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

    this->metadata = std::vector<std::string>();

    this->songfile = song_path;

    if(input_stream.eof()) {
        input_stream.close();
    }
}

Lrc_generator::~Lrc_generator() {
    size_t i;
    // write the metadata (if any) first
    for(auto ln : this->metadata) {
        this->output_stream << ln << '\n';
    }
    for(i = 0; i < this->lrc_text.size(); i++) {
        // first append the time point
        this->output_stream << this->lrc_text[i] << this->lyrics[i] << '\n';
    }
    this->output_stream.close();
}

// function to sync the lyrics to the song
void Lrc_generator::sync(void)
{
    Clock clock;
    std::chrono::time_point<Clock> line_start_tp, current_tp;
    // this duration object stores the time spent in song playback (accounting for pauses)
    // Its value is written on the lrc file when the user marks the beginning of a new line
    MilliSecs tot_playback = MilliSecs::zero();
    // current and next line in the lyrics
    std::string current;
    std::string next;
    // dummy variable
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
    line_start_tp = clock.now();

    // THE SONG (IF LOADED) STARTS PLAYING
    // does not loop when the end is reached by default
    song.play();

    // offsets from the window border
    const int hoff = 2;
    const int woff = 2;
    int height, width;
    getmaxyx(this->lyrics_win, height, width);

    unsigned int tot_lines = this->lyrics.size();
    unsigned int idx = 0;
    next = this->lyrics[0]; // to initialize the loop correctly, since this line will become the current below
    while(idx < tot_lines - 1) {
        current = next;
        next = this->lyrics[idx + 1];

        wclear(this->lyrics_win);
        box(this->lyrics_win, 0, 0);
        // display the current line being synchronized and the next one on stdout
        // or print "(null)" if the line is empty
        mvwprintw(this->lyrics_win, hoff, woff, "Curr:\t%s", (current.empty() ? "(null)" : current.c_str()));
        mvwprintw(this->lyrics_win, hoff + 1, woff, "Next:\t%s", (next.empty() ? "(null)" : next.c_str()));
        mvwprintw(this->lyrics_win, hoff + 3, woff, "Last timestamp [%d.%d s]", tot_playback.count() / 1000, (tot_playback.count() / 10) % 100);
        // bottom command cheatsheet
        mvwaddstr(this->lyrics_win, height - 4, woff, "space: pause");
        mvwaddstr(this->lyrics_win, height - 3, woff, "'s: restart synchronization");
        mvwaddstr(this->lyrics_win, height - 2, woff, "any other key: advance to the next line");
        wrefresh(this->lyrics_win);
        
        // blocks until a character is pressed
        c = wgetch(this->lyrics_win);
       
        if (c == ' ') {
          // Pause the synchronization

          // stores the time passed from the last timestamp up to now
          tot_playback += std::chrono::duration_cast<MilliSecs>(clock.now() - line_start_tp);
          // pause the audio track and
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
          // waits for a key press to resume
          c = wgetch(this->lyrics_win);
          // the time point at the end of a pause is the new starting point for the line
          line_start_tp = clock.now();
          // A little trick: ensure that at the next iteration a line is not skipped
          next = current;
          continue; // to avoid recording a timestamp immediately
        }

        if(c == 's') {
            // Restart sychronization

            // stop the song, clear the output and reset the index and line
            song.stop();
            this->lrc_text.clear();
            // reset the starting clock and the song duration offset
            tot_playback = MilliSecs::zero();
            line_start_tp = clock.now();
            idx = 0;
            next = this->lyrics[0];
            continue; // to avoid recording a timestamp immediately
        }

        // Another key has been pressed: register the time point of this line on
        // the output vector and then advance the index in the input vector

        // save the current time point
        current_tp = clock.now();
        // calculate the duration of the line just finished
        tot_playback += std::chrono::duration_cast<MilliSecs>(current_tp - line_start_tp);
        // and push to the vector the correspoding timestamp
        // formatted as [mm:ss.centsecond]<line>
        std::string str_timestamp = "["
            + std::to_string((tot_playback.count() / 60000) % 60000) + ":"
            + std::to_string((tot_playback.count() / 1000) % 60) + "."
            + std::to_string((tot_playback.count() / 10) % 100) + "]";
        this->lrc_text.push_back(str_timestamp);
        // the starting timepoint for the new line is the one saved above
        line_start_tp = current_tp;
        
        idx++;
    }

    // sync done, the song stops
    song.stop();
    wclear(this->lyrics_win);
    mvwprintw(this->lyrics_win, 0, 0, "Synchronization DONE\n");
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
