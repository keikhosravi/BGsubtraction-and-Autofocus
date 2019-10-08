/*
Outline of field flattness correction algorith for LOCI's CAMM
Date: 07/06/2018
Author: Adib Keikhosravi
*/

#include "stdafx.h"
#include <stdio.h>
#include <iostream>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

//function to find entropy
float findEntropy(Mat image) {
	//Converting the image into a black and white laplacian of itself,
	//which contains only the "edges" of where colors change in the image
	Mat gradImage;
	Laplacian(image, gradImage, CV_8U, 1, 1, 0, BORDER_DEFAULT);

	if (gradImage.channels() == 3) cvtColor(gradImage, gradImage, CV_BGR2GRAY);
	//Establish the number of bins
	int histSize = 256;
	
	//Set the ranges (for B,G,R)
	float range[] = { 0, 256 };
	const float* histRange = { range };
	bool uniform = true; bool accumulate = false;
	
	//Compute the histograms:
	MatND hist;;
	calcHist(&gradImage, 1, 0, Mat(), hist, 1, &histSize, &histRange, uniform, accumulate);
	hist /= gradImage.total();
	//preventing 0
	hist += 1e-4;

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
	/*
	If images don't contain color, use these values:
	inRange(hsv, Scalar(0, 0, 0), Scalar(0, 0, 150), tissue);
	
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

/*
The following main() represents an outline of how the algorithm will roughly flow.
Certian aspects are still written in pseudocode, to be changed once implemented
properly into OpenScan.
*/

//Current Inputs: Starting X, Y, and Z final X and Y, and background image, as seen below:
int X = XStart;
int Y = YStart;
int Z = ZStart;
//X Y and Z represent the current postion of the microscope
int xmax = xmax;
int ymax = ymax;
Mat background;//To be subtracted from every image.
//Data type subject to change for all values

//Current code aquires new images and moves coordinates within algorithm itself.

int main() {
	int focusTracker = 0;
	float entropy;
	float prevEntropy = 0;

	while (Y >= ymax) {

		//pseudocode
		Mat image = Aquire Image at (X, Y, Z);

		Mat newImage = subtractBackground(background,image);

		//if there is no tissue present, then skip finding a focus
		if (tissueDetector(newImage) == false) {
			//pseudocode until line 145
			use / save newImage;

			if (X = xmax) {
				if (Y = ymax) {
					return;//iterated through entire XY grid
				}
				else {//moving to next row
					Move x to XStart;
					Move y + ;
				}
			}
			else {//moving to next image
				Move x +;
			}
		}

		else {

			entropy = findEntropy(newImage);

			while (focusTracker != 2) {

				if (entropy < prevEntropy) {//passed by focus
					focusTracker++;
				}
				
				if (focusTracker == 0) {//not yet past focus, try next

					prevEntropy = entropy;

					//pseudocode
					Move Z +;
					Aquire new image at (X, Y, Z);

					entropy = findEntropy(subtractBackground(background, image));
				}

				else if (focusTracker == 1) {//passed over the focus once, so reversing directions
					
					prevEntropy = entropy;
					//pseudocode
					Move Z -;
					Aquire new image at (X, Y, Z);

					entropy = findEntropy(subtractBackground(background, image));
				}

				else {//focusTracker = 2, meaning the algorithm just passed over the focus for second time

					prevEntropy = 0;

					//everything from here till the end is pseudocode
					use / save previous image taken at (X, Y, Z + 1);

					if (X = xmax) {
						if (Y = ymax) {
							return;//iterated through entire XY grid
						}
						else {//moving to next row
							Move x to XStart;
							Move y +;
						}
					}

					else {
						Move x +;
						//Z is left wherever it was, since it is likely that the next focus is nearby
					}
				}
			}
		}
	}
}
