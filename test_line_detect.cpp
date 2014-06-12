#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <stdio.h>
#include <typeinfo>
#include <math.h>

using namespace cv;
//using namespace std;

class One_Line {
	int distance, length;
	int total_distance, number_values;
	double average_value;
public:
	void initiate();
	void add_value (double);
	double average(); 
};

void One_Line::initiate(){
	printf("Initiating new line\n");
	total_distance = 0;
	number_values = 0;
	average_value = 0.0;
}

void One_Line::add_value(double value){
	total_distance += value;
	number_values += 1;

/*	std::cout << "number value of this line is  " << number_values << std::endl;
	std::cout << "total distance is  " << total_distance << std::endl;*/
}

double One_Line::average(){

	std::cout << "number value of this line is  " << number_values << std::endl;
	std::cout << "total distance is  " << total_distance << std::endl;
	std::cout << "average is  " << average_value << std::endl;
	average_value = (total_distance/number_values);
	return average_value;
}

int main(){
	Mat src, gaussian_result;
	Mat imgHSV, imgThreshed;

	//IplImage* color_img;
	cvNamedWindow("Original Image", 0);
	//cvNamedWindow("HSV Image", 0);
	//cvNamedWindow("Gaussian Blur", 0);

	src = imread("lines.jpg", 1);
	printf("Image loaded\n");

	std::vector<One_Line> onelineobjects (10);
	//onelineobjects.reserve(10);
	int k = 0;

	//Creating the comparative line
	One_Line baseline, new_value;
	baseline.initiate();
	baseline.add_value(0);
	onelineobjects[0] = baseline;

	double line_to_center = 0;
	double distance_changes = 0;

	CvSize dim = src.size();

	
	//Changing color image to HSV to filter color
	cvtColor(src, imgHSV, CV_BGR2HSV);
	inRange(imgHSV, Scalar(60, 70, 70), Scalar(120, 255, 255), imgThreshed);

	//Reduces noise
	GaussianBlur(imgThreshed, gaussian_result, Size(3,3), 2, 2);

	//Create vector to hold the detected lines
	std::vector<Vec4i> lines;

	HoughLinesP(gaussian_result, lines, 1, CV_PI/180, 80, 50, 5);
	for(size_t i=0; i < lines.size(); i++){

		//Drawing each line that we've found
		line(src, Point(lines[i][0], lines[i][1]), 
			Point(lines[i][2], lines[i][3]), Scalar(0,255,0), 1, 8);

		//Declaring distance between each line to the center line
		double distance = (lines[i][1] - dim.height/2);
		std::cout << "distance is  " << distance << std::endl;

		for (size_t j = 0; j < k; j++){
			std::cout << "+-+-+-+-+-+-+" << std::endl;
			std::cout << "iteration  " << j << "  for line  " << i << std::endl;

			if (abs(distance - onelineobjects[j].average()) < 50){
				printf("WITHIN range of existing line\n");
				//cout << "average  " << onelineobjects[i].average() << endl;
				onelineobjects[j].add_value(distance);
				std::cout << "---------" << std::endl;
				goto stop;
			}

			else {
				//printf("NOTHING TO SEE HERE\n");
				continue;
			}

		}	
				printf("ADDING new line\n");
				k += 1;
				new_value.initiate();
				new_value.add_value(distance);
				onelineobjects[k] = new_value;
				std::cout << "--------" << std::endl;
				



			stop:
			std::cout << "ending loop for line  " << i << std::endl;
			std::cout << "number of line objects is  " << k << std::endl;
			std::cout << "-------" << std::endl;
	

				if (abs(distance - line_to_center) > 25){
					line(src, Point(lines[i][0], lines[i][1]), Point(lines[i][0], dim.height/2), Scalar(0,0,255), 1, 8);
					line(src, Point(lines[i][2], lines[i][3]), Point(lines[i][2], dim.height/2), Scalar(0,0,255), 1, 8);
					line_to_center = distance;
					distance_changes += 1;
				}

			Point left_center(0, dim.height/2);
			Point right_center(dim.width, dim.height/2);
			line(src, left_center, right_center, Scalar(255,0,0), 1, 8);

			
		}

		//std::vector<One_Line>(onelineobjects).swap(onelineobjects);
		//onelineobjects.shrink_to_fit();
		std::cout << "number of distance changes  " << distance_changes << std::endl;
		std::cout << "number of lines  " << k << std::endl;
		imshow("Original Image", src);
		//imshow("Gaussian Blur", imgThreshed);
		//imshow("HSV Image", imgHSV);

		waitKey(0);
		return 0;
}