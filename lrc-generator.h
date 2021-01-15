#ifndef LRC_GEN_INCLUDED
#define LRC_GEN_INCLUDED
// std lib headers
#include <fstream>
#include <string>

// simple class as a wrapper for routines for setting up
// the .lrc file
class Lrc_generator {
    private:
        // input and output text streams
		    std::ifstream input_stream;
        std::ofstream output_stream;

        // music stream filename
        std::string songfile;

        // interactively set title
        void set_title(void);
        // interactively set artist
        void set_artist(void);
        // interactively set album
        void set_album(void);
        // interactively set lrc creator
        void set_creator(void);
        // interactively set song lenght
        // void set_lenght(std::ofstream &lrc_file);
        // interactively sync the lyrics to the song
        void sync(void);
    public:
        // interactive menu loop used for setting parameters and syncing
        void run(void);

		// constructor taking an input file and an output file
        Lrc_generator(std::string& in_file, std::string& out_file, std::string& song_fname);
        // constructor taking an input stream and an output stream
        Lrc_generator(std::ifstream& in_stream, std::ofstream& out_stream, std::string& song_fname);
};

#endif
