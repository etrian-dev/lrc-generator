// header file for the generator class
#include "lrc-generator.h"
// SFML headers for music playback
#include <SFML/Audio.hpp>
// standard lib headers
#include <iostream>
#include <iomanip>  // for streams formatting
#include <fstream>
#include <string>
#include <cstdlib>
#include <chrono>   // chrono utilities
#include <ratio>
#include <limits>   // for a stream's max size
#include <utility>  // for std::move, used to move-construct a stream

// constructor taking an input and an output filenames as std::string
Lrc_generator::Lrc_generator(std::string& in_file, std::string& out_file, std::string& song_fname) {
    // open an input and an output stream with the filenames specified
    this->input_stream = std::ifstream(in_file, std::ios_base::in);
    this->output_stream = std::ofstream(out_file, std::ios_base::out);
    if(!this->input_stream.is_open()) {
        std::cerr << "Error opening the input stream on file \"" << in_file << "\".\n";
        exit(1);
    }
    if(!this->output_stream.is_open()) {
        std::cerr << "Error opening the input stream on file \"" << out_file << "\".\n";
        exit(1);
    }

    this->songfile = song_fname;
}
// constructor taking an input and an output stream
Lrc_generator::Lrc_generator(std::ifstream& in_stream, std::ofstream& out_stream, std::string& song_fname) {
    // move assign the streams to the instance variables
    this->input_stream = std::move(in_stream);
    this->output_stream = std::move(out_stream);

    this->songfile = song_fname;
}

// function to interactively set the song's title
void Lrc_generator::set_title(void) {
    std::string title;
    bool not_ok = true;
    std::string c;
    // skips characters up to and including newline
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    while(not_ok) {
        /// TODO:should be replaced with some ncurses function
        #ifdef DEBUG_MODE
          system("clear");
        #endif
        std::cout << "Song title: ";
        std::getline(std::cin, title);
        std::cout << "Is " << title << " ok? [y/n]";
        std::getline(std::cin, c);
        if(c.compare("y") == 0) {
            not_ok = false;
        }
    }

    // now write it to the file
    this->output_stream << "[ti: " << title << "]\n";
}

// function to interactively set the song's artist
void Lrc_generator::set_artist(void) {
    bool not_ok = true;
    std::string c;
    std::string artist;
    // skips characters up to and including newline
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    while(not_ok) {
        /// TODO:should be replaced with some ncurses function
        #ifdef DEBUG_MODE
          system("clear");
        #endif
        std::cout << "Song artist: ";
        std::getline(std::cin, artist);
        std::cout << "Is " << artist << " ok? [y/n]";
        std::getline(std::cin, c);
        if(c.compare("y") == 0) {
            not_ok = false;
        }
    }

    // now write it to the file
    this->output_stream << "[ar: " << artist << "]\n";
}

// function to interactively set the song's album
void Lrc_generator::set_album(void) {
    bool not_ok = true;
    std::string c;
    std::string album;
    // skips characters up to and including newline
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    while(not_ok) {
        /// TODO:should be replaced with some ncurses function
        #ifdef DEBUG_MODE
          system("clear");
        #endif
        std::cout << "Song album: ";
        std::getline(std::cin, album);
        std::cout << "Is " << album << " ok? [y/n]";
        std::getline(std::cin, c);
        if(c.compare("y") == 0) {
            not_ok = false;
        }
    }

    // now write it to the file
    this->output_stream << "[al: " << album << "]\n";
}

// function to interactively set the .lrc file creator
void Lrc_generator::set_creator(void) {
    bool not_ok = true;
    std::string c;
    std::string creator;
    // skips characters up to and including newline
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    while(not_ok) {
        /// TODO:should be replaced with some ncurses function
        #ifdef DEBUG_MODE
          system("clear");
        #endif
        std::cout << ".lcr file creator: ";
        std::getline(std::cin, creator);
        std::cout << "Is " << creator << " ok? [y/n]";
        std::getline(std::cin, c);
        if(c.compare("y") == 0) {
            not_ok = false;
        }
    }

    // now write it to the file
    this->output_stream << "[by: " << creator << "]\n";
}

// function to sync the lyrics to the song
void Lrc_generator::sync(void) {
    // uses system clock to compare timepoints and print elapsed time
    std::chrono::system_clock lrc_clock;
    // variables holding the current and initial timepoints
    std::chrono::time_point<std::chrono::system_clock> this_point, start;
    // stores a duration in seconds
    std::chrono::duration<int> time_secs;

    // lines are stored in this variables, c is just a dummy variable to signal the
    // enter key being pressed
    std::string current;
    std::string next;
    std::string c;

    // load the song in a sf::Music object
    // it's a stream, so it must not be destroyed as long as it's being played
    sf::Music song;
    if(!song.openFromFile(this->songfile)) {
      std::cerr << "Failed to open the song at \"" << this->songfile << "\"\n";
    }

    // stores the initial timepoint
    start = lrc_clock.now();

    // discards characters until newline from cin
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    // THE SONG (IF LOADED) STARTS PLAYING
    // does not loop when the end is reached by default
    song.play();

    // as the user presses enter write the corresponding time difference to the file
    while(std::getline(this->input_stream, next)) {
        /// TODO:should be replaced with some ncurses function
        #ifdef DEBUG_MODE
          system("clear");
        #endif

        // display the current line being synced and the next one on stdout
        // when the current line finishes one can either wait for the next to begin
        // or press enter and make the next appear on screen (when reproduced)
        std::cout   << "Current:\t" << (current.empty() ? "(null)" : current)
                    << "\nNext:\t" << next << "\n";

        // wait until enter is pressed
        std::cout << "press enter...\n";
        // gets a line, just for the sake of detecting the key press
        getline(std::cin, c);

        // get the current time point
        this_point = lrc_clock.now();
        // calculate the difference from the start point
        time_secs = std::chrono::duration_cast<std::chrono::seconds>(this_point - start);
        // and write to the .lrc file a new line
        // formatted as [mm:ss.centsecond]<line text>
        this->output_stream.fill('0'); // set the fill character for unspecified fields
        this->output_stream    << "["
                    << std::setw(2) << (time_secs.count() / 60) % 60 << ":"
                    << std::setw(2) << time_secs.count() % 60 << "."
                    << std::setw(2) << 0 << "]"
                    << next << "\n";

        // then update the previous line
        current = next;
    }

    // sync done, the song stops
    song.stop();
    std::cout << "Syncing done\n";
}

// the menu loop presented by the class to the user
void Lrc_generator::run(void) {
    bool cont = true;
    int action;
    while(cont) {
        /// TODO:should be replaced with some ncurses function
        #ifdef DEBUG_MODE
          system("clear");
        #endif
        std::cout   << "Menu:\n"
                << "\t0: start syncing\n"
                << "\t1: set title\n"
                << "\t2: set artist\n"
                << "\t3: set album\n"
                << "\t4: set creator\n"
                << "\t5: quit program\n";
        std::cin >> action;
        switch(action) {
            case 0: sync(); break;
            case 1: set_title(); break;
            case 2: set_artist(); break;
            case 3: set_album(); break;
            case 4: set_creator(); break;
            default: cont = false;
        }
    }

    // closes the streams
    this->input_stream.close();
    this->output_stream.close();
}
