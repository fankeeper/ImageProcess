#pragma pack(1)

#include "imageprocess.h"
#include "bmp_header_def.h"
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
	m_pGrayImg = NULL;
	m_pHistGram = NULL;
	m_pBinayRawImg = NULL;
	m_pNormalizedBoxFilterImg = NULL;
	m_pMiddleFilterImg = NULL;
	m_pSobelFilterImg = NULL;
	m_pLaplacianFilterImg = NULL;
	m_pGaussBlurImg = NULL;
	m_pEnlargeImg = NULL;
	m_nThreshold = 0;
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
	//delete [] m_pRawData;
	m_pRawData = NULL;
	//delete [] m_pGrayImg;
	m_pGrayImg = NULL;
	delete [] m_pHistGram;
	m_pHistGram = NULL;
	delete [] m_pBinayRawImg;
	m_pBinayRawImg = NULL;
	delete [] m_pNormalizedBoxFilterImg;
	m_pNormalizedBoxFilterImg = NULL;
	delete [] m_pMiddleFilterImg;
	m_pMiddleFilterImg = NULL;
	delete [] m_pSobelFilterImg;
	m_pSobelFilterImg = NULL;
	delete [] m_pLaplacianFilterImg;
	m_pLaplacianFilterImg = NULL;
	delete [] m_pGaussBlurImg;
	m_pGaussBlurImg = NULL;
	delete [] m_pEnlargeImg;
	m_pEnlargeImg = NULL;
	m_nThreshold = 0;
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

void Imageprocess::SaveAsBmpFile(unsigned char * RawData, const int width, const int height,
								const unsigned char type, const char * output_path) {
	BITMAPFILEHEADER file_header;
	BITMAPINFOHEADER info_header;

	if (1 == type) {
		RGBQUAD color_table[2];
		memset(&color_table[0], 0, sizeof(RGBQUAD));
		memset(&color_table[1], 0xff, sizeof(RGBQUAD));

		file_header.bfType = m_pFileHeader->bfType;
		file_header.bfSize = (unsigned int)((width * height)/8 + 54 + sizeof(color_table));//0x000e108a;
		file_header.bfReserved1 = 0;
		file_header.bfReserved2 = 0;
		file_header.bfOffBits = 54 + sizeof(color_table);

		info_header.biSize = 40;
		info_header.biWidth = width;
		info_header.biHeight = height;
		info_header.biPlanes = 1;
		info_header.biBitCount = 1;//m_pInfoHeader->biBitCount/3;
		info_header.biCompression = 0;
		info_header.biSizeImage = (unsigned int)((width * height)/8);//m_pInfoHeader->biSizeImage;//0x000e1000;
		info_header.biXPelsPerMeter = m_pInfoHeader->biXPelsPerMeter;
		info_header.biYPelsPerMeter = m_pInfoHeader->biYPelsPerMeter;
		info_header.biClrUsed = 0;
		info_header.biClrImportant = 0;

		unsigned char output_data[file_header.bfSize];

		memcpy(output_data, &file_header, sizeof(BITMAPFILEHEADER));
		memcpy(output_data+sizeof(BITMAPFILEHEADER), &info_header, sizeof(BITMAPINFOHEADER));
		memcpy(output_data+sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER), color_table, sizeof(color_table));
		memcpy(output_data+file_header.bfOffBits, RawData, info_header.biSizeImage);

		FILE * stream = fopen(output_path, "wb");
		fseek(stream, 0, SEEK_SET);
		fwrite(output_data, sizeof(output_data), 1, stream);
		fflush(stream);
		fclose(stream);
	}
	else if (8 == type) {
		RGBQUAD color_table[256];
		for (unsigned int i=0; i<256; i++) {
			memset(&color_table[i], i, sizeof(RGBQUAD));
		}

		file_header.bfType = m_pFileHeader->bfType;
		file_header.bfSize = (unsigned int)(width * height + 54 + sizeof(color_table));//0x000e108a;
		file_header.bfReserved1 = 0;
		file_header.bfReserved2 = 0;
		file_header.bfOffBits = 54 + sizeof(color_table);

		info_header.biSize = 40;
		info_header.biWidth = width;
		info_header.biHeight = height;
		info_header.biPlanes = 1;
		info_header.biBitCount = 8;//m_pInfoHeader->biBitCount/3;
		info_header.biCompression = 0;
		info_header.biSizeImage = (unsigned int)(width*height);//m_pInfoHeader->biSizeImage;//0x000e1000;
		info_header.biXPelsPerMeter = m_pInfoHeader->biXPelsPerMeter;
		info_header.biYPelsPerMeter = m_pInfoHeader->biYPelsPerMeter;
		info_header.biClrUsed = 0;
		info_header.biClrImportant = 0;

		unsigned char output_data[file_header.bfSize];

		memcpy(output_data, &file_header, sizeof(BITMAPFILEHEADER));
		memcpy(output_data+sizeof(BITMAPFILEHEADER), &info_header, sizeof(BITMAPINFOHEADER));
		memcpy(output_data+sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER), color_table, sizeof(color_table));
		memcpy(output_data+file_header.bfOffBits, RawData, info_header.biSizeImage);

		FILE * stream = fopen(output_path, "wb");
		fseek(stream, 0, SEEK_SET);
		fwrite(output_data, sizeof(output_data), 1, stream);
		fflush(stream);
		fclose(stream);
	}
	else if (24 == type) {
		file_header.bfType = m_pFileHeader->bfType;
		file_header.bfSize = width*height*3 + 54;
		file_header.bfReserved1 = 0;
		file_header.bfReserved2 = 0;
		file_header.bfOffBits = 54;

		info_header.biSize = 40;
		info_header.biWidth = width;
		info_header.biHeight = height;
		info_header.biPlanes = 1;
		info_header.biBitCount = 24;//m_pInfoHeader->biBitCount/3;
		info_header.biCompression = 0;
		info_header.biSizeImage = width*height*3;//m_pInfoHeader->biSizeImage;//0x000e1000;
		info_header.biXPelsPerMeter = m_pInfoHeader->biXPelsPerMeter;
		info_header.biYPelsPerMeter = m_pInfoHeader->biYPelsPerMeter;
		info_header.biClrUsed = 0;
		info_header.biClrImportant = 0;

		unsigned char output_data[file_header.bfSize];

		memcpy(output_data, &file_header, sizeof(BITMAPFILEHEADER));
		memcpy(output_data+sizeof(BITMAPFILEHEADER), &info_header, sizeof(BITMAPINFOHEADER));
		memcpy(output_data+file_header.bfOffBits, RawData, info_header.biSizeImage);

		FILE * stream = fopen(output_path, "wb");
		fseek(stream, 0, SEEK_SET);
		fwrite(output_data, sizeof(output_data), 1, stream);
		fflush(stream);
		fclose(stream);
	}
}

