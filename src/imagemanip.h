/*#############################################################################
 * 文件名：imagemanip.h
 * 功能：  实现了主要的图像处理操作
 * modified by  PRTsinghua@hotmail.com
#############################################################################*/

#if !defined FVS__IMAGEMANIP_HEADER__INCLUDED__
#define FVS__IMAGEMANIP_HEADER__INCLUDED__

/* 包含基本的图像操作函数 */
#include "img_base.h"
#include "floatfield.h"



/******************************************************************************
  * 功能：计算指纹图像脊线的方向。
          该算法在许多论文中都有描述，如果图像做了归一化，并且对比度较高，
          则最后的处理效果也较好。
          方向的值在-PI/2和PI/2之间，弧度和脊并不相同。
          选取的块越大，分析的效果也越好，但所需的处理计算时间也越长。
          由于指纹图像中脊线方向的变化比较缓慢，所以低通滤波器可以较好的
          过虑掉方向中的噪声和错误。
  * 参数：image          指向图像对象的指针
  *       field          指向浮点域对象的指针，保存结果
  *       nBlockSize     块大小
  *       nFilterSize    滤波器大小
  * 返回：错误编号
******************************************************************************/
extern FvsError_t FingerprintGetDirection(const FvsImage_t image, 
								FvsFloatField_t field,
								const FvsInt_t nBlockSize, 
								const FvsInt_t nFilterSize);


/******************************************************************************
  * 功能：获取脊线频率
  * 参数：image      指纹图像，由之提取方向频率
  *       direction  脊线方向
  *       frequency  脊线频率
  * 返回：错误编号
******************************************************************************/
extern FvsError_t FingerprintGetFrequency(const FvsImage_t image, 
							const FvsFloatField_t direction,
							FvsFloatField_t frequency);


/******************************************************************************
  * 功能：修改后的获取脊线频率
  * 参数：image      指纹图像，由之提取方向频率
  *       direction  脊线方向
  *       frequency  脊线频率
  * 返回：错误编号
******************************************************************************/
extern FvsError_t FingerprintGetFrequency1(const FvsImage_t image, 
							const FvsFloatField_t direction,
							FvsFloatField_t frequency);

extern FvsError_t FingerprintGetFrequency2(const FvsImage_t image, 
							const FvsFloatField_t direction,
							FvsFloatField_t frequency);

/******************************************************************************
  * 功能：获取指纹图像的有效区域，以进行进一步的处理。
  *       如果某个区域不可用用，则掩码置为0，包括如下区域：
  *       边界，背景点，图像质量很差的区域。
  *       有效区域的掩码置为255。
  * 参数：image        指纹图像
  *       direction    脊线方向
  *       frequency    脊线频率
  *       mask         输出的掩码
  * 返回：错误编号
******************************************************************************/
extern FvsError_t FingerprintGetMask(const FvsImage_t image, 
							const FvsFloatField_t direction,
							const FvsFloatField_t frequency, 
							FvsImage_t mask);


/******************************************************************************
  * 功能：细化指纹图像
  *       图像必须是二值化过的（只包含0x00或oxFF）
  *       该算法基于领域的判断，决定某个象素该移去还是保留
  * 参数：image   指纹图像
  * 返回：错误编号
******************************************************************************/
extern FvsError_t ImageThinConnectivity(FvsImage_t image);


/******************************************************************************
  * 功能：细化指纹图像，使用“Hit and Miss”结构元素。
  *       图像必须是二值化过的（只包含0x00或oxFF）
  *       该算法的缺点是产生很多伪造的线条（伪特征），
  *       必须由另外的算法来消除，后处理非常必要。
  * 参数：image   指纹图像
  * 返回：错误编号
******************************************************************************/
FvsError_t ImageThinHitMiss(FvsImage_t image);


/******************************************************************************
  * 功能：图像缩放操作
  * 参数：image       指纹图像
  *       size        缩放的图像块大小
  *       tolerance   消去直方图的边界
  * 返回：错误编号
******************************************************************************/
FvsError_t ImageLocalStretch(FvsImage_t image, const FvsInt_t size, 
								const FvsInt_t tolerance);


/******************************************************************************
  * 功能：图像膨胀算法
  * 参数：image   指纹图像
  * 返回：错误编号
******************************************************************************/
FvsError_t ImageDilate(FvsImage_t image);


/******************************************************************************
  * 功能：图像腐蚀算法
  * 参数：image   指纹图像
  * 返回：错误编号
******************************************************************************/
FvsError_t ImageErode(FvsImage_t image);


/******************************************************************************
  * 功能：指纹图像增强算法
  *       该算法描述起来比较复杂，其后处理的部分是基于Gabor滤波器的，
          参数动态计算。图像处理时参数依次改变，所以要做一个原图的备份。
  * 参数：image        指纹图像
  *       direction    脊线方向，需要事先计算
  *       frequency    脊线频率，需要事先计算
  *       mask         指示指纹的有效区域
  *       radius       滤波器半径，大多数情况下，4.0即可。
                       值越大，噪声可以受到更大抑制，但会产生更多的伪特征。
  * 返回：错误编号
******************************************************************************/
extern FvsError_t ImageEnhanceGabor(FvsImage_t image, const FvsFloatField_t direction,
             const FvsFloatField_t frequency, const FvsImage_t mask, 
             const FvsFloat_t radius);


#endif /* FVS__IMAGEMANIP_HEADER__INCLUDED__ */

