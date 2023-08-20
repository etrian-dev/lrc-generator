// header file for the generator class
#include "lrc-generator.h"
// header file for arg parsing
#include "CLI/CLI.hpp"
// logging library
#include "loguru.hpp"
// curses library
#include <ncurses.h>
// other standard lib headers
#include <filesystem>
#include <iostream>
#include <string>
#include <tuple>

namespace fs = std::filesystem;

using std::string;

// class to initialize the ncurses library and do cleanup respectively
struct ncurses_initor
{
  ncurses_initor()
  {
    initscr();            // start curses
    cbreak();             // disable line buffering
    noecho();             // disable input echo
    keypad(stdscr, true); // enable the keypad and fn keys

    refresh();
  }

  ~ncurses_initor()
  {
    endwin();
  }
};

struct files_type
{
  string audio_fname;
  string lyrics_fname;
  string lrc_fname;
};

static files_type
parse_args(int argc, char **argv) {
  std::ios::sync_with_stdio(false);

  CLI::App app{"Lrc generator\n\n" "A simple TUI to generate .lrc files"};

  string audio_fname, lyrics_fname, lrc_fname;
  string log_fname;

  app.set_version_flag("-v,--version", VERSION, "Prints the version number");
  // If audio_fname is empty, Lrc_generator won't load it. This will be
  // improved in the future.
  app.add_option("-a,--audio-file", audio_fname, "Input audio file")
    ->check(CLI::ExistingFile);
  app.add_option("-l,--lyrics-file", lyrics_fname, "Input lyrics file")
    ->required()
    ->check(CLI::ExistingFile);
  app.add_option("-o,--output", lrc_fname, "Output lrc file to be written")
    ->required();
  app.add_option("--log-file", log_fname, "Log file");

  try {
    app.parse(argc, argv);
  } catch (const CLI::ParseError &e) {
    std::exit(app.exit(e));
  }

  if (!log_fname.empty()) {
    loguru::add_file(log_fname.c_str(), loguru::Truncate,
                     loguru::Verbosity_INFO);
  }

  return {audio_fname, lyrics_fname, lrc_fname};
}

int
main(int argc, char **argv) {
  std::ios::sync_with_stdio(false);

  // Most of the program's lifetime will be spent in ncurses. Ncurses handles IO
  // on its own so loguru could interfere with it. Logging to file should be
  // prefered.
  loguru::g_stderr_verbosity = loguru::Verbosity_OFF;

  // Loguru handles the -v command line flag by default. This is unwanted
  // because it interferes with CLI11's command line parsing (there shouldn't be
  // two command line parsers at once) and stderr logging is disabled anyway.
  loguru::Options loguru_opts;
  loguru_opts.verbosity_flag = nullptr;

  loguru::init(argc, argv, loguru_opts);

  files_type files = parse_args(argc, argv);

  fs::path audio_path = fs::path(files.audio_fname);
  fs::path lyrics_path = fs::path(files.lyrics_fname);
  fs::path lrc_path = fs::path(files.lrc_fname);

  // Lrc_generator is a simple class to generate the lrc file (includes a tui)
  LOG_F(INFO, "Arguments summary:");
  LOG_F(INFO, "Lyrics file: %s", lyrics_path.c_str());
  LOG_F(INFO, "Audio file: %s", audio_path.c_str());
  LOG_F(INFO, "Output file: %s", lrc_path.c_str());

  // Instantiates the generator and tries to create output & input streams
  // This is better done before the initialization of curses, so that the
  // terminal does not get garbled by ncurses
  Lrc_generator generator(lyrics_path, lrc_path, audio_path);

  // initialize the curses library for immediate input and keypad enabled
  ncurses_initor ncur;

  generator.run(); // main program loop (menu)

  return 0;
}
