/*
* Name: main.cpp
* Description: General 
* Author: Konstantin Kulakov
* Website: http://kostyakulakov.ru
* Version: 1.1
* License: BSD
*/

#include "anpr.h"

int main(int argc, char** argv)
{	
	Anpr image;
	image.setShowWarning(true);
		
	cv::Mat img = cv::imread((argc >= 2 ? argv[1] : "test.jpg"), 1);
	
	image.recognize(img);
	image.saveLicensePlate();
	image.showLicensePlate();
	
	for(auto& x : image.getLicenseText())
		std::cout << x << std::endl;
	
	cv::waitKey(0);

	return 0;
}