#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <typeinfo> 
#include <math.h>

using namespace cv;

int main(int argc, char** argv){

	int c, key, total_max;
	//Creates matrices available for filling later
	Mat src, gray, sobel, gaussian, canny_output, eroded, overlay_color, overlay;
	Mat contours_output, eroded_raw;
	Mat previous;

	IplImage* color_img;
	CvCapture* cv_cap = cvCreateFileCapture("underwater1.webm"); //previous video
	//CvCapture* cv_cap = cvCaptureFromCAM(1); //USB Cam

	Mat element = getStructuringElement(MORPH_CROSS, Size(5, 5), Point(2, 2));	

 	int border = 20;
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


			Mat drawing = Mat::zeros( canny_output.size(), CV_8UC3 );

			Size image_size = canny_output.size();

			std::vector<int> distance_vector;
			std::vector<int> y_values;

			std::vector<int> maxy_values;

			std::vector<std::vector<Point> > contours;
			std::vector<Vec4i> hierarchy;
			
			// Find contours
			findContours( canny_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
  			
  			// Draw contours
			if (contours.size() > 20){
				//imshow("eroded", previous);
				goto draw_previous;
			}



			for( int l = 0; l < contours.size(); l++ ){
				if(contours[l].size() > 5){
					drawContours( drawing, contours, l, Scalar(255, 255, 255), 2, 8, hierarchy, 0, Point() );
					std::fill(y_values.begin(), y_values.end(),0);
					int max_y;
					for(int m = 0; m < contours[l].size(); m++){
						y_values.push_back(contours[l][m].y);
					}
					max_y = *std::max_element(y_values.begin(), y_values.end());
					std::cout << "max_y " << max_y << std::endl;
					maxy_values.push_back(max_y);

				}
/*				else{
					drawContours( drawing, contours, i, Scalar(255, 255, 255), 2, 8, hierarchy, 0, Point() );
				}*/
			}

 			dilate(drawing, eroded_raw, element);
			
			cvtColor(eroded_raw, eroded, CV_BGR2GRAY);
 			
 			previous = eroded;


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

			if (!maxy_values.empty()) {
				total_max = *std::max_element(maxy_values.begin(), maxy_values.end());
			} else {
				total_max = 0;
			}

			char text1[255];
			sprintf(text1, "Closest %d", total_max);
			putText(overlay, text1, Point(image_size.width -100, border+20), 
    			FONT_HERSHEY_COMPLEX_SMALL, 0.8, Scalar(200,200,250), 1, CV_AA);

			draw_previous:						
			// Show in a window
			imshow("original", src);
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
