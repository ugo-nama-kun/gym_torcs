#!/bin/bash


usage(){
  NAME=$(basename ${BASH_SOURCE})
  echo "Usage: "
  echo "- ${NAME} exp  - for the exp setting"
  echo "- ${NAME} big  - for bigger screen"
}


if [ $# -lt 1 ]; then
  usage
  exit
fi


# TODO: this can be an fixed absolute path
cd "$( dirname "${BASH_SOURCE[0]}" )/exp-settings"


# TODO: make it a base, on top of which you can activate
# small screen
# big screen
# scr alone
# scr + foes
# player (and sound)
case "$1" in
  "exp")
    echo "Copying experiment settings..."
    cp -R * ~/.torcs
    echo "Done."
    ;;
  "big")
    echo "Copying experiment settings..."
    cp -R * ~/.torcs
    echo "Copying big screen settings..."
    cp bigscreen.xml ~/.torcs/config/screen.xml
    echo "Done."
    ;;
  *)
    usage
    exit
    ;;
esac

