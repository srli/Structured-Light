#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <stdio.h>
#include <typeinfo>
#include <math.h>

using namespace cv;
using namespace std;

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
	total_distance = 0;
	number_values = 0;
	average_value = 0.0;
}

void One_Line::add_value(double value){
	total_distance += value;
	number_values += 1;
/*	cout << "number value of this line is  " << number_values << endl;
	cout << "total distance is  " << total_distance << endl;*/
}

double One_Line::average(){

	cout << "number value of this line is  " << number_values << endl;
	cout << "total distance is  " << total_distance << endl;
	cout << "average is  " << average_value << endl;
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

	vector<One_Line> onelineobjects;

	//Creating the comparative line
	One_Line baseline;
	baseline.initiate();
	baseline.add_value(0);
	onelineobjects.push_back(baseline);

	double line_to_center = 0;
	double distance_changes = 0;

	CvSize dim = src.size();

	
	//Changing color image to HSV to filter color
	cvtColor(src, imgHSV, CV_BGR2HSV);
	inRange(imgHSV, Scalar(60, 70, 70), Scalar(120, 255, 255), imgThreshed);

	//Reduces noise
	GaussianBlur(imgThreshed, gaussian_result, Size(3,3), 2, 2);

	//Create vector to hold the detected lines
	vector<Vec4i> lines;

	HoughLinesP(gaussian_result, lines, 1, CV_PI/180, 80, 50, 5);
	for(size_t i=0; i < lines.size(); i++){

		line(src, Point(lines[i][0], lines[i][1]), 
			Point(lines[i][2], lines[i][3]), Scalar(0,255,0), 1, 8);

		double distance = (lines[i][1] - dim.height/2);

		for (size_t j = 0; j < onelineobjects.size(); j++){
			cout << "iteration  " << j << "  for line  " << i << endl;
			if (abs(distance - onelineobjects[i].average()) < 5){
				printf("WITHIN range of existing line\n");
				//cout << "average  " << onelineobjects[i].average() << endl;
				onelineobjects[i].add_value(distance);
				cout << "---------" << endl;
				goto stop;
			}

			else {
				printf("NOTHING TO SEE HERE\n");
				continue;
			}

		}	
				printf("adding new line\n");
				One_Line new_value;
				new_value.initiate();
				new_value.add_value(distance);
				onelineobjects.push_back(new_value);
				cout << "--------" << endl;



			stop:
			cout << "ending loop for line  " << i << endl;
			cout << "-------" << endl;
	

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

		cout << "number of distance changes  " << distance_changes << endl;
		cout << "number of lines  " << onelineobjects.size() << endl;
		imshow("Original Image", src);
		//imshow("Gaussian Blur", imgThreshed);
		//imshow("HSV Image", imgHSV);

		waitKey(0);
		return 0;
}