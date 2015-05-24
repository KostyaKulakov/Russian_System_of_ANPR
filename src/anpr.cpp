#include "Anpr.h"

LicenseSymbolsArea::LicenseSymbolsArea(cv::Mat& mplate, std::vector<mArea>& mplateAreaSymbols) :
	plateAreaSymbols(mplateAreaSymbols), plate(mplate)
{}

Anpr::Anpr() 
{
	cascadeLoad = cascade.load("haarcascade_russian_plate_number.xml");
	/*ocrLoad = OCR.Init(NULL, "amh");
	OCR.SetPageSegMode(tesseract::PSM_SINGLE_CHAR);*/
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
	
	/*if(ocrLoad != 0) // ocr init return 0 on success and -1 on initialization failure.
	{
		std::cerr << "Tesseract OCR: This library is not able to boot" << std::endl;
		return false;
	}*/
	
	
	licensePlate.clear();
	licenseSymbols.clear();
	textLicense.clear();

	std::vector<cv::Rect> plate;
	
	cv::Mat gray, smallImg(sourseImage.rows/scale, sourseImage.cols/scale, CV_8UC1);
	
	cv::cvtColor(sourseImage, gray, CV_BGR2GRAY);
	cv::resize(gray, smallImg, smallImg.size(), 0, 0, cv::INTER_LINEAR);
		
    cascade.detectMultiScale(smallImg, plate,
		1.1, 10, 5,
		cv::Size(70, 21));  

	for(auto& p : plate)
	{
		cv::Point plateBegin	= cv::Point(p.x*scale, p.y*scale);
		cv::Point plateEnd		= cv::Point(p.width*scale, p.height*scale);

		licensePlate.push_back(sourseImage(cv::Rect(plateBegin.x, 
										plateBegin.y,
										plateEnd.x,
										plateEnd.y)));
	}
	
	for(auto& p : licensePlate)
		findLetters(p);
	
	/*if(!licenseSymbols.empty())
		recognizeLetters();*/
	
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
/*
bool Anpr::recognizeLetters()
{	
	for(auto& l : licenseSymbols)
	{
		std::string text;
		
		for(size_t i = 0; i < l.plateAreaSymbols.size(); ++i)
		{
			int minX	= l.plateAreaSymbols.at(i).minX;
			int minY	= l.plateAreaSymbols.at(i).minY;
			int height	= l.plateAreaSymbols.at(i).height;
			int width	= l.plateAreaSymbols.at(i).width;
			
			if(minX-1 <= 0 || minY-1 <= 0 || (minX-1 + width+3) >= l.plate.size().width || (minY-1 + height+3) >= l.plate.size().height)
				continue;
			
 			cv::Mat subImg = (l.plate)(cv::Rect(minX-1, minY-1, width+3, height+3));
			
			cv::Mat gray = cvCreateImage(subImg.size(), 8, 1);            
			cv::Mat image = cvCreateImage(subImg.size(), 8, 1);
			cv::cvtColor(subImg, gray, CV_RGB2GRAY);
			
			if(i == 0 || i == 4 || i == 5)
				cv::resize(image, image, cv::Size(10, 18));
			else
				cv::resize(image, image, cv::Size(15, 24));

			cv::GaussianBlur(image, image, cv::Size(7, 7), 0);
			cv::threshold(gray, image, 0, 255, cv::THRESH_BINARY_INV | cv::THRESH_OTSU);
	
			OCR.SetVariable("tessedit_char_whitelist", ((i == 0 || i == 4 || i == 5) ? symbolChar.c_str() : symbolDigit.c_str()));
			
			OCR.TesseractRect(image.data, 1, image.step1(), 0, 0, image.cols, image.rows);                  
			
			char* symbol = OCR.GetUTF8Text();
			
			if(symbol[0] == ' ')
				symbol[0] = '?';
			
			text.push_back(symbol[0]);
		}
		
		textLicense.push_back(text);
	}
	
	return true;
}*/

bool Anpr::isDuplicat(mArea& a, std::vector<mArea>& vec)
{
	for(auto& v : vec)
		if(a == v)
			return true;

	return false;
}