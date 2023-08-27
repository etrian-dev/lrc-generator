# A simple lrc file generator

This simple program generates a .lrc file used for syncing the lyrics of a song
to its audio track. Music players supporting this format can then display the
lyrics along with the song. Additional information can be supplied as well, such as
title, artist, album name, song lenght and the creator of the lrc file.

More information about this file format can be found on Wikipedia

## Building from source
### Requirements
* A C++ compiler supporting at least C++17
* SFML library ( tested with 2.5.1 )
* ncurses library ( tested with 6.2 )
* meson ( >=0.63 )
* ninja

```
git clone https://github.com/etrian-dev/lrc-generator.git
cd lrc-generator
meson setup build
ninja -C build
```

### Dev tools
Before submitting patches, run ``clang-format`` on the modified files (e.g., by using the
convenient ``git clang-format`` script). The mimimum tested version is 15.0.7.

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
During synchronization the first line's offset is always 0 (it appears as soon as the track starts in the music player).
When synchronizing the current line being sung should always be the one hightlighted; when a key is pressed the timestamp
for the next line is taken and the window refreshes. A menu of available keybindings is available on the left side, during synchronization.
### LICENSE
The license for this software is MIT, as provided in the LICENSE file.
The [cxxopts](https://github.com/jarro2783/cxxopts) library that has been used for command line option parsing
is licensed under the same license.
The [loguru](https://github.com/emilk/loguru) library used for logging is in the public domain, as stated in the linked repository
