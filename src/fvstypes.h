/*#############################################################################
 * 文件名：fvstype.h
 * 功能：  基本类型的定义
 * modified by  PRTsinghua@hotmail.com
#############################################################################*/

#if !defined FVS__FVSTYPES_HEADER__INCLUDED__
#define FVS__FVSTYPES_HEADER__INCLUDED__

/******************************************************************************
  * 这些类型可能已经在系统中定义了，根据自己系统的情况修改
******************************************************************************/
    /* windows用户使用 */

    typedef unsigned char	uint8_t;
    typedef unsigned short	uint16_t;
    typedef unsigned int	uint32_t;

    typedef signed char		int8_t;
    typedef signed short	int16_t;
    typedef signed int		int32_t;


/* 定义指纹整型 */
typedef int				FvsInt_t;


/* 定义指纹无符号整型 */
typedef unsigned int	FvsUint_t;


/* 字节，字，双字 */
typedef int8_t			FvsInt8_t;
typedef int16_t			FvsInt16_t;
typedef int32_t			FvsInt32_t;


/* 无符号字节，字，双字 */
typedef uint8_t			FvsUint8_t;
typedef uint16_t		FvsUint16_t;
typedef uint32_t		FvsUint32_t;

typedef uint8_t			FvsByte_t;
typedef uint16_t		FvsWord_t;
typedef uint32_t		FvsDword_t;


/* 浮点类型 */
typedef double			FvsFloat_t;


/* 指针类型 */
typedef void*			FvsPointer_t;


/* 句柄类型，用来操作不透明的指针 */
typedef void*			FvsHandle_t;


/* 字符串类型 */
typedef char*			FvsString_t;


/* 布尔类型 */
typedef enum FvsBool_t
{
	/* false标记 */
	FvsFalse = 0,
    /* true标记 */
	FvsTrue  = 1
} FvsBool_t;


/* 定义PI值 */
#ifndef M_PI
#define M_PI			3.1415926535897932384626433832795
#endif


/* 定义错误类型 */
typedef enum FvsError_t
{
	/* 为定义错误 */
	FvsFailure			= -1,
	/* 没有错误 */
	FvsOK				= 0,
	/* 内存不足 */
	FvsMemory,
	/* 参数无效 */
	FvsBadParameter,
	/* 文件格式错误 */
	FvsBadFormat,
	/* 输入输出错误 */
	FvsIoError,
} FvsError_t;


#endif /* FVS__FVSTYPES_HEADER__INCLUDED__ */

