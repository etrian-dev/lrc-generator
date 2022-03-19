# A simple lrc file generator
This simple program generates a .lrc file used for syncing the lyrics of a song
to its audio track. Music players supporting this format can then display the
lyrics along with the song. Additional information can be supplied as well, such as
title, artist, album name and the creator of the subtitles file.

More information about this file format can be found on Wikipedia

## Building from source
### Requirements
* A C++ compiler supporting at least c++17 (because of std::filesystem)
* SFML library (tested with 2.5.1)
* ncurses library (tested with 6.2)

```
git clone https://github.com/etrian-dev/lrc-generator.git
cd lrc-generator
make
```
To build the debug executable just ``` make debug ```

## Usage
`lrc-generator -a [audio file] -l [lyrics file] -o [lrc-file]`  
The program expects an audio file, a lyrics file and an output file to write to.  
The lyrics file should be a plaintext file containing strings separated by newlines to be synchronized with the song.  
The audio file containing the song must
be one of the formats supported by SFML: ogg/vorbis, wav of flac (<a href="https://www.sfml-dev.org/tutorials/2.5/audio-sounds.php">source</a>).  
An .lrc file is a text file created by the program. The -o option allows to supply an
arbitrary where the file should be created; otherwise the output file is created by replacing the lyrics file's
extension with .lrc and creating the resulting file in the current working directory.

The interface is a TUI made with ncurses. It allows the user to set various metadata
about the song and perform the actual syncing.
Once the synchronization is started, the song should start playing immediately at 
the maximum volume currently set.

Note that the output file is written only when the program exits correctly, so as to allow to reorder output lines and restart the synchronization, if needed.

### Synchronization
During synchronization the line timestamp (when such line will appear on the music player)
should be set by pressing some key as soon as that line starts, so to set **curr**'s timestamp
one should wait the termination of the line before (not on screen at the moment) and then
press some key. This behaviour will be changed, as it's a bit unintuitive
### LICENSE
The license is MIT, as provided in the LICENSE file.
The [cxxopts](https://github.com/jarro2783/cxxopts) library that has been used for command line option parsing
is licensed under the same license.


