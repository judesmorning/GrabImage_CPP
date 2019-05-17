#ifndef _CLAH_H_
#define _CLAH_H_

#define BYTE_IMAGE 

#ifdef BYTE_IMAGE
typedef unsigned char kz_pixel_t;	 /* for 8 bit-per-pixel images */
#define uiNR_OF_GREY (256)
#else
typedef unsigned short kz_pixel_t;	 /* for 12 bit-per-pixel images (default) */
# define uiNR_OF_GREY (4096)
#endif


int CLAHE(kz_pixel_t* pImage, unsigned int uiXRes, unsigned int uiYRes, kz_pixel_t Min,
		  kz_pixel_t Max, unsigned int uiNrX, unsigned int uiNrY,
		  unsigned int uiNrBins, float fCliplimit);

#endif