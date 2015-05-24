/*
* Name: anpr.h
* Description: This class implements the basic methods for Recognition license plate
* Author: Konstantin Kulakov
* Website: http://kostyakulakov.ru
* Version: 1.1
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
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <math.h>

struct LicenseSymbolsArea
{
	LicenseSymbolsArea(cv::Mat& mplate, std::vector<mArea>& mplateAreaSymbols);
	
	cv::Mat plate;
	std::vector<mArea> plateAreaSymbols;
};

class Anpr
{
public:
	Anpr();
	~Anpr();
	
	bool recognize();
	bool recognize(const cv::Mat& img);
	
	std::vector<std::string> getLicenseText() const;
	std::vector<cv::Mat>     getLicensePlate()      const;
	
	void setImage(const cv::Mat& img);
	void setShowWarning(const bool mshowWarning);
	
	void saveLicensePlate();
	
	void showNormalImage(std::string namewindow);
	void showLicensePlate();
	void showimage(std::string namewindow, cv::Mat image);
	
private:
	bool findLetters(cv::Mat& src);
	bool recognizeLetters();
	bool isDuplicat(mArea& a, std::vector<mArea>& vec);
	
	const std::string symbolDigit = "0123456789";
	const std::string symbolChar  = "abekmhopctyxABEKMHOPCTYX";
	const unsigned thresh         = 160;
	const float scale             = 1.8;
	bool cascadeLoad;
	bool ocrLoad;
	bool showWarning;

	cv::Mat sourseImage;
	std::vector<LicenseSymbolsArea> licenseSymbols;
	std::vector<std::string>		textLicense;
	std::vector<cv::Mat>			licensePlate;
	tesseract::TessBaseAPI			OCR;
	cv::CascadeClassifier			cascade;
};