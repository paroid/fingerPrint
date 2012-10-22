/*#############################################################################
 * 文件名：img_base.cpp
 * 功能：  一些基本的图像操作
#############################################################################*/


#include "img_base.h"

#include "histogram.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>


/******************************************************************************
  * 功能：图像二值化
  * 参数：image       指纹图像
  *       size        阈值
  * 返回：错误编号
******************************************************************************/
FvsError_t ImageBinarize(FvsImage_t image, const FvsByte_t limit) {
    FvsInt_t n;
    FvsByte_t *pimg = ImageGetBuffer(image);
    FvsInt_t size = ImageGetSize(image);
    if (pimg == NULL)
        return FvsMemory;
    /* 循环遍历 */
    for (n = 0; n < size; n++, pimg++) {
        /* 阈值化 */
        *pimg = (*pimg < limit) ? (FvsByte_t)0xFF : (FvsByte_t)0x00;
    }
    return ImageSetFlag(image, FvsImageBinarized);
}

/******************************************************************************
  * 功能：图像二值化
  * 参数：ridgeimage  原指纹图像输入，二值化脊图像输出
  * 参数：valleyimage 二值化谷图像
  * 参数：highsize        阈值
  * 参数：lowsize        阈值
  * 返回：错误编号
******************************************************************************/
FvsError_t MyImageBinarize(FvsImage_t ridgeimage, FvsImage_t valleyimage, FvsByte_t highsize, FvsByte_t lowsize) {
    FvsInt_t n;
    FvsByte_t *ridge_pimg = ImageGetBuffer(ridgeimage);
    FvsByte_t *valley_pimg = ImageGetBuffer(valleyimage);
    FvsInt_t size = ImageGetSize(ridgeimage);
    if (ridge_pimg == NULL || valley_pimg == NULL)
        return FvsMemory;
    /* 循环遍历 */
    for (n = 0; n < size; n++, ridge_pimg++, valley_pimg++) {
        /* 阈值化 */
        *ridge_pimg = (*ridge_pimg < lowsize) ? (FvsByte_t)0x00 : (FvsByte_t)0xFF;
        *valley_pimg = (*ridge_pimg > highsize) ? (FvsByte_t)0x00 : (FvsByte_t)0xFF;
    }
    ImageSetFlag(ridgeimage, FvsImageBinarized);
    ImageSetFlag(valleyimage, FvsImageBinarized);
    return FvsOK;
}


/******************************************************************************
  * 功能：图像翻转操作
  * 参数：image       指纹图像
  * 返回：错误编号
******************************************************************************/
FvsError_t ImageInvert(FvsImage_t image) {
    FvsByte_t* pimg = ImageGetBuffer(image);
    FvsInt_t size = ImageGetSize(image);
    FvsInt_t n;
    if (pimg == NULL)
        return FvsMemory;
    for (n = 0; n < size; n++, pimg++) {
        *pimg = 0xFF - *pimg;
    }
    return FvsOK;
}


/******************************************************************************
  * 功能：图像合并操作
  * 参数：image1    第一个指纹图像，用于保存结果
  *       image2    第二个指纹图像
  * 返回：错误编号
******************************************************************************/
FvsError_t ImageAverage(FvsImage_t image1, const FvsImage_t image2) {
    FvsByte_t* p1 = ImageGetBuffer(image1);
    FvsByte_t* p2 = ImageGetBuffer(image2);
    FvsInt_t size1 = ImageGetSize(image1);
    FvsInt_t size2 = ImageGetSize(image2);
    FvsInt_t i;
    if (p1 == NULL || p2 == NULL)
        return FvsMemory;
    if (size1 != size2)
        return FvsBadParameter;
    for (i = 0; i < size1; i++, p1++) {
        *p1 = (*p1 + *p2++) >> 1;
    }
    return FvsOK;
}



/******************************************************************************
  * 功能：图像逻辑合并操作
  * 参数：image1    第一个指纹图像，用于保存结果
  *       image2    第二个指纹图像
  * 返回：错误编号
******************************************************************************/
FvsError_t ImageLogical
(
    FvsImage_t image1,
    const FvsImage_t image2,
    const FvsLogical_t operation
) {
    FvsByte_t* p1 = ImageGetBuffer(image1);
    FvsByte_t* p2 = ImageGetBuffer(image2);
    FvsInt_t size1 = ImageGetSize(image1);
    FvsInt_t i;
    if (p1 == NULL || p2 == NULL)
        return FvsMemory;
    if (ImageCompareSize(image1, image2) == FvsFalse)
        return FvsBadParameter;
    switch (operation) {
        case FvsLogicalOr:
            for (i = 0; i < size1; i++, p1++)
                *p1 = (*p1) | (*p2++);
            break;
        case FvsLogicalAnd:
            for (i = 0; i < size1; i++, p1++)
                *p1 = (*p1) & (*p2++);
            break;
        case FvsLogicalXor:
            for (i = 0; i < size1; i++, p1++)
                *p1 = (*p1) ^ (*p2++);
            break;
        case FvsLogicalNAnd:
            for (i = 0; i < size1; i++, p1++)
                *p1 = ~((*p1) & (*p2++));
            break;
        case FvsLogicalNOr:
            for (i = 0; i < size1; i++, p1++)
                *p1 = ~((*p1) | (*p2++));
            break;
        case FvsLogicalNXor:
            for (i = 0; i < size1; i++, p1++)
                *p1 = ~((*p1) ^ (*p2++));
            break;
    }
    return FvsOK;
}


