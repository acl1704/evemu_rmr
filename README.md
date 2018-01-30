# EVEmu - Red Moon Rising
This repository is a mirror of the (long abandoned) EVEmu Red Moon Rising tree, with personal edits to get it building on new operating systems. EVEmu is not my project, and the original, unmodified source can be found [here.](https://sourceforge.net/projects/evemu/)

# Notes
The overall status and functionality of this project is unknown. The forums and wiki have been down for a long time, and the server has not been extensively tested since. 

It should build on Windows with Visual Studio 2017.

There's some issues with `eveserver` on x86_64 Linux distributions - the client is dropped immediately on connection, and displays an "Invalid version for selected server" error. Functions properly on i386 distributions, e.g Ubuntu 16.04.3 LTS.

I have only lightly tested the Windows & Linux (i386) builds, using MySQL 4.1.22 and 5.7.21.

# Building
I don't see much point in writing extensive build instructions right now, but the TL;DR is that you need to build `zlib` with `libs/zlib/projects/visualc6/zlib.sln` using LIB Release/Debug, then build `eveserver`. I've only lightly tested using VS 2017, but it should work fine.