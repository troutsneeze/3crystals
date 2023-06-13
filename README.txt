Twin-stick shooter using shim4 and tgui6

Find links to the game at https://b1stable.com.

The code isn't very organized yet and it's C++ not C. For an example with nice
code see Monster RPG 3, Dog-O or Tower To Heaven.


Building
--------

Well on Linux it's not very difficult to build this game, but the paths
in the build scripts are hard-coded.

1) cd ~
2) mkdir code
3) cd code
4) git clone https://github.com/troutsneeze/3crystals.git
5) export PATH=~/code/3crystals/misc/batch_files/build/linux:$PATH
6) ca
7) ba
8) cd 3crystals/build
9) cp -a ../data .
10) ./3\ Crystals

Now you will need certain build tools and "-dev" packages installed to
complete the process.

If you want to compress the data to a datafile, then build the tools,
put compress_dir in your PATH and run these two steps after 7) above
in place of the rest.

8) ia
9) cd 3
10) ./3\ Crystals
