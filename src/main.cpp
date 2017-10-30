#include "imageprocess.h"

#include <iostream>
using namespace std;

#include <stdio.h>

//debug switch
//#define __DEBUG
//#define __DEBUG_IMG

int main(int argc, char * argv[]) {
	Imageprocess imageprocess;

	imageprocess.BmpImgLoad("../img/data.bmp");
	unsigned char * RawData = imageprocess.GetRawData();
	unsigned char * GrayData = imageprocess.GetTransGray(RawData);
	int * HistGram = imageprocess.GetHistGram(GrayData); 
	int Threshold = imageprocess.GetOSTUThreshold(HistGram);
	unsigned char * BinaryzationedData = imageprocess.GetBinaryzationedImgRawData(GrayData, Threshold);
	imageprocess.SaveAsBmpFile(BinaryzationedData, 1, "../img/binarization_img.bmp");
	imageprocess.SaveAsBmpFile(GrayData, 8, "../img/gray_img.bmp");
	unsigned char * AverageFilterData = imageprocess.AverageFilter(GrayData);
	imageprocess.SaveAsBmpFile(AverageFilterData, 8, "../img/average_gray_img.bmp");
	unsigned char * MiddleFilterData = imageprocess.MiddleFilter(GrayData);
	imageprocess.SaveAsBmpFile(MiddleFilterData, 8, "../img/middle_gray_img.bmp");


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