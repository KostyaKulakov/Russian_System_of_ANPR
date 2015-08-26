#include "anpr.h"

LicenseSymbolsArea::LicenseSymbolsArea(cv::Mat& plate, std::vector<mArea>& plateAreaSymbols) :
    plate(plate), plateAreaSymbols(plateAreaSymbols)
{}

Anpr::Anpr() :
    showInfo(false)
{
    if(!cascadePlate.load("haarcascade_russian_plate_number.xml"))
        throw std::logic_error("haarcascade_russian_plate_number.xml file not found.");

    if(!cascadeSymbol.load("haarcascade_russian_plate_number_symbol.xml"))
        throw std::logic_error("haarcascade_russian_plate_number_symbol.xml file not found.");

    OCR.Init(NULL, "amh");
    OCR.SetPageSegMode(tesseract::PSM_SINGLE_CHAR);
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
    if(sourseImage.empty())
        throw std::logic_error("Images for recognize is empty.");

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

    if(!licenseSymbols.empty())
        recognizeLetters();

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

void Anpr::setShowInformation(const bool mShowInforamtion)
{
    this->showInfo = mShowInforamtion;
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
				rectangle(img,	f.min, f.max, cv::Scalar(0,255,0), 2);				
		}
		
		std::string wndname("Кадр " +
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
    cv::Mat cannyOutput, srcGray, srcThreshold;

    cvtColor(src, srcGray, cv::COLOR_BGR2GRAY);
    threshold(srcGray, srcThreshold, 0, 255, CV_THRESH_BINARY  | CV_THRESH_OTSU);
    medianBlur(srcThreshold, srcThreshold, 5);

    double angle = getAngle(srcThreshold);

    if(showInfo)
        std::cout << "Angle: " << angle << std::endl;

    rotateImage(srcThreshold, angle);

    unsigned bottomBound = getBottomBound(srcThreshold);
    unsigned topBound	 = getTopBound(srcThreshold);
    srcThreshold = srcThreshold(cv::Rect(0, topBound, srcThreshold.size().width, bottomBound-topBound));
    unsigned leftBound	 = std::max(getLeftBound(srcThreshold, true), getLeftBound(srcThreshold, false));
    unsigned rightBound	 = std::min(getRightBound(srcThreshold, true), getRightBound(srcThreshold, false));

    rotateImage(src, angle);
    src = src(cv::Rect(leftBound, topBound, rightBound-leftBound, bottomBound-topBound));

    if(showInfo)
    {
        std::cout << "Left: " << leftBound << " Right: " << rightBound << std::endl;

        cv::imshow("Thresold", srcThreshold);
        cv::imshow("Автомобильный номер", src);

        std::cout << "Size width: " << src.size().width << " Height: " << src.size().height << std::endl;
    }

    if(src.size().height < 61 && src.size().width < 240)
        cv::resize(src, src, cv::Size(240, 61));

    cvtColor(src, srcGray, cv::COLOR_BGR2GRAY);

    if(showInfo)
        cv::imshow("Шаг 1: Перевод в Ч/Б", srcGray);

    threshold(srcGray, srcGray, 0, 255, CV_THRESH_BINARY  | CV_THRESH_OTSU);

    if(showInfo)
        cv::imshow("Шаг 2: Бинаризация изображения", srcGray);

    medianBlur(srcThreshold, srcThreshold, 3);

    if(showInfo)
        cv::imshow("Шаг 3: Фильтр средних частот", srcGray);

    cv::blur(srcGray, srcGray, cv::Size(3,3));

    if(showInfo)
        cv::imshow("Шаг 4: Применения размытия по Гауссу", srcGray);

    cv::Canny(srcGray, cannyOutput, 100, 300, 3);

    if(showInfo)
        cv::imshow("Шаг 5: Использование детектора границ Кенни", cannyOutput);

    // Find contours
    cv::findContours(cannyOutput, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

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
        area.min.x < area.width * 0.03 ||
        (area.min.x + area.width) > srcGray.size().width * 0.97 ||
        isDuplicat(area, contursOut))
            continue;

        //unsigned nz = cv::countNonZero(((srcGray)(cv::Rect(area.min.x, area.min.y, area.width, area.height))));
        //auto ratio = (double(nz) * 100)/(area.width * area.height);

        //if(100.0-ratio < 10) // содержание чёрного в номере
        //	continue;

        if(showInfo)
            std::cout << "Height: " << area.height << " width: " << area.width /*<< " NZ: " << nz << " Ratio: " << 100.0-ratio*/ << "%" << " Min x: " << area.min.x << " Width: " << area.width << std::endl;

        contursOut.push_back(area);
    }

    if(contursOut.size() >= 8)
    {
        std::sort(contursOut.begin(), contursOut.end());	// Set the symbols in the correct order
        licenseSymbols.push_back(LicenseSymbolsArea(src, contursOut));
    }

    return !licenseSymbols.empty();
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
	
unsigned Anpr::getBottomBound(cv::Mat& plate)
{
	//equalizeHist(plate, plate);

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

unsigned Anpr::getHistTopBound(cv::Mat& plate)
{
	size_t height = plate.size().height;
	cv::Mat data;
	
	for(unsigned i = 0; i < height/2; ++i)
	{
		data = plate.row(i);
		unsigned count = cv::countNonZero(data);
		
		if(count > height*0.5)
			return i;
	}
	
	return 0;
}

unsigned Anpr::getTopBound(cv::Mat& plate)
{
	//equalizeHist(plate, plate);
    std::vector<cv::Rect> symbols;

    cascadeSymbol.detectMultiScale(plate, symbols);
	
	if(symbols.empty() && showInfo)
		std::cout << "Symbols not found" << std::endl;
	
	std::vector<cv::Rect>::iterator result;
	
	std::sort(symbols.begin(), symbols.end(), [](const cv::Rect& r1, const cv::Rect& r2){return r1.y > r2.y;});
	
	if(showInfo)
		for(auto& s : symbols)
			std::cout << s.y << std::endl;

	unsigned averageHeight = 0;
	for(auto& s : symbols)
			averageHeight += s.y;
	
	if(!symbols.empty()) // is zero
		averageHeight /= symbols.size();
	
	return symbols.empty() ? getHistTopBound(plate) : averageHeight;
}

unsigned Anpr::getLeftBound(cv::Mat plate, bool iswhite)
{
	cv::Mat element = getStructuringElement(cv::MORPH_RECT,
                                       cv::Size( 2*1 + 1, 2*1+1 ),
                                       cv::Point( 1, 1) );
	cv::erode(plate, plate, element);
	cv::dilate(plate, plate, element);

	size_t width = plate.size().width;
	double height= plate.size().height;

	cv::Mat data;

	for(unsigned i = 2; i < width/2; ++i)
	{
		data = plate.col(i);
		unsigned count = cv::countNonZero(data);
		
		if((!iswhite && count > height*0.5) || (iswhite && count < height*0.60))
			return i;
	}
	
	return 0;
}

unsigned Anpr::getRightBound(cv::Mat plate, bool iswhite)
{
	cv::Mat element = getStructuringElement(cv::MORPH_RECT,
                                       cv::Size( 2*1 + 1, 2*1+1 ),
                                       cv::Point( 1, 1) );
	cv::erode(plate, plate, element);
	cv::dilate(plate, plate, element);

	size_t width = plate.size().width;
	double height= plate.size().height;

	cv::Mat data;
	
	for(unsigned i = width-2; i > width/2; --i)
	{
		data = plate.col(i);
		unsigned count = cv::countNonZero(data);
		
		if((!iswhite && count > height*0.5) || (iswhite && count < height*0.60))
			return i+1;
	}
	
	return width;
}

bool Anpr::recognizeLetters()
{	

	for(auto& l : licenseSymbols)
	{	
		std::string text; // change name
		
		for(size_t i = 0; i < l.plateAreaSymbols.size(); ++i)
		{
			cv::Mat recimage = cvCreateImage(cv::Size(100, 100), 8, 1);
			for(int i = 0; i < recimage.size().width; ++i)  
				for(int j=0; j < recimage.size().height; ++j)  
					recimage.at<unsigned char>(j, i) = 255;
				
			int minx	= l.plateAreaSymbols.at(i).min.x;
			int miny	= l.plateAreaSymbols.at(i).min.y;
			int height	= l.plateAreaSymbols.at(i).height+l.plateAreaSymbols.at(i).height*0.05;
			int width	= l.plateAreaSymbols.at(i).width+l.plateAreaSymbols.at(i).width*0.05;
			
 			cv::Mat subImg = (l.plate)(cv::Rect(minx, miny, width, height));
			cv::Mat gray = cvCreateImage(subImg.size(), 8, 1);            
			cv::cvtColor(subImg, gray, CV_RGB2GRAY);
			cv::threshold(gray, gray, 0, 255, cv::THRESH_BINARY_INV | cv::THRESH_OTSU);
			
			cv::resize(gray, gray, cv::Size(20, 30));
			
			for(int i = 0; i < gray.size().width; ++i)  
				for(int j=0; j < gray.size().height; ++j)  
					recimage.at<unsigned char>(40+j, 40+i) = (gray.at<unsigned char>(j, i) == 255 ? 0 : 255);
			
			equalizeHist(recimage, recimage);
			unsigned level = 5;
			cv::GaussianBlur(recimage, recimage, cv::Size(level, level), 0);
			OCR.SetVariable("tessedit_char_whitelist", ((i == 0 || i == 4 || i == 5) ? symbolChar.c_str() : symbolDigit.c_str()));
			OCR.TesseractRect(recimage.data, 1, recimage.step1(), 0, 0, recimage.cols, recimage.rows);
			char* symbol = OCR.GetUTF8Text();
			
			while(symbol[0] == ' ')
			{
				while((level * level) % 2 == 0)
					++level;
				cv::GaussianBlur(recimage, recimage, cv::Size(level, level), 0);
				OCR.TesseractRect(recimage.data, 1, recimage.step1(), 0, 0, recimage.cols, recimage.rows);
				symbol = OCR.GetUTF8Text();
				++level;
			}
			
			text.push_back(symbol[0]);	
		}
		
		textLicense.push_back(text);
	}
	
	return true;
}
bool Anpr::isDuplicat(mArea& a, std::vector<mArea>& vec)
{
	for(auto& v : vec)
		if(a == v)
			return true;

	return false;
}
