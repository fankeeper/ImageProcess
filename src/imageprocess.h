#ifndef _BINARYZATION_H_
#define _BINARYZATION_H_

#include "bmp_header_def.h"

class Imageprocess {

public:
	Imageprocess();//finished
	~Imageprocess();//finished

public:
	unsigned int BmpImgLoad(const char * bmp_file);//finished
	void BmpImgUnLoad();//finished

	unsigned int GetOffBits();//finished
	unsigned int GetRawDataSize();//finished
	unsigned int GetHeight();//finished
	unsigned int GetWidth();//finished
	unsigned char * GetRawData();//finished
	unsigned char * GetAllData();//finished
	//finished
	unsigned char * GetTransGray(const unsigned char * raw_data);
	int * GetHistGram(const unsigned char * gray_img);
	int GetOSTUThreshold(int * HistGram);
	unsigned char * GetBinaryzationedImgRawData(const unsigned char * gray_img, int Threshold);
	void SaveAsBmpFile(unsigned char * RawData, const int width, const int height,
					   const unsigned char type, const char * output_path);
	unsigned char * NormalizedBoxFilter(const unsigned char * gray_img);
	unsigned char * MiddleFilter(const unsigned char * gray_img);
	unsigned char * SobelFilter(const unsigned char * gray_img);
	unsigned char * LaplacianFilter(const unsigned char * gray_img);
	unsigned char * GaussBlur(const unsigned char * gray_img);
	unsigned char * Enlarge(const unsigned char * gray_img, const int width, const int height,
							const unsigned char mutiple);
	unsigned char * Reduce(const unsigned char *gray_img, const int width, const int height,
						   const unsigned char mutiple);

private:
	unsigned char BubbleSort(unsigned char * number, unsigned int odd_n);

private:
	PBITMAPFILEHEADER m_pFileHeader;
	PBITMAPINFOHEADER m_pInfoHeader;
	unsigned char * m_buffer;
	unsigned char * m_pRawData;
	unsigned char * m_pGrayImg;
	int * m_pHistGram;
	unsigned char * m_pBinayRawImg;
	unsigned char * m_pNormalizedBoxFilterImg;
	unsigned char * m_pMiddleFilterImg;
	unsigned char * m_pSobelFilterImg;
	unsigned char * m_pLaplacianFilterImg;
	unsigned char * m_pGaussBlurImg;
	unsigned char * m_pEnlargeImg;
	unsigned char * m_pReduceImg;
	unsigned int m_nThreshold;
	unsigned int m_nWidth;
	unsigned int m_nHeight;
	unsigned int m_nSize;
	unsigned int m_nbfOffBits;
	unsigned char m_nLineByte;
};

#endif