#include "image.h"

LicenseSymbolsArea::LicenseSymbolsArea(cv::Mat& mframe, std::vector<mArea>& mframeAreaSymbols) :
	frameAreaSymbols(mframeAreaSymbols), frame(mframe)
{}

Image::Image() 
{
	iscascadeLoad = cascade.load("haarcascade_russian_plate_number.xml");
	OCR.Init(NULL, "amh");
	OCR.SetPageSegMode(tesseract::PSM_SINGLE_CHAR);
}

Image::~Image()
{
	for(size_t i = 0; i < frames.size(); ++i)
		frames.at(i).release();
	
	mimage.release();
}

bool Image::recognize(const cv::Mat& img)
{
	setImage(img);
	
	return this->recognize();
}

bool Image::recognize()
{	
	frames.clear();
	licenseSymbols.clear();
	textLicense.clear();

	std::vector<cv::Rect> faces;

	cv::Mat gray, smallImg(cvRound (mimage.rows/scale), cvRound(mimage.cols/scale), CV_8UC1);

	cv::cvtColor(mimage, gray, CV_BGR2GRAY);
	cv::resize(gray, smallImg, smallImg.size(), 0, 0, cv::INTER_LINEAR);
	cv::equalizeHist(smallImg, smallImg);

	if(!iscascadeLoad)
		return false;

	cascade.detectMultiScale(smallImg, faces,
		1.1, 2, 0 | cv::CASCADE_SCALE_IMAGE
		,
		cv::Size(10, 10));        

	for(auto& r : faces)
	{
		cv::Point first = cv::Point(r.x*scale, r.y*scale);
		cv::Point two	= cv::Point((r.x + r.width)*scale, (r.y + r.height)*scale);
		
		frames.push_back(mimage(cv::Rect(first.x, first.y, two.x - first.x, two.y - first.y)));
	}

	for(auto& f : frames)
		recognizeSymbols(f);

	if(!licenseSymbols.empty())
		recognizeLicenseNumber();

	return true;
}

std::vector<std::string> Image::getLicenseText() const
{
	return this->textLicense;
}

std::vector<cv::Mat> Image::getFrames() const
{
	return this->frames;
}


void Image::setImage(const cv::Mat& img)
{
	this->mimage = img.clone();
}

void Image::saveFrames()
{	
	for(size_t i = 0; i < frames.size(); ++i)
	{
		char name[8];
		sprintf(name, "f%Iu.jpg", i);
		cv::imwrite(name, frames.at(i));
	}
}
void Image::saveSymbols()
{
	size_t i = 0;
	
	for(auto& l : licenseSymbols)
	{
		cv::Mat src = l.frame;
		
		for(size_t b = 0; b < l.frameAreaSymbols.size(); ++b, ++i)
		{
			char name[8];
			sprintf(name, "%Iu.jpg", i);
			
			cv::Mat image = src(cv::Rect(l.frameAreaSymbols.at(b).minX,
							l.frameAreaSymbols.at(b).minY,
							l.frameAreaSymbols.at(b).width,
							l.frameAreaSymbols.at(b).height));
							
			cv::imwrite(name, image);
		}
		
	}
}

void Image::showimage(std::string namewindow, cv::Mat image)
{
	imshow(namewindow, image);
}
void Image::showNormalImage(std::string namewindow)
{
	showimage(namewindow, mimage);
}

void Image::showSymbol()
{
	size_t nwnd = 0;
	
	for(auto& l : licenseSymbols)
	{
		cv::Mat img = l.frame;
		
		for(size_t i = 0; i < l.frameAreaSymbols.size(); ++i)
			rectangle(img,	cv::Point(l.frameAreaSymbols.at(i).minX, l.frameAreaSymbols.at(i).minY),
							cv::Point(l.frameAreaSymbols.at(i).maxX, l.frameAreaSymbols.at(i).maxY),
							cv::Scalar(0,255,0), 2);
							
		
		char wndname[50];
		sprintf(wndname, "Symbols %Iu", nwnd);
		
		cv::namedWindow(wndname, cv::WINDOW_AUTOSIZE );
		cv::imshow(wndname, img);
		
		++nwnd;
	}
}


bool Image::recognizeSymbols(cv::Mat& src)
{
	std::vector<std::vector<cv::Point> > contours;
	std::vector<mArea> contursOut;
	std::vector<cv::Vec4i> hierarchy;
	cv::Mat cannyOutput, srcGray;
	
	// Convert image to gray and blur it
	cvtColor(src, srcGray, cv::COLOR_BGR2GRAY );
	cv::blur(srcGray, srcGray, cv::Size(3,3) );
	
	// Detect edges using canny
	cv::Canny( srcGray, cannyOutput, thresh, thresh*2, 3 );
	
	// Find contours
	cv::findContours(cannyOutput, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0) );
	
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
	
	return isRealSymbolLicens;
}

bool Image::recognizeLicenseNumber()
{	
	for(auto& l : licenseSymbols)
	{
		std::string text;
		
		for(size_t i = 0; i < l.frameAreaSymbols.size(); ++i)
		{
			int minX	= l.frameAreaSymbols.at(i).minX;
			int minY	= l.frameAreaSymbols.at(i).minY;
			int height	= l.frameAreaSymbols.at(i).height;
			int width	= l.frameAreaSymbols.at(i).width;
			
			if(minX-1 <= 0 || minY-1 <= 0 || (minX-1 + width+3) >= l.frame.size().width || (minY-1 + height+3) >= l.frame.size().height)
				continue;
			
 			cv::Mat subImg = (l.frame)(cv::Rect(minX-1, minY-1, width+3, height+3));
			
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
}

bool Image::isDuplicat(mArea& a, std::vector<mArea>& vec)
{
	for(auto& v : vec)
		if(a == v)
			return true;

	return false;
}