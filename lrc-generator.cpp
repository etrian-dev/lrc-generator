// .lrc generator for songs's lyrics
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <vector>
#include <chrono> // clock utilities, I use the system clock
#include <ratio>

// simple class as a wrapper for routines for setting up
// the .lrc file
class Menu {
	public:
		// the actual syncing
		void start_sync(std::vector<std::string> &lyrics);
};

void Menu::start_sync(std::vector<std::string> &lyrics) {
	// now the real syncing begins
	std::string prev, curr, next;
	char c;
	unsigned int i = 0;

	// clock with system time, used to find timepoints and write out in the lrc file
	std::chrono::system_clock lrc_clock;
	// start timer and stores incremental timepoints
	std::chrono::time_point<std::chrono::system_clock> start = lrc_clock.now(), this_point;

	// read vector line by line
	while(i < lyrics.size()) {
		// clear screen (not portable)
		//system("clear");
		
		// assigns to the current line entry i of the vector
		curr = lyrics[i];
		// and i+1 to next if possible
		if(i+1 < lyrics.size()) {
			next = lyrics[i+1];
		}
		else {
			next.clear();
		}
		
		// display info in terminal about current, previous and next line
		std::cout 	<< "prev line: " << (prev.empty() ? "(null)" : prev)
					<< "\ncurr line: " << curr
					<< "\nnext line: " << (next.empty() ? "(null)" : next) << "\n";
		
		// wait until enter is pressed
		std::cout << "press enter...\n";
		
		std::cin >> std::noskipws >> c;
		// get current time point
		this_point = lrc_clock.now();
		// calculate the time elapsed and print it
		std::cout << "since start: " << std::chrono::duration_cast<std::chrono::seconds>(this_point - start).count() << "s\n";
		// and write down the time point of the clock
					
		// then update the previous line and index
		prev = curr;
		i++;
	}
}
int main(int argc, char** argv) {
	if(argc > 1) {
		// lyrics file is passed as argv[1]
		std::ifstream in_lyrics(argv[1]);
		
		if(!in_lyrics.is_open()) {
			std::cout << "Cannot open file " << argv[1] << ".\n";
			return 1;
		}
		
		// file opened successfully
		// file is read line by line and stored in an array of strings for ease of access
		std::vector<std::string> text;
		std::string line;
		while(std::getline(in_lyrics, line)) {
			text.push_back(line);
		}
		
		// now the file can be closed
		in_lyrics.close();
		
		// create an lrc file with the same title as the lyrics file
		// with the .lrc extension
		std::string lrc_file = argv[1];
		lrc_file.replace(lrc_file.size() - 4, 3, "lrc");
		std::cout << "generated file \"" << lrc_file << "\"\n";
		// a simple class implements a menu for setting up the lrc file
		Menu setup;

		setup.start_sync(text);
		
	}
	else {
		std::cout << "No lyrics provided\nUsage: lrc-generator [LYRICS.txt]\n";
	}
	return 0;
}