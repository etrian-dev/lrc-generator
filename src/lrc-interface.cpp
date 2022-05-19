// header file for the generator class
#include "../headers/lrc-generator.h"
#include "../headers/lrc-interface.hpp"
#include "../headers/spsc.hpp"
#include <tuple>
// C-style asserts
#include <cassert>
#include <iostream>
// curses library
#include <ncurses.h>

Lrc_interface::Lrc_interface(Lrc_generator* generator) {
    // Sets the reference to the generator object
    this->model = generator;
    // splits the terminal: a left section with a menu and a right one with the
    // lyrics
    getmaxyx(stdscr, this->max_height, this->max_width);
    this->menu = newwin(this->max_height, this->max_width / 2, 0, 0);
    this->lyrics_win = newwin(this->max_height, this->max_width / 2, 0, this->max_width / 2);

    // the OK constant is defined in ncurses.h
    assert(this->menu);
    assert(this->lyrics_win);

    // enable the keypad for KEY_UP/DOWN
    keypad(this->lyrics_win, true);
}

Lrc_interface::~Lrc_interface(void) {
    wclear(this->menu);
    wclear(this->lyrics_win);
    wrefresh(this->menu);
    wrefresh(this->lyrics_win);

    delwin(this->menu);
    delwin(this->lyrics_win);
}

void Lrc_interface::run(
    Spsc_queue<int>& key_q,
    Spsc_queue<vector<string>>& content_q,
    Spsc_queue<vector<std::tuple<string, string>>>& menu_q) {

    int action;
    while (this->model->is_running()) {
        draw_menu(menu_q);
        if(this->model->getState() != GeneratorState::MENU) {
            draw_content(content_q);
        }

        action = wgetch(this->menu);
        std::cout << "(interface) action = " << action << "\n";
        key_q.produce(action);
    }
    std::cout << "Interface thread quits\n";
}

void Lrc_interface::draw_menu(Spsc_queue<vector<std::tuple<string, string>>>& menu_q) {
    // update the menu items
    this->menu_items = menu_q.consume();
    // TODO: check valid with assert?

    const int hoff = 1;
    const int woff = 1;
    // draw options on the menu window
    wclear(this->menu);
    wstandout(this->menu);
    mvwaddstr(this->menu, hoff, woff, "Menu");
    wstandend(this->menu);
    unsigned int i = 0;
    for (auto item : this->menu_items) {
        mvwprintw(this->menu, i + hoff, woff, "[%s] => %s", std::get<0>(item), std::get<1>(item));
    }
    box(this->menu, 0, 0);
    wrefresh(this->menu);
}

void Lrc_interface::draw_content(
    Spsc_queue<vector<string>>& content_q) {

    // gets content from the model
    vector<string> content = content_q.consume();

    // offsets from the window border
    const int hoff = 2;
    const int woff = 2;
    int height, width;
    getmaxyx(this->lyrics_win, height, width);

    wclear(this->lyrics_win);
    box(this->lyrics_win, 0, 0);
    // display the current line being synchronized and the next one on stdout
    // or print "(null)" if the line is empty
    unsigned int i = 0;
    for (string s : content) {
        mvwaddstr(this->lyrics_win, hoff + i, woff, (s.empty() ? "(null)" : s.c_str()));
    }

    // Last timestamp
    //mvwprintw(this->lyrics_win, hoff + 4, woff, "Last timestamp [%d.%d s]",
    //          tot_playback.count() / 1000, (tot_playback.count() / 10) % 100);


    wrefresh(this->lyrics_win);

    // TODO: wgetch(tyrics_win) and key_q.produce()?
}

/*void Lrc_generator::set_attr_dialog(std::string attr) {
    WINDOW *dialog = newwin(4, 50, 20, 50);
    bool not_ok = true;
    char value[20] = {0};
    char ans;
    char c;
    int i = 0;
    const int hoff = 1;
    const int woff = 1;
    do {
        wclear(dialog);
        box(dialog, 0, 0);
        mvwprintw(dialog, hoff, woff, "%s: ", attr.c_str());
        i = 0;
        do {
            c = wgetch(dialog);
            value[i] = c;
            waddch(dialog, c);
            wrefresh(dialog);
            i++;
        } while (c != '\n');
        value[i - 1] = '\0';

        mvwprintw(dialog, hoff + 1, woff, "Is %s ok? [y/n]", value);
        box(dialog, 0, 0);
        ans = wgetch(dialog);
        if (ans == 'y') {
            not_ok = false;
        }
        wrefresh(dialog);
    } while (not_ok);

    // now write it to the file
    if (attr.compare("cr") == 0) {
        attr.assign("by"); // the creator attribute has another abbreviation
    }
    // push this metadata (updates if it was already set)
    /// TODO: improve this
    size_t j = 0;
    for (; j < this->metadata.size(); j++) {
        if (this->metadata[j].find("[" + attr.substr(0, 2) + ": ") !=
                std::string::npos) {
            this->metadata[j].erase();
            this->metadata[j] = "[" + attr.substr(0, 2) + ": " + value + "]";
            break;
        }
    }
    if (j == this->metadata.size()) {
        this->metadata.push_back("[" + attr.substr(0, 2) + ": " + value + "]");
    }
    // deletes this window
    delwin(dialog);
}

char Lrc_generator::choice_dialog(std::string msg) {
    int height, width;
    getmaxyx(this->menu, height, width);
    WINDOW *dialog = newwin(4, 75, height / 2, width / 2);
    char value[20] = {0};
    char ans;
    char c;
    int i = 0;
    const int hoff = 1;
    const int woff = 1;

    wclear(dialog);
    box(dialog, 0, 0);
    mvwaddstr(dialog, hoff, woff, msg.c_str());
    mvwaddstr(dialog, hoff + 1, woff, "[Y/n]");
    c = wgetch(dialog);

    // deletes this window
    delwin(dialog);

    return c;
}*/
