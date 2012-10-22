/*#############################################################################
 * 文件名：export.h
 * 功能：  指纹图像输出保存
 * modified by  PRTsinghua@hotmail.com
#############################################################################*/

#if !defined FVS__EXPORT_HEADER__INCLUDED__
#define FVS__EXPORT_HEADER__INCLUDED__

#include "file.h"
#include "image.h"

/******************************************************************************
  * 功能：将一个指纹图像输出到一个文件，文件的格式由文件的扩展名决定
  * 参数：filename  将要保存图像的文件名
  *       image     将要导出的图像
  * 返回：错误代码
******************************************************************************/
FvsError_t  FvsImageExport(const FvsImage_t image, const FvsString_t filename,
		FvsByte_t bmfh[14],BITMAPINFOHEADER *bmih,RGBQUAD *rgbq);


#endif /* FVS__EXPORT_HEADER__INCLUDED__ */

