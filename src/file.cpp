
/*#############################################################################
 * 文件名：file.cpp
 * 功能：  实现了指纹相关文件的操作
#############################################################################*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "file.h"


/* 对象的这些接口实现是私有的，不必为用户所知 */
typedef struct iFvsFile_t {
    FILE	*pf;	/* 文件指针 */
} iFvsFile_t;


/******************************************************************************
  * 功能：创建一个新的文件对象，只有在创建之后，文件对象才能为其它函数所用。
  * 参数：无
  * 返回：若创建失败，返回NULL；否则返回新的对象句柄。
******************************************************************************/
FvsFile_t FileCreate() {
    iFvsFile_t* p = NULL;
    p = (iFvsFile_t*)malloc(sizeof(iFvsFile_t));
    if (p != NULL)
        p->pf = NULL;
    return (FvsFile_t)p;
}


/******************************************************************************
  * 功能：破坏一个已经存在的文件对象，在毁坏之后，文件对象不能再为其它函数所用。
  * 参数：file  即将删除的文件对象指针
  * 返回：无返回值
******************************************************************************/
void FileDestroy(FvsFile_t file) {
    iFvsFile_t* p = NULL;
    if (file == NULL)
        return;
    /* 关闭文件，如果它还打开着 */
    (void)FileClose(file);
    p = (iFvsFile_t *)file;
    free(p);
}


/******************************************************************************
  * 功能：打开一个新的文件。一个文件可以读打开，写打开，或者被创建。
  * 参数：file    文件对象
  *       name    待打开文件的名字
  *       flags   打开标志
  * 返回：错误编号
******************************************************************************/
FvsError_t FileOpen(FvsFile_t file, const FvsString_t name,
                    const FvsFileOptions_t flags) {
    iFvsFile_t* p = (iFvsFile_t*)file;
    char strFlags[10];
    int nflags = (int)flags;
    /* 关闭文件，如果已经打开 */
    (void)FileClose(p);
    strcpy(strFlags, "");
    if ( (nflags & FvsFileRead) != 0   &&
            (nflags & FvsFileWrite) != 0 )
        strcat(strFlags, "rw");
    else {
        if ((nflags & FvsFileRead) != 0)
            strcat(strFlags, "r");
        if ((nflags & FvsFileWrite) != 0)
            strcat(strFlags, "w");
    }
    strcat(strFlags, "b");
    if ((nflags & FvsFileCreate) != 0)
        strcat(strFlags, "+");
    p->pf = fopen(name, strFlags);
    if (FileIsOpen(file) == FvsTrue)
        return FvsOK;
    return FvsFailure;
}


/******************************************************************************
  * 功能：关闭一个文件对象，文件关闭之后，文件不再可用。
  * 参数：file    文件对象
  * 返回：错误编号
******************************************************************************/
FvsError_t FileClose(FvsFile_t file) {
    iFvsFile_t* p = (iFvsFile_t*)file;
    int nerr = -1;
    if (p->pf != NULL) {
        nerr = fclose(p->pf);
        p->pf = NULL;
    }
    if (nerr == 0)
        return FvsOK;
    return FvsFailure;
}


/******************************************************************************
  * 功能：测试一个文件是否打开
  * 参数：file    文件对象
  * 返回：文件打开，则返回true；否则返回false
******************************************************************************/
FvsBool_t FileIsOpen(const FvsFile_t file) {
    iFvsFile_t* p = (iFvsFile_t*)file;
    return (p->pf != NULL) ? FvsTrue : FvsFalse;
}


/******************************************************************************
  * 功能：测试是否到了文件结尾
  * 参数：file    文件对象
  * 返回：到了结尾，返回true；否则返回false
******************************************************************************/
FvsBool_t FileIsAtEOF(const FvsFile_t file) {
    iFvsFile_t* p = (iFvsFile_t*)file;
    if (FileIsOpen(p) == FvsFalse)
        return FvsFalse;
    return (feof(p->pf) != 0) ? FvsTrue : FvsFalse;
}


