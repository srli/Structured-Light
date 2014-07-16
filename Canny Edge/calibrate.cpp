#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <typeinfo> 
#include <math.h>
#include <cmath>

using namespace cv;

int main(int argc, char** argv){

	int c, key, total_max;
	//Creates matrices available for filling later
	Mat src, gray, sobel, gaussian, canny_output, canny_eroded, overlay_color, overlay;
	Mat contours_output, eroded_raw;
	Mat previous;

	Mat color_img;

	int original_height = 5; //in cm
	double laser_theta = CV_PI/6; //degrees
	int focal_length = 1; //in cm

	src = imread("50c.jpg", 1);
	printf("Image loaded\n");

	cvtColor(src, gray, CV_BGR2GRAY);
	Canny(gray, canny_output, 40, 350, 3);
	Mat element = getStructuringElement(MORPH_CROSS, Size(5, 5), Point(2, 2));
	dilate(canny_output, canny_eroded, element);
	
	std::vector<Vec4i> lines;
	HoughLinesP(canny_eroded, lines, 1, CV_PI/180, 80, 150, 2);

	for(size_t i=0; i < lines.size(); i++){
		//Drawing each line that we've found
		line(src, Point(lines[i][0], lines[i][1]), Point(lines[i][2], lines[i][3]), Scalar(0,255,0), 2, 8);
	}

	imshow("src", src);
	//imshow("dst", dst);

	imshow("canny", canny_eroded);
	waitKey(0);

	return 0;
}