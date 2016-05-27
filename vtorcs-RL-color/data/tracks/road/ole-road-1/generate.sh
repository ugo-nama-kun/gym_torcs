#! /bin/bash

name=ole-road-1
category=road
topdir=/usr/local/share/games/torcs/tracks
datadir=$topdir/$category/$name


# elevation
#cd $topdir
#trackgen -a -E 1 -n $name -c $category

# main track
cd $topdir
trackgen -a -n $name -c $category
pwd
cp $datadir/$name.ac $datadir/${name}-bak.ac

# shadows
cd $datadir
accc +shad $name-bak.ac $name-shade.ac >/dev/null
accc -g $name.ac -l0 $name-bak.ac -l1 $name-shade.ac -d3 1000 -d2 500 -d1 300 -S 300 -es >/dev/null


