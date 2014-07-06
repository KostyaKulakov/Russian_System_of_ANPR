#include "area.h"

mArea::mArea(std::vector<cv::Point>& v)
{
	minX = v.at(0).x;
	maxX = v.at(0).x;
	minY = v.at(0).y;
	maxY = v.at(0).y;

	for(size_t z = 0; z < v.size(); ++z)
	{
		if(v.at(z).x < minX)
			minX = v.at(z).x;
		else if(v.at(z).x > maxX)
			maxX = v.at(z).x;

		if(v.at(z).y < minY)
			minY = v.at(z).y;
		else if(v.at(z).y > maxY)
			maxY = v.at(z).y;
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