#pragma pack(1)

#include "imageprocess.h"
#include <stdio.h>
#include <string.h>

#include <iostream>
using namespace std;

#define __DEBUG

Imageprocess::Imageprocess() {
	m_pFileHeader = NULL;
	m_pInfoHeader = NULL;
	m_buffer = NULL;
	m_pRawData = NULL;
	m_nWidth = 0;
	m_nHeight = 0;
	m_nSize = 0;
	m_nbfOffBits = 0;
}

Imageprocess::~Imageprocess() {
	BmpImgUnLoad();
}

unsigned int Imageprocess::BmpImgLoad(const char * bmp_file) {
	FILE * pf = fopen(bmp_file, "rb");
	if (NULL == pf) {
		return 0;
	}
	
	fseek(pf, 0, SEEK_END);
	long length = ftell(pf);
	rewind(pf);
	m_buffer = new unsigned char[length];
	m_pRawData = new unsigned char[m_nSize];
	fread(m_buffer, length, 1, pf);

	m_pFileHeader = (PBITMAPFILEHEADER)(m_buffer);
	m_pInfoHeader = (PBITMAPINFOHEADER)(m_buffer + sizeof(BITMAPFILEHEADER));
	m_nWidth = m_pInfoHeader->biWidth;
	m_nHeight = m_pInfoHeader->biHeight;
	m_nSize = m_pInfoHeader->biSizeImage;
	m_nbfOffBits = m_pFileHeader->bfOffBits;

	//four bytes align
	//0: 0~960 1: (width+1)*j
	unsigned char * UnalignRawData = m_buffer + m_pFileHeader->bfOffBits;
	if (0 == m_nWidth%4) {
		//memcpy(m_pRawData, UnalignRawData, m_nSize);
		m_pRawData = m_buffer + m_pFileHeader->bfOffBits;
	}
	else {
		for (unsigned int i; i<m_nSize; i++){
			m_pRawData[i] = UnalignRawData[(m_nWidth+1)*(i/m_nWidth)+i%m_nWidth];
		}
	}

	//m_pRawData = m_buffer + m_pFileHeader->bfOffBits;

	fclose(pf);

	return 0;
}

void Imageprocess::BmpImgUnLoad() {
	if (m_buffer) {
	m_pFileHeader = NULL;
	m_pInfoHeader = NULL;
	delete [] m_buffer;
	m_buffer = NULL;
	m_pRawData = NULL;
	m_nWidth = 0;
	m_nHeight = 0;
	m_nSize = 0;
	m_nbfOffBits = 0;
	}
}

unsigned int Imageprocess::GetOffBits() {
	return m_nbfOffBits;
}

unsigned int Imageprocess::GetRawDataSize() {
	return m_nSize;
}

unsigned int Imageprocess::GetHeight() {
	return m_nHeight;
}
	
unsigned int Imageprocess::GetWidth() {
	return m_nWidth;
}

unsigned char * Imageprocess::GetRawData() {
	return m_pRawData;
}

unsigned char * Imageprocess::GetAllData() {
	return m_buffer;
}

unsigned char * Imageprocess::GetTransGray(const unsigned char * raw_data) {
	m_pGrayImg = new unsigned char[m_nSize/3];
	memset(m_pGrayImg, 0, m_nSize/3);

	unsigned int instant;
	for (unsigned int i=0; i<m_nSize; i++) {
		if (i%3 == 0) {
			instant = (unsigned int)raw_data[i] * 114;
		}
		if (i%3 == 1) {
			instant += (unsigned int)raw_data[i] * 587;
		}
		if (i%3 == 2) {
			instant = (instant + ((unsigned int)raw_data[i] * 299) + 500) / 1000;
		}
		m_pGrayImg[i/3] = (unsigned char)instant;
	}

	return m_pGrayImg;
}

int * Imageprocess::GetHistGram(const unsigned char * gray_img) {
	m_pHistGram = new int[256];
	memset(m_pHistGram, 0, 256*sizeof(int));

	for (unsigned int i=0; i<(m_nSize/3); i++) {
		for (unsigned int j=0; j<256; j++) {
			if (gray_img[i] == j) { m_pHistGram[j]++; }
		}
	}

	return m_pHistGram;
}

