#include "StdAfx.h"
#include "tl_enhance_distribution.h"
#include "Interpolation.h"
#include "clah.h"
#include "jpeglib.h"

//#include <afxwin.h>         // MFC 核心组件和标准组件

Enhance_Distribute_Moudle *initialize_enhance_distri()
{
	Enhance_Distribute_Moudle *module = new Enhance_Distribute_Moudle;
	memset(module, 0 , sizeof(Enhance_Distribute_Moudle));
	module->interpolation = NULL;
	return module;
}

static void create_find_table(CInterpolation *inter,unsigned char *find_table);
void configure_enhance_distri(Enhance_Distribute_Moudle *module,Enhance_Distribute_Params params)
{
 	module->params.ctr_num = 4;
 	module->params.ctr_px[0] = 0;
 	module->params.ctr_py[0] = 0;//tl20;//20
 	module->params.ctr_px[1] = 50;
 	module->params.ctr_py[1] = 50;//tl100; 
 	module->params.ctr_px[2] = 150;
 	module->params.ctr_py[2] = 150;//tl200; 
 	module->params.ctr_px[3] = 255;
 	module->params.ctr_py[3] = 255;//255
	module->params.gray_num = 256;
	module->params.clah_lg = 0;//最低灰度值
	module->params.clah_hg = 255;//最高灰度值
	module->params.clah_wblock_num = params.clah_wblock_num;
	module->params.clah_hblock_num = params.clah_hblock_num;
	module->params.clip_limit = params.clip_limit;

	//memcpy(&module->params,params,sizeof(Enhance_Distribute_Params));

	CInterpolation *inter;
	inter = new CInterpolation(module->params.ctr_px,module->params.ctr_py,module->params.ctr_num);
	module->interpolation = inter;

	create_find_table(inter,module->find_table);

}


static void create_find_table(CInterpolation *inter,unsigned char *find_table)
{
	double x[256];
	double y[256];
	int t;
	int i;
	inter->GetInterpolationPts(256,x,y);
	for (i=0; i<256; i++)
	{
		t = y[i] + 0.5;
		if (t>255)
			t = 255;
		if (t<0)
			t = 0;
		find_table[i] = t;

		//CString strInfo;
		//strInfo.Format(_T("%d\n"),  t);
		//OutputDebugString(strInfo);
	}
}
#if MODIFY
void process_enhance_distri(Enhance_Distribute_Moudle *module,unsigned char *src_data,unsigned char *rst_data,int width,int height)
#else
void process_enhance_distri(Enhance_Distribute_Moudle *module,IplImage *src,IplImage *rst)
#endif
{
	int i,j;
#if !MODIFY
	int width,height;
	unsigned char *rst_data,*src_data;
#endif
	int widthstep;
	int t;
	unsigned char *tsrc_data,*trst_data;
	unsigned char *find_table = module->find_table;
	Enhance_Distribute_Params *params = &module->params;

#if !MODIFY
	width = src->width;
	height = src->height;
	widthstep = src->widthStep;
	rst_data = (unsigned char *)rst->imageData;
	src_data = (unsigned char *)src->imageData;
#else
	widthstep = width;
#endif

	trst_data = rst_data;
	tsrc_data = src_data;

	for ( i=0; i<height; i++ )
		{
		for (j=0; j<width; j++)
		{
            int ndata1 = tsrc_data[j];
			t = find_table[ ndata1 ];
			trst_data[j] = t;

			//CString strInfo;
			//strInfo.Format(_T("%d == %d\n"), ndata1, t);
			//OutputDebugString(strInfo);
		}
		trst_data += widthstep;
		tsrc_data += widthstep;
	}
	CLAHE((unsigned char *)rst_data,width,height,params->clah_lg,params->clah_hg,params->clah_wblock_num,params->clah_hblock_num,params->gray_num,params->clip_limit);//0.005f
}
void terminate_enhance_distri(Enhance_Distribute_Moudle *module)
{
	CInterpolation *interpolate = (CInterpolation *)module->interpolation;
	delete interpolate;
	delete module;
}

