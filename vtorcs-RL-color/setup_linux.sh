#! /bin/bash

[ -z "$1" ] && exit 1
[ ! -d "$1" ] && exit 1

mkdir -p $1/drivers/human/tracks/dirt-2 2>/dev/null
if [ ! -e  $1/drivers/human/tracks/dirt-2/car-pw-206wrc.xml ] || [ drivers/human/tracks/dirt-2/car-pw-206wrc.xml -nt $1/drivers/human/tracks/dirt-2/car-pw-206wrc.xml ]
then
    if [ -e $1/drivers/human/tracks/dirt-2/car-pw-206wrc.xml ]
    then
        echo "Saving $1/drivers/human/tracks/dirt-2/car-pw-206wrc.xml to $1/drivers/human/tracks/dirt-2/car-pw-206wrc.xml.old"
        cp -f $1/drivers/human/tracks/dirt-2/car-pw-206wrc.xml $1/drivers/human/tracks/dirt-2/car-pw-206wrc.xml.old
    fi
    cp -f drivers/human/tracks/dirt-2/car-pw-206wrc.xml $1/drivers/human/tracks/dirt-2/car-pw-206wrc.xml
    chmod 640 $1/drivers/human/tracks/dirt-2/car-pw-206wrc.xml
fi
mkdir -p $1/drivers/human 2>/dev/null
if [ ! -e  $1/drivers/human/car.xml ] || [ drivers/human/car.xml -nt $1/drivers/human/car.xml ]
then
    if [ -e $1/drivers/human/car.xml ]
    then
        echo "Saving $1/drivers/human/car.xml to $1/drivers/human/car.xml.old"
        cp -f $1/drivers/human/car.xml $1/drivers/human/car.xml.old
    fi
    cp -f drivers/human/car.xml $1/drivers/human/car.xml
    chmod 640 $1/drivers/human/car.xml
fi
if [ ! -e  $1/drivers/human/human.xml ] || [ drivers/human/human.xml -nt $1/drivers/human/human.xml ]
then
    if [ -e $1/drivers/human/human.xml ]
    then
        echo "Saving $1/drivers/human/human.xml to $1/drivers/human/human.xml.old"
        cp -f $1/drivers/human/human.xml $1/drivers/human/human.xml.old
    fi
    cp -f drivers/human/human.xml $1/drivers/human/human.xml
    chmod 640 $1/drivers/human/human.xml
fi
if [ ! -e  $1/drivers/human/preferences.xml ] || [ drivers/human/preferences.xml -nt $1/drivers/human/preferences.xml ]
then
    if [ -e $1/drivers/human/preferences.xml ]
    then
        echo "Saving $1/drivers/human/preferences.xml to $1/drivers/human/preferences.xml.old"
        cp -f $1/drivers/human/preferences.xml $1/drivers/human/preferences.xml.old
    fi
    cp -f drivers/human/preferences.xml $1/drivers/human/preferences.xml
    chmod 640 $1/drivers/human/preferences.xml
fi
mkdir -p $1/config 2>/dev/null
if [ ! -e  $1/config/raceengine.xml ] || [ config/raceengine.xml -nt $1/config/raceengine.xml ]
then
    if [ -e $1/config/raceengine.xml ]
    then
        echo "Saving $1/config/raceengine.xml to $1/config/raceengine.xml.old"
        cp -f $1/config/raceengine.xml $1/config/raceengine.xml.old
    fi
    cp -f config/raceengine.xml $1/config/raceengine.xml
    chmod 640 $1/config/raceengine.xml
fi
if [ ! -e  $1/config/style.xsl ] || [ config/style.xsl -nt $1/config/style.xsl ]
then
    if [ -e $1/config/style.xsl ]
    then
        echo "Saving $1/config/style.xsl to $1/config/style.xsl.old"
        cp -f $1/config/style.xsl $1/config/style.xsl.old
    fi
    cp -f config/style.xsl $1/config/style.xsl
    chmod 640 $1/config/style.xsl
fi
mkdir -p $1/config 2>/dev/null
if [ ! -e  $1/config/screen.xml ] || [ config/screen.xml -nt $1/config/screen.xml ]
then
    if [ -e $1/config/screen.xml ]
    then
        echo "Saving $1/config/screen.xml to $1/config/screen.xml.old"
        cp -f $1/config/screen.xml $1/config/screen.xml.old
    fi
    cp -f config/screen.xml $1/config/screen.xml
    chmod 640 $1/config/screen.xml
