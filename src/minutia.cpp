
/*#############################################################################
 * 文件名：minutia.cpp
 * 功能：  细节点的一些函数接口
#############################################################################*/

#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#include "minutia.h"


typedef struct iFvsMinutiaSet_t {
    FvsInt_t     nbminutia;
    FvsInt_t     tablesize;
    FvsMinutia_t ptable[1];
} iFvsMinutiaSet_t;


/******************************************************************************
  * 功能：创建一个细节点集合
  * 参数：size  集合的大小
  * 返回：若失败，返回空；否则返回新的对象句柄
******************************************************************************/
FvsMinutiaSet_t MinutiaSetCreate(const FvsInt_t size) {
    iFvsMinutiaSet_t* p = NULL;
    p = (iFvsMinutiaSet_t*)malloc(sizeof(iFvsMinutiaSet_t)
                                  + size * sizeof(FvsMinutia_t));
    if (p != NULL) {
        /* 表中无细节点 */
        p->nbminutia = 0;
        p->tablesize = size;
    }
    return (FvsMinutiaSet_t)p;
}


/******************************************************************************
  * 功能：销毁细节点集合。
  *       一旦销毁，该对象不再可以为任何函数所用，直到重新申请。
  * 参数：minutia      细节点集合
  * 返回：无
******************************************************************************/
void MinutiaSetDestroy(FvsMinutiaSet_t minutia) {
    iFvsMinutiaSet_t* p = NULL;
    if (minutia == NULL)
        return;
    p = (iFvsMinutiaSet_t *)minutia;
    free(p);
}


/******************************************************************************
  * 功能：获得细节点集合的大小
  * 参数：minutia      细节点集合
  * 返回：细节点集合大小
******************************************************************************/
FvsInt_t MinutiaSetGetSize(const FvsMinutiaSet_t min) {
    const iFvsMinutiaSet_t* minutia = (iFvsMinutiaSet_t*)min;
    FvsInt_t nret = 0;
    if (minutia != NULL)
        nret = minutia->tablesize;
    return nret;
}


/******************************************************************************
  * 功能：细节点集合的实际元素个数
  * 参数：minutia      细节点集合
  * 返回：元素个数
******************************************************************************/
FvsInt_t MinutiaSetGetCount(const FvsMinutiaSet_t min) {
    const iFvsMinutiaSet_t* minutia = (iFvsMinutiaSet_t*)min;
    FvsInt_t nret = 0;
    if (minutia != NULL)
        nret = minutia->nbminutia;
    return nret;
}


/******************************************************************************
  * 功能：返回细节点集合的数据缓冲区指针
  * 参数：minutia      细节点集合
  * 返回：指针
******************************************************************************/
FvsMinutia_t* MinutiaSetGetBuffer(FvsMinutiaSet_t min) {
    iFvsMinutiaSet_t* minutia = (iFvsMinutiaSet_t*)min;
    FvsMinutia_t* pret = NULL;
    if (minutia != NULL)
        pret = minutia->ptable;
    return pret;
}


/******************************************************************************
  * 功能：清空细节点集合
  * 参数：minutia      细节点集合
  * 返回：错误编号
******************************************************************************/
FvsError_t MinutiaSetEmpty(FvsMinutiaSet_t min) {
    iFvsMinutiaSet_t* minutia = (iFvsMinutiaSet_t*)min;
    FvsError_t nRet = FvsOK;
    if (minutia != NULL)
        minutia->nbminutia = 0;
    else
        nRet = FvsMemory;
    return nRet;
}


