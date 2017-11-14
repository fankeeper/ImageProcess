#include "imageprocess.h"

#include <iostream>
using namespace std;

#include <stdio.h>

//debug switch
//#define __DEBUG
//#define __DEBUG_IMG

int main(int argc, char * argv[]) {
	Imageprocess imageprocess;

	imageprocess.BmpImgLoad("../img/source_img.bmp");
	unsigned char * RawData = imageprocess.GetRawData();
	unsigned char * GrayData = imageprocess.GetTransGray(RawData);
	int * HistGram = imageprocess.GetHistGram(GrayData); 
	int Threshold = imageprocess.GetOSTUThreshold(HistGram);
	unsigned char * BinaryzationedData = imageprocess.GetBinaryzationedImgRawData(GrayData, Threshold);
	imageprocess.SaveAsBmpFile(BinaryzationedData, 1, "../img/binarization_img.bmp");
	cout << "save binarization_img sucessfully." << endl;
	imageprocess.SaveAsBmpFile(GrayData, 8, "../img/gray_img.bmp");
	cout << "save gray_img sucessfully." << endl;
	unsigned char * NormalizedBoxFilterData = imageprocess.NormalizedBoxFilter(GrayData);
	imageprocess.SaveAsBmpFile(NormalizedBoxFilterData, 8, "../img/normalized_gray_img.bmp");
	cout << "save normalized_gray_img sucessfully." << endl;
	unsigned char * MiddleFilterData = imageprocess.MiddleFilter(GrayData);
	imageprocess.SaveAsBmpFile(MiddleFilterData, 8, "../img/middle_gray_img.bmp");
	cout << "save middle_gray_img sucessfully." << endl;
	unsigned char * SobelFilterData = imageprocess.SobelFilter(GrayData);
	imageprocess.SaveAsBmpFile(SobelFilterData, 8, "../img/sobel_img.bmp");
	cout << "save sobel_img sucessfully." << endl;


#ifdef __DEBUG
	cout << "size:  " << imageprocess.GetRawDataSize() << endl;
	cout << "width: " << imageprocess.GetWidth() << endl;
	cout << "height:" << imageprocess.GetHeight() << endl;

	unsigned int z;
	for (unsigned int i=0; i<(921600*3); i++) {
		if (RawData[i] == 0) z++;
	}
	cout << "number of 0(RawData):" << z << endl;
	cout << "Threshold:" << Threshold << endl;
#endif

#ifdef __DEBUG_IMG
	for (unsigned int i; i<2000; i++) {
		printf("%d ", AverageFilterData[i]);
	}
#endif

	return 0;	
}