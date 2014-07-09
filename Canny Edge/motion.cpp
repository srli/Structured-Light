#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <typeinfo> 
#include <math.h>

using namespace cv;

int main(int argc, char** argv){

	int c, key;
	//Creates matrices available for filling later
	Mat src, gray, sobel, gaussian, dest, dest_eroded, overlay;
	Mat tmp, alpha;
	Mat rgb[3];
	Mat previous1, previous2, previous3;

	IplImage* color_img;
	CvCapture* cv_cap = cvCreateFileCapture("underwater1.webm"); //previous video
	//CvCapture* cv_cap = cvCaptureFromCAM(1); //USB Cam

/*	previous1 = cvQueryFrame(cv_cap);
	previous1 = Mat::zeros(1, 1, CV_64F);

	previous2 = previous3 = previous1;*/

	std::vector<int> distance_vector;

	int z = 0;

	for(;;){
		color_img = cvQueryFrame(cv_cap);

		z += 1;

		if (color_img != 0){
			src = color_img;



			//Changing color image to gray
			cvtColor(src, tmp, CV_BGR2GRAY);


/*			threshold(tmp, alpha, 100, 255, THRESH_BINARY);
			split(src, rgb);
			Mat rgba[4] = {rgb[0], rgb[1], rgb[2], alpha};
			merge(rgba, 4, gray);
*/
			gray = tmp;

			if (z == 1){
				printf("Initializing\n");
				previous1 = gray;
				previous2 = gray;
				previous3 = gray;
			}


			//Sobel filter for horizontal lines, then canny to detect edges	
			Sobel(gray, sobel, -1, 0, 1, 3, 1);
			GaussianBlur(sobel, gaussian, Size(5,5), 2, 2);
			Canny(gaussian, dest, 40, 350, 3);

			std::vector<std::vector<Point> > contours;
			std::vector<Vec4i> hierarchy;

			findContours(dest, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

/*			for( int i = 0; i < contours.size(); i++){
				drawContours( dest_eroded, contours, i, Scalar(255, 0, 0), 2, 8, hierarchy, 0, Point());
			}*/

/*			Mat element = getStructuringElement(MORPH_CROSS, Size(5, 5), Point(2, 2));
			dilate(dest, dest_eroded, element);
	
			//addWeighted(dest_eroded, 0.5, previous3, 0.5, 0.0, overlay);

			bitwise_or(dest_eroded, gray, overlay);
*/
			/*previous1 = dest_eroded;
			previous2 = previous1;
			previous3 = previous2;*/
/*
			std::vector<Vec2f> lines;

			HoughLines(dest_eroded, lines, 1, CV_PI/180, 50);

			std::cout << "number of lines: " << lines.size() << std::endl;

			for(size_t i = 0; i < lines.size(); i++){

				line(dest_eroded, Point(lines[i][0], lines[i][1]), Point(lines[i][2], lines[i][3]), Scalar(0,255,0), 1, 8);

			}

*/

			copyMakeBorder( dest_eroded, overlay, 20, 20, 20, 20, BORDER_CONSTANT, Scalar(255, 255, 255));

		//	imshow("Canny Edge", dest_eroded);
			imshow("Overlay", overlay);

/*	
			imshow("Grayscale", gray);
			imshow("Sobel", sobel);
			imshow("Canny Edge", dest_eroded);
			imshow("Gaussian", gaussian);
*/
			c = cvWaitKey(10);
			if (c == 27){
				break;



			}
		}
	}
	
	cvReleaseCapture( &cv_cap);
	cvDestroyWindow("Video");
	return 0;
}