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

			Mat element = getStructuringElement(MORPH_CROSS, Size(5, 5), Point(2, 2));
			dilate(dest, dest_eroded, element);
	
			//addWeighted(dest_eroded, 0.5, previous3, 0.5, 0.0, overlay);

			bitwise_or(dest_eroded, gray, overlay);

			previous1 = dest_eroded;
			previous2 = previous1;
			previous3 = previous2;

			imshow("Canny Edge", previous3);
			imshow("Overlay", overlay);

/*	
			imshow("Grayscale", gray);
			imshow("Sobel", sobel);
			imshow("Canny Edge", dest_eroded);
			imshow("Gaussian", gaussian);
*/
			c = cvWaitKey(1000);
			if (c == 27){
				break;



			}
		}
	}
	
	cvReleaseCapture( &cv_cap);
	cvDestroyWindow("Video");
	return 0;
}