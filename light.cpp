#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <stdio.h>
#include <typeinfo>
#include <math.h>

using namespace cv;
using namespace std;

int main(int argc, char *argv[])
{
	int c, key;

	Mat src, gaussian_result;
	Mat imgHSV, imgThreshed;

	IplImage* color_img;
	CvCapture* cv_cap = cvCaptureFromCAM(0);
	cvNamedWindow("Original Image", 0);
	cvNamedWindow("HSV Image", 0);
	cvNamedWindow("Gaussian Blur", 0);
	printf("Press a to see data\n");

	for(;;){
		color_img = cvQueryFrame(cv_cap); //gets one frame
		if (color_img != 0){
			src = color_img;
			double line_to_center = 0;
			double distance_changes = 0;

			CvSize dim = cvGetSize(color_img);


			//Changing color image to HSV to filter color
			cvtColor(src, imgHSV, CV_BGR2HSV);
			inRange(imgHSV, Scalar(60, 70, 70), Scalar(120, 255, 255), imgThreshed);

			//Reduces noise
			GaussianBlur(imgThreshed, gaussian_result, Size(3,3), 2, 2);

			//Create vector to hold the detected lines
			vector<Vec4i> lines;
			HoughLinesP(gaussian_result, lines, 1, CV_PI/180, 80, 50, 5);
			for(size_t i=0; i < lines.size(); i++){

				line(src, Point(lines[i][0], lines[i][1]), 
					Point(lines[i][2], lines[i][3]), Scalar(0,255,0), 1, 8);

				double distance = (lines[i][1] - dim.height/2);
				if (abs(distance - line_to_center) > 25){
					line(src, Point(lines[i][0], lines[i][1]), Point(lines[i][0], dim.height/2), Scalar(0,0,255), 1, 8);
					line(src, Point(lines[i][2], lines[i][3]), Point(lines[i][2], dim.height/2), Scalar(0,0,255), 1, 8);
					line_to_center = distance;
					distance_changes += 1;
				}

				key = cvWaitKey(50);

                if(key == 97){

                cout << "number of lines  " << lines.size() << endl;
                cout << "current distance  " << distance << endl;
                cout << "-------------" << endl;
                }


			}
			//cout << lines.size() << endl;
			Point left_center(0, dim.height/2);
			Point right_center(dim.width, dim.height/2);
			line(src, left_center, right_center, Scalar(255,0,0), 1, 8);
		}

		imshow("Original Image", src);
		imshow("Gaussian Blur", imgThreshed);
		imshow("HSV Image", imgHSV);



		c = cvWaitKey(10);
		if(c == 27){
			break;

		}

	}
	cvReleaseCapture( &cv_cap);
	cvDestroyWindow("Video");

	return 0;
}