void LinearSharpen_Avg(unsigned char* lpImage, int nWidth, int nHeight, float temp_num, unsigned char* SharpenImage)  
{  
	// 遍历图象的纵坐标  
	int y;  

	// 遍历图象的横坐标  
	int x;  

	double * pdGrad ;  

	// 设置模板系数  
	double a = temp_num;  
	//static double nWeight[3][3] = {{-a,-a,-a},{-a,8*a,-a},{-a,-a,-a}};  
	double nWeight[3][3] = {{-a,-a,-a},{-a,8*a,-a},{-a,-a,-a}};  

	//这个变量用来表示Laplacian算子象素值  
	double rTmp[3][3];  


	// 临时变量  
	double rGrad;//,gGrad,bGrad;  

	// 模板循环控制变量  
	int yy ;  
	int xx ;  

	pdGrad = (double *)malloc(nWidth*nHeight*sizeof(double));  

	// 初始化为0  
	memset(pdGrad, 0, nWidth*nHeight*sizeof(double));  

	for(y=1; y<nHeight-1 ; y++ )  
		for(x=1 ; x<nWidth-1 ; x++ ){  
			rGrad = 0 ;  
			// Laplacian算子需要的各点象素值  

			// 模板第一行  
			rTmp[0][0] = (lpImage[(y-1)*nWidth+x-3 ]+lpImage[(y-1)*nWidth+x-2]+lpImage[(y-1)*nWidth+x-1])/3 ;   
			rTmp[0][1] = (lpImage[(y-1)*nWidth+x   ]+lpImage[(y-1)*nWidth+x+1]+lpImage[(y-1)*nWidth+x+2])/3 ;   
			rTmp[0][2] = (lpImage[(y-1)*nWidth+x+3 ]+lpImage[(y-1)*nWidth+x+4]+lpImage[(y-1)*nWidth+x+5])/3 ;   

			// 模板第二行  

			rTmp[1][0] = (lpImage[y*nWidth+x-3 ]+lpImage[y*nWidth+x-2 ]+lpImage[y*nWidth+x-1])/3 ;   
			rTmp[1][1] = (lpImage[y*nWidth+x   ]+lpImage[y*nWidth+x+1 ]+lpImage[y*nWidth+x+2])/3 ;   
			rTmp[1][2] = (lpImage[y*nWidth+x+3 ]+lpImage[y*nWidth+x+4 ]+lpImage[y*nWidth+x+5])/3 ;   

			// 模板第三行  

			rTmp[2][0] = (lpImage[(y+1)*nWidth+x-3 ]+lpImage[(y+1)*nWidth+x-2 ]+lpImage[(y+1)*nWidth+x-1])/3 ;   
			rTmp[2][1] = (lpImage[(y+1)*nWidth+x   ]+lpImage[(y+1)*nWidth+x+1 ]+lpImage[(y+1)*nWidth+x+2])/3 ;   
			rTmp[2][2] = (lpImage[(y+1)*nWidth+x+3 ]+lpImage[(y+1)*nWidth+x+4 ]+lpImage[(y+1)*nWidth+x+5])/3 ;   

			// 计算梯度  
			for(yy=0; yy<3; yy++)  
				for(xx=0; xx<3; xx++){  
					rGrad += rTmp[yy][xx] * nWeight[yy][xx] ;  
				}  

				// 梯度值写入内存  
				*(pdGrad+y*nWidth+x)=rGrad;  
		}  

		for(y=0; y<nHeight ; y++ ){  
			for(x=0 ; x<nWidth ; x++ ){  
				int temp1 = lpImage[y*nWidth+x] + (int)pdGrad[y*nWidth+x];  

				if(temp1 > 255)  
					temp1 = 255;  
				else if(temp1 < 0)  
					temp1 = 0;  

				SharpenImage[y*nWidth+x] = (unsigned char)temp1;  
			}  
		}  

		free(pdGrad) ;  
		pdGrad = NULL   ;  
}  

