
/*#############################################################################
 * 文件名：histogram.cpp
 * 功能：  实现了指纹直方图的操作
#############################################################################*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "histogram.h"

/* 直方图可以快速计算位图的一些信息，比如均值，方差等 */
typedef struct iFvsHistogram_t {
    FvsUint_t       ptable[256];    /* 8位图像的直方图 */
    FvsInt_t        ncount;         /* 直方图中的点数 */
    FvsInt_t        nmean;          /* -1 = 还没有计算 */
    FvsInt_t        nvariance;      /* -1 = 还没有计算 */
} iFvsHistogram_t;


/******************************************************************************
  * 功能：创建一个新的直方图对象
  * 参数：无
  * 返回：失败返回空，否则返回直方图对象
******************************************************************************/
FvsHistogram_t HistogramCreate() {
    iFvsHistogram_t* p = NULL;
    p = (iFvsHistogram_t *)(FvsHistogram_t)malloc(sizeof(iFvsHistogram_t));
    if (p != NULL) {
        /* 重置表 */
        HistogramReset(p);
    }
    return (FvsHistogram_t)p;
}


/******************************************************************************
  * 功能：破坏一个存在的直方图对象
  * 参数：histogram 直方图对象指针
  * 返回：错误编号
******************************************************************************/
void HistogramDestroy(FvsHistogram_t histogram) {
    iFvsHistogram_t* p = NULL;
    if (histogram == NULL)
        return;
    p = (iFvsHistogram_t *)histogram;
    free(p);
}


/******************************************************************************
  * 功能：重置一个存在的直方图对象为0
  * 参数：histogram 直方图对象指针
  * 返回：错误编号
******************************************************************************/
FvsError_t HistogramReset(FvsHistogram_t hist) {
    iFvsHistogram_t* histogram = (iFvsHistogram_t*)hist;
    int i;
    for (i = 0; i < 256; i++)
        histogram->ptable[i] = 0;
    histogram->ncount    = 0;
    histogram->nmean     = -1;
    histogram->nvariance = -1;
    return FvsOK;
}


/******************************************************************************
  * 功能：计算一个8-bit图像的直方图
  * 参数：histogram 直方图对象指针
  *       image     图像指针
  * 返回：错误编号
******************************************************************************/
FvsError_t HistogramCompute(FvsHistogram_t hist, const FvsImage_t image) {
    iFvsHistogram_t* histogram = (iFvsHistogram_t*)hist;
    FvsError_t nRet = FvsOK;
    FvsInt_t w      = ImageGetWidth(image);
    FvsInt_t h      = ImageGetHeight(image);
    FvsInt_t pitch  = ImageGetPitch(image);
    uint8_t* p      = ImageGetBuffer(image);
    FvsInt_t x, y;
    if (histogram == NULL || p == NULL)
        return FvsMemory;
    /* 首先重置直方图 */
    nRet = HistogramReset(hist);
    /* 计算 */
    if (nRet == FvsOK) {
        FvsInt_t pos;
        for (y = 0; y < h; y++) {
            pos = pitch * y;
            for (x = 0; x < w; x++) {
                histogram->ptable[p[pos++]]++;
            }
        }
        histogram->ncount = w * h;
    }
    return nRet;
}


/******************************************************************************
  * 功能：计算一个直方图对象的均值
  * 参数：histogram 直方图对象指针
  * 返回：均值
******************************************************************************/
FvsByte_t HistogramGetMean(const FvsHistogram_t hist) {
    iFvsHistogram_t* histogram = (iFvsHistogram_t*)hist;
    FvsInt_t val, i;
    val = histogram->nmean;
    if (val == -1) {
        val = 0;
        for (i = 1; i < 255; i++)
            val += i * histogram->ptable[i];
        i = histogram->ncount;
        if (i > 0)
            val = val / i;
        else
            val = 0;
        histogram->nmean = val;
    }
    return (uint8_t)val;
}


/******************************************************************************
  * 功能：计算一个直方图对象的方差
  * 参数：histogram 直方图对象指针
  * 返回：方差
******************************************************************************/
FvsUint_t HistogramGetVariance(const FvsHistogram_t hist) {
    iFvsHistogram_t* histogram = (iFvsHistogram_t*)hist;
    FvsInt_t val;
    FvsInt_t i;
    uint8_t mean;
    val = histogram->nvariance;
    if (val == -1) {
        /* 计算均值 */
        mean = HistogramGetMean(hist);
        val  = 0;
        for (i = 0; i < 255; i++)
            val += histogram->ptable[i] * (i - mean) * (i - mean);
        i = histogram->ncount;
        if (i > 0)
            val = val / i;
        else
            val = 0;
        histogram->nvariance = val;
    }
    return (FvsUint_t)val;
}



