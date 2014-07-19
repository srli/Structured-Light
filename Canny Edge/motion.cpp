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

	int original_height = 2; //in cm
	double laser_theta = CV_PI / 12; //degrees
	//double laser_theta = 0; //degrees
	double focal_length = 0.3; //in cm

	VideoCapture cv_cap("media/underwater1.webm"); //previous video
	cv_cap.open("media/underwater1.webm");

/*	VideoCapture cv_cap(1);
	cv_cap.open(1);
*/
/*
	VideoCapture cv_cap("media/distance_test.mkv");
	cv_cap.open("media/distance_test.mkv");*/

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

			//Creating Mat to fill in lines later
			Mat drawing = Mat::zeros( canny_output.size(), CV_8UC3 );

			//Allows us to scale code to image width
			Size image_size = canny_output.size();

			//drawing stuff in occupancy grid
			Mat occupancy_grid = Mat::zeros( canny_output.size(), CV_8UC3 );
			std::vector<Point> occupancy_points;

			for(int g = 0; g < 485; g += 35){
				line(occupancy_grid, Point(g, 0), Point(g, 450), Scalar(0, 0, 255), 1, 8, 0);
				line(occupancy_grid, Point(0, g), Point(450, g), Scalar(0, 0, 255), 1, 8, 0);
			}

			//Vectors defined to hold found contours
			std::vector<std::vector<Point> > contours;
			std::vector<Vec4i> hierarchy;
			
			//Defined here to find countour endpoints
			int proj_x, proj_y = 0;

			//Values used to hold calculated end points of contours
			std::vector<int> values;
			std::vector<int> distance_values;

			// Find contours
			findContours( canny_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
  			
  			//If we find too many contours, skips all calculations and waits
			if (contours.size() > 20){
				//imshow("eroded", previous);
				goto draw_previous;
			}

			// Draw contours
			for( int l = 0; l < contours.size(); l++ ){
				if(contours[l].size() > 5){
					drawContours( drawing, contours, l, Scalar(255, 255, 255), 2, 8, hierarchy, 0, Point() );
				}
			}

			//Thickens found countour lines
 			dilate(drawing, eroded_raw, element);
			
			cvtColor(eroded_raw, eroded, CV_BGR2GRAY);

			//Sets previous image, for skipping purposes
 			previous = eroded;
			
 			//Finds intensity per column
			for (int i = 0; i < eroded.cols; i++){
				for (int j = 0; j < eroded.rows; j++){
					int intensity = (int)eroded.at<uchar>(j, i);
					if (intensity > 240){
						eroded.at<uchar>(j, i) = 150;
						if (abs(j - proj_y) > 5){  //&& abs(i - proj_x) > 10
							
							values.push_back(proj_y); //These let us draw the end points
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
			
			//Brings back color to the overlay
			cvtColor(eroded, overlay_color, CV_GRAY2BGR);
			
			if ( values.size() > 2){
				//Draws countour enpoint lines
				for (int s = 2; s < values.size(); s +=4 ){
					line(overlay_color, Point(values[s + 1], values[s]), Point(values[s + 1], image_size.height), Scalar(0,244,0), 2, 8, 0);
					line(overlay_color, Point(values[s + 3], values[s + 2]), Point(values[s + 3], image_size.height), Scalar(244,244,0), 2, 8, 0);
					line(overlay_color, Point(values[s + 1], values[s]), Point(values[s + 3], values[s + 2]), Scalar(0,244,244), 2, 8, 0);

					double distance;
					distance = ((values[s] + values[s+2]) / 2) * 0.0003; //pixel distance from top of screen converted to cm
					

					//CURRENTLY A SHITSHOW, WILL FIX LATER


					//distance = (values[s] + values[s+2]) / 2;
					//printf("%f\n", distance);

					float distance_x3;
/*					distance_x3 = (distance * original_height) / tan(laser_theta);
					printf("%f\n", distance_x3);
					int round_distance;
					round_distance = round(distance_x3);
					distance_values.push_back(round_distance);
*/
					distance_x3 = (focal_length * original_height) / (distance + focal_length*tan(laser_theta));

					//distance_x3 = (-original_height + sqrt((original_height * original_height - 4 * tan(laser_theta) * (-6 * distance)))) / (2 * tan(laser_theta));
//					float x2 = (-original_height - sqrt((original_height * original_height - 4 * tan(laser_theta) * (-6 * distance)))) / (2 * tan(laser_theta));

					printf("%f\n", distance_x3);


					//We have left and right bearing, does trig to find angle
					Point bearing;
					bearing = Point((values[s + 1] - image_size.width/2) * 1/6, (values[s + 3] - image_size.width/2) * 1/6);

					int bearing_left= round(atan(bearing.x / distance_x3) * 180 / CV_PI);
					int bearing_right=round(atan(bearing.y / distance_x3) * 180 / CV_PI);

					//Puts distance and bearings on the overlay screen
					char text[255];
					sprintf(text, "D: %2f", distance_x3);
					putText(overlay_color, text, Point(values[s + 1] + 10, values[s] + 30), 
	    								FONT_HERSHEY_COMPLEX_SMALL, 0.5, Scalar(200,200,250), 1, CV_AA);

					char text2[255];
					sprintf(text2, "B: %d, %d", bearing_left, bearing_right);
					putText(overlay_color, text2, Point(values[s + 1] + 10, values[s] + 40), 
	    								FONT_HERSHEY_COMPLEX_SMALL, 0.5, Scalar(200,200,250), 1, CV_AA);

					
					//We being to draw occupancy grid here, each grid is 10cm by 6degrees, or 32 * 32 pixels

					//Rounds to nearest 10th
					int distance_coordinate =  round((distance_x3/10)*10) * 32;

					//Left square, rounds to nearest 6th
					int bearing_coordinates_l = round((bearing_left/6)*6);
					int left_add;
					if(bearing_coordinates_l > 0){
						left_add = 6;
					}
					else{
						left_add = -6;
					}
					Point tl_left(bearing_coordinates_l * 32, round((distance_x3/10)*10) * 32);
					Point br_left((bearing_coordinates_l + left_add) *32, round((distance_x3/10)*10 - 10) * 32);

					//Right square
					int bearing_coordinates_r = round((bearing_right/6)*6);
					int right_add;
					if(bearing_coordinates_r > 0){
						right_add = 6;
					}
					else{
						right_add = -6;
					}

					Point tl_right(bearing_coordinates_r * 32, round((distance_x3/10)*10) * 32);
					Point br_right((bearing_coordinates_r + right_add) * 32, round((distance_x3/10)*10 - 10) * 32);
					//std::cout << tl_right.x << " : " << tl_right.y << std::endl;

					//Draws left and right squares
					rectangle(occupancy_grid, tl_left, br_left, Scalar(255,255,0), -1, 8, 0);
					rectangle(occupancy_grid, tl_right, br_right, Scalar(255, 0, 255), -1, 8, 0);

					//Connects squares for the distance of the curve
					for(int j = bearing_coordinates_l * 32; j < bearing_coordinates_r * 32; j += 6*32){
						rectangle(occupancy_grid, Point(j, distance_coordinate), Point(j + 32*6, distance_coordinate - 320), Scalar(255, 255, 0), -1, 8, 0);
					}

				}
			}
			
			//Makes border, for better visuals
 			copyMakeBorder(overlay_color, overlay, border, border, border, border, BORDER_CONSTANT, Scalar(255, 255, 255));

 			//Finds max distance
			if (!distance_values.empty()) {
				total_max = *std::max_element(distance_values.begin(), distance_values.end());
				if (total_max > 500){
					total_max = 500;
				}
			} 
			else {
				total_max = 0;
			}

			//Prints max distance
			char text1[255];
			sprintf(text1, "Closest %d", total_max);
			putText(overlay, text1, Point(image_size.width -100, border+20), 
    			FONT_HERSHEY_COMPLEX_SMALL, 0.8, Scalar(200,200,250), 1, CV_AA);

			//We skip to here if > 20 contours are found
			draw_previous:
			// Show in a window
			imshow("original", src);
			if(!overlay.empty()){
				imshow("canny edge", canny_output);
				imshow("drawing", overlay);
				imshow("occupancy_grid", occupancy_grid);
			}
			c = cvWaitKey(100);
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