void BrightnessAndContrast(unsigned char *pImgData, int nHeight, int nWidth, unsigned char *pDestData)
{
	//亮度增强
	int nTemp = 0;
	double brightness = 10;
	double contrast = 1;

	for (int i = 0; i < nHeight; i++)
	{
		for(int j = 0; j < nWidth; j++)
		{
			nTemp = pImgData[i*nWidth + j] * contrast + brightness; 
			if (nTemp < 0)
			{
				nTemp = 0;
			}
			else if (nTemp > 255)
			{
				nTemp = 255;
			}

			pDestData[i*nWidth + j] = nTemp;
		}
	}

}

int normalize(unsigned char *pImg, double saturated, int width,int height)
{
	int histogram[256];
	int hsize = 256;
	int pixelCount = height *width;

	memset(&histogram,0x00,sizeof(int)*256);

	for (int i = 0;i < height; i++) //计算差分矩阵直方图
	{
		for (int j = 0; j < width; j++)
		{
			unsigned char GrayIndex = pImg[i*width + j];
			histogram[GrayIndex] ++ ;
		}
	}

	int hmin, hmax;
	int threshold;

	if (saturated>0.0)
		threshold = (int)(pixelCount*saturated/200.0);
	else
		threshold = 0;
	int i = -1;
	boolean found = false;
	int count = 0;
	int maxindex = hsize-1;
	do {
		i++;
		count += histogram[i];
		found = count>threshold;
	} while (!found && i<maxindex);
	hmin = i;

	i = hsize;
	count = 0;
	do {
		i--;
		count += histogram[i];
		found = count>threshold;
	} while (!found && i>0);
	hmax = i;

	double histMin = 0;
	double histMax = 255;
	double binSize = (histMax-histMin)/ hsize;
	double min = histMin + hmin * binSize;
	double max = histMin + hmax * binSize;

	int min2 = 0;
	int max2 = 255;
	int range = 256;
	int lut[256]={0}; // = new int[range];
	for (int i=0; i<range; i++) {
		if (i<=min)
			lut[i] = 0;
		else if (i>=max)
			lut[i] = max2;
		else
			lut[i] = (int)(((double)(i-min)/(max-min))*max2);
	}

	for (int i = 0; i < height; i++)
	{
		for(int j = 0; j < width; j++)
		{
			pImg[i*width + j] = lut[pImg[i*width + j]];
		}
	}

	return 1;
}


double getWeightedValue(int *histogram, int i) 
{
	int h = histogram[i];
	if (h<2 ) return (double)h;
	return sqrt((double)(h));
}

int equalize(unsigned char *pImg, int width,int height)
{
	int histogram[256];
	int hsize = 256;
	int pixelCount = height *width;

	memset(&histogram,0x00,sizeof(int)*256);

	for (int i = 0;i < height; i++) //计算差分矩阵直方图
	{
		for (int j = 0; j < width; j++)
		{
			unsigned char GrayIndex = pImg[i*width + j];
			histogram[GrayIndex] ++ ;
		}
	}

	int max = 255;
	int range = 255;

	double sum;
	sum = getWeightedValue(histogram, 0);
	for (int i=1; i<max; i++)
		sum += 2 * getWeightedValue(histogram, i);
	sum += getWeightedValue(histogram, max);
	double scale = range/sum;
	int lut[256]={0};
	sum = getWeightedValue(histogram, 0);
	for (int i=1; i<max; i++) {
		double delta = getWeightedValue(histogram, i);
		sum += delta;
		lut[i] = (int)(sum*scale);
		sum += delta;
	}

	lut[max] = max;

	for (int i = 0; i < height; i++)
	{
		for(int j = 0; j < width; j++)
		{
			pImg[i*width + j] = lut[pImg[i*width + j]];
		}
	}

	return 1;
}

int Imglut[5][256];

void EnhanceImg(unsigned char *pImg, int width,int height, int nCamIndex)
{
	for (int i = 0; i < height; i++)
	{
		for(int j = 0; j < width; j++)
		{
			pImg[i*width + j] = Imglut[nCamIndex][pImg[i*width + j]];
		}
	}
}

