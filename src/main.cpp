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
	imageprocess.SaveAsBmpFile(BinaryzationedData, 1);
	imageprocess.SaveAsBmpFile(GrayData, 8);

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
	unsigned int j;
	for (unsigned int i; i<256; i++) {
		printf("%d : %d\n", i, HistGram[i]);
		j += HistGram[i];
	}
	cout << "number of hist: " << j << endl;
#endif

	return 0;	
}