/******************************************************************************
  * 功能：图像合并操作
  *       使用了模计算，0和255的结果是0而不是上一个函数的127。
  * 参数：image1    第一个指纹图像，用于保存结果
  *       image2    第二个指纹图像
  * 返回：错误编号
******************************************************************************/
FvsError_t ImageAverageModulo(FvsImage_t image1, const FvsImage_t image2) {
    FvsByte_t* p1 = ImageGetBuffer(image1);
    FvsByte_t* p2 = ImageGetBuffer(image2);
    FvsInt_t size1 = ImageGetSize(image1);
    FvsInt_t size2 = ImageGetSize(image2);
    FvsInt_t i;
    FvsByte_t v1, v2;
    if (size1 != size2)
        return FvsBadParameter;
    if (p1 == NULL || p2 == NULL)
        return FvsMemory;
    for (i = 0; i < size1; i++) {
        v1 = *p1;
        v2 = *p2;
        if (v1 < 128) v1 += 256;
        if (v2 < 128) v2 += 256;
        v1 += v2;
        v1 >>= 1;
        v1 = v1 % 256;
        *p1++ = (uint8_t)v1;
    }
    return FvsOK;
}


/******************************************************************************
  * 功能：图像平移操作
  * 参数：image    指纹图像
  *       vx       X方向的平移量
  *       vy       Y方向的平移量
  * 返回：错误编号
******************************************************************************/
FvsError_t ImageTranslate(FvsImage_t image, const FvsInt_t vx, const FvsInt_t vy) {
    return FvsOK;
}


#define P(x,y)      p[((x)+(y)*pitch)]


/******************************************************************************
  * 功能：图像纹理
  * 参数：image       指纹图像
  *       horizontal  水平或垂直纹理
  * 返回：错误编号
******************************************************************************/
FvsError_t ImageStripes(FvsImage_t image, const FvsBool_t horizontal) {
    FvsByte_t* p = ImageGetBuffer(image);
    FvsInt_t w     = ImageGetWidth (image);
    FvsInt_t h     = ImageGetHeight(image);
    FvsInt_t pitch = ImageGetPitch (image);
    FvsInt_t x, y;
    if (p == NULL)
        return FvsMemory;
    if (horizontal == FvsFalse) {
        for (y = 0; y < h; y++)
            for (x = 0; x < w; x++)
                P(x, y) = (FvsByte_t)x % 256;
    }
    else {
        for (y = 0; y < h; y++)
            for (x = 0; x < w; x++)
                P(x, y) = (FvsByte_t)y % 256;
    }
    return FvsOK;
}


/******************************************************************************
  * 功能：改变图像的发光度，使其在[255..255]之间变动
  * 参数：image         指纹图像
  *       luminosity    相关的发光度
  * 返回：错误编号
******************************************************************************/
FvsError_t ImageLuminosity(FvsImage_t image, const FvsInt_t luminosity) {
    FvsByte_t* p = ImageGetBuffer(image);
    FvsInt_t  w = ImageGetWidth (image);
    FvsInt_t  h = ImageGetHeight(image);
    FvsInt_t pitch = ImageGetPitch (image);
    FvsInt_t x, y;
    FvsFloat_t fgray, a, b;
    if (p == NULL)
        return FvsMemory;
    if (luminosity > 0) {
        a = (255.0 - abs(luminosity)) / 255.0;
        b = (FvsFloat_t)luminosity;
    }
    else {
        a = (255.0 - abs(luminosity)) / 255.0;
        b = 0.0;
    }
    for (y = 0; y < h; y++)
        for (x = 0; x < w; x++) {
            fgray = (FvsFloat_t)P(x, y);
            fgray = b + a * fgray;
            if (fgray < 0.0)    fgray = 0.0;
            if (fgray > 255.0)  fgray = 255.0;
            P(x, y) = (uint8_t)fgray;
        }
    return FvsOK;
}


