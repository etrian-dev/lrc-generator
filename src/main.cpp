// header file for the generator class
#include "../headers/lrc-generator.h"
// header file for arg parsing
#include "../headers/cxxopts.hpp"
// logging library
#include "../headers/loguru.hpp"
// curses library
#include <ncurses.h>
// other standard lib headers
#include <filesystem>
#include <iostream>
#include <string>
#include <tuple>

namespace fs = std::filesystem;

using std::string;
using std::literals::string_literals::operator""s;

// functions to initialize the ncurses library and do cleanup respectively
void init_ncurses() {
    initscr();            // start curses
    cbreak();             // disable line buffering
    noecho();             // disable input echo
    keypad(stdscr, true); // enable the keypad and fn keys

    refresh();
}
void cleanup_ncurses(void) {
    endwin();
}

std::vector<string> parse_args(int argc, char **argv) {
    std::vector<string> files = std::vector<string>();

    cxxopts::Options all_opts("Lrc generator",
                              "A simple TUI to generate .lrc files");
    all_opts.add_options()("h,help", "Help message")("v,version",
            "Prints the version number")(
                "o,output", "Output file to be written", cxxopts::value<string>())(
                    "a,audio-file", "Input audio file", cxxopts::value<string>())(
                        "l,lyrics-file", "Input lyrics file", cxxopts::value<string>());

    auto res = all_opts.parse(argc, argv);
    if (res.count("help") > 0) {
        std::cout << all_opts.help() << "\n";
        return files;
    }
    if (res.count("version") > 0) {
        std::cout << argv[0] << ": " << VERSION << "\n";
        return files;
    }
    string audio_fname = string();
    string lyrics_fname = string();
    string lrc_fname = string();
    try {
        if (res.count("audio-file") > 0 || res.count("lyrics-file") > 0) {
            audio_fname = res["audio-file"].as<string>();
            lyrics_fname = res["lyrics-file"].as<string>();
        } else {
            std::cout << "Required args missing\n";
            std::cout << all_opts.help() << "\n";
            return files;
        }
    } catch (std::exception &e) {
        std::cout << "Exception: " << e.what() << "\n" << all_opts.help() << "\n";
        return files;
    }
    if (res.count("output") == 1) {
        lrc_fname = res["output"].as<string>();
    } else {
        // default to text file filename (later on the extension is changed to .lrc)
        lrc_fname = lyrics_fname;
    }
    files.push_back(audio_fname);
    files.push_back(lyrics_fname);
    files.push_back(lrc_fname);
    return files;
}

int main(int argc, char **argv) {
    loguru::init(argc, argv);
    string logfile(argv[0]);
    logfile += ".log";
    loguru::add_file(logfile.c_str(), loguru::Truncate, loguru::Verbosity_INFO);

    std::vector<string> files = parse_args(argc, argv);
    if (files.size() == 0) {
        return 1;
    }
    string audio_fname = files[0];
    string lyrics_fname = files[1];
    string lrc_fname = files[2];

    fs::path audio_path = fs::path(audio_fname);
    fs::path lyrics_path = fs::path(lyrics_fname);
    fs::path lrc_path = fs::path(lrc_fname);
    if (fs::exists(audio_path) and fs::exists(lyrics_path)) {
        if (lyrics_path.has_extension()) {
            if (lyrics_path.compare(lrc_path) == 0) {
                lrc_path = lrc_path.replace_extension(".lrc");
            }
        } else {
            lrc_path += ".lrc";
        }
    } else {
        LOG_IF_F(ERROR, not fs::exists(audio_path), "File not found: %s", lyrics_path.c_str());
        LOG_IF_F(ERROR, not fs::exists(audio_path), "File not found: %s", audio_path.c_str());
        return 1;
    }

    // TODO: simple workaround to pass strings to the generator
    audio_fname = audio_path.filename();
    lyrics_fname = lyrics_path.filename();
    lrc_fname = lrc_path.filename();

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
    init_ncurses();

    generator.run(); // main program loop (menu)

    // does the cleanup
    cleanup_ncurses();

    return 0;
}
