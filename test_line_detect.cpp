#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <stdio.h>
#include <typeinfo>
#include <math.h>

using namespace cv;

/* Some variables:
i -> iterates through lines found by HoughLinesP
j -> iterates through all the values inside onelineobjects
k -> used to insert new lines into oneline objects*/


//This class defines one line for comparison later
class One_Line {
	int distance, length;
	int total_distance, number_values;
	double average_value;
	public:
		void initiate();
		void add_value(double);
		double average(); 
};

//We have to initiate with zeros since new lines need default values
void One_Line::initiate(){
	//printf("Initiating new line\n");
	total_distance = 0;
	number_values = 0;
	average_value = 0.0;
}

void One_Line::add_value(double value){
	total_distance += value;
	number_values += 1;
}

double One_Line::average(){
	std::cout << "the total number of values in this line is  " << number_values << std::endl;
	std::cout << "total distance is  " << total_distance << std::endl;
	std::cout << "average is  " << average_value << std::endl;
	average_value = (total_distance/number_values);
	return average_value;
}

int main(){
	//Creates matrices available for filling later
	Mat src, gaussian_result;
	Mat imgHSV, imgThreshed;

	cvNamedWindow("Original Image", 0);
	//cvNamedWindow("HSV Image", 0);
	//cvNamedWindow("Gaussian Blur", 0);

	src = imread("lines.jpg", 1);
	printf("Image loaded\n");

	//We create a vector of size 10 to hold all our potential line objects.
	//This also means that we can only detect 10 lines at once
	std::vector<One_Line> onelineobjects (10);
	int k = 0; //initiates index of this vector

	//Creating the comparative line
	One_Line baseline, new_value;
	baseline.initiate();
	baseline.add_value(0);
	onelineobjects[0] = baseline;
	
	//Changing color image to HSV to filter color
	cvtColor(src, imgHSV, CV_BGR2HSV);
	inRange(imgHSV, Scalar(60, 70, 70), Scalar(120, 255, 255), imgThreshed);

	//Reduces noise
	GaussianBlur(imgThreshed, gaussian_result, Size(3,3), 2, 2);

	//Create vector to hold the detected lines
	std::vector<Vec4i> lines;
	HoughLinesP(gaussian_result, lines, 1, CV_PI/180, 80, 50, 5);

	//Finds the dimension of the image
	CvSize dim = src.size();

	//Draws centerline
	Point left_center(0, dim.height/2);
	Point right_center(dim.width, dim.height/2);
	line(src, left_center, right_center, Scalar(255,0,0), 1, 8);

	for(size_t i=0; i < lines.size(); i++){

		//Drawing each line that we've found
		line(src, Point(lines[i][0], lines[i][1]), Point(lines[i][2], lines[i][3]), Scalar(0,255,0), 1, 8);

		//Drawing red line between found line and center line
		line(src, Point(lines[i][0], lines[i][1]), Point(lines[i][0], dim.height/2), Scalar(0,0,255), 1, 8);
		line(src, Point(lines[i][2], lines[i][3]), Point(lines[i][2], dim.height/2), Scalar(0,0,255), 1, 8);

		//Calculates distance between each line to the center line
		double distance = (lines[i][1] - dim.height/2);
		std::cout << "distance is  " << distance << std::endl;

		for (size_t j = 0; j < k+1; j++){
			std::cout << "+-+-" << std::endl;
			std::cout << "iteration  " << j << "  for line  " << i << std::endl;

			if (abs(distance - onelineobjects[j].average()) < 15){
				printf("WITHIN range of existing line\n");
				onelineobjects[j].add_value(distance);
				std::cout << "---------" << std::endl;
				goto stop;
			}
			else {
				//printf("NOTHING TO SEE HERE\n");
				continue;
			}
		}	

		//If we progress through entire loop without finding match, creates new line
		printf("ADDING new line\n");
		k += 1;
		new_value.initiate();
		new_value.add_value(distance);
		onelineobjects[k] = new_value;
		std::cout << "--------" << std::endl;
			
		//Skips to here if value is within range of existing line
		stop:
		std::cout << "ending loop for line  " << i << std::endl;
		std::cout << "number of line objects is  " << k << std::endl;
		std::cout << "---------" << std::endl;
	}

	std::cout << "number of lines  " << k << std::endl;
	imshow("Original Image", src);
	//imshow("Gaussian Blur", imgThreshed);
	//imshow("HSV Image", imgHSV);

	waitKey(0);
	return 0;
}