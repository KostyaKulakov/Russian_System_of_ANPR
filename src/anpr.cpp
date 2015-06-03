#include "Anpr.h"

LicenseSymbolsArea::LicenseSymbolsArea(cv::Mat& mplate, std::vector<mArea>& mplateAreaSymbols) :
	plateAreaSymbols(mplateAreaSymbols), plate(mplate)
{}

Anpr::Anpr() 
{
	cascadePlateLoad = cascadePlate.load("haarcascade_russian_plate_number.xml");
	cascadeSymbolLoad = cascadeSymbol.load("haarcascade_russian_plate_number_symbol.xml");
}

Anpr::~Anpr()
{
	for(auto& p : licensePlates)
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
	
	if(!cascadePlateLoad || !cascadeSymbolLoad)
	{
		std::cerr << "Cascade not load. Check your directory \"haarcascade_russian_plate_number.xml\" \"haarcascade_russian_plate_number_symbol.xml\"" << std::endl;
		return false;
	}
	
	licensePlates.clear();
	licenseSymbols.clear();
	textLicense.clear();

	std::vector<cv::Rect> plates;
	
	cv::Mat gray;
	
	cv::cvtColor(sourseImage, gray, CV_BGR2GRAY);
	
	bool resize = sourseImage.size().width/scale > 480 || sourseImage.size().height/scale > 320;
	
	if(resize)
		cv::resize(gray, gray, cv::Size(sourseImage.size().width/scale, sourseImage.size().height/scale), 0, 0, cv::INTER_LINEAR);
		
    cascadePlate.detectMultiScale(gray, plates,
		1.1, 10, 0,
		cv::Size(70, 21), cv::Size(500, 150));  

	for(auto& p : plates)
	{
		cv::Point plateBegin	= cv::Point(p.x*(resize ? scale : 1), p.y*(resize ? scale : 1));
		cv::Point plateEnd		= cv::Point(p.width*(resize ? scale : 1), p.height*(resize ? scale : 1));

		licensePlates.push_back(sourseImage(cv::Rect(plateBegin.x, 
										plateBegin.y,
										plateEnd.x,
										plateEnd.y)));
	}
	
	for(auto& p : licensePlates)
		findLetters(p);
	
	return true;
}

std::vector<std::string> Anpr::getLicenseText() const
{
	return this->textLicense;
}

std::vector<cv::Mat> Anpr::getLicensePlates() const
{
	return this->licensePlates;
}


void Anpr::setImage(const cv::Mat& img)
{
	this->sourseImage = img;
}

void Anpr::setShowWarning(const bool mshowWarning)
{
	this->showWarning = mshowWarning;
}

void Anpr::saveLicensePlates()
{
	size_t id = 0;
	for(auto& p : licensePlates)
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

void Anpr::showLicensePlates()
{
	size_t nwnd = 0;

	for(auto& p : licensePlates)
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
	
	imshow("Before", src);
	rotateImage(src, getAngle(src));

	unsigned bottomBound = getBottomBound(src);
	unsigned topBound	 = getTopBound(src);
	
	src = src(cv::Rect(0, topBound, src.size().width, bottomBound-topBound));
	
	cvtColor(src, srcGray, cv::COLOR_BGR2GRAY);

	threshold(srcGray, srcGray, 0, 255, CV_THRESH_BINARY  | CV_THRESH_OTSU);
	std::cout << "Size width: " << srcGray.size().width << " Height: " << srcGray.size().height << std::endl;
	// Convert image to gray and blur it
	cv::blur(srcGray, srcGray, cv::Size(3,3));
	// Detect edges using canny
	cv::Canny(srcGray, cannyOutput, 100, 300, 3);

	// Find contours
	cv::findContours(cannyOutput, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

	for(auto& c : contours)
	{
		mArea area(c);
		
		if(((int)(area.height * area.width)) == 0)
			continue;
		
		if(area.height < 0.3 * srcGray.size().height || 
		area.height > 0.9 * srcGray.size().height || 
		area.width < srcGray.size().width * 0.03 || 
		area.width > srcGray.size().width * 0.15 ||
		area.width > area.height ||
		area.minX < area.width * 0.03 ||
		(area.minX + area.width) > srcGray.size().width * 0.97)
			continue;
		
		unsigned nz = cv::countNonZero(((srcGray)(cv::Rect(area.minX, area.minY, area.width, area.height))));
		auto ratio = (double(nz) * 100)/(area.width * area.height);
		
		if(ratio < 30)
			continue;
		
		std::cout << "Height: " << area.height << " width: " << area.width << " NZ: " << nz << " Ratio: " << ratio << "%" << " Min x: " << area.minX << " Width: " << area.width << std::endl;
		
		contursOut.push_back(area);
	}

	bool isRealSymbolLicens = (contursOut.size() == 8 || contursOut.size() == 9);
	
	if(isRealSymbolLicens || true)
	{
		std::sort(contursOut.begin(), contursOut.end());	// Set the symbols in the correct order
		licenseSymbols.push_back(LicenseSymbolsArea(src, contursOut));
	}
	else
		if(showWarning)
			std::cerr << "Do not found a sufficient number of characters" << std::endl;
	
	return isRealSymbolLicens;
}

double Anpr::getAngle(cv::Mat& plate) // Optimized
{
	unsigned min = plate.size().height;
	double angle = 0;
	cv::Mat temp;

	for(double a = minDegree; a < maxDegree; a += stepDegree) //a - angle 
	{
		temp = plate.clone();
		rotateImage(temp, a);

		unsigned bottomBound = getBottomBound(temp);
		if(bottomBound < min)
		{
			angle = a;
			min = bottomBound;
		}
	}

   return angle; 
}

void Anpr::rotateImage(cv::Mat& image, const double angle)
{
	cv::Mat rot_mat(2, 3, CV_32FC1);

	cv::Point center = cv::Point(image.cols/2, image.rows/2);
	double scale = 1;

	rot_mat = getRotationMatrix2D(center, angle, scale);

	warpAffine(image, image, rot_mat, image.size());	
}
	
unsigned Anpr::getBottomBound(cv::Mat plate)
{
	cv::cvtColor(plate, plate, CV_BGR2GRAY);
	equalizeHist(plate, plate);
	//threshold(plate, plate, 120, 255, cv::THRESH_BINARY);
	threshold(plate, plate, 0, 255, CV_THRESH_BINARY  | CV_THRESH_OTSU);
	size_t height = plate.size().height;
	unsigned lastCount = 0;
	cv::Mat data;
	
	for(unsigned i = height/2; i < height; ++i)
	{
		data = plate.row(i);
		unsigned count = cv::countNonZero(data);
		
		if(count < lastCount/2)
			return i;

		lastCount = count;
	}
	
	return height;
}

unsigned Anpr::getTopBound(cv::Mat plate)
{
	cv::cvtColor(plate, plate, CV_BGR2GRAY);
	equalizeHist(plate, plate);
	std::vector<cv::Rect> symbols;
	
	cascadeSymbol.detectMultiScale(plate, symbols);
	
	if(symbols.empty())
		std::cout << "Symbols not found (((" << std::endl;
	
	std::vector<cv::Rect>::iterator result;
	
	std::sort(symbols.begin(), symbols.end(), [](const cv::Rect& r1, const cv::Rect& r2){return r1.y > r2.y;});
	
	for(auto& s : symbols)
		std::cout << s.y << std::endl;
	

	return symbols.empty() ? 0 : symbols.at(0).y;
}

bool Anpr::isDuplicat(mArea& a, std::vector<mArea>& vec)
{
	for(auto& v : vec)
		if(a == v)
			return true;

	return false;
}