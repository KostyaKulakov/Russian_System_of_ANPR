/*
* Name: image.h
* Description: This class contains a set of methods for ANPR
* Author: Konstantin Kulakov
* Website: http://kostyakulakov.ru
* Version: 1.0
* License: BSD
*/

#pragma once
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <opencv/cv.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include "area.h"
#include <string>
#include <vector>
#include <math.h>

struct LicenseSymbolsArea
{
	LicenseSymbolsArea(cv::Mat& mframe, std::vector<mArea>& mframeAreaSymbols);
	
	cv::Mat frame;
	std::vector<mArea> frameAreaSymbols;
};

class Image
{
public:
	Image();
	~Image();
	
	bool recognize();
	bool recognize(const cv::Mat& img);
	
	std::vector<std::string>	getLicenseText() const;
	std::vector<cv::Mat>		getFrames()		 const;
	
	void setImage(const cv::Mat& img);
	
	void saveFrames();
	void saveSymbols();
	
	void showNormalImage(std::string namewindow);
	void showSymbol();
	void showimage(std::string namewindow, cv::Mat image);
	
private:
	bool recognizeSymbols(cv::Mat& src);
	bool recognizeLicenseNumber();
	bool isDuplicat(mArea& a, std::vector<mArea>& vec);
	//bool isInserted(mArea& a, std::vector<mArea>& vec);
	
	std::string		symbolDigit	=	"0123456789";
	std::string		symbolChar	=	"abekmhopctyxABEKMHOPCTYX";
	const unsigned	thresh		=	160;
	const float		scale		=	1.8;
	bool 			iscascadeLoad;

	cv::Mat mimage;
	std::vector<LicenseSymbolsArea> licenseSymbols;
	std::vector<std::string>		textLicense;
	std::vector<cv::Mat> frames;
	tesseract::TessBaseAPI OCR;
	cv::CascadeClassifier cascade;
};