<h3>A simple .lrc file generator</h3>
<p>
This simple program generates an .lrc file used for syncing the lyrics of a song
to its audio track. Music players supporting this format can then display the
lyrics along with the song. Additional information is also supported, such as
title, artist, album name and the creator of the subtitles file.
</p>
<h5>TODO</h5>
<ul>
	<li>Add lenght tag</li>
	<li>Force writing tags at the top of the .lrc file so that they can be read by the players</li>
</ul>
<p>
Requires c++11's standard library (and a compiler supporting that standard) <br/>
as well as the SFML library (installed in the default path) for the song playback when syncing<br/>
The executable can be compiled with the given Makefile (both stable and debug versions)<br/>
<blockquote>make stable</blockquote>
</p>
<p>
Usage: the program expects a filename and an audio file as arguments. The lyrics file
provided must contain plain text representing the lyrics of the song (for instance lyrics.txt)
and will be read sequentially line by line. The audio file containing the song must
be one of the formats supported by SFML: ogg/vorbis, wav of flac (<a href="https://www.sfml-dev.org/tutorials/2.5/audio-sounds.php">source</a>). A .lrc file is created by the program with the same
base name as the one provided by the lyrics file (this may be subject to changes).
A textual menu is presented, with multiple options to set metadata or start syncing the song.
Once the syncing is started, the song should start playing immediately at the maximum volume
currently set and when the syncing is finished, it stops. Note that the output file is written
syncronously, so the song cannot be resynced within the same execution, but this
aspect is subject to changes.<br/>
When syncing and the current line finishes one can either wait for the next to begin
and then immediately press enter to store its timepoint (meaning that the next line appears as
soon as the current has been sung) or wait for the next to begin (a few moments earlier
is the recommended choice) and then press enter.
</p>
