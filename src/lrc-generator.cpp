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
using namespace std::string_literals;
using std::string;
using std::vector;

// define more practical names for std::chrono things
using Clock = std::chrono::system_clock;
using MilliSecs = std::chrono::milliseconds;

// constructor taking an input and an output filenames as std::string
Lrc_generator::Lrc_generator(fs::path &in_file, fs::path &out_file,
                             fs::path &song_path) {

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
        this->lyrics.push_back(line);
    }

    this->metadata = vector<string>();

    // TODO: get Music's lenght with sf::Music.getDuration().asSecods()

    this->songfile = song_path;

    if (input_stream.eof()) {
        input_stream.close();
    }

    // initialize the generator's state to "MENU"
    this->state = GeneratorState::MENU;
}

Lrc_generator::~Lrc_generator() {
    // write the metadata (if any) first
    for (auto ln : this->metadata) {
        this->output_stream << "[" << ln << "]\n";
    }
    for(auto line : this->lrc_lines) {
        string str_timestamp =
            "[" + std::to_string((line.get_delay() / 60000) % 60000) + ":" +
            std::to_string((line.get_delay() / 1000) % 60) + "." +
            std::to_string((line.get_delay() / 10) % 100) + "]";
        this->output_stream << str_timestamp << line.get_text() << "\n";
    }
    this->output_stream.close();
}

bool Lrc_generator::is_running(void) {
    return this->state != GeneratorState::QUITTING;
}

vector<string> pause_resume {
    "PAUSED"s,
    "(press any key to resume)"s
};

// function to sync the lyrics to the song
void Lrc_generator::sync(
    Spsc_queue<int>& key_q,
    Spsc_queue<vector<string>>& content_q) {

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

    const size_t slider_sz = 50;
    const float volume_step = 2.0f;
    string vol_str = "volume: "s;
    string vol_slider = string(slider_sz, '-');
    vol_slider.insert(0, 1, '[');
    vol_slider.push_back(']');

    vector<string> content;

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
        this->lrc_lines.push_back(Line(current, tot_playback.count()));
        /*string str_timestamp =
            "[" + std::to_string((tot_playback.count() / 60000) % 60000) + ":" +
            std::to_string((tot_playback.count() / 1000) % 60) + "." +
            std::to_string((tot_playback.count() / 10) % 100) + "]";
        this->lrc_text.push_back(str_timestamp);
        this->delays.push_back(tot_playback.count());*/

        // Update the volume slider
        size_t vol_level = (size_t)std::max(0.0, this->volume / 2.0);
        vol_slider.replace(1, slider_sz, slider_sz, '-');
        vol_slider.replace(1, vol_level, vol_level, '#');

        // Send content lines
        content.push_back(prev);
        content.push_back(current);
        content.push_back(next);
        content.push_back(vol_str + std::to_string(this->volume));
        content.push_back(vol_slider);
        content_q.produce(content);
        content.clear(); // clear it

        int c = key_q.consume();
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

            content_q.produce(pause_resume);
            c = key_q.consume();

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
            this->lrc_lines.clear();
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
}

// previews the synchronized lyrics
void Lrc_generator::preview_lrc(
    Spsc_queue<int>& key_q,
    Spsc_queue<vector<string>>& content_q
) {
    /*
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
    */
    return;
}

void Lrc_generator::set_metadata(
    Spsc_queue<int>& key_q,
    Spsc_queue<vector<string>>& content_q) {

    // Sets metadata fields
    // available fields: ti, al, ar, by, length

    bool keep_going = true;
    int action;
    // contains "set *" prompt and the current string typed by the user
    vector<string> message;
    do {
        message.clear();
        string metadata = "";
        action = key_q.consume();
        switch(action - '0') {
        case 0:
            // set title
            message.push_back("Set song title: "s);
            content_q.produce(message);
            metadata += "ti: ";
            break;
        case 1:
            // set album
            message.push_back("Set song album: "s);
            content_q.produce(message);
            metadata += "al: ";
            break;
        case 2:
            // set artist
            message.push_back("Set song artist: "s);
            content_q.produce(message);
            metadata += "ar: ";
            break;
        case 3:
            // set subtitle creator
            message.push_back("Set subtitle file creator: "s);
            content_q.produce(message);
            metadata += "by: ";
            break;
        default:
            keep_going = false;
        }
        if (keep_going) {
            // get character from key key queue
            while((action = key_q.consume()) != '\n') {
                metadata += action;
                // echo the partial metadata back to the user
                // TODO: improve this, maybe do a little local work in the
                // interface to avoid allocations and
                // traffic on the queue for each character
                message.push_back(metadata.substr(3, std::string::npos));
                content_q.produce(message);
                message.pop_back();
            }
            this->metadata.push_back(metadata);
        }
    } while(keep_going);
}

vector<std::tuple<string, string>> main_menu {
    std::make_tuple("0"s, "sync"s),
    std::make_tuple("1"s, "preview"s),
    std::make_tuple("2"s, "set metadata"s),
    std::make_tuple("other keys"s, "quit"s)

};

vector<std::tuple<string, string>> sync_menu {
    std::make_tuple("\'' \'"s, "pause"s),
    std::make_tuple("KEY_UP"s, "volume up"s),
    std::make_tuple("KEY_DOWN"s, "volume down"s),
    std::make_tuple("s"s, "restart"s),
    std::make_tuple("other keys"s, "set timestamp"s)
};

vector<std::tuple<string, string>> preview_menu {
    std::make_tuple("\'' \'"s, "pause"s),
    std::make_tuple("s"s, "restart"s),
};

vector<std::tuple<string, string>> metadata_menu {
    std::make_tuple("0"s, "set title"s),
    std::make_tuple("1"s, "set album"s),
    std::make_tuple("2"s, "set artist"s),
    std::make_tuple("3"s, "set subtitle creator"s),
    std::make_tuple("other keys"s, "return to main menu")
};


// the menu loop presented by the class to the user
void Lrc_generator::run(
    Spsc_queue<int>& key_q,
    Spsc_queue<vector<string>>& content_q,
    Spsc_queue<vector<std::tuple<string, string>>>& menu_q) {

    this->state = GeneratorState::MENU;

    int action;
    while (this->state != GeneratorState::QUITTING) {
        if(this->state == GeneratorState::MENU) {
            menu_q.produce(main_menu);

            // Gets an action from the interface
            action = key_q.consume();

            std::cout << "action = " << action << "\n";

            switch (action - '0') {
            case 0:
                // Set the state to syncing, update menu items
                this->state = GeneratorState::SYNCING;
                menu_q.produce(sync_menu);
                sync(key_q, content_q);
                this->state = GeneratorState::MENU;
                break;
            case 1:
                // Set the state to preview, update menu items
                this->state = GeneratorState::PREVIEW;
                menu_q.produce(preview_menu);
                preview_lrc(key_q, content_q);
                this->state = GeneratorState::MENU;
                break;
            case 2:
                // Set the state to metadata, update menu items
                this->state = GeneratorState::METADATA;
                menu_q.produce(metadata_menu);
                set_metadata(key_q, content_q);
                this->state = GeneratorState::MENU;
                break;
            default:
                // quit the program
                this->state = GeneratorState::QUITTING;
            }
        }
    }
    std::cout << "Generator thread quits\n";
}
