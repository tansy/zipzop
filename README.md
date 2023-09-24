zipzop
======

Zipzop is a zip recompression program, that uses libzopfli to re/compress deflate streams. It uses iterations like g/zip compression levels, but from 1 to infinity.
It originated from Komiya Atsushi's [zipzop](https://github.com/komiya-atsushi/zipzop). [Licence](license.txt) and original [README](README.md.jp) (in Japanese) is inherited from original.

Usage:
$ zipzop \<iterations\> \<in_file\> \<out_file\>

In test directory there are simple sef tests.
