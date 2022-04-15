// my headers
#include "../headers/lrc-generator.h"
#include "../headers/spsc.hpp"
#include "../headers/line.h"
// SFML headers for music playback
#include <SFML/Audio.hpp>
// curses library
#include <ncurses.h>
// standard lib headers
#include <algorithm>
#include <cassert>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip> // streams formatting functions
#include <iostream>
#include <limits> // to obtain a stream's max size
#include <ratio>
#include <string>
#include <thread>
#include <utility> // to use std::move, used to move-construct a stream

namespace ns = std::filesystem;
using std::string;
using std::vector;

// define more practical names for std::chrono things
using Clock = std::chrono::system_clock;
using MilliSecs = std::chrono::milliseconds;

// constructor taking an input and an output filenames as std::string
Lrc_generator::Lrc_generator(fs::path &in_file, fs::path &out_file,
                             fs::path &song_path) {
    // initialize the generator's state to "MENU"
    this->state = GeneratorState.MENU;

    // open an input and an output stream with the filenames specified
    std::ifstream input_stream = std::ifstream(in_file, std::ios_base::in);
    this->output_stream = std::ofstream(out_file, std::ios_base::out);
    if (!input_stream.is_open()) {
        std::cerr << "Error opening the input stream on file \"" << in_file
                  << "\".\n";
        exit(1);
    }
    if (!this->output_stream.is_open()) {
        std::cerr << "Error opening the input stream on file \"" << out_file
                  << "\".\n";
        exit(1);
    }

    while (input_stream.good()) {
        string line;
        std::getline(input_stream, line);
        if (line.empty()) {
            continue;
        }
        this->lrc_lines.push_back(Line(line));
    }

    this->metadata = vector<string>();

    this->songfile = song_path;

    this->menu_mutex = std::mutex();

    if (input_stream.eof()) {
        input_stream.close();
    }
}

Lrc_generator::~Lrc_generator() {
    size_t i;
    // write the metadata (if any) first
    for (auto ln : this->metadata) {
        this->output_stream << ln << '\n';
    }
    for (i = 0; i < this->lrc_text.size(); i++) {
        // first append the time point
        this->output_stream << this->lrc_text[i] << this->lyrics[i] << '\n';
    }
    this->output_stream.close();
}

bool Lrc_generator::is_running(void) {
    return this->state != GeneratorState.QUITTING;
}

vector<std::tuple<string, string>> main_menu {
    std::make_tuple("0"s, "sync"s),
    std::make_tuple("1"s, "preview"s),
    std::make_tuple("1"s, "preview"s),
    std::make_tuple("2"s, "set title"s),
    std::make_tuple("3"s, "set album"s),
    std::make_tuple("4"s, "set artist"s),
    std::make_tuple("5"s, "set subtitle creator"s)
};

vector<std::tuple<string, string>> Lrc_generator::send_menu_items(void) {
    switch (this->state) {
    case GeneratorState.MENU:
        return main_menu;
    case GeneratorState.SYNCING:
    case GeneratorState.PREWIEW:
    case GeneratorState.METADATA:
    case GeneratorState.QUITTING:
    default:
        return nullptr;
    }
}

