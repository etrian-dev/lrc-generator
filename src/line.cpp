#include "../headers/line.h"

#include <string>
using std::string;

Line::Line() {
    this->text = string();
    this->delay_ms = 0;
}

Line::Line(string s) {
    this->text = std::move(s);
}

Line::Line(string ln, size_t delay) {
    this->text = ln;
    this->delay_ms = delay;
}

bool Line::set_string(string s) {
    this->text = s;
    return true;
}

bool Line::set_delay(size_t delay) {
    this->delay_ms = delay;
    return true;
}

string Line::get_text() {
    return this->text;
}
size_t Line::get_delay() {
    return this->delay_ms;
}