void createlut(unsigned char *pImg, double saturated, int width,int height, int flag, int nCamIndex)
{
	for (int i=0; i<256; i++)
	{
		Imglut[nCamIndex][i] = i;
	}

	if (flag == 2)
	{

		int histogram[256];
		int hsize = 256;
		int pixelCount = height *width;

		memset(&histogram,0x00,sizeof(int)*256);

		for (int i = 0;i < height; i++) //计算差分矩阵直方图
		{
			for (int j = 0; j < width; j++)
			{
				unsigned char GrayIndex = pImg[i*width + j];
				histogram[GrayIndex] ++ ;
			}
		}

		int max = 255;
		int range = 255;

		double sum;
		sum = getWeightedValue(histogram, 0);
		for (int i=1; i<max; i++)
			sum += 2 * getWeightedValue(histogram, i);
		sum += getWeightedValue(histogram, max);
		double scale = range/sum;
		//int lut[256]={0};
		sum = getWeightedValue(histogram, 0);
		for (int i=1; i<max; i++) {
			double delta = getWeightedValue(histogram, i);
			sum += delta;
			Imglut[nCamIndex][i] = (int)(sum*scale);
			sum += delta;
		}

		Imglut[nCamIndex][max] = max;
	}
	else if(flag == 3)
	{
		int histogram[256];
		int hsize = 256;
		int pixelCount = height *width;

		memset(&histogram,0x00,sizeof(int)*256);

		for (int i = 0;i < height; i++) //计算差分矩阵直方图
		{
			for (int j = 0; j < width; j++)
			{
				unsigned char GrayIndex = pImg[i*width + j];
				histogram[GrayIndex] ++ ;
			}
		}

		int hmin, hmax;
		int threshold;

		if (saturated>0.0)
			threshold = (int)(pixelCount*saturated/200.0);
		else
			threshold = 0;
		int i = -1;
		boolean found = false;
		int count = 0;
		int maxindex = hsize-1;
		do {
			i++;
			count += histogram[i];
			found = count>threshold;
		} while (!found && i<maxindex);
		hmin = i;

		i = hsize;
		count = 0;
		do {
			i--;
			count += histogram[i];
			found = count>threshold;
		} while (!found && i>0);
		hmax = i;

		double histMin = 0;
		double histMax = 255;
		double binSize = (histMax-histMin)/ hsize;
		double min = histMin + hmin * binSize;
		double max = histMin + hmax * binSize;

		int min2 = 0;
		int max2 = 255;
		int range = 256;
		//int lut[256]={0}; // = new int[range];
		for (int i=0; i<range; i++) {
			if (i<=min)
				Imglut[nCamIndex][i] = 0;
			else if (i>=max)
				Imglut[nCamIndex][i] = max2;
			else
				Imglut[nCamIndex][i] = (int)(((double)(i-min)/(max-min))*max2);
		}
	}
}

#define BLUR_MORE               1
#define FIND_EDGES              2
#define MEDIAN_FILTER           3
#define MIN                     4
#define MAX                     5

int findMedian (int values[], int nIndex) {
	//Finds the 5th largest of 9 values

	for (int i = 1; i <= nIndex/2; i++) {
		int max = 0;
		int mj = 1;
		for (int j = 1; j <= nIndex; j++)
			if (values[j] > max) {
				max = values[j];
				mj = j;
			}
			values[mj] = 0;
	}
	int max = 0;
	for (int j = 1; j <= nIndex; j++)
		if (values[j] > max)
			max = values[j];
	return max;
}

int getEdgePixel(byte pixels2[], int x, int y, int width,int height) 
{
	if (x<=0) x = 0;
	if (x>=width) x = width-1;
	if (y<=0) y = 0;
	if (y>=height) y = height-1;
	return pixels2[x+y*width]&255;
}

