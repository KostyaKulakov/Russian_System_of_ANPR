/*
* Name: area.h
* Description: This class contains a structure for storing location symbols
* Author: Konstantin Kulakov
* Website: http://kostyakulakov.ru
* Version: 1.0
* License: BSD
*/

#pragma once
#include <opencv2/opencv.hpp>
#include <vector>

struct mArea
{
	mArea(std::vector<cv::Point>& v);
	unsigned minX;
	unsigned maxX;
	unsigned minY;
	unsigned maxY;
	unsigned height;
	unsigned width;
	
	bool operator < (const mArea& a) const;
	bool operator == (const mArea& a) const;
};