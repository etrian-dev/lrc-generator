#include "../headers/line.h"

#include <string>
using std::string;

Line::Line() {
  this->text = string();
  this->delay_ms = 0;
}

Line::Line(string ln, uint_fast_64_t delay) {
  this->text = ln;
  this->delay_ms = delay;
}

void Line::get_text() { return this->text; }
void Line::get_delay() { return this->delay_ms; }