void filterEdge(int type, byte *pixels, byte *pixels2, int n, int x, int y, int xinc, int yinc, int width,int height, int loopnum=1) 
{
	int p1, p2, p3, p4, p5, p6, p7, p8, p9;
	int sum=0, sum1, sum2;
	int tempx = x;
	int tempy = y;


	for (int j=0; j<loopnum; j++)
	{
		for (int i=0; i<n; i++)
		{
			p1=getEdgePixel(pixels2,x-1,y-1,width,height); p2=getEdgePixel(pixels2,x,y-1,width,height); p3=getEdgePixel(pixels2,x+1,y-1,width,height);
			p4=getEdgePixel(pixels2,x-1,y,width,height); p5=getEdgePixel(pixels2,x,y,width,height); p6=getEdgePixel(pixels2,x+1,y,width,height);
			p7=getEdgePixel(pixels2,x-1,y+1,width,height); p8=getEdgePixel(pixels2,x,y+1,width,height); p9=getEdgePixel(pixels2,x+1,y+1,width,height);

			switch (type) {
			case BLUR_MORE:
				sum = (p1+p2+p3+p4+p5+p6+p7+p8+p9+4)/9;
				break;
			case FIND_EDGES: // 3x3 Sobel filter
				sum1 = p1 + 2*p2 + p3 - p7 - 2*p8 - p9;
				sum2 = p1  + 2*p4 + p7 - p3 - 2*p6 - p9;
				sum = (int)sqrt(double(sum1*sum1 + sum2*sum2));
				if (sum> 255) sum = 255;
				break;
			case MIN:
				sum = p5;
				if (p1<sum) sum = p1;
				if (p2<sum) sum = p2;
				if (p3<sum) sum = p3;
				if (p4<sum) sum = p4;
				if (p6<sum) sum = p6;
				if (p7<sum) sum = p7;
				if (p8<sum) sum = p8;
				if (p9<sum) sum = p9;
				break;
			case MAX:
				sum = p5;
				if (p1>sum) sum = p1;
				if (p2>sum) sum = p2;
				if (p3>sum) sum = p3;
				if (p4>sum) sum = p4;
				if (p6>sum) sum = p6;
				if (p7>sum) sum = p7;
				if (p8>sum) sum = p8;
				if (p9>sum) sum = p9;
				break;
			}

			pixels[x+y*width] = (byte)sum;
			x+=xinc; y+=yinc;
		}

		if (xinc == 1)
		{
			x = tempx;
			y = ++tempy;
		}

		if (yinc == 1)
		{
			x = ++tempx;
			y = tempy;
		}
	}
}