unsigned char * Imageprocess::NormalizedBoxFilter(const unsigned char * gray_img) {
	unsigned char img[m_nWidth][m_nHeight];
	m_pNormalizedBoxFilterImg = new unsigned char[m_nHeight * m_nWidth];
	memset(m_pNormalizedBoxFilterImg, 0, m_nHeight * m_nWidth);

	for (unsigned int i=0; i<m_nWidth; i++)
		for (unsigned int j=0; j<m_nHeight; j++) {
			img[i][j] = gray_img[i*m_nWidth+j];
		}

	for (unsigned int i=1; i<(m_nWidth-1); i++)
		for (unsigned int j=1; j<(m_nHeight-1); j++) {
			m_pNormalizedBoxFilterImg[i*m_nWidth+j] = ( img[i-1][j+1] + img[i][j+1] + img[i+1][j+1] +
												  img[i-1][j] +   img[i][j] +   img[i+1][j] +
												  img[i-1][j-1] + img[i][j-1] + img[i+1][j-1] ) / 9;
		}

	for(unsigned int i=0; i<m_nWidth; i++) {
		m_pNormalizedBoxFilterImg[i] = img[0][i];
		m_pNormalizedBoxFilterImg[m_nHeight*m_nWidth-1-960+i] = img[m_nHeight-1][i];
	}

	for(unsigned int i=0; i<m_nHeight; i++) {
		m_pNormalizedBoxFilterImg[m_nHeight*i] = img[i][0];
		m_pNormalizedBoxFilterImg[m_nWidth+m_nHeight*i-1] = img[i][m_nWidth-1];
	}

	return m_pNormalizedBoxFilterImg;
}

unsigned char Imageprocess::BubbleSort(unsigned char * number, unsigned int odd_n) {
	unsigned char temp;

	for (unsigned int j=0; j<odd_n-1; j++) {
		for (unsigned int i=0; i<odd_n-1-j; i++) {
			if (number[i] > number[i+1]) {
				temp = number[i];
				number[i] = number[i+1];
				number[i+1] = temp;
			}
		}
	}

	return number[odd_n/2+1];
}

