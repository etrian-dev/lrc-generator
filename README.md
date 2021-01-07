<h3>A simple .lrc file generator</h3>
<p>
This simple program generates an .lrc file used for syncing the lyrics of a song to its audio track.
Media player supporting this format can then display the lyrics along with the song and additional information too,
such as title, artist and the album name.
</p>
<h5>TODO</h5>
<ul>
	<li>Play the audio file within the program when syncing (find some library)</li>
</ul>
<p>
No additional requirements other than C++'s standard library and a compiler supporting at least c++11<br/>
Compilation: g++ *.cpp -o <exename>
Usage: the program expects a filename as argument; the file provided must contain plain text (ascii only)
representing the lyrics of the song (for instance lyrics.txt) and will be read sequentially line by line.
A .lrc file is created by the program with the same base name as the one provided (this may be extended).
A textual menu is presented at startup, with multiple options to set metadata or start syncing the song.
At the moment the song must be played in an external player of choice, but the ability to play it automatically
may be added eventually.
</p>
