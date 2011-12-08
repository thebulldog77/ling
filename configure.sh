#!/bin/bash
# This script _ensures_ that you can build & install Wintermute.

echo "Downloading tools..."
sudo apt-get install cmake cmake-curses-gui libqt4-dev

echo "Preparing build directory..."
mkdir -p build
cd build

echo "Purging build directory..."
sudo rm * -rvf

echo "Configuring system..."
cmake -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Debug ..
ccmake ..

echo "Building..."
make DESTDIR=/usr

echo "Cleaning system..."
sudo rm -v /usr/lib/wntr/libwplugin-ling*
sudo rm -rv /usr/share/wintermute/plugins/ling
sudo rm -rv /usr/include/wintermute/plugins/ling
sudo rm -rv /usr/share/cmake/WntrLing*

echo "Installing to system..."
sudo make install