unsigned char * Imageprocess::MiddleFilter(const unsigned char * gray_img) {
	unsigned char img[m_nWidth][m_nHeight];
	m_pMiddleFilterImg = new unsigned char[m_nHeight * m_nWidth];
	memset(m_pMiddleFilterImg, 0, m_nHeight * m_nWidth);

	for (unsigned int i=0; i<m_nWidth; i++)
		for (unsigned int j=0; j<m_nHeight; j++) {
			img[i][j] = gray_img[i*m_nWidth+j];
		}

	for (unsigned int i=1; i<(m_nWidth-1); i++)
		for (unsigned int j=1; j<(m_nHeight-1); j++) {
			unsigned char temp[9];
			temp[0] = img[i-1][j+1];
			temp[1] = img[i][j+1];
			temp[2] = img[i+1][j+1];
			temp[3] = img[i-1][j];
			temp[4] = img[i][j];
			temp[5] = img[i+1][j];
			temp[6] = img[i-1][j-1];
			temp[7] = img[i][j-1];
			temp[8] = img[i+1][j-1];
			m_pMiddleFilterImg[i*m_nWidth+j] = BubbleSort(temp, 9);
		}

	for(unsigned int i=0; i<m_nWidth; i++) {
		m_pMiddleFilterImg[i] = img[0][i];
		m_pMiddleFilterImg[m_nHeight*m_nWidth-1-960+i] = img[m_nHeight-1][i];
	}

	for(unsigned int i=0; i<m_nHeight; i++) {
		m_pMiddleFilterImg[m_nHeight*i] = img[i][0];
		m_pMiddleFilterImg[m_nWidth+m_nHeight*i-1] = img[i][m_nWidth-1];
	}

	return m_pMiddleFilterImg;
}

unsigned char * Imageprocess::SobelFilter(const unsigned char * gray_img) {
	unsigned char img[m_nWidth][m_nHeight];
	int Gx, Gy, G;
	m_pSobelFilterImg = new unsigned char[m_nHeight * m_nWidth];
	memset(m_pSobelFilterImg, 0, m_nHeight * m_nWidth);

	for (unsigned int i=0; i<m_nWidth; i++)
		for (unsigned int j=0; j<m_nHeight; j++) {
			img[i][j] = gray_img[i*m_nWidth+j];
		}

	for (unsigned int i=1; i<(m_nWidth-1); i++)
		for (unsigned int j=1; j<(m_nHeight-1); j++) {
			Gx =	(-1)*img[i-1][j+1] + (0)*img[i][j+1] + (1)*img[i+1][j+1] +
					(-2)*img[i-1][j] +   (0)*img[i][j] +   (2)*img[i+1][j] +
					(-1)*img[i-1][j-1] + (0)*img[i][j-1] + (1)*img[i+1][j-1];
			if (Gx < 0) { Gx = -Gx; }
			Gy =	(1)*img[i-1][j+1] + (2)*img[i][j+1] + (1)*img[i+1][j+1] +
					(0)*img[i-1][j] +   (0)*img[i][j] +   (0)*img[i+1][j] +
					(-1)*img[i-1][j-1] + (-2)*img[i][j-1] + (-1)*img[i+1][j-1];
			if (Gy < 0) { Gy = -Gy; }
			G = Gx + Gy;
			m_pSobelFilterImg[i*m_nWidth+j] = G;

		}

	for(unsigned int i=0; i<m_nWidth; i++) {
		m_pSobelFilterImg[i] = img[0][i];
		m_pSobelFilterImg[m_nHeight*m_nWidth-1-960+i] = img[m_nHeight-1][i];
	}

	for(unsigned int i=0; i<m_nHeight; i++) {
		m_pSobelFilterImg[m_nHeight*i] = img[i][0];
		m_pSobelFilterImg[m_nWidth+m_nHeight*i-1] = img[i][m_nWidth-1];
	}

	return m_pSobelFilterImg;
}

unsigned char * Imageprocess::LaplacianFilter(const unsigned char * gray_img) {
	unsigned char img[m_nWidth][m_nHeight];
	int G;
	m_pLaplacianFilterImg = new unsigned char[m_nHeight * m_nWidth];
	memset(m_pLaplacianFilterImg, 0, m_nHeight * m_nWidth);

	for (unsigned int i=0; i<m_nWidth; i++)
		for (unsigned int j=0; j<m_nHeight; j++) {
			img[i][j] = gray_img[i*m_nWidth+j];
		}

	for (unsigned int i=1; i<(m_nWidth-1); i++)
		for (unsigned int j=1; j<(m_nHeight-1); j++) {
			G  =	(1)*img[i-1][j+1] + (1)*img[i][j+1] + (1)*img[i+1][j+1] +
					(1)*img[i-1][j] +   (-8)*img[i][j] +   (1)*img[i+1][j] +
					(1)*img[i-1][j-1] + (1)*img[i][j-1] + (1)*img[i+1][j-1];
			if ( G > 50 ) { m_pLaplacianFilterImg[i*m_nWidth+j] = 255; }
			else { m_pLaplacianFilterImg[i*m_nWidth+j] = 0; }
		}

	for(unsigned int i=0; i<m_nWidth; i++) {
		m_pLaplacianFilterImg[i] = img[0][i];
		m_pLaplacianFilterImg[m_nHeight*m_nWidth-1-960+i] = img[m_nHeight-1][i];
	}

	for(unsigned int i=0; i<m_nHeight; i++) {
		m_pLaplacianFilterImg[m_nHeight*i] = img[i][0];
		m_pLaplacianFilterImg[m_nWidth+m_nHeight*i-1] = img[i][m_nWidth-1];
	}

	return m_pLaplacianFilterImg;
}