/******************************************************************************
  * 功能：改变图像的对比度，使其在[-127..127]变动
  * 参数：image      指纹图像
  *       contrast   对比度因子
  * 返回：错误编号
******************************************************************************/
FvsError_t ImageContrast(FvsImage_t image, const FvsInt_t contrast) {
    FvsByte_t* p = ImageGetBuffer(image);
    FvsInt_t  w = ImageGetWidth (image);
    FvsInt_t  h = ImageGetHeight(image);
    FvsInt_t pitch = ImageGetPitch (image);
    FvsInt_t x, y;
    FvsFloat_t fgray, a, b;
    if (p == NULL)
        return FvsMemory;
    a = (FvsFloat_t)((127.0 + contrast) / 127.0);
    b = (FvsFloat_t)(-contrast);
    for (y = 0; y < h; y++)
        for (x = 0; x < w; x++) {
            fgray = (FvsFloat_t)P(x, y);
            fgray = b + a * fgray;
            if (fgray < 0.0)    fgray = 0.0;
            if (fgray > 255.0)  fgray = 255.0;
            P(x, y) = (uint8_t)fgray;
        }
    return FvsOK;
}


/******************************************************************************
  * 功能：图像软化操作，通过计算均值实现
  * 参数：image     指纹图像
  *       size      软化窗口大小
  * 返回：错误编号
******************************************************************************/
FvsError_t ImageSoftenMean(FvsImage_t image, const FvsInt_t size) {
    FvsByte_t* p1  = ImageGetBuffer(image);
    FvsByte_t* p2;
    FvsInt_t   w   = ImageGetWidth (image);
    FvsInt_t   h   = ImageGetHeight(image);
    FvsInt_t pitch = ImageGetPitch (image);
    FvsInt_t pitch2;
    FvsInt_t x, y, s, p, q, a, c;
    FvsImage_t im2;
    im2 = ImageCreate();
    if (im2 == NULL || p1 == NULL)
        return FvsMemory;
    s = size >> 1;		/* 大小 */
    a = size * size;	/* 面积 */
    if (a == 0)
        return FvsBadParameter;
    /* 拷贝图像进行计算 */
    ImageCopy(im2, image);
    p2 = ImageGetBuffer(im2);
    if (p2 == NULL) {
        ImageDestroy(im2);
        return FvsMemory;
    }
    pitch2 = ImageGetPitch (im2);
    for (y = s; y < h - s; y++)
        for (x = s; x < w - s; x++) {
            c = 0;
            for (q = -s; q <= s; q++)
                for (p = -s; p <= s; p++) {
                    c += p2[(x + p) + (y + q) * pitch2];
                }
            p1[x + y * pitch] = c / a;
        }
    ImageDestroy(im2);
    return FvsOK;
}


/******************************************************************************
  * 功能：图像归一化操作，使其具有给定的均值和方差
  * 参数：image     指纹图像
  *       mean      给定的均值
  *       variance  给定的标准方差
  * 返回：错误编号
******************************************************************************/
FvsError_t ImageNormalize(FvsImage_t image, const FvsByte_t mean, const FvsUint_t variance) {
    FvsByte_t* p = ImageGetBuffer(image);
    FvsInt_t   w = ImageGetWidth (image);
    FvsInt_t   h = ImageGetHeight(image);
    FvsInt_t   pitch = ImageGetPitch (image);
    FvsInt_t   x, y;
    FvsFloat_t fmean, fsigma, fmean0, fsigma0, fgray;
    FvsFloat_t fcoeff = 0.0;
    FvsHistogram_t histogram = NULL;
    FvsError_t nRet;
    if (p == NULL)
        return FvsMemory;
    histogram = HistogramCreate();
    if (histogram != NULL) {
        /* 计算直方图 */
        nRet = HistogramCompute(histogram, image);
        if (nRet == FvsOK) {
            /* 计算方差和均值 */
            fmean   = (FvsFloat_t)HistogramGetMean(histogram);
            fsigma  = sqrt((FvsFloat_t)HistogramGetVariance(histogram));
            fmean0  = (FvsFloat_t)mean;
            fsigma0 = sqrt((FvsFloat_t)variance);
            if (fsigma > 0.0)
                fcoeff = fsigma0 / fsigma;
            for (y = 0; y < h; y++)
                for (x = 0; x < w; x++) {
                    fgray = (FvsFloat_t)P(x, y);
                    fgray = fmean0 + fcoeff * (fgray - mean);
                    if (fgray < 0.0)    fgray = 0.0;
                    if (fgray > 255.0)  fgray = 255.0;
                    P(x, y) = (uint8_t)fgray;
                }
        }
        HistogramDestroy(histogram);
    }
    return nRet;
}



