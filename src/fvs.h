/*#############################################################################
 * 文件名：fvs.h
 * 功能：  提供了一个指纹识别的框架
 * modified by  PRTsinghua@hotmail.com
#############################################################################*/

#if !defined FVS__GLOBAL_HEADER__INCLUDED__
#define FVS__GLOBAL_HEADER__INCLUDED__


/**********          Fingerprint Verification System - FVS         ************
  * 提供了用户接口，可以打开指纹文件，处理指纹图像并分析。
******************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>

/* 包含自动配置文件 */
//#include "config.h"

/* 基本类型定义文件 */
#include "fvstypes.h"

/* 文件输入/输出 */
#include "import.h"		/* 自动检测输入文件的类型 */
#include "export.h"		/* 将指纹图像输出到文件   */

/* 图像对象 */
#include "image.h"

/* 浮点域对象 */
#include "floatfield.h"

/* 文件对象 */
#include "file.h"

/* 处理细节 */
#include "minutia.h"

/* 直方图对象 */
#include "histogram.h"

/* 图像处理操作 */
#include "imagemanip.h"

/* 匹配算法 */
#include "matching.h"

/* 版本 */
//const FvsString_t FvsGetVersion(void);

#endif /* FVS__GLOBAL_HEADER__INCLUDED__*/
