/*
* Name: main.cpp
* Description: General 
* Author: Konstantin Kulakov
* Website: http://kostyakulakov.ru
* Version: 1.1
* License: BSD
*/

#include "anpr.h"

int main(int argc, char** argv)
{	
    cv::Mat img = cv::imread((argc >= 2 ? argv[1] : "test.jpg"), CV_LOAD_IMAGE_COLOR);

    Anpr* image;

    try
    {
        image = new Anpr;
        image->recognize(img);

        image->saveLicensePlates();
        image->showLicensePlates();

        for(auto& x : image->getLicenseText())
            std::cout << x << std::endl;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    cv::waitKey(0);

    return 0;
}
