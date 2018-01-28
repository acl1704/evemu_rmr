# EVEmu - Red Moon Rising
This repository is a mirror of the (long abandoned) EVEmu Red Moon Rising tree, with personal edits to get it building on new operating systems. EVEmu is not my project, and the original, unmodified source can be found [here.](https://sourceforge.net/projects/evemu/)

# Notes
The overall status and functionality of this project is unknown. The forums and wiki have been down for a long time, and the server has not been extensively tested since. 

It should build on Windows with Visual Studio 2017 - it requires zlib-1.2.3 built in `C:/zlib-1.2.3`, and MySQL Connector C placed in `C:/mysql/`, with libraries in `C:/mysql/lib`. It requires more cleaning, but getting it to function properly on Linux is more important as of now.

While it can build on Linux with modification, it does not seem to function properly on my system. From what I can tell, it's sending garbage to the client during the handshake, which it interprets as an invalid client version.

I have only lightly tested the Windows build, using MySQL 4.1.22.