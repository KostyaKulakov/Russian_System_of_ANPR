#include "area.h"

mArea::mArea(std::vector<cv::Point>& vec)
{
	min = vec.at(0);
	max = vec.at(0);

	for(auto& v : vec)
	{
		if(v.x < min.x)
			min.x = v.x;
		else if(v.x > max.x)
			max.x = v.x;

		if(v.y < min.y)
			min.y = v.y;
		else if(v.y > max.y)
			max.y = v.y;
	}

	height = max.y - min.y;
	width  = max.x - min.x;
}

bool mArea::operator < (const mArea& a) const
{
	return (this->min.x < a.min.x);
}

bool mArea::operator == (const mArea& a) const
{
	return ((this->min.x >= a.min.x && this->min.x <= a.max.x) || (this->max.x <= a.max.x && this->max.x >= a.max.x));
}