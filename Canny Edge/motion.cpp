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
	Mat src, gray, sobel, gaussian, canny_output, eroded, overlay_color, overlay;
	Mat contours, eroded_raw;
	Mat previous;
	double minVal; double maxVal; Point minLoc; Point maxLoc;

	IplImage* color_img;
	CvCapture* cv_cap = cvCreateFileCapture("underwater1.webm"); //previous video
	//CvCapture* cv_cap = cvCaptureFromCAM(1); //USB Cam

/*	previous1 = cvQueryFrame(cv_cap);
	previous1 = Mat::zeros(1, 1, CV_64F);

	previous2 = previous3 = previous1;*/

	std::vector<int> distance_vector;

	bool start = true; 

	for(;;){
		color_img = cvQueryFrame(cv_cap);

		if (color_img != 0){
			src = color_img;

			//Changing color image to gray
			cvtColor(src, gray, CV_BGR2GRAY);
			
			if (start){
				printf("Initialize\n");
				previous = gray;
				start = false;
			}

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
			Mat drawing = Mat::zeros( canny_output.size(), CV_8UC3 );
			Mat element = getStructuringElement(MORPH_CROSS, Size(5, 5), Point(2, 2));
			Size image_size = canny_output.size();
 			int border = 20;


			std::vector<std::vector<Point> > contours;
			std::vector<Vec4i> hierarchy;
			std::vector<int> maxy_values;

			// Find contours
			findContours( canny_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
  			// Draw contours
			//std::cout << contours.size() << std::endl;
			if (contours.size() > 20){
				//imshow("eroded", previous);
				goto draw_previous;
			}



			for( int i = 0; i < contours.size(); i++ ){
				if(contours[i].size() > 5){
					drawContours( drawing, contours, i, Scalar(255, 255, 255), 2, 8, hierarchy, 0, Point() );
					std::vector<int> y_values;
					int max_y;
					for(int j = 0; j < contours[i].size(); j++){
						y_values.push_back(contours[i][j].y);
					}
					max_y = *std::max_element(y_values.begin(), y_values.end());
					maxy_values.push_back(max_y);

				}
/*				else{
					drawContours( drawing, contours, i, Scalar(255, 255, 255), 2, 8, hierarchy, 0, Point() );
				}*/
			}

 			dilate(drawing, eroded_raw, element);
			
			cvtColor(eroded_raw, eroded, CV_BGR2GRAY);
 			
 			eroded = previous;

/*					minMaxLoc( eroded_raw, &minVal, &maxVal, &minLoc, &maxLoc);
					std::cout << minVal << std::endl;
*/
			for (int i = 0; i < eroded.cols; i++){
				for (int j = 0; j < eroded.rows; j++){
					int intensity = (int)eroded.at<uchar>(j, i);
					if (intensity > 240){
						eroded.at<uchar>(j, i) = 150;
						break;
					}
					else{
						eroded.at<uchar>(j, i) = 50;
					}
				}
			}

			imshow("original", src);
			
			cvtColor(eroded, overlay_color, CV_GRAY2BGR);
			
 			copyMakeBorder(overlay_color, overlay, border, border, border, border, BORDER_CONSTANT, Scalar(255, 255, 255));

			line(overlay, Point(border,border), Point(border, image_size.height + border + 10), Scalar(0,255,0), 3, 8, 0);
			line(overlay, Point(border - 10,image_size.height + border), Point(image_size.width + border, image_size.height + border), Scalar(0,255,0), 3, 8, 0);
			
			for (int z = 0; z < maxy_values.size(); z++){
				line(overlay, Point(border, maxy_values[z] + border), Point(image_size.width + border, maxy_values[z] + border), Scalar(0,255,255), 2, 8, 0);
				char text[255];
				sprintf(text, "Distance %d", maxy_values[z]);
				putText(overlay, text, Point(border, maxy_values[z] + 10), 
    								FONT_HERSHEY_COMPLEX_SMALL, 0.8, Scalar(200,200,250), 1, CV_AA);
			}

			int total_max = *std::max_element(maxy_values.begin(), maxy_values.end());
			char text1[255];
			sprintf(text1, "Closests %d", total_max);
			putText(overlay, text1, Point(image_size.width -100, border), 
    			FONT_HERSHEY_COMPLEX_SMALL, 0.8, Scalar(200,200,250), 1, CV_AA);

			draw_previous:						
			// Show in a window
			imshow("drawing", overlay);

			c = cvWaitKey(10);
			if (c == 27){
				break;



			}
		}
	

	cvReleaseCapture( &cv_cap);
	cvDestroyWindow("Video");
	return 0;
}