/******************************************************************************
  * 功能：提交对文件所作的更改
  * 参数：file    文件对象
  * 返回：错误编号
******************************************************************************/
FvsError_t FileCommit(FvsFile_t file) {
    iFvsFile_t* p = (iFvsFile_t*)file;
    return (fflush(p->pf) == 0) ? FvsOK : FvsFailure;
}


/******************************************************************************
  * 功能：跳到文件的开头
  * 参数：file    文件对象
  * 返回：错误编号
******************************************************************************/
FvsError_t FileSeekToBegin(FvsFile_t file) {
    iFvsFile_t* p = (iFvsFile_t*)file;
    if (FileIsOpen(p) == FvsTrue) {
        if (fseek(p->pf, 0, SEEK_SET) != 0)
            return FvsFailure;
        return FvsOK;
    }
    return FvsFailure;
}


/******************************************************************************
  * 功能：跳到文件的结尾
  * 参数：file    文件对象
  * 返回：错误编号
******************************************************************************/
FvsError_t FileSeekToEnd(FvsFile_t file) {
    iFvsFile_t* p = (iFvsFile_t*)file;
    if (FileIsOpen(p) == FvsTrue) {
        if (fseek(p->pf, 0, SEEK_END) != 0)
            return FvsFailure;
        return FvsOK;
    }
    return FvsFailure;
}


/******************************************************************************
  * 功能：得到当前的文件指针位置
  * 参数：file    文件对象
  * 返回：当前的指针位置
******************************************************************************/
FvsUint_t FileGetPosition(FvsFile_t file) {
    iFvsFile_t* p = (iFvsFile_t*)file;
    if (FileIsOpen(p) == FvsTrue)
        return (FvsUint_t)ftell(p->pf);
    return 0;
}


/******************************************************************************
  * 功能：跳到文件的指定位置
  * 参数：file     文件对象
  *       position 指定的文件位置
  * 返回：错误编号
******************************************************************************/
FvsError_t FileSeek(FvsFile_t file, const FvsUint_t position) {
    iFvsFile_t* p = (iFvsFile_t*)file;
    if (FileIsOpen(p) == FvsTrue) {
        if (fseek(p->pf, (long int)position, SEEK_SET) != 0)
            return FvsFailure;
        return FvsOK;
    }
    return FvsFailure;
}


/******************************************************************************
  * 功能：从文件中读数据，所读取的字节数由length决定。读取的数据保存于指针data。
  * 参数：file    文件对象
  *       data    指向存储数据的数组
  *       length  要读取的字节数
  * 返回：实际读取的字节数
******************************************************************************/
FvsUint_t FileRead(FvsFile_t file, FvsPointer_t data, const FvsUint_t length) {
    iFvsFile_t* p = (iFvsFile_t*)file;
    return (FvsUint_t)fread(data, (size_t)1, (size_t)length, p->pf);
}


/******************************************************************************
  * 功能：往文件中写数据，所写的字节数由length决定。要写入的数据保存于指针data。
  * 参数：file    文件对象
  *       data    指向存储数据的数组
  *       length  要写入的字节数
  * 返回：实际写入的字节数
******************************************************************************/
FvsUint_t FileWrite(FvsFile_t file, const FvsPointer_t data, const FvsUint_t length) {
    iFvsFile_t* p = (iFvsFile_t*)file;
    return (FvsUint_t)fwrite(data, (size_t)1, (size_t)length, p->pf);
}


/******************************************************************************
  * 功能：从文件中得到一个字节
  * 参数：file    文件对象
  * 返回：读取的字节
******************************************************************************/
FvsByte_t FileGetByte(FvsFile_t file) {
    iFvsFile_t* p = (iFvsFile_t*)file;
    return (FvsByte_t)fgetc(p->pf);
}


/******************************************************************************
  * 功能：从文件中读取一个字
  * 参数：file    文件对象
  * 返回：读取的字
******************************************************************************/
FvsWord_t FileGetWord(FvsFile_t file) {
    iFvsFile_t* p = (iFvsFile_t*)file;
    FvsWord_t w = (FvsWord_t)fgetc(p->pf);
    return (w << 8) + fgetc(p->pf);
}

