
//#include "stdafx.h"
#include <stdio.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <chrono>
#include <stdlib.h>

#ifdef _WIN32
	#define WINPAUSE system("pause")
#endif


using namespace cv;
using namespace std;
using namespace std::chrono;

//function to find entropy
float findEntropy(Mat image) {
	//Converting the image into a black and white laplacian of itself,
	//which contains only the "edges" of where colors change in the image
	Mat gradImage;
	Laplacian(image, gradImage, CV_8U, 1, 1, 0, BORDER_DEFAULT);

	if (gradImage.channels() == 3) cvtColor(gradImage, gradImage, CV_BGR2GRAY);
	/// Establish the number of bins
	int histSize = 256;
	/// Set the ranges ( for B,G,R) )
	float range[] = { 0, 256 };
	const float* histRange = { range };
	bool uniform = true; bool accumulate = false;
	/// Compute the histograms:
	MatND hist;;
	calcHist(&gradImage, 1, 0, Mat(), hist, 1, &histSize, &histRange, uniform, accumulate);
	hist /= gradImage.total();
	hist += 1e-4; //prevent 0

	Mat logP;
	cv::log(hist, logP);

	float entropy = -1 * sum(hist.mul(logP)).val[0];

	return entropy;
}

//function to subtract background
Mat subtractBackground(Mat backgroundImage, Mat rawImage) {

	//calculating average value of background
	Scalar temp = mean(backgroundImage);
	float meanBackgroundValue = temp.val[0];

	//subtracting that value from raw image
	Mat result = rawImage / backgroundImage * meanBackgroundValue;
	return result;
}


//function to detect tissue
bool tissueDetector(Mat bgrImage) {
	//converting bgr image to hsv image
	Mat hsvImage;
	cvtColor(bgrImage, hsvImage, COLOR_BGR2HSV);

	//calculaing image size
	double imageSize = hsvImage.cols*hsvImage.rows;

	//converting the image into a form where tissue appears white
	//and everything else appears black.
	Mat tissue;
	inRange(hsvImage, Scalar(130, 40, 50), Scalar(179, 255, 255), tissue);
	//If images don't contain color, use these values
	//inRange(hsv, Scalar(0, 0, 0), Scalar(0, 0, 150), tissue);
	/*
	Upon testing, the value "150" may have to change based on what the established backgrount is.
	For image samples that are darker, something smaller than 150 is necesarry,
	but for lighter sample sets, 150 should suffice.
	*/

	//Calculating percentage of image is white (is tissue)
	double tissuePercent = 100.0*((double)countNonZero(tissue)) / imageSize;

	//Empty images will yeild less than 1 percent.
	if (tissuePercent < 1) {
		return false;
	}
	else {
		return true;
	}
}


int main(int argc, char** argv) {

	Mat background = imread(argv[1], 1);

	int n = 23;//number of images + 1
	int m = 5;//number of repetitions, meaning: Total number of tests: n*m

	double average = 0;

	//letting the program "warm up"
	for (int q = 0; q < 1; q++) {
		for (int i = 1; i < 3; i++) {
			Mat image = imread(argv[i], 1);
			subtractBackground(background, image);
			tissueDetector(image);
			findEntropy(image);
		}
		
	}
	
	for (int q = 0; q < m; q++) {
		for (int i = 1; i < n; i++) {
			Mat image = imread(argv[i], 1);
			high_resolution_clock::time_point t1 = high_resolution_clock::now();
			subtractBackground(background, image);
			high_resolution_clock::time_point t2 = high_resolution_clock::now();

			auto duration = duration_cast<microseconds>(t2 - t1).count();
			//cout << duration << endl;
			average += duration;
		}
	}
	//cout << "Average = " << average << endl;
	cout << "Average Runtime of subtractBackground (microseconds): " << average /((float)n*(float)m) << endl;
	cout << "(" << average / ((float)n*(float)m) / 1000000 << " seconds)" << endl;

	average = 0;
	
	for (int q = 0; q < m; q++) {
		for (int i = 1; i < n; i++) {
			Mat image = imread(argv[i], 1);
			high_resolution_clock::time_point t1 = high_resolution_clock::now();
			tissueDetector(image);
			high_resolution_clock::time_point t2 = high_resolution_clock::now();

			auto duration = duration_cast<microseconds>(t2 - t1).count();
			//cout << duration << endl;
			average += duration;
		}
	}
	//cout << "Average = " << average << endl;
	cout << "Average Runtime of tissueDetector (microseconds): " << average / ((float)n*(float)m) << endl;
	cout << "(" << average / ((float)n*(float)m) / 1000000 << " seconds)" << endl;


	average = 0;
	

	for (int q = 0; q < m; q++) {
		for (int i = 1; i < n; i++) {
			Mat image = imread(argv[i], 1);
			high_resolution_clock::time_point t1 = high_resolution_clock::now();
			float entropy = findEntropy(image);
			high_resolution_clock::time_point t2 = high_resolution_clock::now();

			//cout << "Entropy of Image" << i-2 << ": " << entropy << endl;

			auto duration = duration_cast<microseconds>(t2 - t1).count();
			//cout << duration << endl;
			average += duration;
		}
	}
	//cout << "Average = " << average << endl;
	cout << "Average Runtime of findEntropy (microseconds): " << average / ((float)n*(float)m) << endl;
	cout << "(" << average / ((float)n*(float)m) / 1000000 << " seconds)" << endl;

	average = 0;
	
	for (int q = 0; q < m; q++) {
		for (int i = 1; i < n; i++) {
			Mat image = imread(argv[i], 1);
			high_resolution_clock::time_point t1 = high_resolution_clock::now();
			subtractBackground(background, image);
			tissueDetector(image);
			findEntropy(image);
			high_resolution_clock::time_point t2 = high_resolution_clock::now();

			auto duration = duration_cast<microseconds>(t2 - t1).count();
			//cout << duration << endl;
			average += duration;
		}
	}
	//cout << "Average = " << average << endl;
	cout << "Average Runtime of all three at once (microseconds): " << average / ((float)n*(float)m) << endl;
	cout << "(" << average / ((float)n*(float)m) / 1000000 << " seconds)" << endl;
	
	WINPAUSE;

	return 0;
}

//Results on my laptop (which I believe to be a slower computer)
/*
m = 5

subtractbackground (microseconds): 9.24874e+06
tissueDetector (microseconds): 346686
findEntropy (microseconds): 1.3849e+07
All three together (microseconds): 2.38351e+07

WITH NEW FUNCTIONS:
m=1

subtractbackground (seconds): 0.199651
tissueDetector (seconds): 0.0679876
findEntropy (seconds): 0.0192734
All three together (seconds): 0.286686

m=5

subtractbackground (seconds): 0.200957
tissueDetector (seconds): 0.0686019
findEntropy (seconds): 0.0189308
All three together (seconds): 0.286258

m=10

subtractbackground (seconds): 0.200306
tissueDetector (seconds): 0.0680299
findEntropy (seconds): 0.0189634
All three together (seconds): 0.286431

*/
