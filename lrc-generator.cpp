// header file for the generator class
#include "lrc-generator.h"

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
Lrc_generator::Lrc_generator(std::string& in_file, std::string& out_file) {
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
}
// constructor taking an input and an output stream
Lrc_generator::Lrc_generator(std::ifstream& in_stream, std::ofstream& out_stream) {
    // move assign the streams to the instance variables
    this->input_stream = std::move(in_stream);
    this->output_stream = std::move(out_stream);
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
        system("clear");

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
        system("clear");

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
        system("clear");

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
        system("clear");

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
/// TODO: add support to actually play the song directly as well
void Lrc_generator::sync(void) {
    // uses system clock to compare timepoints and print elapsed time
    std::chrono::system_clock lrc_clock;
    // variables holding the current and initial timepoint
    std::chrono::time_point<std::chrono::system_clock> this_point, start;
    // stores a duration in seconds (default)
    std::chrono::duration<int> time_secs;

    // lines are stored here
    std::string prev;
    std::string current;
    std::string c;
    unsigned int i = 0;

    // stores the initial timepoint
    start = lrc_clock.now();

    // discards characters until newline from cin
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    
    // as the user presses enter write the corresponding time difference to the file
    while(std::getline(this->input_stream, current)) {
        /// TODO:should be replaced with some ncurses function
        system("clear");
        // display the previous, next and current line on stdout
        std::cout   << "previous:\t" << (prev.empty() ? "(null)" : prev)
                    << "\ncurrent:\t" << current << "\n";
        
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
                    << current << "\n";

        // then update the previous line
        prev = current;
    }
    // sync done
    std::cout << "Syncing done\n";
}

// the menu loop presented by the class to the user
void Lrc_generator::run(void) {
    bool cont = true;
    int action;
    while(cont) {
        /// TODO:should be replaced with some ncurses function
        system("clear");

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