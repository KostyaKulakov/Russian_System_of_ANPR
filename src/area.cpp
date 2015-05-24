#include "area.h"

mArea::mArea(std::vector<cv::Point>& vec)
{
	minX = vec.at(0).x;
	maxX = vec.at(0).x;
	minY = vec.at(0).y;
	maxY = vec.at(0).y;

	for(auto& v : vec)
	{
		if(v.x < minX)
			minX = v.x;
		else if(v.x > maxX)
			maxX = v.x;

		if(v.y < minY)
			minY = v.y;
		else if(v.y > maxY)
			maxY = v.y;
	}

	height = maxY - minY;
	width  = maxX - minX;
}

bool mArea::operator < (const mArea& a) const
{
	return (this->minX < a.minX);
}

bool mArea::operator == (const mArea& a) const
{
	return ((this->minX >= a.minX && this->minX <= a.maxX) || (this->maxX <= a.maxX && this->maxX >= a.maxX));
}