/*
* Name: area.h
* Description: This class contains a structure for storing location symbols
* Author: Konstantin Kulakov
* Website: http://kostyakulakov.ru
* Version: 1.1
* License: BSD
*/

#pragma once
#include <opencv2/opencv.hpp>
#include <vector>

struct mArea
{
	mArea(std::vector<cv::Point>& vec);
	
	cv::Point min;
	cv::Point max;
	
	unsigned height;
	unsigned width;
	
	bool operator < (const mArea& a) const;
	bool operator == (const mArea& a) const;
};