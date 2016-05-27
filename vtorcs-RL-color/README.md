# Visual TOCRCS server for Reinforcement Learning with Color Vision
 
## Introduction
This is the modified torcs source code for the visual reinforcement learning in the car racing situation. This code code is almost entierly base on vtorcs-nosefault by Giuseppe Cuccu. 

vtorcs : https://github.com/giuseppecuccu/vtorcs/tree/nosegfault

This version is the extension of vtorcs for the treatment of color vision (that is, RGB).


## Introduction from original vtorcs:
This is an all in one package of TORCS. Be aware that some included
artwork has non free (in the GPL sense) licenses, you will find a "readme.txt"
in those directories. The rest is either licensed under the GPL or the Free
Art License.


## Non-Free content (in GPL sense)
Here the list with the directories containing non free content, look at the
readme.txt for details:
- data/cars/models/pw-*
- data/cars/models/kc-*


## Linux Installation from Source

- Requires plib 1.8.5, FreeGLUT or GLUT, be aware to compile plib with -fPIC
  on AMD64 if you run a 64 bit version of Linux. Be aware that maybe just
  1.8.5 works.
- Untar the archive
- cd into the vtorcs-RL-color directory
- ./configure (use --help for showing the options, of interest might be
  --enable-debug and --disable-xrandr).

```
sudo apt-get install libglib2.0-dev  libgl1-mesa-dev libglu1-mesa-dev  freeglut3-dev  libplib-dev  libopenal-dev libalut-dev libxi-dev libxmu-dev libxrender-dev  libxrandr-dev libpng12-dev 
./configure
make
make install
make datainstall
```
start with "torcs" (without vision) or "torcs -vision" (with vision)


## Command line arguments:
* -l list the dynamically linked libraries
* -d run under gdb and print stack trace on exit, makes most sense when compile
     with --enable-debug
* -e display the commands to issue when you want to run under gdb
* -s disable multitexturing, important for older graphics cards
* -m use X mouse cursor and do not hide it during races
* -vision use vision input
