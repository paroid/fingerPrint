
/*#############################################################################
 * 文件名：image.cpp
 * 功能：  实现了指纹图像的基本操作
#############################################################################*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "image.h"




/******************************************************************************
  * 功能：创建一个新的图像对象
  * 参数：无
  * 返回：失败返回空，否则返回新的图像对象
******************************************************************************/
FvsImage_t ImageCreate() {
    iFvsImage_t* p = NULL;
    p = (iFvsImage_t *)(FvsImage_t)malloc(sizeof(iFvsImage_t));
    if (p != NULL) {
        p->h        = 0;
        p->w        = 0;
        p->pitch    = 0;
        p->pimg     = NULL;
        p->flags    = FvsImageGray; /* 缺省的标记 */
    }
    return (FvsImage_t)p;
}


/******************************************************************************
  * 功能：销毁一个图像对象
  * 参数：image  指向图像对象的指针
  * 返回：无
******************************************************************************/
void ImageDestroy(FvsImage_t image) {
    iFvsImage_t* p = NULL;
    if (image == NULL)
        return;
    (void)ImageSetSize(image, 0, 0);
    p = (iFvsImage_t *)image;
    free(p);
}


/******************************************************************************
  * 功能：设置图像标记，该操作大部分由库函数自动完成
  * 参数：image  指向图像对象的指针
  *       flag   标记
  * 返回：错误编号
******************************************************************************/
FvsError_t ImageSetFlag(FvsImage_t img, const FvsImageFlag_t flag) {
    iFvsImage_t* image = (iFvsImage_t*)img;
    image->flags = flag;
    return FvsOK;
}


/******************************************************************************
  * 功能：获得图像标记
  * 参数：image  指向图像对象的指针
  * 返回：图像标记
******************************************************************************/
FvsImageFlag_t ImageGetFlag(const FvsImage_t img) {
    iFvsImage_t* image = (iFvsImage_t*)img;
    return image->flags;
}


/******************************************************************************
  * 功能：设置一个图像对象的大小
  * 参数：image   指向图像对象的指针
  *       width   图像宽度
  *       height  图像高度
  * 返回：错误编号
******************************************************************************/
FvsError_t ImageSetSize(FvsImage_t img, const FvsInt_t width,
                        const FvsInt_t height) {
    iFvsImage_t* image = (iFvsImage_t*)img;
    FvsError_t nRet = FvsOK;
    FvsInt_t newsize = width * height;
    /* size为0的情况 */
    if (newsize == 0) {
        if (image->pimg != NULL) {
            free(image->pimg);
            image->pimg = NULL;
            image->w = 0;
            image->h = 0;
            image->pitch = 0;
        }
        return FvsOK;
    }
    if (image->h * image->w != newsize) {
        free(image->pimg);
        image->w = 0;
        image->h = 0;
        image->pitch = 0;
        /* 申请内存 */
        image->pimg = (uint8_t*)malloc((size_t)newsize);
    }
    if (image->pimg == NULL)
        nRet = FvsMemory;
    else {
        image->h = height;
        image->w = width;
        image->pitch = width;
    }
    return nRet;
}


/******************************************************************************
  * 功能：拷贝图像
  * 参数：destination  指向目标图像对象的指针
  *       source       指向源图像对象的指针
  * 返回：错误编号
******************************************************************************/
FvsError_t ImageCopy(FvsImage_t destination, const FvsImage_t source) {
    iFvsImage_t* dest = (iFvsImage_t*)destination;
    iFvsImage_t* src  = (iFvsImage_t*)source;
    FvsError_t nRet = FvsOK;
    nRet = ImageSetSize(dest, src->w, src->h);
    if (nRet == FvsOK)
        memcpy(dest->pimg, src->pimg, (size_t)src->h * src->w);
    /* 拷贝标记 */
    dest->flags = src->flags;
    return nRet;
}


/******************************************************************************
  * 功能：清空图像
  * 参数：image  指向图像对象的指针
  * 返回：错误编号
******************************************************************************/
FvsError_t ImageClear(FvsImage_t img) {
    return ImageFlood(img, 0);
}


