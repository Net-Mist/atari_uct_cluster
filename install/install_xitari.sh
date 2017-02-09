#!/bin/bash

# Compile
cd
git clone https://github.com/deepmind/xitari.git
cd xitari
cmake .
make

# Update bashrc
cd
echo \#xitari >> .bashrc
echo export LD_LIBRARY_PATH=$HOME/xitari/:\$LD_LIBRARY_PATH >> .bashrc


