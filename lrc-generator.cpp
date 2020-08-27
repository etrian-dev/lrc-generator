// .lrc generator for songs's lyrics
#include <iostream>
#include <iomanip>  // for streams formatting
#include <fstream>
#include <string>
#include <cstdlib>
#include <vector>
#include <chrono>   // for time utilities
#include <ratio>
#include <utility>  // for std::move used to move-construct a stream
#include <limits>   // for a stream's max size

// simple class as a wrapper for routines for setting up
// the .lrc file
class Lrc_generator {
    private:
        std::ofstream lrc_file;
        std::string title;
        std::vector<std::string> lyrics;

        // set title
        // defaults to argv[1] (without extension) if not set
        void set_title(void);

        // set artist
        void set_artist(void);
        
        // set album
        void set_album(void);
        
        // set lrc creator
        void set_creator(void);
        
        // set song lenght
        // void set_lenght(std::ofstream &lrc_file);
        
        // the actual syncing
        void start_sync(void);
        
    public:
        // menu for setting various parameters
        void menu(void);
        
        Lrc_generator(std::string &name, std::vector<std::string> &text);
};

void Lrc_generator::set_title(void) {
    bool not_ok = true;
    std::string c;
    // skips characters up to and including newline
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    while(not_ok) {
        system("clear");
        std::cout << "Song title [defaults to input filename]: ";
        std::getline(std::cin, title);
        std::cout << "Is " << title << " ok? [y/n]";
        std::getline(std::cin, c);
        if(c.compare("y") == 0) {
            not_ok = false;
        }
    }
    
    // now write it to the file
    lrc_file << "[ti: " << title << "]\n";
}

void Lrc_generator::set_artist(void) {
    bool not_ok = true;
    std::string c;
    std::string artist;
    // skips characters up to and including newline
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    while(not_ok) {
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
    lrc_file << "[ar: " << artist << "]\n";
}

void Lrc_generator::set_album(void) {
    bool not_ok = true;
    std::string c;
    std::string album;
    // skips characters up to and including newline
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    while(not_ok) {
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
    lrc_file << "[al: " << album << "]\n";
}

void Lrc_generator::set_creator(void) {
    bool not_ok = true;
    std::string c;
    std::string creator;
    // skips characters up to and including newline
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    while(not_ok) {
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
    lrc_file << "[by: " << creator << "]\n";
}

void Lrc_generator::menu(void) {
    bool cont = true;
    int action;
    while(cont) {
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
            case 0: start_sync(); break;
            case 1: set_title(); break;
            case 2: set_artist(); break;
            case 3: set_album(); break;
            case 4: set_creator(); break;
            default: cont = false;
        }
    }

    // closes the file, otherwise it can't be displayed
    lrc_file.close();
}

void Lrc_generator::start_sync(void) {
    // now the real syncing begins

    // clock with system time, used to find timepoints and write out in the lrc file
    std::chrono::system_clock lrc_clock;
    // start timer and stores incremental timepoints
    std::chrono::time_point<std::chrono::system_clock> start = lrc_clock.now(), this_point;
    // stores a duration in seconds
    std::chrono::duration<int> time_secs;

    std::string prev, curr, next;
    std::string c;
    unsigned int i = 0;
    
    // read vector line by line
    while(i < lyrics.size()) {
        // clear screen (not portable)
        system("clear");
        
        // assigns to the current line entry i of the vector
        curr = lyrics[i];
        // and i+1 to next if possible
        if(i+1 < lyrics.size()) {
            next = lyrics[i+1];
        }
        else {
            next.clear();
        }
        
        // display info in terminal about current, previous and next line
        std::cout   << "prev line: " << (prev.empty() ? "(null)" : prev)
                    << "\ncurr line: " << curr
                    << "\nnext line: " << (next.empty() ? "(null)" : next) << "\n";
        
        // wait until enter is pressed
        std::cout << "press enter...\n";
        // gets line buffer from cin
        //std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        getline(std::cin, c);
        
        // get current time point
        this_point = lrc_clock.now();
        // calculate the time difference from the start of the clock (in milliseconds)
        time_secs =     std::chrono::duration_cast<std::chrono::seconds>
                    (this_point - start);
        
        // and write to the lrc file a new line
        // formatted as [mm:ss.centsecond]line
        lrc_file.fill('0'); // set fill character
        lrc_file    << "[" 
                    << std::setw(2) << (time_secs.count() / 60) % 60 << ":" 
                    << std::setw(2) << time_secs.count() % 60 << "." 
                    << std::setw(2) << 0 << "]" 
                    << curr << "\n";
        // also to stdout
        std::cout.fill('0');
        std::cout   << "["
                    << std::setw(2) << (time_secs.count() / 60) % 60 << ":" 
                    << std::setw(2) << time_secs.count() % 60 << "." 
                    << std::setw(2) << 0 << "]" 
                    << curr << "\n";
        // then update the previous line and index
        prev = curr;
        i++;
    }
    // sync done
    std::cout << "Syncing done\n";
}

Lrc_generator::Lrc_generator(std::string &name, std::vector<std::string> &text) {
    // creates and opens a file with filename name
    lrc_file.open(name);
    if(!lrc_file.is_open()) {
        std::cout << "Cannot open file \"" << name << "\"\n";
        return;
    }
    // sets the title to the filename, without extension
    title = name;
    title.erase(title.size() - 4, 4);
    // stores the lyrics in the private variable lyrics
    lyrics = text;
}

int main(int argc, char** argv) {
    if(argc > 1) { // check if arguments have been given to the program
        
        // open the lyrics file
        std::ifstream in_lyrics(argv[1]);
        if(!in_lyrics.is_open()) {
            std::cout << "Cannot open file " << argv[1] << ".\n";
            return 1;
        }
        
        /* 
         * file opened successfully
         * file is read line by line and stored in 
         * an array of strings for ease of access
         */
        std::vector<std::string> text;
        std::string line;
        while(std::getline(in_lyrics, line)) {
            text.push_back(line);
        }
        
        // now the file can be closed
        in_lyrics.close();
        
        // the filename of the lrc file is the same as the lyrics file
        std::string lrc_filename = argv[1];
        lrc_filename.replace(lrc_filename.size() - 3, 3, "lrc");
        
        // a simple class implements a menu for setting up the lrc file
        Lrc_generator factory(lrc_filename, text);
        factory.menu(); // main program loop (menu)
    }
    else {
        std::cout << "No lyrics provided\nUsage: lrc-generator [LYRICS.txt]\n";
    }
    return 0;
}
