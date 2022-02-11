#ifndef LRC_GEN_INCLUDED
#define LRC_GEN_INCLUDED
// std lib headers
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
// ncurses header
#include <ncurses.h>

namespace fs = std::filesystem;

// simple class as a wrapper for routines for setting up
// the .lrc file
class Lrc_generator
{
private:
    // the output text stream to write to
    std::ofstream output_stream;

    // the song's text
    std::vector<std::string> lyrics;

    // the text to be written to the output file
    std::vector<std::string> lrc_text;

    // music stream filename
    fs::path songfile;

    // interactively set song lenght
    // void set_lenght(std::ofstream &lrc_file);
    // interactively sync the lyrics to the song
    void sync(void);

    // TUI functions & variables
    WINDOW *menu;
    WINDOW *lyrics_win;
    int height;
    int width;
    void interface_setup(void);
    // utility function to draw the menu
    void draw_menu(void);
    // creates a dialog to set the chosen attribute
    void set_attr_dialog(std::string attr);

public:
    // interactive menu (tui) used for setting parameters and syncing
    void run(void);

    // constructor taking an input file and an output file
    Lrc_generator(fs::path &in_file, fs::path &out_file, fs::path &song_fname);
    ~Lrc_generator();
};

#endif
