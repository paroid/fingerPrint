
/*#############################################################################
 * 文件名：floatfield.cpp
 * 功能：  实现了指纹浮点域的操作
#############################################################################*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "floatfield.h"

/* 指纹浮点域结构 */
typedef struct iFvsFloatField_t {
    FvsFloat_t		*pimg;		/* 浮点数指针数组 */
    FvsInt_t		w;			/* 宽度 */
    FvsInt_t		h;			/* 高度 */
    FvsInt_t		pitch;		/* 倾斜度 */
} iFvsFloatField_t;


/******************************************************************************
  * 功能：创建一个心的浮点域对象
  * 参数：无
  * 返回：创建失败，返回空；否则返回新的对象句柄
******************************************************************************/
FvsFloatField_t FloatFieldCreate() {
    iFvsFloatField_t* p = NULL;
    p = (iFvsFloatField_t *)(FvsFloatField_t)malloc(sizeof(iFvsFloatField_t));
    if (p != NULL) {
        p->h        = 0;
        p->w        = 0;
        p->pitch    = 0;
        p->pimg     = NULL;
    }
    return (FvsFloatField_t)p;
}


/******************************************************************************
  * 功能：破坏已经存在的浮点域对象
  * 参数：field   指向浮点域对象的指针
  * 返回：无
******************************************************************************/
void FloatFieldDestroy(FvsFloatField_t field) {
    iFvsFloatField_t* p = NULL;
    if (field == NULL)
        return;
    p = (iFvsFloatField_t *)field;
    (void)FloatFieldSetSize(field, 0, 0);
    free(p);
}


/******************************************************************************
  * 功能：设置浮点域对象的大小。
          内存操作自动完成，如果失败，返回一个错误编号
  * 参数：field   指向浮点域对象的指针
  *       width   宽
  *       height  高
  * 返回：错误编号
******************************************************************************/
FvsError_t FloatFieldSetSize(FvsFloatField_t img, const FvsInt_t width,
                             const FvsInt_t height) {
    iFvsFloatField_t* field = (iFvsFloatField_t*)img;
    FvsError_t nRet = FvsOK;
    FvsInt_t newsize = (FvsInt_t)(width * height * sizeof(FvsFloat_t));
    /* 大小为0的情况 */
    if (newsize == 0) {
        if (field->pimg != NULL) {
            free(field->pimg);
            field->pimg = NULL;
            field->w = 0;
            field->h = 0;
            field->pitch = 0;
        }
        return FvsOK;
    }
    if ((FvsInt_t)(field->h * field->w * sizeof(FvsFloat_t)) != newsize) {
        free(field->pimg);
        field->w = 0;
        field->h = 0;
        field->pitch = 0;
        /* 申请内存 */
        field->pimg = (FvsFloat_t*)malloc((size_t)newsize);
    }
    if (field->pimg == NULL)
        nRet = FvsMemory;
    else {
        field->h = height;
        field->w = width;
        field->pitch = width;
    }
    return nRet;
}


/******************************************************************************
  * 功能：拷贝一个源图像到目标图像，内存操作和大小重置操作自动完成
  * 参数：destination 指向目标浮点域对象的指针
  *       source      指向源浮点域对象的指针
  * 返回：错误编号
******************************************************************************/
FvsError_t FloatFieldCopy(FvsFloatField_t destination,
                          const FvsFloatField_t source) {
    iFvsFloatField_t* dest = (iFvsFloatField_t*)destination;
    iFvsFloatField_t* src  = (iFvsFloatField_t*)source;
    FvsError_t nRet = FvsOK;
    nRet = FloatFieldSetSize(dest, src->w, src->h);
    if (nRet == FvsOK)
        memcpy(dest->pimg, src->pimg, src->h * src->w * sizeof(FvsFloat_t));
    return nRet;
}


/******************************************************************************
  * 功能：清空图像，设置浮点域对象指针为空
  * 参数：field 指向浮点域对象的指针
  * 返回：错误编号
******************************************************************************/
FvsError_t FloatFieldClear(FvsFloatField_t img) {
    return FloatFieldFlood(img, 0.0);
}


/******************************************************************************
  * 功能：给浮点域对象的所以数值设置特定值
  * 参数：field  指向浮点域对象的指针
  *       value  要设置的值
  * 返回：错误编号
******************************************************************************/
FvsError_t FloatFieldFlood(FvsFloatField_t img, const FvsFloat_t value) {
    iFvsFloatField_t* field = (iFvsFloatField_t*)img;
    FvsError_t nRet = FvsOK;
    FvsInt_t i;
    if (field->pimg != NULL) {
        for (i = 0; i < field->h * field->w; i++)
            field->pimg[i] = value;
    }
    return nRet;
}


/******************************************************************************
  * 功能：为浮点域中的特定区域设置特定值
  * 参数：field  指向浮点域对象的指针
  *       x      X轴坐标
  *       y      Y轴坐标
  *       val    要设定的值
  * 返回：无
******************************************************************************/
void FloatFieldSetValue(FvsFloatField_t img, const FvsInt_t x,
                        const FvsInt_t y, const FvsFloat_t val) {
    iFvsFloatField_t* field = (iFvsFloatField_t*)img;
    int address = y * field->w + x;
    field->pimg[address] = val;
}


/******************************************************************************
  * 功能：得到特定位置的值
  * 参数：field  指向浮点域对象的指针
  *       x      X轴坐标
  *       y      Y轴坐标
  * 返回：浮点值
******************************************************************************/
FvsFloat_t FloatFieldGetValue(FvsFloatField_t img, const FvsInt_t x,
                              const FvsInt_t y) {
    iFvsFloatField_t* field = (iFvsFloatField_t*)img;
    /* 数组中的位置 */
    int address = y * field->pitch + x;
    return field->pimg[address];
}


/******************************************************************************
  * 功能：得到浮点域缓冲区指针
  * 参数：field  指向浮点域对象的指针
  * 返回：内存缓冲区指针
******************************************************************************/
FvsFloat_t* FloatFieldGetBuffer(FvsFloatField_t img) {
    iFvsFloatField_t* field = (iFvsFloatField_t*)img;
    return field->pimg;
}


/******************************************************************************
  * 功能：获得宽度
  * 参数：field  指向浮点域对象的指针
  * 返回：宽度
******************************************************************************/
FvsInt_t FloatFieldGetWidth(const FvsFloatField_t img) {
    iFvsFloatField_t* field = (iFvsFloatField_t*)img;
    return field->w;
}


/******************************************************************************
  * 功能：获得高度
  * 参数：field  指向浮点域对象的指针
  * 返回：高度
******************************************************************************/
FvsInt_t FloatFieldGetHeight(const FvsFloatField_t img) {
    iFvsFloatField_t* field = (iFvsFloatField_t*)img;
    return field->h;
}


/******************************************************************************
  * 功能：获得倾斜程度
  * 参数：field  指向浮点域对象的指针
  * 返回：倾斜程度
******************************************************************************/
FvsInt_t FloatFieldGetPitch(const FvsFloatField_t img) {
    iFvsFloatField_t* field = (iFvsFloatField_t*)img;
    return field->pitch;
}


