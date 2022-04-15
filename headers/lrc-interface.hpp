#ifndef LRC_INTERFACE_INCLUDED
#define LRC_INTERFACE_INCLUDED

#include "spsc.hpp"
#include <string>
#include <vector>
#include <tuple>
// ncurses header
#include <ncurses.h>

using std::string;
using std::vector;

class Lrc_interface {
private:
    // Generator instance reference
    Lrc_generator *model;
    // TUI functions & variables
    WINDOW *menu;
    WINDOW *lyrics_win;
    int max_height, max_width;
    // menu items (updated dynamically by the model)
    vector<std::tuple<string, string>> menu_items;


    void interface_setup(void);
    // utility function to draw the menu
    void draw_menu(void);
    // draw content
    void draw_content();
    // creates a dialog to set the chosen attribute
    void set_attr_dialog(string attr);
    // displays a simple choiche dialog
    char choice_dialog(string msg);
public:
    // runs the interface
    void run(Spsc_queue<int>& key_q, Spsc_queue<float>& vol_q, Spsc_queue<vector<string>>& content_q, Spsc_queue<std::tuple<string, string>>>& menu_q);

    Lrc_interface(Lrc_generator& model);
    ~Lrc_interface();

};

#endif