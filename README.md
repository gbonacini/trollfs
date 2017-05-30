Description:
============

Yes, I know, sometimes I'm an horrible person, but even the Unix programmers just want to have fun. :-) 

![alt text](screenshoots/fun.png "Have fun")

FEATURES:
=========

" Trollfs  permits to create and "overmount" a fake in-memory file system (using FUSE) that reproduce almost exactly the structure  of  the  sub-tree  present  in  the  mount point, but, for each read request returns fake data instead of the real content, The payload  is  freely  configurable  creating  o modifying the files present in the payloads' directory. A file created in that directory, with an  extension  related  to one  or more real files present in the original directory, will be used as fake data".

Prerequisites:
==============

- C++11 compiler;
- libfuse
- libmagic

Tested on:

- Ubuntu 16.10

Installation:
=============

- launch the configure script:
  ./configure
- Compile the program:
  make
- Install the program and the man page (optional):
  sudo make install



Instructions:
=============

See the man page included in the release.

