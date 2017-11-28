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

	imageprocess.SaveAsBmpFile(BinaryzationedData, 960, 960, 1, "../img/binarization_img.bmp");
	cout << "save binarization_img sucessfully." << endl;

	imageprocess.SaveAsBmpFile(GrayData, 960, 960, 8, "../img/gray_img.bmp");
	cout << "save gray_img sucessfully." << endl;

	unsigned char * NormalizedBoxFilterData = imageprocess.NormalizedBoxFilter(GrayData);
	imageprocess.SaveAsBmpFile(NormalizedBoxFilterData, 960, 960, 8, "../img/normalized_gray_img.bmp");
	cout << "save normalized_gray_img sucessfully." << endl;

	unsigned char * MiddleFilterData = imageprocess.MiddleFilter(GrayData);
	imageprocess.SaveAsBmpFile(MiddleFilterData, 960, 960, 8, "../img/middle_gray_img.bmp");
	cout << "save middle_gray_img sucessfully." << endl;

	unsigned char * SobelFilterData = imageprocess.SobelFilter(GrayData);
	imageprocess.SaveAsBmpFile(SobelFilterData, 960, 960, 8, "../img/sobel_img.bmp");
	cout << "save sobel_img sucessfully." << endl;

	unsigned char * LaplacianFilterData = imageprocess.LaplacianFilter(GrayData);
	imageprocess.SaveAsBmpFile(LaplacianFilterData, 960, 960, 8, "../img/laplacian_img.bmp");
	cout << "save laplacian_img sucessfully." << endl;

	unsigned char * GaussBlurData = imageprocess.GaussBlur(GrayData);
	imageprocess.SaveAsBmpFile(GaussBlurData, 960, 960, 8, "../img/gaussblur_img.bmp");
	cout << "save gaussblur_img sucessfully." << endl;

	unsigned char * EnlargeImg = imageprocess.Enlarge(GrayData, 960, 960, 2);
	imageprocess.SaveAsBmpFile(EnlargeImg, 960*2, 960*2, 8, "../img/enlarge_img.bmp");
	cout << "save enlarge_img suncessfully." << endl;

	unsigned char * ReduceImg = imageprocess.Reduce(GrayData, 960, 960, 2);
	imageprocess.SaveAsBmpFile(ReduceImg, 960/2, 960/2, 8, "../img/reduce_img.bmp");
	cout << "save reduce_img suncessfully." << endl;

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
	for (unsigned int i=1800*1800; i<1920*1920; i++) {
		printf("%d ", EnlargeImg[i]);
	}
#endif

	return 0;	
}