/******************************************************************************
  * 功能：在集合中添加一个细节点，如果满了，返回一个错误
  * 参数：minutia      细节点集合
  *       x            细节点的X坐标
  *       y            细节点的Y坐标
  *       type         细节点类型
  *       angle        角度
  * 返回：错误编号
******************************************************************************/
FvsError_t MinutiaSetAdd(FvsMinutiaSet_t min,
                         const FvsFloat_t x, const FvsFloat_t y,
                         const FvsMinutiaType_t type, const FvsFloat_t angle) {
    iFvsMinutiaSet_t* minutia = (iFvsMinutiaSet_t*)min;
    FvsError_t nRet = FvsOK;
    if (minutia->nbminutia < minutia->tablesize) {
        minutia->ptable[minutia->nbminutia].x       = x;
        minutia->ptable[minutia->nbminutia].y       = y;
        minutia->ptable[minutia->nbminutia].type    = type;
        minutia->ptable[minutia->nbminutia].angle   = angle;
        minutia->nbminutia++;
    }
    else {
        /* 表中无空间 */
        nRet = FvsMemory;
        fprintf(stdout, "no space!\n");
    }
    return nRet;
}


static FvsError_t MinutiaSetCheckClean(FvsMinutiaSet_t min) {
    iFvsMinutiaSet_t* minutia = (iFvsMinutiaSet_t*)min;
    FvsError_t    nRet = FvsOK;
    FvsFloat_t    tx, ty, ta;
    FvsInt_t      i, j;
    FvsMinutia_t* mi, *mj;
    tx = 4.0;
    ty = 4.0;
    ta = 0.5;
    if (minutia != NULL && minutia->nbminutia > 1) {
        /* 检验表中是否已有该细节点 */
        for (j = 0;   j < minutia->nbminutia; j++)
            for (i = j + 1; i < minutia->nbminutia; i++) {
                mi = minutia->ptable + i;
                mj = minutia->ptable + j;
                /* 比较细节点i，j */
                /* 规则 1: 相似的细节点彼此靠近 -> 删除一个 */
                if ( (fabs(mi->x     - mj->x    ) < tx) &&
                        (fabs(mi->y     - mj->y    ) < ty) &&
                        (fabs(mi->angle - mj->angle) < ta)
                   ) {
                    minutia->nbminutia--;
                    *mi = minutia->ptable[minutia->nbminutia];
                }
                /* 规则 2: 方向相反，距离靠近 -> 同时删除 */
            }
    }
    else
        /* 表中无空间 */
        nRet = FvsMemory;
    return nRet;
}


#define P(x,y)      p[(x)+(y)*pitch]


/******************************************************************************
  * 功能：在图像中画出细节点，不改变背景
  * 参数：minutia      细节点集合
  *       image        指纹图像
  * 返回：错误编号
******************************************************************************/
FvsError_t MinutiaSetDraw(const FvsMinutiaSet_t min, FvsImage_t image) {
    FvsInt_t w       = ImageGetWidth(image);
    FvsInt_t h       = ImageGetHeight(image);
    FvsInt_t pitch   = ImageGetPitch(image);
    FvsByte_t* p     = ImageGetBuffer(image);
    FvsInt_t n, x, y;
    FvsFloat_t fx, fy;
    FvsMinutia_t* minutia = MinutiaSetGetBuffer(min);
    FvsInt_t nbminutia    = MinutiaSetGetCount(min);
    fprintf(stdout, "%d\n", nbminutia);
    if (minutia == NULL || p == NULL)
        return FvsMemory;
    /* 画出每个细节点 */
    for (n = 0; n < nbminutia; n++) {
        x = (FvsInt_t)minutia[n].x;
        y = (FvsInt_t)minutia[n].y;
        if (x < w - 5 && x > 4) {
            if (y < h - 5 && y > 4) {
                switch (minutia[n].type) {
                    case FvsMinutiaTypeEnding:
                        P(x, y)    = 0xFF;
                        P(x - 1, y) = 0xA0;
                        P(x + 1, y) = 0xA0;
                        P(x, y - 1) = 0xA0;
                        P(x, y + 1) = 0xA0;
                        break;
                    case FvsMinutiaTypeBranching:
                        P(x, y)    = 0xFF;
                        P(x - 1, y - 1) = 0xA0;
                        P(x + 1, y - 1) = 0xA0;
                        P(x - 1, y + 1) = 0xA0;
                        P(x + 1, y + 1) = 0xA0;
                        break;
                    default:
                        continue;
                }
                fx = sin(minutia[n].angle);
                fy = -cos(minutia[n].angle);
                P(x + (int32_t)(fx)    , y + (int32_t)(fy)    ) = 0xFF;
                P(x + (int32_t)(fx * 2.0), y + (int32_t)(fy * 2.0)) = 0xFF;
                P(x + (int32_t)(fx * 3.0), y + (int32_t)(fy * 3.0)) = 0xFF;
                P(x + (int32_t)(fx * 4.0), y + (int32_t)(fy * 4.0)) = 0xFF;
                P(x + (int32_t)(fx * 5.0), y + (int32_t)(fy * 5.0)) = 0xFF;
            }
        }
    }
    return FvsOK;
}