fi
mkdir -p $1/config 2>/dev/null
if [ ! -e  $1/config/graph.xml ] || [ config/graph.xml -nt $1/config/graph.xml ]
then
    if [ -e $1/config/graph.xml ]
    then
        echo "Saving $1/config/graph.xml to $1/config/graph.xml.old"
        cp -f $1/config/graph.xml $1/config/graph.xml.old
    fi
    cp -f config/graph.xml $1/config/graph.xml
    chmod 640 $1/config/graph.xml
fi
if [ ! -e  $1/config/sound.xml ] || [ config/sound.xml -nt $1/config/sound.xml ]
then
    if [ -e $1/config/sound.xml ]
    then
        echo "Saving $1/config/sound.xml to $1/config/sound.xml.old"
        cp -f $1/config/sound.xml $1/config/sound.xml.old
    fi
    cp -f config/sound.xml $1/config/sound.xml
    chmod 640 $1/config/sound.xml
fi
mkdir -p $1/results/champ 2>/dev/null
mkdir -p $1/results/dtmrace 2>/dev/null
mkdir -p $1/results/endrace 2>/dev/null
mkdir -p $1/results/ncrace 2>/dev/null
mkdir -p $1/results/practice 2>/dev/null
mkdir -p $1/results/quickrace 2>/dev/null
mkdir -p $1/config/raceman 2>/dev/null
if [ ! -e  $1/config/raceman/champ.xml ] || [ config/raceman/champ.xml -nt $1/config/raceman/champ.xml ]
then
    if [ -e $1/config/raceman/champ.xml ]
    then
        echo "Saving $1/config/raceman/champ.xml to $1/config/raceman/champ.xml.old"
        cp -f $1/config/raceman/champ.xml $1/config/raceman/champ.xml.old
    fi
    cp -f config/raceman/champ.xml $1/config/raceman/champ.xml
    chmod 640 $1/config/raceman/champ.xml
fi
if [ ! -e  $1/config/raceman/dtmrace.xml ] || [ config/raceman/dtmrace.xml -nt $1/config/raceman/dtmrace.xml ]
then
    if [ -e $1/config/raceman/dtmrace.xml ]
    then
        echo "Saving $1/config/raceman/dtmrace.xml to $1/config/raceman/dtmrace.xml.old"
        cp -f $1/config/raceman/dtmrace.xml $1/config/raceman/dtmrace.xml.old
    fi
    cp -f config/raceman/dtmrace.xml $1/config/raceman/dtmrace.xml
    chmod 640 $1/config/raceman/dtmrace.xml
fi
if [ ! -e  $1/config/raceman/endrace.xml ] || [ config/raceman/endrace.xml -nt $1/config/raceman/endrace.xml ]
then
    if [ -e $1/config/raceman/endrace.xml ]
    then
        echo "Saving $1/config/raceman/endrace.xml to $1/config/raceman/endrace.xml.old"
        cp -f $1/config/raceman/endrace.xml $1/config/raceman/endrace.xml.old
    fi
    cp -f config/raceman/endrace.xml $1/config/raceman/endrace.xml
    chmod 640 $1/config/raceman/endrace.xml
fi
if [ ! -e  $1/config/raceman/ncrace.xml ] || [ config/raceman/ncrace.xml -nt $1/config/raceman/ncrace.xml ]
then
    if [ -e $1/config/raceman/ncrace.xml ]
    then
        echo "Saving $1/config/raceman/ncrace.xml to $1/config/raceman/ncrace.xml.old"
        cp -f $1/config/raceman/ncrace.xml $1/config/raceman/ncrace.xml.old
    fi
    cp -f config/raceman/ncrace.xml $1/config/raceman/ncrace.xml
    chmod 640 $1/config/raceman/ncrace.xml
fi
if [ ! -e  $1/config/raceman/practice.xml ] || [ config/raceman/practice.xml -nt $1/config/raceman/practice.xml ]
then
    if [ -e $1/config/raceman/practice.xml ]
    then
        echo "Saving $1/config/raceman/practice.xml to $1/config/raceman/practice.xml.old"
        cp -f $1/config/raceman/practice.xml $1/config/raceman/practice.xml.old
    fi
    cp -f config/raceman/practice.xml $1/config/raceman/practice.xml
    chmod 640 $1/config/raceman/practice.xml
fi
if [ ! -e  $1/config/raceman/quickrace.xml ] || [ config/raceman/quickrace.xml -nt $1/config/raceman/quickrace.xml ]
then
    if [ -e $1/config/raceman/quickrace.xml ]
    then
        echo "Saving $1/config/raceman/quickrace.xml to $1/config/raceman/quickrace.xml.old"
        cp -f $1/config/raceman/quickrace.xml $1/config/raceman/quickrace.xml.old
    fi
    cp -f config/raceman/quickrace.xml $1/config/raceman/quickrace.xml
    chmod 640 $1/config/raceman/quickrace.xml
fi