void filter3x3(unsigned char *pixels, int type, int width,int height) 
{
	if (width <=3 || height <=3)
	{
		return;
	}

	int p1, p2, p3, p4, p5, p6, p7, p8, p9;

	byte *pixels2 = new byte[width * height];
	memset(pixels2, 0, width * height);
	memcpy(pixels2, pixels, width * height);

	int yMin = 1;
	int yMax = height - 2;
	int xMin = 1;
	int xMax = width - 2;

	int offset, sum1, sum2=0, sum=0;
	int values[10] = {0};
	int rowOffset = width;
	//int count;//delete by zhuxy20190319 未引用的局部变量
	for (int y=yMin; y<=yMax; y++) {
		offset = xMin + y * width;
		p2 = pixels2[offset-rowOffset-1]&0xff;
		p3 = pixels2[offset-rowOffset]&0xff;
		p5 = pixels2[offset-1]&0xff;
		p6 = pixels2[offset]&0xff;
		p8 = pixels2[offset+rowOffset-1]&0xff;
		p9 = pixels2[offset+rowOffset]&0xff;

		for (int x=xMin; x<=xMax; x++) {
			p1 = p2; p2 = p3;
			p3 = pixels2[offset-rowOffset+1]&0xff;
			p4 = p5; p5 = p6;
			p6 = pixels2[offset+1]&0xff;
			p7 = p8; p8 = p9;
			p9 = pixels2[offset+rowOffset+1]&0xff;

			switch (type) {
			case BLUR_MORE:
				sum = (p1+p2+p3+p4+p5+p6+p7+p8+p9+4)/9;
				break;
			case FIND_EDGES: // 3x3 Sobel filter
				{
					sum1 = p1 + 2*p2 + p3 - p7 - 2*p8 - p9;
					sum2 = p1  + 2*p4 + p7 - p3 - 2*p6 - p9;
					sum = (int)sqrt((double)(sum1*sum1 + sum2*sum2));
					if (sum> 255) sum = 255;
				}

				break;
			case MEDIAN_FILTER:
				values[1]=p1; values[2]=p2; values[3]=p3; values[4]=p4; values[5]=p5;
				values[6]=p6; values[7]=p7; values[8]=p8; values[9]=p9;
				sum = findMedian(values,9);
				break;
			case MIN:
				sum = p5;
				if (p1<sum) sum = p1;
				if (p2<sum) sum = p2;
				if (p3<sum) sum = p3;
				if (p4<sum) sum = p4;
				if (p6<sum) sum = p6;
				if (p7<sum) sum = p7;
				if (p8<sum) sum = p8;
				if (p9<sum) sum = p9;
				break;
			case MAX:
				sum = p5;
				if (p1>sum) sum = p1;
				if (p2>sum) sum = p2;
				if (p3>sum) sum = p3;
				if (p4>sum) sum = p4;
				if (p6>sum) sum = p6;
				if (p7>sum) sum = p7;
				if (p8>sum) sum = p8;
				if (p9>sum) sum = p9;
				break;
			}

			pixels[offset++] = (byte)sum;
		}
	}

	filterEdge(type, pixels, pixels2, height, 0, 0, 0, 1, width, height);
	filterEdge(type, pixels, pixels2, width, 0, 0, 1, 0, width, height);
	filterEdge(type, pixels, pixels2, height, width-1, 0, 0, 1, width, height);
	filterEdge(type, pixels, pixels2, width, 0, height-1, 1, 0, width, height);

	delete [] pixels2;
}

