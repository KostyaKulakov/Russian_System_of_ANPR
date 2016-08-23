### ANPR System: The Russian system of automatic number plate recognition

####Dependencies
* OpenCV >= 2.4.9
* leptonica >= 1.71
* Tesseract OCR >= 3.02

####Build (Ubuntu)
* Build dependencies

'''
sudo add-apt-repository --yes ppa:xqms/opencv-nonfree
sudo apt-get update
sudo apt-get install build-essential cmake git libgtk2.0-dev pkg-config libavcodec-dev libavformat-dev libswscale-dev libopencv-dev libtesseract-dev libleptonica-dev  liblog4cplus-dev libcurl3-dev libopencv-nonfree-dev
'''

* Getting last build and setuping

'''
cd ~/<my_working _directory>
git clone https://github.com/hardkun/Russian_System_of_ANPR.git
cd Russian_System_of_ANPR/
sudo cp -v runtime_data/tessdata/* /usr/share/tesseract-ocr/tessdata
sudo cp -v runtime_data/haarcascade_russian_plate_number.xml src/
sudo cp -v runtime_data/haarcascade_russian_plate_number_symbol.xml src/
'''

* Building app

'''
g++ *.cpp  -l:libopencv_core.so.2.4.9 -l:libopencv_highgui.so.2.4.9  -l:libopencv_video.so.2.4.9  -l:libopencv_calib3d.so.2.4.9  -l:libopencv_contrib.so.2.4.9  -l:libopencv_features2d.so.2.4.9  -l:libopencv_flann.so.2.4.9  -l:libopencv_gpu.so.2.4.9  -l:libopencv_imgproc.so.2.4.9  -l:libopencv_legacy.so.2.4.9  -l:libopencv_ml.so.2.4.9  -l:libopencv_nonfree.so.2.4.9  -l:libopencv_objdetect.so.2.4.9  -l:libopencv_ocl.so.2.4.9  -l:libopencv_photo.so.2.4.9   -l:libopencv_stitching.so.2.4.9  -l:libopencv_superres.so.2.4.9  -ltesseract -llept -std=c++11 -o rusalpr
'''

* Runing app

'''
./rusalpr path/to/image
'''


####Screenshot

![Gittip](http://i.imgur.com/3WfcwvR.png)
![Gittip](http://i.imgur.com/jCFUDqF.png)
![Gittip](http://i.imgur.com/7MloYGh.png)
![Gittip](http://i.imgur.com/qgbpOto.png)
![Gittip](http://i.imgur.com/1XiqEo3.png)
![Gittip](http://i.imgur.com/Uv8E4IA.png)
