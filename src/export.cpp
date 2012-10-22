
/*#############################################################################
 * 文件名：export.cpp
 * 功能：  指纹图像输出保存
#############################################################################*/

#include "export.h"

#include <stdio.h>


/******************************************************************************
  * 功能：将一个指纹图像输出到一个文件，文件的格式由文件的扩展名决定
  * 参数：filename  将要保存图像的文件名
  *       image     将要导出的图像
  * 返回：错误代码
******************************************************************************/
FvsError_t  FvsImageExport(const FvsImage_t image, const FvsString_t filename,
                           FvsByte_t bmfh[14], BITMAPINFOHEADER *bmih, RGBQUAD *rgbq) {
    FvsError_t ret = FvsOK;
    FvsByte_t*		buffer;
    FvsInt_t		pitch;
    FvsInt_t		height;
    FvsInt_t		width;
    FvsInt_t		i;
    FvsFile_t	file;
    file	  = FileCreate();
    if(FileOpen(file, filename, (FvsFileOptions_t)(FvsFileWrite | FvsFileCreate)) == FvsFailure) {
        ret = FvsFailure;
    }
    if(FileWrite(file, bmfh, 14) != 14)
        ret = FvsFailure;
    if(FileWrite(file, bmih, sizeof(BITMAPINFOHEADER)) != sizeof(BITMAPINFOHEADER))
        ret = FvsFailure;
    if(FileWrite(file, rgbq, sizeof(RGBQUAD) * 256) != sizeof(RGBQUAD) * 256)
        ret = FvsFailure;
    if(ret == FvsFailure) {
        printf("Write file error");
        return ret;
    }
    else {
        /* 获得缓冲区 */
        buffer = ImageGetBuffer(image);
        pitch  = ImageGetPitch(image);
        height = ImageGetHeight(image);
        width  = ImageGetWidth(image);
        /* 拷贝数据 */
        for (i = height - 1; i >= 0; i--) {
            FileWrite(file, buffer + i * pitch, WIDTHBYTES(pitch * 8));
        }
    }
    FileDestroy(file);
    return ret;
}



