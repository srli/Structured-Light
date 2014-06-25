#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <functional>
#include <numeric>
#include <stdio.h>
#include <stdlib.h>
#include <typeinfo> 
#include <math.h>

using namespace cv;

class Single_Line {
	//float avg_rho, avg_theta;
	float rho_values, theta_values;
	int count;

public:
	void add_value(float, float);
	void initialize();
	double average_theta();
	double average_rho();
};

void Single_Line::initialize(){
	count = 0;
	rho_values = theta_values = 0;
}

void Single_Line::add_value(float rho, float theta){
	std::cout << "adding rho: " << rho << " adding theta: " << theta << std::endl;
	count += 1;
	rho_values += rho;
	theta_values += theta;
}

double Single_Line::average_theta(){
	double mean = theta_values / count;

	std::cout << "mean is: " << mean << std::endl; //" count is: " << count << " value is: " << j << std::endl;
	return mean;

}

double Single_Line::average_rho(){
	double mean = rho_values / count;

	std::cout << "mean is: " << mean << std::endl; //" count is: " << count << " value is: " << j << std::endl;
	return mean;

}

int main(){

	//Creates matrices available for filling later
	Mat src, src2, gray, gaussian_result, img, dest;
	Mat sobel, imgHSV, imgThreshed, imgThreshedHSV;

/*	cvNamedWindow("Original Image", 0);
	cvNamedWindow("HSV Image", 0);*/
	//cvNamedWindow("Gaussian Blur", 0);

	src = imread("test.jpg", 1);
	src2 = imread("test.jpg", 1);
	printf("Image loaded\n");

	//Changing color image to HSV to filter color
	cvtColor(src, imgHSV, CV_BGR2HSV);
	cvtColor(src, gray, CV_BGR2GRAY);
	//imshow("Gray", gray);
	Sobel(gray, sobel, -1, 1, 1, 5);

	//imshow("Sobel", sobel);

	inRange(sobel, Scalar(150), Scalar(255), imgThreshed);
	//imshow("Threshed", imgThreshed);
	
	//imshow("HSV", imgHSV);
	inRange(imgHSV, Scalar(30, 30, 30), Scalar(120, 255, 255), imgThreshedHSV);
	//imshow("HSV Threshed", imgThreshedHSV);

	bitwise_and(imgThreshed, imgThreshedHSV, dest);
	

	//FOLLOWING USES HOUGH LINES
	//Reduces noise
	GaussianBlur(dest, gaussian_result, Size(3,3), 2, 2);

	//imshow("Final", gaussian_result);

	std::vector<Vec2f> lines2;
	HoughLines(gaussian_result, lines2, 1, CV_PI/180, 100);

	std::cout << "number of lines: " << lines2.size() << std::endl;

	Single_Line new_value;
	std::vector<Single_Line> single_line_objects(lines2.size());
	int k = 0;

	for(size_t i = 0; i < lines2.size(); i++)
	{
		float rho = lines2[i][0];
		float theta = lines2[i][1];

		
		double a = cos(theta), b = sin(theta);
		double x0 = a*rho, y0 = b*rho;
		Point pt1(cvRound(x0 + 1000*(-b)),
				cvRound(y0 + 1000*(a)));
		Point pt2(cvRound(x0 - 1000*(-b)),
				cvRound(y0 - 1000*(a)));

		if (theta < 0.8 ){
			//printf("passing\n");
			goto stop1;
		}
		else{
			std::cout << "angle values: " << theta << std::endl;
		//line(src, pt1, pt2, Scalar(0,0,255), 1, 8);
		}

		for(size_t j = 0; j < k + 1; j++){
			std::cout << "+-+-" << std::endl;
			//std::cout << "iteration  " << j << "  for line  " << i << std::endl;
			if (fabs(theta - single_line_objects[j].average_theta()) < 0.05){
				//std::cout << theta << " : " << single_line_objects[k].average(2) << " : " << fabs(theta - single_line_objects[k].average(2)) << std::endl;
				//printf("Within existing line\n");
				single_line_objects[j].add_value(rho, theta);
				goto stop3;
			}
			else{
				continue;
			}

			}
		//printf("Adding new line\n");
		k += 1;
		new_value.initialize();
		new_value.add_value(rho, theta);
		single_line_objects[k] = new_value;
		

		stop3:
		//std::cout << "ending line loop" << std::endl;

		stop1:
		{};
	}

	std::cout << "k is: " << k << std::endl;
	for (int p = 0 ; p < k + 1; p++){

		float rho = single_line_objects[p].average_rho();
		float theta = single_line_objects[p].average_theta();
		double a = cos(theta), b = sin(theta);
		double x0 = a*rho, y0 = b*rho;
		Point pt1(cvRound(x0 + 1000*(-b)),
				cvRound(y0 + 1000*(a)));
		Point pt2(cvRound(x0 - 1000*(-b)),
				cvRound(y0 - 1000*(a)));

		line(src, pt1, pt2, Scalar(255,0,255), 1, 8);
	}


	imshow("Normal", src);


/*	//FOLLOWING USES PROBABILISTIC HOUGH LINES
	
	//Create vector to hold the detected lines`
	std::vector<Vec4i> lines;
	HoughLinesP(gaussian_result, lines, 1, CV_PI/180, 80, 150, 2);

	//Finds the dimension of the image
	CvSize dim = src.size();

	//Draws centerline
	Point left_center(0, dim.height/2);
	Point right_center(dim.width, dim.height/2);
	line(src2, left_center, right_center, Scalar(255,0,0), 1, 8);

	for(size_t i=0; i < lines.size(); i++){
	//for(size_t i=0; i < 7; i++){

		double slope = (lines[i][1] - lines[i][3]) / (lines[i][0] - lines[i][2]);
		//std::cout << "slope is: " << slope << std::endl;

		if (abs(slope) > 0.75){
			goto stop;
		}
		//Drawing each line that we've found
		line(src2, Point(lines[i][0], lines[i][1]), Point(lines[i][2], lines[i][3]), Scalar(255,0,0), 1, 8);

		//Drawing red line between found line and center line
		line(src2, Point(lines[i][0], lines[i][1]), Point(lines[i][0], dim.height/2), Scalar(0,0,255), 1, 8);
		line(src2, Point(lines[i][2], lines[i][3]), Point(lines[i][2], dim.height/2), Scalar(0,0,255), 1, 8);

		stop:
		{}
	}

	imshow("Probablistic", src2);
*/
	//std::cout << "number of lines  " << k << std::endl;

	//std::cout << "number of lines  " << lines.size() << std::endl;

/*	std::cout << "dimensions of this screen are:  " << dim.width << " : " << dim.height << std::endl; 
	imshow("Original Image", src);
	//imshow("Gaussian Blur", imgThreshed);
	imshow("HSV Image", skel);*/

	waitKey(0);
	return 0;
}