# atari_uct_cluster
A version of the atari_uct code that run on NUS Computer Center cluster.

## Dependances :
 - OpenCV. To install run the script install/install_opencv.sh. The program was tested with openCV 3.2.0. Here we don't need the python support of OpenCV, so the python version doesn't need to be specified during the compilation. For more details see [this post](http://gunheeleo.blogspot.sg/2013/07/install-opencv-without-root-permission.html)
 - Xitari. Xitari is the atari emulator used. To install run the script install/install_xitari.sh. For more detail go to https://github.com/deepmind/xitari
 - Gflags. Gflags is a useful google library for flag. To install it run the script install/install_gflags.sh. During the install, vim will open. You will need to change the  minimum version to 2.8.11

## Compilation
Go to the code directory and run
```
mkdir build
cd build
cmake ..
make
```

## Running a simulation
The script launch_pong.sh is used to run a simulation. It puts the 800 processes in the cluster queue.

## Process the output images
The script process_output_CNN.py load, crop, resize the images and then transform them to gray-scale images before saving them by stacks of 4, for preparing the input of the neural network.
