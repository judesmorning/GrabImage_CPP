#ifndef _ENHANCE_DISTRIBUTION_H_
#define _ENHANCE_DISTRIBUTION_H_

#define MAX_POINTS 6
#define MODIFY 1

#if !MODIFY
#include "cv.h"
#include "cxcore.h"
#include "highgui.h"

#pragma comment(lib,"highgui")
#pragma comment(lib,"cv")
#pragma comment(lib,"cxcore")

#endif

typedef struct  
{
	//unsigned char ctr_num;
	unsigned int ctr_num;
	double ctr_px[MAX_POINTS];
	double ctr_py[MAX_POINTS];
	int clah_lg;//最低灰度值
	int clah_hg;//最高灰度值
	int clah_wblock_num;
	int clah_hblock_num;
	int gray_num;
	float clip_limit;
}Enhance_Distribute_Params;



typedef struct  
{
	Enhance_Distribute_Params params;
	unsigned char find_table[256];
	void *interpolation;
}Enhance_Distribute_Moudle;

Enhance_Distribute_Moudle *initialize_enhance_distri();
void configure_enhance_distri(Enhance_Distribute_Moudle *module,Enhance_Distribute_Params params);
#if MODIFY
void process_enhance_distri(Enhance_Distribute_Moudle *module,unsigned char *src,unsigned char *rst,int width,int height);
#else
void process_enhance_distri(Enhance_Distribute_Moudle *module,IplImage *src,IplImage *rst);
#endif
void terminate_enhance_distri(Enhance_Distribute_Moudle *module);

//锐化
void LinearSharpen_Avg(unsigned char* lpImage, int nWidth, int nHeight, float temp_num, unsigned char* SharpenImage);

//亮度 对比度
void BrightnessAndContrast(unsigned char *pImgData, int nHeight, int nWidth, unsigned char *pDestData);

int normalize(unsigned char *pImg, double saturated, int width,int height);
int equalize(unsigned char *pImg, int width,int height);

void filter3x3(unsigned char *pixels, int type, int width,int height);
void filter5x5(unsigned char *pixels, int type, int width,int height);

bool SaveJpgImg(wchar_t *strSavePath, int nWidth, int nHeight, BYTE *imgDataBuf);
bool GetJpgCompressData(int nWidth, int nHeight, BYTE *imgDataBuf, BYTE **outbuffer, size_t *outsize, int nImgQuality=70);

void flipHorz(BYTE *data, int columns, int rows);

void createlut(unsigned char *pImg, double saturated, int width,int height, int flag, int nCamIndex);
void EnhanceImg(unsigned char *pImg, int width,int height, int nCamIndex);


#endif