/******************************************************************************
  * 功能：设置图像中所有象素为特定值
  * 参数：image  指向图像对象的指针
  *       value  要设定的值
  * 返回：错误编号
******************************************************************************/
FvsError_t ImageFlood(FvsImage_t img, const FvsByte_t value) {
    FvsError_t nRet = FvsOK;
    iFvsImage_t* image = (iFvsImage_t*)img;
    if (image == NULL) return FvsMemory;
    if (image->pimg != NULL)
        memset(image->pimg, (int)value, (size_t)(image->h * image->w));
    return nRet;
}


/******************************************************************************
  * 功能：设置图像中某个象素的值
  * 参数：image  指向图像对象的指针
  *       x      X轴坐标
  *       y      Y轴坐标
  *       val    要设定的值
  * 返回：无
******************************************************************************/
void ImageSetPixel(FvsImage_t img, const FvsInt_t x, const FvsInt_t y,
                   const FvsByte_t val) {
    iFvsImage_t* image = (iFvsImage_t*)img;
    int address = y * image->w + x;
    image->pimg[address] = val;
}


/******************************************************************************
  * 功能：获得图像中某个象素的值
  * 参数：image  指向图像对象的指针
  *       x      X轴坐标
  *       y      Y轴坐标
  * 返回：象素的值
******************************************************************************/
FvsByte_t ImageGetPixel(const FvsImage_t img, const FvsInt_t x,
                        const FvsInt_t y) {
    iFvsImage_t* image = (iFvsImage_t*)img;
    /* 数组中的位置 */
    int address = y * image->pitch + x;
    return image->pimg[address];
}


/******************************************************************************
  * 功能：获得图像缓冲区指针
  * 参数：image  指向图像对象的指针
  * 返回：指向图像内存缓冲区的指针
******************************************************************************/
FvsByte_t* ImageGetBuffer(FvsImage_t img) {
    iFvsImage_t* image = (iFvsImage_t*)img;
    if (image == NULL)
        return NULL;
    return image->pimg;
}


/******************************************************************************
  * 功能：获得图像宽度
  * 参数：image  指向图像对象的指针
  * 返回：图像宽度
******************************************************************************/
FvsInt_t ImageGetWidth(const FvsImage_t img) {
    iFvsImage_t* image = (iFvsImage_t*)img;
    if (image == NULL)
        return -1;
    return image->w;
}


/******************************************************************************
  * 功能：获得图像高度
  * 参数：image  指向图像对象的指针
  * 返回：图像高度
******************************************************************************/
FvsInt_t ImageGetHeight(const FvsImage_t img) {
    iFvsImage_t* image = (iFvsImage_t*)img;
    if (image == NULL)
        return -1;
    return image->h;
}


/******************************************************************************
  * 功能：获得图像缓冲区的大小
  * 参数：image  指向图像对象的指针
  * 返回：缓冲区大小
******************************************************************************/
FvsInt_t ImageGetSize(const FvsImage_t img) {
    iFvsImage_t* image = (iFvsImage_t*)img;
    if (image == NULL)
        return 0;
    return image->h * image->w;
}


/******************************************************************************
  * 功能：获得图像倾斜度
  * 参数：image  指向图像对象的指针
  * 返回：倾斜度
******************************************************************************/
FvsInt_t ImageGetPitch(const FvsImage_t img) {
    iFvsImage_t* image = (iFvsImage_t*)img;
    if (image == NULL)
        return -1;
    return image->pitch;
}


/******************************************************************************
  * 功能：比较两个图像大小
  * 参数：image1  指向图像对象1的指针
  *       image2  指向图像对象2的指针
  * 返回：若两个图像大小相等，返回true；否则返回false
******************************************************************************/
FvsBool_t ImageCompareSize(const FvsImage_t image1, const FvsImage_t image2) {
    if (ImageGetWidth(image1) != ImageGetWidth(image2))
        return FvsFalse;
    if (ImageGetHeight(image1) != ImageGetHeight(image2))
        return FvsFalse;
    return FvsTrue;
}