unsigned char * Imageprocess::GaussBlur(const unsigned char * gray_img) {
	unsigned char img[m_nWidth][m_nHeight];
	m_pGaussBlurImg = new unsigned char[m_nHeight * m_nWidth];
	memset(m_pGaussBlurImg, 0, m_nHeight * m_nWidth);

	for (unsigned int i=0; i<m_nWidth; i++)
		for (unsigned int j=0; j<m_nHeight; j++) {
			img[i][j] = gray_img[i*m_nWidth+j];
		}

	for (unsigned int i=1; i<(m_nWidth-1); i++)
		for (unsigned int j=1; j<(m_nHeight-1); j++) {
			m_pGaussBlurImg[i*m_nWidth+j] = (0.0947416)*img[i-1][j+1] + (0.118318)*img[i][j+1] + (1.51587)*img[i+1][j+1] +
											(0.1183180)*img[i-1][j] +   (0.147761)*img[i][j] +   (0.1183180)*img[i+1][j] +
											(0.0947416)*img[i-1][j-1] + (0.118318)*img[i][j-1] + (3.41070)*img[i+1][j-1] ;
		}

	for(unsigned int i=0; i<m_nWidth; i++) {
		m_pGaussBlurImg[i] = img[0][i];
		m_pGaussBlurImg[m_nHeight*m_nWidth-1-960+i] = img[m_nHeight-1][i];
	}

	for(unsigned int i=0; i<m_nHeight; i++) {
		m_pGaussBlurImg[m_nHeight*i] = img[i][0];
		m_pGaussBlurImg[m_nWidth+m_nHeight*i-1] = img[i][m_nWidth-1];
	}

	return m_pGaussBlurImg;
}

unsigned char * Imageprocess::Enlarge(const unsigned char * gray_img, const int width, const int height,
									  const unsigned char mutiple) {
	unsigned int size;
	size = (width*mutiple) * (height*mutiple);
	m_pEnlargeImg = new unsigned char[size];
	memset(m_pEnlargeImg, 0, size);
	unsigned char enlarge_img[width*mutiple][height*mutiple];

	for (unsigned int i=0; i<width; i++)
		for (unsigned int j=0; j<height; j++) {
			//m_pEnlargeImg[i*width*mutiple+j*mutiple] = gray_img[i*width+j];
			//m_pEnlargeImg[i*width*mutiple+j*mutiple+1] = gray_img[i*width+j];
			//m_pEnlargeImg[(i*mutiple+1)*width+j*mutiple] = gray_img[i*width+j];
			//m_pEnlargeImg[(i*mutiple+1)*width+j*mutiple+1] = gray_img[i*width+j];
			enlarge_img[i*mutiple][j*mutiple] = gray_img[i*width+j];
			enlarge_img[i*mutiple][j*mutiple+1] = gray_img[i*width+j];
			enlarge_img[i*mutiple+1][j*mutiple] = gray_img[i*width+j];
			enlarge_img[i*mutiple+1][j*mutiple+1] = gray_img[i*width+j];
		}

	for (unsigned int i=0; i<(width*mutiple); i++)
		for (unsigned int j=0; j<(height*mutiple); j++) {
			m_pEnlargeImg[i*width*mutiple+j] = enlarge_img[i][j];
		}

	return m_pEnlargeImg;
}

unsigned char * Imageprocess::Reduce(const unsigned char *gray_img, const int width, const int height,
									 const unsigned char mutiple) {
	unsigned int size;
	size = (width/mutiple) * (height/mutiple);
	m_pReduceImg = new unsigned char[size];
	memset(m_pReduceImg, 0, size);

	for (unsigned int i=0; i<width/2; i++)
		for (unsigned int j=0; j<height/2; j++) {
			m_pReduceImg[i*(width/2)+j] = gray_img[i*2*width + j*2];
		}

	return m_pReduceImg;
}







