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
	Mat src, gray, sobel, gaussian, canny_output, eroded, overlay_color, overlay;
	Mat contours_output, eroded_raw;
	Mat previous;

	Mat color_img;

	int original_height = 3.81; //in cm
	double laser_theta = CV_PI/6; //degrees
	double focal_length = 4; //in cm

/*	VideoCapture cv_cap("media/underwater1.webm"); //previous video
	cv_cap.open("media/underwater1.webm");
*/
	VideoCapture cv_cap(0);
	cv_cap.open(0);

	cv_cap.set(CV_CAP_PROP_FRAME_WIDTH, 640);
	cv_cap.set(CV_CAP_PROP_FRAME_HEIGHT, 480);

	 //= cvCaptureFromCAM(0); //USB Cam


	Mat element = getStructuringElement(MORPH_CROSS, Size(5, 5), Point(2, 2));	

 	int border = 20;
	bool start = true; 

	for(;;){
		color_img = cv_cap.grab();

		if (cv_cap.read(color_img)){
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

			std::vector<int> distance_values;
			std::vector<int> values;

			std::vector<std::vector<Point> > contours;
			std::vector<Vec4i> hierarchy;
			int proj_x, proj_y = 0;
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
				}
			}

 			dilate(drawing, eroded_raw, element);
			
			cvtColor(eroded_raw, eroded, CV_BGR2GRAY);
 			
 			previous = eroded;
			

			for (int i = 0; i < eroded.cols; i++){
				for (int j = 0; j < eroded.rows; j++){
					int intensity = (int)eroded.at<uchar>(j, i);
					if (intensity > 240){
						eroded.at<uchar>(j, i) = 150;
						if (abs(j - proj_y) > 5){  //&& abs(i - proj_x) > 10
							
							values.push_back(proj_y);
							values.push_back(proj_x);

							values.push_back(j);
							values.push_back(i);
						}
						proj_y = j;
						proj_x = i;
						break;
					}
					else if (i == eroded.cols - 1 && j == eroded.rows - 1){
						values.push_back(proj_y);
						values.push_back(proj_x);
					}

					else{
						eroded.at<uchar>(j, i) = 50;
					}
				}
			}
			
			cvtColor(eroded, overlay_color, CV_GRAY2BGR);
			
			if ( values.size() > 2){
				for (int s = 2; s < values.size(); s +=4 ){
					line(overlay_color, Point(values[s + 1], values[s]), Point(values[s + 1], image_size.height), Scalar(0,244,0), 2, 8, 0);
					line(overlay_color, Point(values[s + 3], values[s + 2]), Point(values[s + 3], image_size.height), Scalar(244,244,0), 2, 8, 0);
					line(overlay_color, Point(values[s + 1], values[s]), Point(values[s + 3], values[s + 2]), Scalar(0,244,244), 2, 8, 0);

/*					float distance;
					//distance = image_size.height - ((values[s] + values[s+2]) / 2); //pixel distance from bottom of screen
					
					distance = (values[s] + values[s+2]) / 2;
					//printf("%f\n", distance);

					float distance_x3;
					distance_x3 = (-focal_length * original_height) / (distance + (focal_length * tan(laser_theta)));
					printf("%f\n", distance_x3);
					int round_distance;
					round_distance = round(distance_x3);
					distance_values.push_back(round_distance);





					Point bearing;
					bearing = Point(values[s + 1], values[s + 3]);

					int bearing_left = round(((bearing.x + (image_size.width / 2))*distance_x3) / focal_length);
					int bearing_right = round(((bearing.y + (image_size.width / 2))*distance_x3) / focal_length);

					char text[255];
					sprintf(text, "D: %2f", distance);
					putText(overlay_color, text, Point(values[s + 1] + 10, values[s] + 30), 
	    								FONT_HERSHEY_COMPLEX_SMALL, 0.5, Scalar(200,200,250), 1, CV_AA);

					char text2[255];
					sprintf(text2, "B: %d, %d", bearing_left, bearing_right);
					putText(overlay_color, text2, Point(values[s + 1] + 10, values[s] + 40), 
	    								FONT_HERSHEY_COMPLEX_SMALL, 0.5, Scalar(200,200,250), 1, CV_AA);*/
				}
			}
			
 			copyMakeBorder(overlay_color, overlay, border, border, border, border, BORDER_CONSTANT, Scalar(255, 255, 255));

			//line(overlay, Point(border,border), Point(border, image_size.height + border + 10), Scalar(0,255,0), 3, 8, 0);
			//line(overlay, Point(border - 10,image_size.height + border), Point(image_size.width + border, image_size.height + border), Scalar(0,255,0), 3, 8, 0);

			if (!distance_values.empty()) {
				total_max = *std::max_element(distance_values.begin(), distance_values.end());
				if (total_max > 500){
					total_max = 500;
				}
			} 
			else {
				total_max = 0;
			}

			char text1[255];
			sprintf(text1, "Closest %d", total_max);
			putText(overlay, text1, Point(image_size.width -100, border+20), 
    			FONT_HERSHEY_COMPLEX_SMALL, 0.8, Scalar(200,200,250), 1, CV_AA);

			draw_previous:
			//printf("showing images\n");
			// Show in a window
			imshow("original", src);
			if(!overlay.empty()){
				imshow("canny edge", canny_output);
				imshow("drawing", overlay);
			}
			c = cvWaitKey(30);
			//getchar();
			if (c == 27){
				break;
			}
		}
	}
	

	cv_cap.release();
	//cvDestroyWindow("Video");
	return 0;
}