void filter5x5(unsigned char *pixels, int type, int width,int height) 
{
	//int p1, p2, p3, p4, p5, p6, p7, p8, p9;
	int p[25];

	byte *pixels2 = new byte[width * height];
	memset(pixels2, 0, width * height);
	memcpy(pixels2, pixels, width * height);

	int yMin = 2;
	int yMax = height - 3;
	int xMin = 2;
	int xMax = width - 3;

	int offset,sum1, sum2=0, sum=0;
	int values[26] = {0};
	int rowOffset = width;
	//int count;//delete by zhuxy20190319 未引用的局部变量
	for (int y=yMin; y<=yMax; y++) {
		offset = xMin + y * width;

		p[1] = pixels2[offset-rowOffset*2-2]&0xff;
		p[2] = pixels2[offset-rowOffset*2-1]&0xff;
		p[3] = pixels2[offset-rowOffset*2]&0xff;
		p[4] = pixels2[offset-rowOffset*2+1]&0xff;

		p[6] = pixels2[offset-rowOffset-2]&0xff;
		p[7] = pixels2[offset-rowOffset-1]&0xff;
		p[8] = pixels2[offset-rowOffset]&0xff;
		p[9] = pixels2[offset-rowOffset+1]&0xff;

		p[11] = pixels2[offset-2]&0xff;
		p[12] = pixels2[offset-1]&0xff;
		p[13] = pixels2[offset]&0xff;
		p[14] = pixels2[offset+1]&0xff;

		p[16] = pixels2[offset+rowOffset-2]&0xff;
		p[17] = pixels2[offset+rowOffset-1]&0xff;
		p[18] = pixels2[offset+rowOffset]&0xff;
		p[19] = pixels2[offset+rowOffset+1]&0xff;

		p[21] = pixels2[offset+rowOffset*2-2]&0xff;
		p[22] = pixels2[offset+rowOffset*2-1]&0xff;
		p[23] = pixels2[offset+rowOffset*2]&0xff;
		p[24] = pixels2[offset+rowOffset*2+1]&0xff;

		for (int x=xMin; x<=xMax; x++) {

			p[0] = p[1]; p[1] = p[2]; p[2] = p[3]; p[3]=p[4];
			p[4] = pixels2[offset-rowOffset*2+2]&0xff;

			p[5] = p[6]; p[6] = p[7]; p[7] = p[8]; p[8]=p[9];
			p[9] = pixels2[offset-rowOffset+2]&0xff;

			p[10] = p[11]; p[11] = p[12]; p[12] = p[13]; p[13]=p[14];
			p[14] = pixels2[offset+2]&0xff;

			p[15] = p[16]; p[16] = p[17]; p[17] = p[18]; p[18]=p[19];
			p[19] = pixels2[offset+rowOffset+2]&0xff;

			p[20] = p[21]; p[21] = p[22]; p[22] = p[23]; p[23]=p[24];
			p[24] = pixels2[offset+rowOffset*2+2]&0xff;

			switch (type) {
			case BLUR_MORE:
				{
					sum1 = 0;
					for (int i = 0; i < 25; i++)
					{
						sum1 += p[i]; 
					}

					sum = (sum1+12)/25;
				}	
				break;
			case FIND_EDGES: // 3x3 Sobel filter
				{
					//sum1 = p1 + 2*p2 + p3 - p7 - 2*p8 - p9;
					//sum2 = p1  + 2*p4 + p7 - p3 - 2*p6 - p9;
					//sum = (int)sqrt((double)(sum1*sum1 + sum2*sum2));
					//if (sum> 255) sum = 255;
				}

				break;
			case MEDIAN_FILTER:
				{
					for (int i=0; i<25; i++)
					{
						values[i]=p[i];
					}

					sum = findMedian(values,25);
				}
				break;
			case MIN:
				{
					sum = 255;
					for (int i = 0; i < 25; i++)
					{
						if (sum > p[i])
						{
							sum = p[i];
						}
					}
				}

				break;
			case MAX:
				{
					sum = 0;
					for (int i = 0; i < 25; i++)
					{
						if (sum < p[i])
						{
							sum = p[i];
						}
					}
				}

				break;
			}

			pixels[offset++] = (byte)sum;
		}
	}

	filterEdge(type, pixels, pixels2, height, 0, 0, 0, 1, width, height, 2);
	filterEdge(type, pixels, pixels2, width, 0, 0, 1, 0, width, height, 2);
	filterEdge(type, pixels, pixels2, height, width-2, 0, 0, 1, width, height, 2);
	filterEdge(type, pixels, pixels2, width, 0, height-2, 1, 0, width, height, 2);

	delete [] pixels2;
}

bool SaveJpgImg(wchar_t *strSavePath, int nWidth, int nHeight, BYTE *imgDataBuf)
{
	FILE *fp = fopen(wstring2string(strSavePath).c_str(), "wb");
	if (fp==NULL) 
	{
		return false;
	}

	struct jpeg_compress_struct jcs;
	struct jpeg_error_mgr jem;

	jcs.err = jpeg_std_error(&jem);
	jpeg_create_compress(&jcs);
	jpeg_stdio_dest(&jcs, fp);

	jcs.image_width = nWidth; 			
	jcs.image_height = nHeight;
	jcs.input_components = 1;			 
	jcs.in_color_space = JCS_GRAYSCALE; 

	jpeg_set_defaults(&jcs);	
	jpeg_set_quality (&jcs, 50, true);
	jpeg_start_compress(&jcs, TRUE);

	JSAMPROW row_pointer[1];			
	int row_stride = jcs.image_width;	

	while (jcs.next_scanline < jcs.image_height) 
	{
		row_pointer[0] = &imgDataBuf[jcs.next_scanline * row_stride];
		jpeg_write_scanlines(&jcs, row_pointer, 1);
	}

	jpeg_finish_compress(&jcs);
	jpeg_destroy_compress(&jcs);
	fclose(fp);

	return  true;
}

