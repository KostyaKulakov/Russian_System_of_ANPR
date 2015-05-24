#include "Anpr.h"

LicenseSymbolsArea::LicenseSymbolsArea(cv::Mat& mplate, std::vector<mArea>& mplateAreaSymbols) :
	plateAreaSymbols(mplateAreaSymbols), plate(mplate)
{}

Anpr::Anpr() 
{
	cascadeLoad = cascade.load("haarcascade_russian_plate_number.xml");
}

Anpr::~Anpr()
{
	for(auto& p : licensePlate)
		p.release();
	
	sourseImage.release();
}

bool Anpr::recognize(const cv::Mat& img)
{
	setImage(img);
	
	return this->recognize();
}

bool Anpr::recognize()
{
	if(sourseImage.empty() && showWarning)
	{
		std::cerr << "Image is empty" << std::endl;
		return false;
	}
	
	if(!cascadeLoad)
	{
		std::cerr << "Cascade not load. Check your directory \"haarcascade_russian_plate_number.xml\"" << std::endl;
		return false;
	}
	
	licensePlate.clear();
	licenseSymbols.clear();
	textLicense.clear();

	std::vector<cv::Rect> plate;
	
	cv::Mat gray;
	
	cv::cvtColor(sourseImage, gray, CV_BGR2GRAY);
	
	bool resize = sourseImage.size().width/scale > 480 || sourseImage.size().height/scale > 320;
	
	if(resize)
		cv::resize(gray, gray, cv::Size(sourseImage.size().width/scale, sourseImage.size().height/scale), 0, 0, cv::INTER_LINEAR);
		
    cascade.detectMultiScale(gray, plate,
		1.1, 10, 5,
		cv::Size(70, 21), cv::Size(500, 150));  

	for(auto& p : plate)
	{
		cv::Point plateBegin	= cv::Point(p.x*(resize ? scale : 1), p.y*(resize ? scale : 1));
		cv::Point plateEnd		= cv::Point(p.width*(resize ? scale : 1), p.height*(resize ? scale : 1));

		licensePlate.push_back(sourseImage(cv::Rect(plateBegin.x, 
										plateBegin.y,
										plateEnd.x,
										plateEnd.y)));
	}
	
	for(auto& p : licensePlate)
		findLetters(p);
	
	return true;
}

std::vector<std::string> Anpr::getLicenseText() const
{
	return this->textLicense;
}

std::vector<cv::Mat> Anpr::getLicensePlate() const
{
	return this->licensePlate;
}


void Anpr::setImage(const cv::Mat& img)
{
	this->sourseImage = img;
}

void Anpr::setShowWarning(const bool mshowWarning)
{
	this->showWarning = mshowWarning;
}

void Anpr::saveLicensePlate()
{
	size_t id = 0;
	for(auto& p : licensePlate)
	{
		std::string name("license plate "	+
						std::to_string(id)	+
						".jpg");
						
		cv::imwrite(name, p);
		++id;
	}
}

void Anpr::showimage(std::string namewindow, cv::Mat image)
{
	imshow(namewindow, image);
}
void Anpr::showNormalImage(std::string namewindow)
{
	showimage(namewindow, sourseImage);
}

void Anpr::showLicensePlate()
{
	size_t nwnd = 0;

	for(auto& p : licensePlate)
	{
		cv::Mat img = p.clone();
		
		for(auto& l : licenseSymbols)
		{
			if(!std::equal(p.begin<unsigned>(), p.end<unsigned>(), l.plate.begin<unsigned>()))
				continue;
			
			for(auto& f : l.plateAreaSymbols)
				rectangle(img,	cv::Point(f.minX, f.minY),
								cv::Point(f.maxX, f.maxY),
								cv::Scalar(0,255,0), 2);
								
		}
		
		std::string wndname("License plate " +
							std::to_string(nwnd));

		cv::namedWindow(wndname, cv::WINDOW_AUTOSIZE);

		++nwnd;
		
		cv::imshow(wndname, img);
	}
}


bool Anpr::findLetters(cv::Mat& src)
{
	std::vector<std::vector<cv::Point> > contours;
	std::vector<mArea> contursOut;
	std::vector<cv::Vec4i> hierarchy;
	cv::Mat cannyOutput, srcGray;
	
	auto bound = getLowerBoundary(src);
	imshow("After", src);

	src = src(cv::Rect(0, bound.second, src.size().width, bound.first-bound.second));

	
	// Convert image to gray and blur it
	cvtColor(src, srcGray, cv::COLOR_BGR2GRAY);
	cv::blur(srcGray, srcGray, cv::Size(3,3));
	
	// Detect edges using canny
	cv::Canny( srcGray, cannyOutput, thresh, thresh*2, 3);
	
	// Find contours
	cv::findContours(cannyOutput, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
	
	for(auto& c : contours)
	{
		mArea area(c);
		
		if(((int)(area.height * area.width)) == 0)
			continue;
			
		int ratio = (src.cols * src.rows)/(area.height * area.width);
		
		if(area.height < area.width || area.width < 2 || ratio < 30 || ratio > 85 || isDuplicat(area, contursOut))
			continue;
			
		contursOut.push_back(area);
	}

	bool isRealSymbolLicens = (contursOut.size() == 8 || contursOut.size() == 9);
	
	if(isRealSymbolLicens)
	{
		std::sort(contursOut.begin(), contursOut.end());	// Set the symbols in the correct order
		licenseSymbols.push_back(LicenseSymbolsArea(src, contursOut));
	}
	else
		if(showWarning)
			std::cerr << "Do not found a sufficient number of characters" << std::endl;
	
	return isRealSymbolLicens;
}

std::pair<unsigned, unsigned> Anpr::getLowerBoundary(cv::Mat plate)
{
	cv::cvtColor(plate, plate, CV_BGR2GRAY);
	threshold(plate, plate, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
	size_t height = plate.size().height;
	unsigned lastCount = 0;
	cv::Mat data;
	
	unsigned lowerBound = 0;
	for(unsigned i = height/2; i < height && lowerBound == 0; ++i)
	{
		data = plate.row(i);
		unsigned count = cv::countNonZero(data);
		
		if(count < lastCount/2)
			lowerBound = i;

		lastCount = count;
	}
	
	lastCount = 0;
	
	unsigned upperBound = 0;
	for(unsigned i = height/2; i > 0 && upperBound == 0; --i)
	{
		data = plate.row(i);
		unsigned count = cv::countNonZero(data);
		
		if(count*2 < lastCount)
			upperBound = i+1;

		lastCount = count;
	}
	
	return std::pair<unsigned, unsigned>(lowerBound, upperBound);
}

bool Anpr::isDuplicat(mArea& a, std::vector<mArea>& vec)
{
	for(auto& v : vec)
		if(a == v)
			return true;

	return false;
}