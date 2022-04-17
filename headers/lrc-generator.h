#ifndef LRC_GEN_INCLUDED
#define LRC_GEN_INCLUDED

// my headers
#include "line.h"
#include "spsc.hpp"
// std lib headers
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <mutex>

// Version definition
#define VERSION "0.1.1"

namespace fs = std::filesystem;
using std::string_view;
using std::vector;

enum GeneratorState {
    MENU,
    SYNCING,
    PREVIEW,
    METADATA,
    QUITTING
};

// This class represents the data model for the generator
class Lrc_generator {
private:
    // the generator's state
    GeneratorState state;

    // the output text stream to write to
    std::ofstream output_stream;

    // the song's lyrics
    vector<string> lyrics;

    // the synchronized output lines
    vector<Line> lrc_lines;

    // metadata to be written at the top of the output file
    vector<string> metadata;

    // music stream file path
    fs::path songfile;

    float volume;

    // interactively sync the lyrics to the song
    void sync(
        Spsc_queue<int>& key_q,
        Spsc_queue<vector<string>>& content_q);
    // preview the sycnhronized lyrics (iff already sync'd)
    void preview_lrc(
        Spsc_queue<int>& key_q,
        Spsc_queue<vector<string>>& content_q);
    // Sets metadata
    void set_metadata(
        Spsc_queue<int>& key_q,
        Spsc_queue<vector<string>>& content_q);

public:
    // interactive menu (tui) used for setting parameters and syncing
    void run(
        Spsc_queue<int>& key_q, 
        Spsc_queue<vector<string>>& content_q, 
        Spsc_queue<vector<std::tuple<string, string>>>& menu_q);

    bool is_running(void);
    GeneratorState getState(void) {
        return this->state;
    }

    // constructor taking an input file and an output file
    Lrc_generator(fs::path &in_file, fs::path &out_file, fs::path &song_fname);
    ~Lrc_generator();
};

#endif
