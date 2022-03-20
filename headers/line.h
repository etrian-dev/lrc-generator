#ifndef LRC_LINE_INCLUDED
#define LRC_LINE_INCLUDED
// std lib headers
#include <string>

using std::string;

// This class represent a synchronized line
class Line {
private:
  string text;
  // the delay, in ms, from the start of the song
  uint_fast64_t delay_ms;

public:
  Line();
  Line(string ln, uint_fast64_t delay);

  string get_text();
  uint_fast64_t get_delay();
};

#endif