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
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <math.h>
#include <stdexcept>
#include "area.h"

struct LicenseSymbolsArea
{
	LicenseSymbolsArea(cv::Mat& plate, std::vector<mArea>& plateAreaSymbols);
	
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
	std::vector<cv::Mat>     getLicensePlates() const;
	
    void setImage(const cv::Mat& img);
    void setShowInformation(const bool mShowInforamtion);
	
	void saveLicensePlates();
	
	void showNormalImage(std::string namewindow);
	void showLicensePlates();
	void showimage(std::string namewindow, cv::Mat image);
	
private:
    bool findLetters(cv::Mat& src);
	double getAngle(cv::Mat& plate);
	void rotateImage(cv::Mat& image, const double angle);
	unsigned getBottomBound(cv::Mat& plate);
	unsigned getTopBound(cv::Mat& plate);
	unsigned getHistTopBound(cv::Mat& plate);
	unsigned getRightBound(cv::Mat plate, bool iswhite);
	unsigned getLeftBound(cv::Mat plate, bool iswhite);
	bool recognizeLetters();
	bool isDuplicat(mArea& a, std::vector<mArea>& vec);
	
	const std::string symbolDigit = "0123456789";
	const std::string symbolChar  = "abekmhopctyxABEKMHOPCTYX";
	const unsigned thresh         = 160;
	const unsigned scale          = 2;
	const double minDegree = -10;
	const double maxDegree = 10;
	const double stepDegree= 0.1;
    bool cascadePlateLoad, cascadeSymbolLoad;
    bool showInfo;

    cv::Mat sourseImage;
	std::vector<LicenseSymbolsArea> licenseSymbols;
	std::vector<std::string>		textLicense;
	std::vector<cv::Mat>			licensePlates;
	tesseract::TessBaseAPI			OCR;
    cv::CascadeClassifier			cascadePlate, cascadeSymbol;
};
