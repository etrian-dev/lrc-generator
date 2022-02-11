# A simple .lrc file generator
This simple program generates an .lrc file used for syncing the lyrics of a song
to its audio track. Music players supporting this format can then display the
lyrics along with the song. Additional information can be supplied as well, such as
title, artist, album name and the creator of the subtitles file.

### Requirements to build from source
* Compiler supporting at least c++17 (because of std::filesystem)
* SFML library (tested with 2.5.1)
* ncurses library (tested with 6.2)

The executable can be built using the supplied Makefile.  
For the stable build the command is `make stable`

### Usage
`lrc-generator -a [audio file] -l [lyrics file] -o [lrc-file]`  
The program expects a filename and an audio file as arguments. The lyrics file
provided must contain plain text representing the lyrics of the song and will be read sequentially line by line.  
The audio file containing the song must
be one of the formats supported by SFML: ogg/vorbis, wav of flac (<a href="https://www.sfml-dev.org/tutorials/2.5/audio-sounds.php">source</a>).  
An .lrc file is a text file created by the program. The -o option allows to supply an
arbitrary destination file; otherwise the output file is created by replacing the lyrics file's
extension with .lrc.

The interface is a TUI made with ncurses. It allows the user to set various metadata
about the song and start the actual syncing.
Once the synchronization is started, the song should start playing immediately at 
the maximum volume currently set.
Note that the output file is written syncronously, so it's not an idempotent operation,
but this caveat may be fixed in the future.
#### Synchronization
During the synchronization timestamps of lines are taken when the next line becomes the current (by pressing some key).
A simple example:

Suppose the synchronization started 10s ago and the following is on screen
- curr: some line
- next: other line
After pressing Enter the following line will be appended to the output file:
- [0:10.00]other line

### LICENSE
The license is MIT, as provided in the LICENSE file.
The [cxxopts](https://github.com/jarro2783/cxxopts) library that has been used for command line option parsing
is licensed under the same license.

### TODO
* Add lenght tag
* Force writing metadata at the top of the .lrc file
* Support pausing and resetting the syncing (partial in v0.1)
* Better file and options handling
* Volume controls while synchronizing