int Imageprocess::GetOSTUThreshold(int * HistGram) {
	int X, Y, Amount = 0;
    int PixelBack = 0, PixelFore = 0, PixelIntegralBack = 0, PixelIntegralFore = 0, PixelIntegral = 0;
    double OmegaBack, OmegaFore, MicroBack, MicroFore, SigmaB, Sigma;              // 类间方差;
    int MinValue, MaxValue;
    int Threshold = 0;

    for (MinValue = 0; MinValue < 256 && HistGram[MinValue] == 0; MinValue++) ;
    for (MaxValue = 255; MaxValue > MinValue && HistGram[MinValue] == 0; MaxValue--) ;
    if (MaxValue == MinValue) return MaxValue;          // 图像中只有一个颜色             
    if (MinValue + 1 == MaxValue) return MinValue;      // 图像中只有二个颜色

    for (Y = MinValue; Y <= MaxValue; Y++) Amount += HistGram[Y];        //  像素总数

    PixelIntegral = 0;
    for (Y = MinValue; Y <= MaxValue; Y++) PixelIntegral += HistGram[Y] * Y;
    SigmaB = -1;
    for (Y = MinValue; Y < MaxValue; Y++)
    {
        PixelBack = PixelBack + HistGram[Y];
        PixelFore = Amount - PixelBack;
        OmegaBack = (double)PixelBack / Amount;
        OmegaFore = (double)PixelFore / Amount;
        PixelIntegralBack += HistGram[Y] * Y;
        PixelIntegralFore = PixelIntegral - PixelIntegralBack;
        MicroBack = (double)PixelIntegralBack / PixelBack;
        MicroFore = (double)PixelIntegralFore / PixelFore;
        Sigma = OmegaBack * OmegaFore * (MicroBack - MicroFore) * (MicroBack - MicroFore);
        if (Sigma > SigmaB)
        {
            SigmaB = Sigma;
            Threshold = Y;
        }
    }

    return Threshold;
}

unsigned char * Imageprocess::GetBinaryzationedImgRawData(const unsigned char * gray_img,
													   int Threshold) {
/*	m_pBinayRawImg = new unsigned char[m_nSize];
	memset(m_pBinayRawImg, 0, m_nSize);

	for(unsigned int i=0; i<(m_nSize/3); i++) {
		if (gray_img[i] <= Threshold) {
			m_pBinayRawImg[i*3] = 0;
			m_pBinayRawImg[i*3+1] = 0;
			m_pBinayRawImg[i*3+2] = 0;
		}
		else {
			m_pBinayRawImg[i*3] = 255;
			m_pBinayRawImg[i*3+1] = 255;
			m_pBinayRawImg[i*3+2] = 255;
		}
	}
*/

	m_pBinayRawImg = new unsigned char[m_nSize/24];
	memset(m_pBinayRawImg, 0, m_nSize/24);

	for(unsigned int i=0; i<(m_nSize/3); i++) {
		if (gray_img[i] <= Threshold) {
			m_pBinayRawImg[i>>3] = ((m_pBinayRawImg[i/8] << 1) | 0);
		}
		else {
			m_pBinayRawImg[i>>3] = ((m_pBinayRawImg[i/8] << 1) | 1);
		}
	}

	return m_pBinayRawImg;
}

