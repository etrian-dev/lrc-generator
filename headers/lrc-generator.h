#ifndef LRC_GEN_INCLUDED
#define LRC_GEN_INCLUDED

// my headers
#include "line.h"
#include <SFML/Audio.hpp>
// std lib headers
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
// ncurses header
#include <ncurses.h>

// Version definition
#define VERSION "0.1.1"

namespace fs = std::filesystem;
using std::string;
using std::vector;

// simple class as a wrapper for routines for setting up
// the .lrc file
class Lrc_generator {
private:
  // the output text stream to write to
  std::ofstream output_stream;

  // the song's text
  vector<string> lyrics;

  // metadata to be written at the top of the output file
  vector<string> metadata;
  // Synchronized to be written to the output file
  vector<string> lrc_text;
  vector<uint_fast64_t> delays;

  // music stream filename
  fs::path songfile;
  sf::Music song;

  // interactively sync the lyrics to the song
  void sync(void);
  // preview the sycnhronized lyrics (iff the function above has been already
  // run)
  void preview_lrc(void);

  // TUI functions & variables
  WINDOW *menu;
  WINDOW *lyrics_win;
  int height;
  int width;
  void interface_setup(void);
  void render_win(WINDOW *win,  vector<string>& content, vector<attr_t>& style);
  // utility function to draw the menu
  void draw_menu(void);
  // creates a dialog to set the chosen attribute
  void set_attr_dialog(string msg, string attr);
  // displays a simple choiche dialog
  char choice_dialog(string msg);

public:
  // interactive menu (tui) used for setting parameters and syncing
  void run(void);

  // constructor taking an input file and an output file
  Lrc_generator(fs::path &in_file, fs::path &out_file, fs::path &song_fname);
  ~Lrc_generator();
};

#endif
