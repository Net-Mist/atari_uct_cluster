#!/bin/bash

# Change these variables to adapt your system
install_path=$HOME/gfinstall

# Install
cd
git clone https://github.com/gflags/gflags.git
cd gflags
vim CMakeLists.txt # change minimum version to 2.8.11
mkdir build
cd build
cmake -D BUILD_SHARED_LIBS=ON -D CMAKE_INSTALL_PREFIX=$install_path  ..
make
make install

# Clean
cd
rm -rf gflags

# Add gflags path in the bashrc
cd
echo \#gflags >> .bashrc
echo export LD_LIBRARY_PATH=$install_path/lib:\$LD_LIBRARY_PATH >> .bashrc

