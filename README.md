# atari_uct_cluster
The version of the atari_uct code that run on NUS Computer Center cluster

## Dependances : 

### OpenCV
See install_opencv.sh. For more details see http://gunheeleo.blogspot.sg/2013/07/install-opencv-without-root-permission.html

The program was tested with openCV 3.2.0.
Here we don't need the python support of OpenCV, so the python version doesn't need to be specified during the compilation

### Xitari
Xitari is the atari emulator used. For more detail go to https://github.com/deepmind/xitari

### gflags
Gflags is a usefull google library for flag

If the cluster is (still :/) not update, edit CMakeLists.txt to change minimum version to 2.8.11




