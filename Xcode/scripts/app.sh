#!/bin/bash

DIR=$(dirname "$0")
cd "$DIR"
clear

HPPRGM=/Users/Shared/Untiled.hpprgm
PATH=/Users/Shared
NAME=Application

mkdir $PATH/$NAME.hpappdir
cp $HPPRGM $PATH/$NAME.hpappdir/$NAME.hpappprgm
cp ../app.hpappdir/icon.png $PATH/$NAME.hpappdir/icon.png
cp ../app.hpappdir/app.hpapp $PATH/$NAME.hpappdir/$NAME.hpapp
