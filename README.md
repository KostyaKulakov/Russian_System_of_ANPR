### ANPR System: The Russian system of automatic number plate recognition

####Dependencies
* OpenCV >= 2.4.9
* leptonica >= 1.71
* Tesseract OCR >= 3.02

####Build
Copy the folder `runtime_data/tessdata` in root directory TesseartOCR or use `TESSDATA_PREFIX`

Copy the file `runtime_data/haarcascade_russian_plate_number.xml` to a folder with the compiled program
```
g++ *.cpp -lopencv_core249 -lopencv_highgui249 -lopencv_video249 -lopencv_calib3d249 -lopencv_contrib249 -lopencv_features2d249 -lopencv_flann249 -lopencv_gpu249 -lopencv_imgproc249 -lopencv_legacy249 -lopencv_ml249 -lopencv_nonfree249 -lopencv_objdetect249 -lopencv_ocl249 -lopencv_photo249  -lopencv_stitching249 -lopencv_superres249 -ltesseract -llept -lws2_32 -std=c++11 -o rusalpr
```

####Screenshot

![Gittip](http://files.kostyakulakov.ru/images/1.png)
![Gittip](http://files.kostyakulakov.ru/images/2.png)
