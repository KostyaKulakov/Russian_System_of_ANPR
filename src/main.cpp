/*
* Name: main.cpp
* Description: General 
* Author: Konstantin Kulakov
* Website: http://kostyakulakov.ru
* Version: 1.0
* License: BSD
*/

#include "image.h"

int main(int argc, char** argv)
{	
	Image alpr;
	
	cv::Mat img = cv::imread((argc >= 2 ? argv[1] : "test.jpg"), 1);
	
	alpr.recognize(img);
	alpr.saveFrames();
	//alpr.saveSymbols();
	alpr.showSymbol();
	
	for(auto& x : alpr.getLicenseText())
		std::cout << x << std::endl;
	
	cv::waitKey(0);

	return 0;
}