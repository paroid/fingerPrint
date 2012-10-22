/*#############################################################################
 * 文件名：minutia.h
 * 功能：  细节点的一些函数接口
 * modified by  PRTsinghua@hotmail.com
#############################################################################*/

#if !defined FVS__MINUTIA_HEADER__INCLUDED__
#define FVS__MINUTIA_HEADER__INCLUDED__


/* 基本类型定义 */
#include "fvstypes.h"
#include "image.h"
#include "floatfield.h"


/* 不同细节点类型的定义 */
typedef enum FvsMinutiaType_t
{
    FvsMinutiaTypeEnding		= 0, /* 端点   */
    FvsMinutiaTypeBranching		= 1, /* 分歧点 */
    FvsMinutiaTypeCore			= 2, /* 核心点 */
    FvsMinutiaTypeDelta			= 3, /* 三角点 */
} FvsMinutiaType_t;


/* 细节点结构，使用了FBI的模型，尽可能准确的提取细节点的类型，坐标，角度 */
typedef struct FvsMinutia_t
{
    /* 细节点类型 */
    FvsMinutiaType_t type;
    /* X轴坐标    */
    FvsFloat_t    x;
    /* Y轴坐标    */
    FvsFloat_t    y;
    /* 细节点方向 */
    FvsFloat_t    angle;
} FvsMinutia_t;


/* 对象的这些接口实现是私有的，不必为用户所知 */
typedef FvsHandle_t FvsMinutiaSet_t;


/******************************************************************************
  * 功能：创建一个细节点集合
  * 参数：size  集合的大小
  * 返回：若失败，返回空；否则返回新的对象句柄
******************************************************************************/
FvsMinutiaSet_t MinutiaSetCreate (const FvsInt_t size);


/******************************************************************************
  * 功能：销毁细节点集合。
  *       一旦销毁，该对象不再可以为任何函数所用，直到重新申请。
  * 参数：minutia      细节点集合
  * 返回：无
******************************************************************************/
void  MinutiaSetDestroy(FvsMinutiaSet_t minutia);


/******************************************************************************
  * 功能：获得细节点集合的大小
  * 参数：minutia      细节点集合
  * 返回：细节点集合大小
******************************************************************************/
FvsInt_t MinutiaSetGetSize(const FvsMinutiaSet_t minutia);


/******************************************************************************
  * 功能：细节点集合的实际元素个数
  * 参数：minutia      细节点集合
  * 返回：元素个数
******************************************************************************/
FvsInt_t MinutiaSetGetCount(const FvsMinutiaSet_t minutia);


/******************************************************************************
  * 功能：返回细节点集合的数据缓冲区指针
  * 参数：minutia      细节点集合
  * 返回：指针
******************************************************************************/
FvsMinutia_t* MinutiaSetGetBuffer(FvsMinutiaSet_t minutia);


/******************************************************************************
  * 功能：清空细节点集合
  * 参数：minutia      细节点集合
  * 返回：错误编号
******************************************************************************/
FvsError_t MinutiaSetEmpty(FvsMinutiaSet_t minutia);


/******************************************************************************
  * 功能：在集合中添加一个细节点，如果满了，返回一个错误
  * 参数：minutia      细节点集合
  *       x            细节点的X坐标
  *       y            细节点的Y坐标
  *       type         细节点类型
  *       angle        角度
  * 返回：错误编号
******************************************************************************/
FvsError_t MinutiaSetAdd
    (
    FvsMinutiaSet_t        minutia,
    const FvsFloat_t       x,
    const FvsFloat_t       y,
    const FvsMinutiaType_t type,
    const FvsFloat_t       angle
    );


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
    );


/******************************************************************************
  * 功能：在图像中画出细节点，不改变背景
  * 参数：minutia      细节点集合
  *       image        指纹图像
  * 返回：错误编号
******************************************************************************/
FvsError_t MinutiaSetDraw
    (
    const FvsMinutiaSet_t minutia,
    FvsImage_t image
    );


#endif /* FVS__MINUTIA_HEADER__INCLUDED__ */

