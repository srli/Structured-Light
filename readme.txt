This is the folder containing Sophie's Summer 2014 research.

The final code is in the "Structured Light Object Detection" folder.


STRUCTURED LIGHT
===================
You have to calibrate the camera before you can accurately run the actual script.

The executables are in the /bin folder, "calib" is the calibration script and "motion" is the object detection script. Follow onscreen directions to calibrate.

To compile the code, you need to link the OpenCV libraries and use special g++ flags to account for multitreading in the calibration script.

TO COMPILE:
g++ laser_calibrate.cpp $(pkg-config --libs opencv) -o calib -pthread -std=c++11
g++ motion.cpp $(pkg-config --libs opencv) -o motion

NOTE: You need to have OpenCV installed in order to run both scripts.



OTHER CODE
==================
I also did a lot of other code stuff this summer. In the "Useful Code" folder, you'll find various scripts. 

The Circle Detection script was also aimed at emulating April Tags, but based on the size and location of spheres in the USB camera image plane.

The "Structured Light" folder contains various takes on solving the structured light problem, including Hough Lines and other kinds of image transforms.