void Imageprocess::SaveAsBmpFile(unsigned char * RawData) {
	BITMAPFILEHEADER file_header;
	BITMAPINFOHEADER info_header;

	int color_table[2];
	color_table[0] = 0;
	color_table[1] = 0xffffff;

	file_header.bfType = m_pFileHeader->bfType;
	file_header.bfSize = m_nSize/24 + 54 + sizeof(color_table);//0x000e108a;
	file_header.bfReserved1 = 0;
	file_header.bfReserved2 = 0;
	file_header.bfOffBits = 54 + sizeof(color_table);

	info_header.biSize = 40;
	info_header.biWidth = m_pInfoHeader->biWidth;
	info_header.biHeight = m_pInfoHeader->biHeight;
	info_header.biPlanes = 1;
	info_header.biBitCount = 1;//m_pInfoHeader->biBitCount/3;
	info_header.biCompression = 0;
	info_header.biSizeImage = m_nSize/24;//m_pInfoHeader->biSizeImage;//0x000e1000;
	info_header.biXPelsPerMeter = m_pInfoHeader->biXPelsPerMeter;
	info_header.biYPelsPerMeter = m_pInfoHeader->biYPelsPerMeter;
	info_header.biClrUsed = 0;
	info_header.biClrImportant = 0;

	unsigned char output_data[file_header.bfSize];

	memcpy(output_data, &file_header, sizeof(BITMAPFILEHEADER));
	memcpy(output_data+sizeof(BITMAPFILEHEADER), &info_header, sizeof(BITMAPINFOHEADER));
	memcpy(output_data+sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER), color_table, sizeof(color_table));
	memcpy(output_data+file_header.bfOffBits, RawData, info_header.biSizeImage);

/*
	file_header.bfType = m_pFileHeader->bfType;
	file_header.bfSize = m_nSize + 54 ;//0x000e108a;
	file_header.bfReserved1 = 0;
	file_header.bfReserved2 = 0;
	file_header.bfOffBits = 54;

	info_header.biSize = 40;
	info_header.biWidth = m_pInfoHeader->biWidth;
	info_header.biHeight = m_pInfoHeader->biHeight;
	info_header.biPlanes = 1;
	info_header.biBitCount = 24;//m_pInfoHeader->biBitCount/3;
	info_header.biCompression = 0;
	info_header.biSizeImage = m_nSize;//m_pInfoHeader->biSizeImage;//0x000e1000;
	info_header.biXPelsPerMeter = m_pInfoHeader->biXPelsPerMeter;
	info_header.biYPelsPerMeter = m_pInfoHeader->biYPelsPerMeter;
	info_header.biClrUsed = 0;
	info_header.biClrImportant = 0;

	unsigned char output_data[file_header.bfSize];

	memcpy(output_data, &file_header, sizeof(BITMAPFILEHEADER));
	memcpy(output_data+sizeof(BITMAPFILEHEADER), &info_header, sizeof(BITMAPINFOHEADER));
	memcpy(output_data+file_header.bfOffBits, RawData, info_header.biSizeImage);
*/
	FILE * stream = fopen("../img/binarization_img.bmp", "wb");
	fseek(stream, 0, SEEK_SET);
	fwrite(output_data, sizeof(output_data), 1, stream);
	fflush(stream);
	fclose(stream);
}

unsigned char * Imageprocess::AverageFilter(const unsigned char * gray_img) {
	unsigned char img[m_nWidth][m_nHeight];
	m_pAverageFilterImg = new unsigned char[m_nHeight * m_nWidth];

	for (unsigned int i=0; i<m_nWidth; i++)
		for (unsigned int j=0; j<m_nHeight; j++) {
			img[i][j] = gray_img[i+j];
		}

	for (unsigned int i=1; i<(m_nWidth-1); i++)
		for (unsigned int j=1; j<(m_nHeight-1); j++) {
			m_pAverageFilterImg[i+j] = ( img[i-1][j+1] + img[i][j+1] + img[i+1][j+1] +
										 img[i-1][j] +   img[i][j] +   img[i+1][j] +
										 img[i-1][j-1] + img[i][j-1] + img[i+1][j-1] ) / 9;
		}

	for(unsigned int i=0; i<m_nWidth; i++) {
		m_pAverageFilterImg[i] = img[0][i];
		m_pAverageFilterImg[m_nHeight*m_nWidth-1-960] = img[m_nHeight-1][i];
	}

	for(unsigned int i=0; i<m_nHeight; i++) {
		m_pAverageFilterImg[m_nHeight*i] = img[i][0];
		m_pAverageFilterImg[m_nWidth+m_nHeight*i-1] = img[i][m_nWidth-1];
	}

	return m_pAverageFilterImg;
}



















