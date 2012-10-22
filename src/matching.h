/*#############################################################################
 * 文件名：matching.h
 * 功能：  实现了指纹匹配算法
 * modified by  PRTsinghua@hotmail.com
#############################################################################*/

#if !defined FVS__MATCHING_HEADER__INCLUDED__
#define FVS__MATCHING_HEADER__INCLUDED__

#include "image.h"
#include "minutia.h"


/******************************************************************************
  * 功能：匹配两个指纹
  * 参数：image1      指纹图像1
  *       image2      指纹图像2
  *       pgoodness   匹配度，越高越好
  * 返回：错误编号
******************************************************************************/
FvsError_t MatchingCompareImages(const FvsImage_t image1,
                                 const FvsImage_t image2,
                                 FvsInt_t* pgoodness);


/******************************************************************************
  * 功能：匹配指纹细节点
  * 参数：minutia1      细节点集合1
  *       minutia2      细节点集合2
  *       pgoodness   匹配度，越高越好
  * 返回：错误编号
******************************************************************************/
FvsError_t MatchingCompareMinutiaSets(const FvsMinutiaSet_t minutia1,
                                      const FvsMinutiaSet_t minutia2,
                                      FvsInt_t* pgoodness);


#endif /* __MATCHING_HEADER__INCLUDED__ */

