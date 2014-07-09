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
	Mat src, gray, sobel, gaussian, canny_output, eroded, overlay;
	Mat contours;
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
			cvtColor(src, gray, CV_BGR2GRAY);


			//Sobel filter for horizontal lines, then canny to detect edges	
			Sobel(gray, sobel, -1, 0, 1, 3, 1);
			GaussianBlur(sobel, gaussian, Size(5,5), 2, 2);
			Canny(gaussian, canny_output, 40, 350, 3);

/*			std::vector<std::vector<Point> > contours;
			std::vector<Vec4i> hierarchy;

			findContours(dest, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

			drawContours( dest_eroded, contours, -1, Scalar(255, 0, 0), 2, 8, hierarchy, 0, Point());
		

			copyMakeBorder( dest_eroded, overlay, 20, 20, 20, 20, BORDER_CONSTANT, Scalar(255, 255, 255));
*/


			std::vector<std::vector<Point> > contours;
			std::vector<Vec4i> hierarchy;

			// Find contours
			findContours( canny_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
  			// Draw contours
			 
			Mat drawing = Mat::zeros( canny_output.size(), CV_8UC3 );
			for( int i = 0; i< contours.size(); i++ ){
				if(contours[i].size() > 20){
					drawContours( drawing, contours, i, Scalar(255, 255, 255), 2, 8, hierarchy, 0, Point() );
				}
/*				else{
					drawContours( drawing, contours, i, Scalar(255, 255, 255), 2, 8, hierarchy, 0, Point() );
				}*/
			}

			Mat element = getStructuringElement(MORPH_CROSS, Size(5, 5), Point(2, 2));
 			dilate(drawing, eroded, element);

 			Size image_size = eroded.size();

			for (int i = 0; i < eroded.rows; i++){
				for (int j = eroded.cols; j >= 1; --j){
					int intensity = (int)eroded.at<uchar>(i, j);
					if (intensity == 255){
						int distance = j; //modify this later
						line(eroded, Point(j, image_size.height), Point(j, i), Scalar(0,0,255), 1, 8, 0);
					}
				}
			}

 			int border = 20;
 			copyMakeBorder( eroded, overlay, border, border, border, border, BORDER_CONSTANT, Scalar(255, 255, 255));

			line(overlay, Point(border,border), Point(border, image_size.height + border + 10), Scalar(0,255,0), 3, 8, 0);
			line(overlay, Point(border - 10,image_size.height + border), Point(image_size.width + border, image_size.height + border), Scalar(0,255,0), 3, 8, 0);

			// Show in a window
			imshow("drawing", overlay);

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