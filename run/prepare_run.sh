#!/bin/bash

echo "Copying OPAQ framework to run"
cp ../../../framework/main/build/opaq .
cp ../../../framework/pfw/build/libopaqpfw.so .
cp ../../../framework/confighandler/build/libopaqch.so .
cp ../../../framework/common/build/libopaqcommon.so .
cp ../../../framework/engine/build/libopaqengine.so .

echo "Copying data/meteo plugins"
cp ../../data/build/*.so .

echo "Copying the FeedForwardModel library"
cp ../src/libFeedForwardModel.so .


if [ ! -h meteo ]; then
    echo "Linking ECMWF meteo archive"
    ln -s /projects/N78C0/OVL/ECMWF/2011 meteo
fi