// function to sync the lyrics to the song
void Lrc_generator::sync(void) {
    Clock clock;
    std::chrono::time_point<Clock> line_start_tp, current_tp;
    // this duration object stores the time spent in song playback
    // (accounting for pauses).
    // Its value is written on the lrc file when the user marks the
    // beginning of a new line
    MilliSecs tot_playback = MilliSecs::zero();

    // load the song in a sf::Music object
    // it's a stream, so it must not be destroyed as long as it's being played
    // supported formats are those listed at
    // https://www.sfml-dev.org/tutorials/2.5/audio-sounds.php
    sf::Music song;
    if (!song.openFromFile(this->songfile.string())) {
        std::cerr << "Failed to open the song file at \"" << this->songfile
                  << "\"\n";
        return;
    }

    // current previous and next line in the lyrics
    string prev;
    string current;
    string next;
    // line indices
    unsigned int tot_lines = this->lyrics.size();
    unsigned int idx = 0;

    // THE SONG (IF LOADED) STARTS PLAYING
    // does not loop when the end is reached by default
    song.play();
    this->volume = song.getVolume();
    // stores the initial timepoint
    line_start_tp = clock.now();

    while (idx < tot_lines) {
        if (idx > 0) {
            prev = current;
        }
        current = this->lyrics[idx];
        if (idx < tot_lines - 1) {
            next = this->lyrics[idx + 1];
        } else {
            next.erase();
        }

        // Synchronize the current line
        // formatted as [mm:ss.centsecond]<line>
        this->lrc_lines.push_back(new Line(current, tot_playback.count()));
        /*string str_timestamp =
            "[" + std::to_string((tot_playback.count() / 60000) % 60000) + ":" +
            std::to_string((tot_playback.count() / 1000) % 60) + "." +
            std::to_string((tot_playback.count() / 10) % 100) + "]";
        this->lrc_text.push_back(str_timestamp);
        this->delays.push_back(tot_playback.count());*/


        int c;
        if (c == KEY_UP) {
            this->volume = std::min(100.0f, this->volume + volume_step);
            song.setVolume(this->volume);
            this->volume = song.getVolume();
            current = prev;
            continue;
        }
        if (c == KEY_DOWN) {
            this->volume = std::max(0.0f, this->volume - volume_step);
            song.setVolume(this->volume);
            this->volume = song.getVolume();
            current = prev;
            continue;
        }

        if (c == ' ') {
            // Pause the synchronization

            // stores the time passed from the last timestamp up to now
            tot_playback +=
                std::chrono::duration_cast<MilliSecs>(clock.now() - line_start_tp);
            // pause the audio track and
            song.pause();

            // the time point at the end of a pause is the new starting point for the
            // line
            line_start_tp = clock.now();
            // A little trick: ensure that at the next iteration a line is not skipped
            current = prev;
            continue; // to avoid recording a timestamp immediately
        }

        if (c == 's') {
            // Restart sychronization

            // stop the song, clear the output and reset the index and line
            song.stop();
            this->lrc_text.clear();
            this->delays.clear();
            // reset the starting clock and the song duration offset
            tot_playback = MilliSecs::zero();
            idx = 0;
            prev.erase();
            song.play(); // restart playing the song
            line_start_tp = clock.now();
            continue; // to avoid recording a timestamp immediately
        }

        // save the current time point
        current_tp = clock.now();
        // calculate the duration of the line just finished
        tot_playback +=
            std::chrono::duration_cast<MilliSecs>(current_tp - line_start_tp);

        // the starting timepoint for the new line is the one saved above
        line_start_tp = current_tp;

        idx++;
    }

    // sync done, the song stops
    song.stop();
    wclear(this->lyrics_win);
    mvwprintw(this->lyrics_win, 0, 0, "Synchronization DONE\n");
}

// previews the synchronized lyrics
void Lrc_generator::preview_lrc(void) {
    // offsets from the window border
    const int hoff = 2;
    const int woff = 2;
    int height, width;
    getmaxyx(this->lyrics_win, height, width);

    if (this->lrc_text.empty()) {
        char choice =
            choice_dialog("Song not synchronized yet. Start synchronization?");
        wclear(this->menu);
        draw_menu();
        wrefresh(this->menu);
        if (choice == 'y') {
            sync();
        }
        return;
    }

    sf::Music song;
    if (!song.openFromFile(this->songfile.string())) {
        return;
    }
    song.play();

    wattron(this->lyrics_win, A_BOLD);
    string curr;
    for (size_t i = 0; i < this->delays.size() - 1; i++) {
        curr = this->lyrics[i];

        wclear(this->lyrics_win);
        box(this->lyrics_win, 0, 0);
        // print lines
        mvwprintw(this->lyrics_win, hoff, woff, "PREVIEW \"%s\"",
                  this->songfile.filename().c_str());
        mvwprintw(this->lyrics_win, hoff + 1, woff, "%d: %s", i + 1, curr.c_str());
        wrefresh(this->lyrics_win);
        MilliSecs dur = MilliSecs(this->delays[i + 1] - this->delays[i]);
        std::this_thread::sleep_for(dur);
    }
    wclear(this->lyrics_win);
    box(this->lyrics_win, 0, 0);
    // print lines
    mvwprintw(this->lyrics_win, hoff + 1, woff, "%d: %s", this->lyrics.size(),
              this->lyrics[this->lyrics.size() - 1].c_str());
    wattroff(this->lyrics_win, A_BOLD);
    mvwaddstr(this->lyrics_win, hoff + 2, woff, "END (press any key to quit)");
    wrefresh(this->lyrics_win);

    song.stop();

    int x = wgetch(this->lyrics_win);
}

// the menu loop presented by the class to the user
void Lrc_generator::run(Spsc_queue<int>& key_q, Spsc_queue<float>& vol_q, Spsc_queue<vector<string>>& content_q, Spsc_queue<std::tuple<string, string>>>& menu_q) {
    // setup the interface
    interface_setup();

    bool cont = true;
    int action;
    while (cont) {
        // draws the menu
        draw_menu();
        // gets a character from the menu window and triggers the action accordingly
        action = wgetch(this->menu);
        switch (action - '0') {
        case 0:
            sync();
            break;
        case 1:
            preview_lrc();
            break;
        case 2:
            set_attr_dialog("Song title");
            break;
        case 3:
            set_attr_dialog("Song artist");
            break;
        case 4:
            set_attr_dialog("Song album");
            break;
        case 5:
            set_attr_dialog("Lrc creator");
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
