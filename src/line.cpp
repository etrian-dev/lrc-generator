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

Line::Line(string ln, uint_fast_64_t delay) {
    this->text = ln;
    this->delay_ms = delay;
}

bool Line::set_string(string s) {
    this->text = s;
    return true;
}

bool set_delay(uint_fast64_t delay) {
    this->delay_ms = delay;
    return true;
}

void Line::get_text() {
    return this->text;
}
void Line::get_delay() {
    return this->delay_ms;
}
