#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <typeinfo> 
#include <math.h>
#include <cmath>

#include <fstream>

using namespace cv;

class Block
{

	public:
	bool occupied;
	Point position, diag_pos;
	int block_width;
	Point distance_range, angle_range; //actually, these are tuples. I'm just lazy in my implementation


	void initiate(int, int, int, int);
	void draw_square(Mat);
	void change_occupancy();
};

void Block::initiate(int posx, int posy, int d, int ang){
	occupied = false;
	block_width = 35;
	position = Point(posx, posy);
	diag_pos = Point(posx + block_width, posy + block_width);
	distance_range = Point(d, d+10);
	angle_range = Point(ang, ang+12);
}

void Block::draw_square(Mat image){
	int line;
	if (occupied){
		line = -1;
	}
	else{
		line = 1;
	}
	rectangle(image, position, diag_pos, Scalar(0,244,244), line, 8, 0);
}

void Block::change_occupancy(){
	occupied = true;
}

int main(int argc, char** argv){

	int c, key, total_max;
	//Creates matrices available for filling later
	Mat src, gray, sobel, gaussian, canny_output, eroded, overlay_color, overlay;
	Mat contours_output, eroded_raw;
	Mat previous;

	Mat color_img;

	std::fstream calibration_data("exp_reg.txt", std::ios_base::in);
	float A, r;

	calibration_data >> A >> r;

	int original_height = 6; //in cm
	double laser_theta = 0.42; //degrees
	//double laser_theta = 0; //degrees
	double focal_length = 0.3; //in cm
/*
	VideoCapture cv_cap("media/underwater1.webm"); //previous video
	cv_cap.open("media/underwater1.webm");*/

/*	VideoCapture cv_cap(1);
	cv_cap.open(1);
*/

	//Scaled tests start at 20cm, then go up by increments of 10cm
	VideoCapture cv_cap("media/scaled_t1.webm");
	cv_cap.open("media/scaled_t1.webm");

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
			Mat occupancy_grid = Mat::zeros(530, 1060, CV_8UC3 );
			std::vector<Block> grid_blocks;
			for (int x = 0; x < 30; x++){
				for (int y = 0; y < 15; y++){
					Block block;
					block.initiate(x*35, y*35, x*10, -45 + y*3);
					//block.draw_square(occupancy_grid);
					grid_blocks.push_back(block);
				}
			}

/*			std::vector<Point> occupancy_points;

			for(int g = 0; g < 485; g += 35){
				line(occupancy_grid, Point(g, 0), Point(g, 450), Scalar(0, 0, 255), 1, 8, 0);
				line(occupancy_grid, Point(0, g), Point(450, g), Scalar(0, 0, 255), 1, 8, 0);
			}*/

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


					int mid_line = image_size.height / 2;
 					int d1 = mid_line - values[s];

					int distance_x3;
					distance_x3 = round(A * (pow(r, d1)));

					distance_values.push_back(distance_x3);
					//printf("%f\n", distance_x3);


					Point bearing;
					bearing = Point((values[s + 1] - image_size.width/2), (values[s + 3] - image_size.width/2));

					//We have left and right bearing, does trig to find angle
/*					double xbearing = A * (pow(r, bearing.x));
					double ybearing = A * (pow(r, bearing.y));*/
					
					//std::cout << "xbearing " << xbearing << " ybearing " << ybearing << std::endl;

					float bearing_left= atan2(bearing.x, d1) * (180 / CV_PI);
					float bearing_right= atan2(bearing.y, d1) * (180 / CV_PI);

					//Puts distance and bearings on the overlay screen
					char text[255];
					sprintf(text, "D1: %d cm", distance_x3);
					putText(overlay_color, text, Point(values[s + 1] + 10, values[s] + 30), 
	    								FONT_HERSHEY_COMPLEX_SMALL, 0.5, Scalar(200,200,250), 1, CV_AA);

					char text2[255];
					sprintf(text2, "B: %2f, %2f", bearing_left, bearing_right);
					putText(overlay_color, text2, Point(values[s + 1] + 10, values[s] + 40), 
	    								FONT_HERSHEY_COMPLEX_SMALL, 0.5, Scalar(200,200,250), 1, CV_AA);

					
					//We being to draw occupancy grid here, each grid is 10cm by 12 degrees, or 32 * 32 pixels
					for (int g = 0; g < grid_blocks.size(); g++){
						if (distance_x3 > grid_blocks[g].distance_range.x && distance_x3 < grid_blocks[g].distance_range.y){
							for (int v = 0; v < grid_blocks.size(); v++){
								if (bearing_left > grid_blocks[g].angle_range.x && bearing_left< grid_blocks[g].angle_range.y){
									grid_blocks[g].change_occupancy();
								}

								if (bearing_right > grid_blocks[g].angle_range.x && bearing_right< grid_blocks[g].angle_range.y){
									grid_blocks[g].change_occupancy();
								}
							}	
						}
					}
				}
			}
			
			for (int k = 0; k < grid_blocks.size(); k++){
				grid_blocks[k].draw_square(occupancy_grid);
			}


			//Makes border, for better visuals
 			copyMakeBorder(overlay_color, overlay, border, border, border, border, BORDER_CONSTANT, Scalar(255, 255, 255));

 			//Finds max distance
			if (!distance_values.empty()) {
				total_max = *std::min_element(distance_values.begin(), distance_values.end());
				if (total_max > 70){
					total_max = 70;
				}
			}
			else {
				total_max = 0;
			}

			//Prints max distance
			char text1[255];
			sprintf(text1, "Closest %d cm", total_max);
			putText(overlay, text1, Point(image_size.width -150, border+20), 
    			FONT_HERSHEY_COMPLEX_SMALL, 0.8, Scalar(200,200,250), 1, CV_AA);

			//We skip to here if > 20 contours are found
			draw_previous:
			// Show in a window
			imshow("original", src);
			if(!overlay.empty()){
				//imshow("canny edge", canny_output);
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