/* 宏定义 */
#define P1  P(x  ,y-1)
#define P2  P(x+1,y-1)
#define P3  P(x+1,y  )
#define P4  P(x+1,y+1)
#define P5  P(x  ,y+1)
#define P6  P(x-1,y+1)
#define P7  P(x-1,y  )
#define P8  P(x-1,y-1)


/******************************************************************************
  * 功能：从细化图像中提取细节点，并储存到集合中。
  *       申请的细节点集合必须足够大，如果太小了，满了后会停止搜索细节点。
  * 参数：minutia      细节点集合，用来保存细节点
  *       image        细化后的图像
  *       direction    用来计算方向用
  *       mask         用来表示有效的指纹区域
  * 返回：错误编号
******************************************************************************/
FvsError_t MinutiaSetExtract
(
    FvsMinutiaSet_t       minutia,
    const FvsImage_t      image,
    const FvsFloatField_t direction,
    const FvsImage_t      mask
) {
    FvsInt_t w      = ImageGetWidth(image);
    FvsInt_t h      = ImageGetHeight(image);
    FvsInt_t pitch  = ImageGetPitch(image);
    FvsInt_t pitchm = ImageGetPitch(mask);
    FvsByte_t* p    = ImageGetBuffer(image);
    FvsByte_t* m    = ImageGetBuffer(mask);
    FvsInt_t   x, y;
    FvsFloat_t angle = 0.0;
    FvsInt_t   whitecount;
    int cnt = 0;
    if (m == NULL || p == NULL)
        return FvsMemory;
    (void)MinutiaSetEmpty(minutia);
    /* 遍历图像，提取细节点 */
    for (y = 1; y < h - 1; y++)
        for (x = 1; x < w - 1; x++) {
            if (m[x + y * pitchm] == 0)
                continue;
            if (P(x, y) == 0xFF) {
                whitecount = 0;
                if (P1 != 0) whitecount++;
                if (P2 != 0) whitecount++;
                if (P3 != 0) whitecount++;
                if (P4 != 0) whitecount++;
                if (P5 != 0) whitecount++;
                if (P6 != 0) whitecount++;
                if (P7 != 0) whitecount++;
                if (P8 != 0) whitecount++;
                switch(whitecount) {
                    case 0:
                        /* 孤立点，忽略 */
                        break;
                    case 1:
                        /* 检测角度 */
                        angle = FloatFieldGetValue(direction, x, y);
                        (void)MinutiaSetAdd(minutia, (FvsFloat_t)x, (FvsFloat_t)y, FvsMinutiaTypeEnding, (FvsFloat_t)angle);
                        ++cnt;
                        break;
                    case 2:
                        break;
                    default: {
                        angle = FloatFieldGetValue(direction, x, y);
                        (void)MinutiaSetAdd(minutia, (FvsFloat_t)x, (FvsFloat_t)y, FvsMinutiaTypeBranching, (FvsFloat_t)angle);
                        ++cnt;
                    }
                    break;
                }
            }
        }
    (void)MinutiaSetCheckClean(minutia);
    fprintf(stdout, "%d\n", cnt);
    return FvsOK;
}

