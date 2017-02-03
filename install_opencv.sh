#!/bin/bash

echo before lunching this script go to https://sourceforge.net/projects/opencvlibrary/files/ and download the last version of OpenCV. Put the zip file in the home directory

# Change these variables to adapt your system
actual_version=3.2.0
install_path=$HOME/opencv

# Install
cd
unzip opencv-$actual_version.zip
cd  ~/opencv-$actual_version/
mkdir build
cd build
cmake -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=$install_path  ..
make 
make install

# Clean
cd
rm -rf opencv-3.2.0

# Add opencv path in the bashrc
cd
echo \#opencv >> .bashrc
echo export LD_LIBRARY_PATH=$install_path/lib:\$LD_LIBRARY_PATH >> .bashrc
echo export PKG_CONFIG_PATH=$install_path/lib/pkgconfig/:\$PKG_CONFIG_PATH >> .bashrc