//bool GetJpgCompressData(int nWidth, int nHeight, unsigned char  *imgDataBuf, unsigned char **outbuffer, size_t *outsize)
//{
//	try
//	{
//		struct jpeg_compress_struct jcs;
//		struct jpeg_error_mgr jem;
//		jcs.err=jpeg_std_error(&jem);
//		jpeg_create_compress(&jcs);
//
//		//使用jpeg_mem_dest动态分配内存，压缩后返回长度,jpeglib版
//		//网传jpeglib-turbo版需要自己申请压缩缓冲，实测不用
//
//		size_t dwNewAndSendLen=nWidth * nHeight;
//		char *pCompressBuffer=NULL;
//
//		jpeg_mem_dest(&jcs,(unsigned char **)&pCompressBuffer,&dwNewAndSendLen);
//
//		jcs.image_width=nWidth;
//		jcs.image_height=nHeight;
//		jcs.in_color_space=JCS_GRAYSCALE;
//		jcs.input_components=1;
//		jpeg_set_defaults(&jcs);
//		jpeg_set_quality(&jcs,70,TRUE);
//
//		JSAMPROW *rowPointer = new JSAMPROW[jcs.image_height];//压缩源数据
//		for (int dy = 0; dy < jcs.image_height; dy++)
//			rowPointer[dy] = (JSAMPROW)(&(imgDataBuf)[dy * jcs.image_width]);
//
//		jpeg_start_compress(&jcs, TRUE);
//		while (jcs.next_scanline < jcs.image_height)
//		{
//			jpeg_write_scanlines(&jcs, &rowPointer[jcs.next_scanline],jcs.image_height - jcs.next_scanline);
//		}
//		jpeg_finish_compress(&jcs);
//
//		if(NULL!=*outbuffer)
//			free(*outbuffer);
//		*outbuffer=(BYTE*)malloc(dwNewAndSendLen);
//
//		memset(*outbuffer,0,dwNewAndSendLen);
//		memcpy(*outbuffer,pCompressBuffer, dwNewAndSendLen);
//
//		free(pCompressBuffer);
//
//		jpeg_destroy_compress(&jcs);
//
//		*outsize = dwNewAndSendLen;
//	}
//	catch (...)
//	{
//		return false;
//	}
//	
//	return true;
//}

bool GetJpgCompressData(int nWidth, int nHeight, unsigned char  *imgDataBuf, unsigned char **outbuffer, size_t *outsize, int nImgQuality)
{
	try
	{
		struct jpeg_compress_struct jcs;
		struct jpeg_error_mgr jem;

		jcs.err = jpeg_std_error(&jem);
		jpeg_create_compress(&jcs);

		*outsize = nWidth * nHeight;
		jpeg_mem_dest(&jcs, outbuffer, outsize);

		jcs.image_width = nWidth; 				
		jcs.image_height = nHeight;
		jcs.input_components = 1;			 
		jcs.in_color_space = JCS_GRAYSCALE; 

		jpeg_set_defaults(&jcs);	
		jpeg_set_quality (&jcs, nImgQuality, true);
		jpeg_start_compress(&jcs, TRUE);

		JSAMPROW row_pointer[1];			
		int row_stride = jcs.image_width;	

		while (jcs.next_scanline < jcs.image_height) 
		{
			row_pointer[0] = &imgDataBuf[jcs.next_scanline * row_stride];
			jpeg_write_scanlines(&jcs, row_pointer, 1);
		}

		jpeg_finish_compress(&jcs);
		jpeg_destroy_compress(&jcs);
	}
	catch(...)
	{
		return false;
	}

	return  true;
}

void flipHorz(BYTE *data, int columns, int rows)
{
	register int x;
	register int y;
	register BYTE *p;
	register BYTE *q;
	register unsigned short t;

	BYTE *r = data;
	for (y = columns; y != 0; --y)
	{
		p = r;
		r += rows;
		q = r;
		for (x = rows / 2; x != 0; --x)
		{
			t = *p;
			*p++ = *--q;
			*q = t;
		}
	}
}
