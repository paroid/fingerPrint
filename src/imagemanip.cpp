
/*#############################################################################
 * 文件名：imagemanip.cpp
 * 功能：  实现了主要的图像处理操作
#############################################################################*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "imagemanip.h"

#ifndef min
#define min(a,b) (((a)<(b))?(a):(b))
#endif


/* 宏定义 */
#define PIJKL p[i+k + (j+l)*nSizeX]


/******************************************************************************
  * 功能：图像缩放操作
  * 参数：image       指纹图像
  *       size        缩放的图像块大小
  *       tolerance   消去直方图的边界
  * 返回：错误编号
******************************************************************************/
FvsError_t ImageLocalStretch(FvsImage_t image, const FvsInt_t size,
                             const FvsInt_t tolerance) {
    /* 定义一些变量 */
    int nSizeX = ImageGetWidth(image)  - size + 1;
    int nSizeY = ImageGetHeight(image) - size + 1;
    FvsInt_t i, j, t, l;
    FvsInt_t sum, denom;
    FvsByte_t a = 0;
    FvsInt_t k = 0;
    FvsByte_t b = 255;
    int hist[256];
    FvsByte_t* p = ImageGetBuffer(image);
    if (p == NULL)
        return FvsMemory;
    for (j = 0; j < nSizeY; j += size) {
        for (i = 0; i < nSizeX; i += size) {
            /* 计算直方图 */
            memset(hist, 0, 256 * sizeof(int));
            for (l = 0; l < size; l++)
                for (k = 0; k < size; k++)
                    hist[PIJKL]++;
            /* 伸缩 */
            for (k = 0,   sum = 0;   k < 256; k++) {
                sum += hist[k];
                a = (FvsByte_t)k;
                if (sum > tolerance) break;
            }
            for (k = 255, sum = 0; k >= 0; k--) {
                sum += hist[k];
                b = (FvsByte_t)k;
                if (sum > tolerance) break;
            }
            denom = (FvsInt_t)(b - a);
            if (denom != 0) {
                for (l = 0; l < size; l++) {
                    for (k = 0; k < size; k++) {
                        if (PIJKL < a) PIJKL = a;
                        if (PIJKL > b) PIJKL = b;
                        t = (FvsInt_t)((((PIJKL) - a) * 255) / denom);
                        PIJKL = (FvsByte_t)(t);
                    }
                }
            }
        }
    }
    return FvsOK;
}


#define P(x,y)      ((int32_t)p[(x)+(y)*pitch])

/******************************************************************************
** 估算脊线的方向
** 给定一个归一化的指纹图像，算法的主要步骤如下：
**
** 1 - 将G分成大小为 w x w - (15 x 15) 的块；
**
** 2 - 计算每个象素 (i,j)的梯度 dx(i,j) 和 dy(i,j) ,
**     根据计算的需求，梯度算子可以从简单的Sobel算子到复杂的Marr-Hildreth 算子。
**
** 3 - 估算优势方向(i,j), 使用如下的操作：
**
**               i+w/2 j+w/2
**               ---   ---
**               \     \
**     Nx(i,j) =  --    -- 2 dx(u,v) dy(u,v)
**               /     /
**               ---   ---
**            u=i-w/2 v=j-w/2
**
**               i+w/2 j+w/2
**               ---   ---
**               \     \
**     Ny(i,j) =  --    -- dx(u,v) - dy(u,v)
**               /     /
**               ---   ---
**            u=i-w/2 v=j-w/2
**
**                  1    -1  / Nx(i,j) \
**     Theta(i,j) = - tan   |  -------  |
**                  2        \ Ny(i,j) /
**
**     这里，Theta(i,j)是局部脊线方向的最小方差估计，以像素 (i,j) 为中心。
**     从数学的角度看，它代表傅立叶频谱中直角占有时的方向。
**
** 4 - 由于有噪声，脊线的中断，细节点等等的存在，在输入图像中，对局部脊线
**     方向的估计并不总是正确的。由于局部脊线方向变化缓慢，所以可以用低通
**     滤波器来修正不正确的脊线方向。为了运用低通滤波器，方向图必须转换成
**     连续的矢量域，定义如下：
**       Phi_x(i,j) = cos( 2 x theta(i,j) )
**       Phi_y(i,j) = sin( 2 x theta(i,j) )
**     在矢量域，可以用如下的卷积低通滤波：
**       Phi2_x(i,j) = (W @ Phi_x) (i,j)
**       Phi2_y(i,j) = (W @ Phi_y) (i,j)
**     W是一个二维的低通滤波器。
**
** 5 - 用如下公式计算 (i,j) 处的方向：
**
**              1    -1  / Phi2_y(i,j) \
**     O(i,j) = - tan   |  -----------  |
**              2        \ Phi2_x(i,j) /
**
** 用这个算法可以得到相当平滑的方向图
**
*/

static FvsError_t FingerprintDirectionLowPass(FvsFloat_t* theta,
        FvsFloat_t* out, FvsInt_t nFilterSize,
        FvsInt_t w, FvsInt_t h) {
    FvsError_t nRet = FvsOK;
    FvsFloat_t* filter = NULL;
    FvsFloat_t* phix   = NULL;
    FvsFloat_t* phiy   = NULL;
    FvsFloat_t* phi2x  = NULL;
    FvsFloat_t* phi2y  = NULL;
    FvsInt_t fsize  = nFilterSize * 2 + 1;
    size_t nbytes = (size_t)(w * h * sizeof(FvsFloat_t));
    FvsFloat_t nx, ny;
    FvsInt_t val;
    FvsInt_t i, j, x, y;
    filter = (FvsFloat_t*)malloc((size_t)fsize * fsize * sizeof(FvsFloat_t));
    phix  = (FvsFloat_t*)malloc(nbytes);
    phiy  = (FvsFloat_t*)malloc(nbytes);
    phi2x = (FvsFloat_t*)malloc(nbytes);
    phi2y = (FvsFloat_t*)malloc(nbytes);
    if (filter == NULL || phi2x == NULL || phi2y == NULL || phix == NULL || phiy == NULL)
        nRet = FvsMemory;
    else {
        /* 置 0 */
        memset(filter, 0, (size_t)fsize * fsize * sizeof(FvsFloat_t));
        memset(phix,   0, nbytes);
        memset(phiy,   0, nbytes);
        memset(phi2x,  0, nbytes);
        memset(phi2y,  0, nbytes);
        /* 步骤4 */
        for (y = 0; y < h; y++)
            for (x = 0; x < w; x++) {
                val = x + y * w;
                phix[val] = cos(theta[val]);
                phiy[val] = sin(theta[val]);
            }
        /* 构造低通滤波器 */
        nx = 0.0;
        for (j = 0; j < fsize; j++)
            for (i = 0; i < fsize; i++) {
                filter[j * fsize + i] = 1.0;
                nx += filter[j * fsize + i]; /* 系数和 */
            }
        if (nx > 1.0) {
            for (j = 0; j < fsize; j++)
                for (i = 0; i < fsize; i++)
                    /* 归一化结果 */
                    filter[j * fsize + i] /= nx;
        }
        /* 低通滤波 */
        for (y = 0; y < h - fsize; y++)
            for (x = 0; x < w - fsize; x++) {
                nx = 0.0;
                ny = 0.0;
                for (j = 0; j < fsize; j++)
                    for (i = 0; i < fsize; i++) {
                        val = (x + i) + (j + y) * w;
                        nx += filter[j * fsize + i] * phix[val];
                        ny += filter[j * fsize + i] * phiy[val];
                    }
                val = x + y * w;
                phi2x[val] = nx;
                phi2y[val] = ny;
            }
        /* 销毁 phix, phiy */
        if (phix != NULL) {
            free(phix);
            phix = NULL;
        }
        if (phiy != NULL) {
            free(phiy);
            phiy = NULL;
        }
        /* 步骤5 */
        for (y = 0; y < h - fsize; y++)
            for (x = 0; x < w - fsize; x++) {
                val = x + y * w;
                out[val] = atan2(phi2y[val], phi2x[val]) * 0.5;
            }
    }
    if (phix != NULL)  free(phix);
    if (phiy != NULL)  free(phiy);
    if (phi2x != NULL) free(phi2x);
    if (phi2y != NULL) free(phi2y);
    if (filter != NULL)free(filter);
    return nRet;
}


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
FvsError_t FingerprintGetDirection(const FvsImage_t image,
                                   FvsFloatField_t field, const FvsInt_t nBlockSize,
                                   const FvsInt_t nFilterSize) {
    /* 输入图像的宽度和高度 */
    FvsInt_t w       = ImageGetWidth (image);
    FvsInt_t h       = ImageGetHeight(image);
    FvsInt_t pitch   = ImageGetPitch (image);
    FvsByte_t* p     = ImageGetBuffer(image);
    FvsInt_t i, j, u, v, x, y;
    FvsFloat_t dx[16][16];
    FvsFloat_t dy[16][16];
//    FvsFloat_t dx[(nBlockSize*2+1)][(nBlockSize*2+1)];
//    FvsFloat_t dy[(nBlockSize*2+1)][(nBlockSize*2+1)];
    FvsFloat_t nx, ny;
    FvsFloat_t* out;
    FvsFloat_t* theta  = NULL;
    FvsError_t nRet = FvsOK;
    /* 输出图像 */
    nRet = FloatFieldSetSize(field, w, h);
    if (nRet != FvsOK) return nRet;
    nRet = FloatFieldClear(field);
    if (nRet != FvsOK) return nRet;
    out = FloatFieldGetBuffer(field);
    /* 为方向数组申请内存 */
    if (nFilterSize > 0) {
        theta = (FvsFloat_t*)malloc(w * h * sizeof(FvsFloat_t));
        if (theta != NULL)
            memset(theta, 0, (w * h * sizeof(FvsFloat_t)));
    }
    /* 内存错误，返回 */
    if (out == NULL || (nFilterSize > 0 && theta == NULL))
        nRet = FvsMemory;
    else {
        /* 1 - 图像分块 */
        for (y = nBlockSize + 1; y < h - nBlockSize - 1; y++)
            for (x = nBlockSize + 1; x < w - nBlockSize - 1; x++) {
                /* 2 - 计算梯度 */
                for (j = 0; j < (nBlockSize * 2 + 1); j++)
                    for (i = 0; i < (nBlockSize * 2 + 1); i++) {
                        dx[i][j] = (FvsFloat_t)
                                   (P(x + i - nBlockSize,   y + j - nBlockSize) -
                                    P(x + i - nBlockSize - 1, y + j - nBlockSize));
                        dy[i][j] = (FvsFloat_t)
                                   (P(x + i - nBlockSize,   y + j - nBlockSize) -
                                    P(x + i - nBlockSize,   y + j - nBlockSize - 1));
                    }
                /* 3 - 计算方向 */
                nx = 0.0;
                ny = 0.0;
                for (v = 0; v < (nBlockSize * 2 + 1); v++)
                    for (u = 0; u < (nBlockSize * 2 + 1); u++) {
                        nx += 2 * dx[u][v] * dy[u][v];
                        ny += dx[u][v] * dx[u][v] - dy[u][v] * dy[u][v];
                    }
                /* 计算角度 (-pi/2 .. pi/2) */
                if (nFilterSize > 0)
                    theta[x + y * w] = atan2(nx, ny);
                else
                    out[x + y * w] = atan2(nx, ny) * 0.5;
            }
        if (nFilterSize > 0)
            nRet = FingerprintDirectionLowPass(theta, out, nFilterSize, w, h);
    }
    if (theta != NULL) free(theta);
    return nRet;
}


/* 指纹频率域 */

/******************************************************************************
** 这个步骤里，我们估计指纹脊线的频率。在局部邻域里，没有凸现的细节点或者孤点，
** 沿着脊线和谷底，可以用一个正弦曲线波形作为模型，因此，局部脊线频率是指纹图
** 像的另一个本质的特征。对指纹图像G进行归一化，O是其方向图，估算局部脊线频率
** 的步骤如下：
**
** 1 - 图像分块 w x w - (16 x 16)
**
** 2 - 对每块，计算大小为l x w (32 x 16)的方向图窗口
**
** 3 - 对中心在 (i,j) 的每块, 计算脊线和谷底的 x-signature
**     X[0], X[1], ... X[l-1] 采用如下公式：
**
**              --- w-1
**            1 \
**     X[k] = -  --  G (u, v), k = 0, 1, ..., l-1
**            w /
**              --- d=0
**
**     u = i + (d - w/2).cos O(i,j) + (k - l/2).sin O(i,j)
**
**     v = j + (d - w/2).sin O(i,j) - (k - l/2).cos O(i,j)
**
**     如果方向图窗口中没有细节点和孤立的点，则x-signature形成了一个离散
**     的正弦曲线波，与方向图中脊线和谷底的频率一样。因此，脊线和谷底的
**     频率可以由x-signature来估计。设T(i,j)是两个峰顶的平均距离，则频率
**     OHM(i,j)可以这样计算：OHM(i,j) = 1 / T(i,j)。
**
**     如果没有两个连续的峰顶，则频率置为-1，说明其无效。
**
** 4 - 对于一个指纹图像而言，脊线频率的值在一个范围之内变动，比如说对于500
**     dpi的图像，变动范围为[1/3, 1/25]，因此，如果估计出的频率不在这个范
**     围内，说明频率估计无效，同意置为-1。
**
** 5 - 如果某块有断点或者细节点，则不会有正弦曲线，其频率可以由邻块的频率
**     插值估计（比如说高斯函数，均值为0，方差为9，宽度为7）。
**
** 6 - 脊线内部距离变化缓慢，可以用低通滤波器
**
*/

/* 宽度 */
#define BLOCK_W     16
#define BLOCK_W2     8

/* 长度 */
#define BLOCK_L     32
#define BLOCK_L2    16

#define EPSILON     0.0001
#define LPSIZE      3

#define LPFACTOR    (1.0/((LPSIZE*2+1)*(LPSIZE*2+1)))


FvsError_t FingerprintGetFrequency(const FvsImage_t image, const FvsFloatField_t direction,
                                   FvsFloatField_t frequency) {
    /* 输入图像的宽度和高度 */
    FvsError_t nRet = FvsOK;
    FvsInt_t w      = ImageGetWidth (image);
    FvsInt_t h      = ImageGetHeight(image);
    FvsInt_t pitchi = ImageGetPitch (image);
    FvsByte_t* p    = ImageGetBuffer(image);
    FvsFloat_t* out;
    FvsFloat_t* freq;
    FvsFloat_t* orientation = FloatFieldGetBuffer(direction);
    FvsInt_t x, y, u, v, d, k;
    size_t size;
    if (p == NULL)
        return FvsMemory;
    /* 输出图像的内存申请 */
    nRet = FloatFieldSetSize(frequency, w, h);
    if (nRet != FvsOK) return nRet;
    (void)FloatFieldClear(frequency);
    freq = FloatFieldGetBuffer(frequency);
    if (freq == NULL)
        return FvsMemory;
    /* 输出的内存申请 */
    size = w * h * sizeof(FvsFloat_t);
    out  = (FvsFloat_t*)malloc(size);
    if (out != NULL) {
        FvsFloat_t dir = 0.0;
        FvsFloat_t cosdir = 0.0;
        FvsFloat_t sindir = 0.0;
        FvsInt_t peak_pos[BLOCK_L];		/* 顶点			*/
        FvsInt_t peak_cnt;				/* 顶点数目		*/
        FvsFloat_t peak_freq;			/* 顶点频率		*/
        FvsFloat_t Xsig[BLOCK_L];		/* x signature	*/
        FvsFloat_t pmin, pmax;
        memset(out,  0, size);
        memset(freq, 0, size);
        /* 1 - 图像分块  BLOCK_W x BLOCK_W - (16 x 16) */
        for (y = BLOCK_L2; y < h - BLOCK_L2; y++)
            for (x = BLOCK_L2; x < w - BLOCK_L2; x++) {
                /* 2 - 脊线方向的窗口 l x w (32 x 16) */
//            dir = orientation[(x+BLOCK_W2) + (y+BLOCK_W2)*w];
                dir = orientation[(x) + (y) * w];
                cosdir = cos(dir);
                sindir = sin(dir);
                /* 3 - 计算 x-signature X[0], X[1], ... X[l-1] */
                for (k = 0; k < BLOCK_L; k++) {
                    Xsig[k] = 0.0;
                    for (d = 0; d < BLOCK_W; d++) {
                        u = (FvsInt_t)(x + (k - BLOCK_L2) * cosdir - (d - BLOCK_W2) * sindir);
                        v = (FvsInt_t)(y + (k - BLOCK_L2) * sindir + (d - BLOCK_W2) * cosdir);
                        /* clipping */
                        if (u < 0) u = 0;
                        else if (u > w - 1) u = w - 1;
                        if (v < 0) v = 0;
                        else if (v > h - 1) v = h - 1;
                        Xsig[k] += p[u + (v * pitchi)];
                    }
                    Xsig[k] /= BLOCK_W;
                }
                /* 计算 T(i,j) */
                /* 寻找 x signature 中的顶点 */
                peak_cnt = 0;
                pmax = pmin = Xsig[0];
                for (k = 1; k < BLOCK_L; k++) {
                    if (pmin > Xsig[k]) pmin = Xsig[k];
                    if (pmax < Xsig[k]) pmax = Xsig[k];
                }
                if ((pmax - pmin) > 64.0) {
                    for (k = 1; k < BLOCK_L - 1; k++)
                        if ((Xsig[k - 1] < Xsig[k]) && (Xsig[k] >= Xsig[k + 1])) {
                            peak_pos[peak_cnt++] = k;
                        }
                }
                /* 计算均值 */
                peak_freq = 0.0;
                if (peak_cnt >= 2) {
                    for (k = 0; k < peak_cnt - 1; k++)
                        peak_freq += peak_pos[k + 1] - peak_pos[k];
                    peak_freq /= peak_cnt - 1;
                }
                /* 4 - 验证频率范围 [1/25-1/3] */
                /*     可以扩大到 [1/30-1/2]   */
                if (peak_freq > 30.0)
                    out[x + y * w] = 0.0;
                else if (peak_freq < 2.0)
                    out[x + y * w] = 0.0;
                else
                    out[x + y * w] = 1.0 / peak_freq;
            }
        /* 5 - 未知点 */
        for (y = BLOCK_L2; y < h - BLOCK_L2; y++)
            for (x = BLOCK_L2; x < w - BLOCK_L2; x++) {
                if (out[x + y * w] < EPSILON) {
                    if (out[x + (y - 1)*w] > EPSILON) {
                        out[x + (y * w)] = out[x + (y - 1) * w];
                    }
                    else {
                        if (out[x - 1 + (y * w)] > EPSILON)
                            out[x + (y * w)] = out[x - 1 + (y * w)];
                    }
                }
            }
        /* 6 - 频率插值 */
        for (y = BLOCK_L2; y < h - BLOCK_L2; y++)
            for (x = BLOCK_L2; x < w - BLOCK_L2; x++) {
                k = x + y * w;
                peak_freq = 0.0;
                for ( v = -LPSIZE; v <= LPSIZE; v++)
                    for ( u = -LPSIZE; u <= LPSIZE; u++)
                        peak_freq += out[(x + u) + (y + v) * w];
                freq[k] = peak_freq * LPFACTOR;
            }
        free(out);
    }
    return nRet;
}


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
FvsError_t FingerprintGetMask(const FvsImage_t image,
                              const FvsFloatField_t direction,
                              const FvsFloatField_t frequency, FvsImage_t mask) {
    FvsError_t nRet = FvsOK;
    FvsFloat_t freqmin = 1.0 / 25;
    FvsFloat_t freqmax = 1.0 / 3;
    /* 输入图像的宽度高度 */
    FvsInt_t w      = ImageGetWidth (image);
    FvsInt_t h      = ImageGetHeight(image);
    FvsByte_t* out;
    FvsInt_t pitchout;
    FvsInt_t pos, posout, x, y;
    FvsFloat_t* freq = FloatFieldGetBuffer(frequency);
    if (freq == NULL)
        return FvsMemory;
    /* 需要做改进：检查 */
    nRet = ImageSetSize(mask, w, h);
    if (nRet == FvsOK)
        nRet = ImageClear(mask);
    out = ImageGetBuffer(mask);
    if (out == NULL)
        return FvsMemory;
    if (nRet == FvsOK) {
        pitchout = ImageGetPitch(mask);
        for (y = 0; y < h; y++)
            for (x = 0; x < w; x++) {
                pos    = x + y * w;
                posout = x + y * pitchout;
                out[posout] = 0;
                if (freq[pos] >= freqmin && freq[pos] <= freqmax) {
                    out[posout] = 255;
                }
            }
        /* 补洞 */
        for (y = 0; y < 4; y++)
            (void)ImageDilate(mask);
        /* 去除边界 */
        for (y = 0; y < 12; y++)
            (void)ImageErode(mask);
    }
    return nRet;
}


/* 细化算法 */
#undef P
#define P(x,y)      ((x)+(y)*pitch)
#define REMOVE_P    { p[P(x,y)]=0x80; changed = FvsTrue; }


/******************************************************************************
** 邻域点定义如下:
**     9 2 3
**     8 1 4
**     7 6 5
******************************************************************************/
/* 宏定义 */
#define P1  p[P(x  ,y  )]
#define P2  p[P(x  ,y-1)]
#define P3  p[P(x+1,y-1)]
#define P4  p[P(x+1,y  )]
#define P5  p[P(x+1,y+1)]
#define P6  p[P(x  ,y+1)]
#define P7  p[P(x-1,y+1)]
#define P8  p[P(x-1,y  )]
#define P9  p[P(x-1,y-1)]


FvsError_t ImageRemoveSpurs(FvsImage_t image) {
    FvsInt_t w       = ImageGetWidth(image);
    FvsInt_t h       = ImageGetHeight(image);
    FvsInt_t pitch   = ImageGetPitch(image);
    FvsByte_t* p     = ImageGetBuffer(image);
    FvsInt_t x, y, n, t, c;
    c = 0;
    do {
        n = 0;
        for (y = 1; y < h - 1; y++)
            for (x = 1; x < w - 1; x++) {
                if( p[P(x, y)] == 0xFF) {
                    t = 0;
                    if (P3 == 0 && P2 != 0 && P4 == 0) t++;
                    if (P5 == 0 && P4 != 0 && P6 == 0) t++;
                    if (P7 == 0 && P6 != 0 && P8 == 0) t++;
                    if (P9 == 0 && P8 != 0 && P2 == 0) t++;
                    if (P3 != 0 && P4 == 0) t++;
                    if (P5 != 0 && P6 == 0) t++;
                    if (P7 != 0 && P8 == 0) t++;
                    if (P9 != 0 && P2 == 0) t++;
                    if (t == 1) {
                        p[P(x, y)] = 0x80;
                        n++;
                    }
                }
            }
        for (y = 1; y < h - 1; y++)
            for (x = 1; x < w - 1; x++) {
                if( p[P(x, y)] == 0x80)
                    p[P(x, y)] = 0;
            }
    }
    while (n > 0 && ++c < 5);
    return FvsOK;
}


/* a) 验证其有2-6个邻点 */
#define STEP_A  n = 0; /* 邻点个数 */ \
                if (P2!=0) n++; if (P3!=0) n++; if (P4!=0) n++; if (P5!=0) n++; \
                if (P6!=0) n++; if (P7!=0) n++; if (P8!=0) n++; if (P9!=0) n++; \
                if (n>=2 && n<=6)

/* b) 统计由0变1的个数 */
#define STEP_B  t = 0; /* 变化的数目 */ \
                if (P9==0 && P2!=0) t++; if (P2==0 && P3!=0) t++; \
                if (P3==0 && P4!=0) t++; if (P4==0 && P5!=0) t++; \
                if (P5==0 && P6!=0) t++; if (P6==0 && P7!=0) t++; \
                if (P7==0 && P8!=0) t++; if (P8==0 && P9!=0) t++; \
                if (t==1)

/******************************************************************************
  * 功能：细化指纹图像
  *       图像必须是二值化过的（只包含0x00或oxFF）
  *       该算法基于领域的判断，决定某个象素该移去还是保留
  * 参数：image   指纹图像
  * 返回：错误编号
******************************************************************************/
FvsError_t ImageThinConnectivity(FvsImage_t image) {
    FvsInt_t w       = ImageGetWidth(image);
    FvsInt_t h       = ImageGetHeight(image);
    FvsInt_t pitch   = ImageGetPitch(image);
    FvsByte_t* p     = ImageGetBuffer(image);
    FvsInt_t x, y, n, t;
    FvsBool_t changed = FvsTrue;
    if (p == NULL)
        return FvsMemory;
    if (ImageGetFlag(image) != FvsImageBinarized)
        return FvsBadParameter;
    while (changed == FvsTrue) {
        changed = FvsFalse;
        for (y = 1; y < h - 1; y++)
            for (x = 1; x < w - 1; x++) {
                if (p[P(x, y)] == 0xFF) {
                    STEP_A {
                        STEP_B
                        {
                            /*
                            c) 2*4*6=0  (2,4 ,or 6 为0)
                            d) 4*6*8=0
                            */
                            if (P2*P4 * P6 == 0 && P4*P6 * P8 == 0)
                                REMOVE_P;

                        }
                    }
                }
            }
        for (y = 1; y < h - 1; y++)
            for (x = 1; x < w - 1; x++)
                if (p[P(x, y)] == 0x80)
                    p[P(x, y)] = 0;
        for (y = 1; y < h - 1; y++)
            for (x = 1; x < w - 1; x++) {
                if (p[P(x, y)] == 0xFF) {
                    STEP_A {
                        STEP_B
                        {
                            /*
                            c) 2*6*8=0
                            d) 2*4*8=0
                            */
                            if (P2*P6 * P8 == 0 && P2*P4 * P8 == 0)
                                REMOVE_P;

                        }
                    }
                }
            }
        for (y = 1; y < h - 1; y++)
            for (x = 1; x < w - 1; x++)
                if (p[P(x, y)] == 0x80)
                    p[P(x, y)] = 0;
    }
    ImageRemoveSpurs(image);
    return ImageSetFlag(image, FvsImageThinned);
}



/* 重新定义 REMOVE_P */
#undef REMOVE_P


#define REMOVE_P    { p[P(x,y)]=0x00; changed = FvsTrue; }


/******************************************************************************
  * 功能：细化指纹图像，使用“Hit and Miss”结构元素。
  *       图像必须是二值化过的（只包含0x00或oxFF）
  *       该算法的缺点是产生很多伪造的线条（伪特征），
  *       必须由另外的算法来消除，后处理非常必要。
  * 参数：image   指纹图像
  * 返回：错误编号
******************************************************************************/
FvsError_t ImageThinHitMiss(FvsImage_t image) {
    FvsInt_t w      = ImageGetWidth(image);
    FvsInt_t h      = ImageGetHeight(image);
    FvsInt_t pitch  = ImageGetPitch(image);
    FvsByte_t* p    = ImageGetBuffer(image);
    /*
    //
    // 0 0 0      0 0
    //   1      1 1 0
    // 1 1 1      1
    //
    */
    FvsInt_t x, y;
    FvsBool_t changed = FvsTrue;
    if (p == NULL)
        return FvsMemory;
    if (ImageGetFlag(image) != FvsImageBinarized)
        return FvsBadParameter;
    while (changed == FvsTrue) {
        changed = FvsFalse;
        for (y = 1; y < h - 1; y++)
            for (x = 1; x < w - 1; x++) {
                if (p[P(x, y)] == 0xFF) {
                    /*
                    // 0 0 0  0   1  1 1 1  1   0
                    //   1    0 1 1    1    1 1 0
                    // 1 1 1  0   1  0 0 0  1   0
                    */
                    if (p[P(x - 1, y - 1)] == 0 && p[P(x, y - 1)] == 0 && p[P(x + 1, y - 1)] == 0 &&
                            p[P(x - 1, y + 1)] != 0 && p[P(x, y + 1)] != 0 && p[P(x + 1, y + 1)] != 0)
                        REMOVE_P;
                    if (p[P(x - 1, y - 1)] != 0 && p[P(x, y - 1)] != 0 && p[P(x + 1, y - 1)] != 0 &&
                            p[P(x - 1, y + 1)] == 0 && p[P(x, y + 1)] == 0 && p[P(x + 1, y + 1)] == 0)
                        REMOVE_P;
                    if (p[P(x - 1, y - 1)] == 0 && p[P(x - 1, y)] == 0 && p[P(x - 1, y + 1)] == 0 &&
                            p[P(x + 1, y - 1)] != 0 && p[P(x + 1, y)] != 0 && p[P(x + 1, y + 1)] != 0)
                        REMOVE_P;
                    if (p[P(x - 1, y - 1)] != 0 && p[P(x - 1, y)] != 0 && p[P(x - 1, y + 1)] != 0 &&
                            p[P(x + 1, y - 1)] == 0 && p[P(x + 1, y)] == 0 && p[P(x + 1, y + 1)] == 0)
                        REMOVE_P;
                    /*
                    //   0 0  0 0      1      1
                    // 1 1 0  0 1 1  0 1 1  1 1 0
                    //   1      1    0 0      0 0
                    */
                    if (p[P(x, y - 1)] == 0 && p[P(x + 1, y - 1)] == 0 && p[P(x + 1, y)] == 0 &&
                            p[P(x - 1, y)] != 0 && p[P(x, y + 1)] != 0)
                        REMOVE_P;
                    if (p[P(x - 1, y - 1)] == 0 && p[P(x, y - 1)] == 0 && p[P(x - 1, y)] == 0 &&
                            p[P(x + 1, y)] != 0 && p[P(x, y + 1)] != 0)
                        REMOVE_P;
                    if (p[P(x - 1, y + 1)] == 0 && p[P(x - 1, y)] == 0 && p[P(x, y + 1)] == 0 &&
                            p[P(x + 1, y)] != 0 && p[P(x, y - 1)] != 0)
                        REMOVE_P;
                    if (p[P(x + 1, y + 1)] == 0 && p[P(x + 1, y)] == 0 && p[P(x, y + 1)] == 0 &&
                            p[P(x - 1, y)] != 0 && p[P(x, y - 1)] != 0)
                        REMOVE_P;
                }
            }
    }
    ImageRemoveSpurs(image);
    return ImageSetFlag(image, FvsImageThinned);
}


/* modified
*/
FvsError_t FingerprintGetFrequency1(const FvsImage_t image, const FvsFloatField_t direction,
                                    FvsFloatField_t frequency) {
    /* 输入图像的宽度和高度 */
    FvsError_t nRet = FvsOK;
    FvsInt_t w      = ImageGetWidth (image);
    FvsInt_t h      = ImageGetHeight(image);
    FvsInt_t pitchi = ImageGetPitch (image);
    FvsByte_t* p    = ImageGetBuffer(image);
    FvsFloat_t* out;
    FvsFloat_t* freq;
    FvsFloat_t* orientation = FloatFieldGetBuffer(direction);
    FvsFloat_t dir, dir1, dir2;
    FvsFloat_t cosdir, sindir, cosdir1, sindir1, cosdir2, sindir2;
    FvsInt_t x, y, u, v, d, k;
    size_t size;
    if (p == NULL)
        return FvsMemory;
    /* 输出图像的内存申请 */
    nRet = FloatFieldSetSize(frequency, w, h);
    if (nRet != FvsOK) return nRet;
    (void)FloatFieldClear(frequency);
    freq = FloatFieldGetBuffer(frequency);
    if (freq == NULL)
        return FvsMemory;
    /* 输出的内存申请 */
    size = w * h * sizeof(FvsFloat_t);
    out  = (FvsFloat_t*)malloc(size);
    if (out != NULL) {
        FvsInt_t peak_pos[BLOCK_L];		/* 顶点			*/
        FvsInt_t peak_cnt;				/* 顶点数目		*/
        FvsFloat_t peak_freq, save[50];			/* 顶点频率		*/
        FvsFloat_t Xsig[BLOCK_L];		/* x signature	*/
        FvsFloat_t pmin, pmax;
        memset(out,  0, size);
        memset(freq, 0, size);
        /* 1 - 图像分块  BLOCK_W x BLOCK_W - (16 x 16) */
        for (y = BLOCK_L2; y < h - BLOCK_L2; y++)
            for (x = BLOCK_L2; x < w - BLOCK_L2; x++) {
                /* 2 - 脊线方向的窗口 l x w (32 x 16) */
                dir = orientation[x + y * w];
                cosdir = cos(dir);
                sindir = sin(dir);
                u = (FvsInt_t)(-sindir * BLOCK_L2 / 2) + x;
                v = (FvsInt_t)(cosdir * BLOCK_L2 / 2) + y;
                dir1 = orientation[u + v * w];
                cosdir1 = cos(dir1);
                sindir1 = sin(dir1);
                u = (FvsInt_t)(sindir * BLOCK_L2 / 2) + x;
                v = (FvsInt_t)(-cosdir * BLOCK_L2 / 2) + y;
                dir2 = orientation[u + v * w];
                cosdir2 = cos(dir2);
                sindir2 = sin(dir2);
                /* 3 - 计算 x-signature X[0], X[1], ... X[l-1] */
                for (k = 0; k < BLOCK_L; k++) {
                    Xsig[k] = 0.0;
                    for (d = 0; d < BLOCK_W; d++) {
                        if(d - BLOCK_W2 > 0) {
                            u = (FvsInt_t)(x + (k - BLOCK_L2) * cosdir1 - (d - BLOCK_W2) * sindir1);
                            v = (FvsInt_t)(y + (k - BLOCK_L2) * sindir1 + (d - BLOCK_W2) * cosdir1);
                        }
                        else {
                            u = (FvsInt_t)(x + (k - BLOCK_L2) * cosdir2 - (d - BLOCK_W2) * sindir2);
                            v = (FvsInt_t)(y + (k - BLOCK_L2) * sindir2 + (d - BLOCK_W2) * cosdir2);
                        }
                        /* clipping */
                        if (u < 0) u = 0;
                        else if (u > w - 1) u = w - 1;
                        if (v < 0) v = 0;
                        else if (v > h - 1) v = h - 1;
                        Xsig[k] += p[u + (v * pitchi)];
                    }
                    Xsig[k] /= BLOCK_W;
                }
                /* 计算 T(i,j) */
                /* 寻找 x signature 中的顶点 */
                peak_cnt = 0;
                pmax = pmin = Xsig[0];
                for (k = 1; k < BLOCK_L; k++) {
                    if (pmin > Xsig[k]) pmin = Xsig[k];
                    if (pmax < Xsig[k]) pmax = Xsig[k];
                }
                if ((pmax - pmin) > 64.0) {
                    for (k = 1; k < BLOCK_L - 1; k++)
                        if ((Xsig[k - 1] < Xsig[k]) && (Xsig[k] >= Xsig[k + 1])) {
                            peak_pos[peak_cnt++] = k;
                        }
                }
                /* 计算均值 */
                peak_freq = 0.0;
                if (peak_cnt > 2) {
                    for (k = 0; k < peak_cnt - 1; k++)
                        peak_freq += peak_pos[k + 1] - peak_pos[k];
                    peak_freq /= peak_cnt - 1;
                }
                /* 4 - 验证频率范围 [1/25-1/3] */
                /*     可以扩大到 [1/30-1/2]   */
                if (peak_freq > 25.0 && peak_freq < 3.0)
                    peak_freq = 0.0;
                if (peak_freq == 0.0)
                    out[x + y * w] = 0.0;
                else
                    out[x + y * w] = 1.0 / peak_freq;
                if(x < 230 && x > 220 && y == 46)
                    x = x;
            }
        /* 5 - 未知点 */
        /*        for (y = BLOCK_L2; y < h-BLOCK_L2; y++)
                for (x = BLOCK_L2; x < w-BLOCK_L2; x++)
                {
                    if (out[x+y*w]<EPSILON)
                    {
                        if (out[x+(y-1)*w]>EPSILON)
                        {
                            out[x+(y*w)] = out[x+(y-1)*w];
                        }
                        else
                        {
                            if (out[x-1+(y*w)]>EPSILON)
                                out[x+(y*w)] = out[x-1+(y*w)];
                        }
                    }
                }
                /* 6 - 频率插值 */
        for (y = BLOCK_L2; y < h - BLOCK_L2; y++)
            for (x = BLOCK_L2; x < w - BLOCK_L2; x++) {
                k = x + y * w;
                peak_freq = 0.0;
                for ( v = -LPSIZE; v <= LPSIZE; v++)
                    for ( u = -LPSIZE; u <= LPSIZE; u++) {
                        save[(v + LPSIZE) * (LPSIZE * 2 + 1) + u + LPSIZE] = out[(x + u) + (y + v) * w];
                        peak_freq += out[(x + u) + (y + v) * w];
                    }
                freq[k] = peak_freq * LPFACTOR;
                if(x < 230 && x > 220 && y == 46)
                    x = x;
            }
        free(out);
    }
    return nRet;
}


/* modified
*/
struct mycomplex {
    FvsFloat_t real;
    FvsFloat_t imag;
};


static void fft(FvsFloat_t data[32]) {
    FvsInt_t i, j, k, bfsize, p, count, r = 5;
    FvsFloat_t	angle;
    struct mycomplex *w, *x1, *x2, *x;
    count = 1 << r;
    w = (struct mycomplex*)malloc(sizeof(struct mycomplex) * count / 2);
    x1 = (struct mycomplex*)malloc(sizeof(struct mycomplex) * count);
    x2 = (struct mycomplex*)malloc(sizeof(struct mycomplex) * count);
    for(i = 0; i < count / 2; i++) {
        angle = -i * M_PI * 2 / count;
        w[i].real = cos(angle);
        w[i].imag = sin(angle);
    }
    for(j = 0; j < count; j++) {
        p = 0;
        for(i = 0; i < r; i++) {
            if(j & (1 << i)) {
                p += 1 << (r - i - 1);
            }
        }
        x1[p].real = data[j];
        x1[p].imag = 0;
    }
    for(k = 0; k < r; k++) {
        bfsize = 1 << (k + 1);
        for(j = 0; j < 1 << (r - k - 1); j++) {
            for(i = 0; i < bfsize / 2; i++) {
                p = j * bfsize;
                x2[i + p].real = x1[i + p].real + x1[i + p + bfsize / 2].real * w[i * 1 << (r - k - 1)].real \
                                 -x1[i + p + bfsize / 2].imag * w[i * 1 << (r - k - 1)].imag;
                x2[i + p].imag = x1[i + p].imag + x1[i + p + bfsize / 2].imag * w[i * 1 << (r - k - 1)].real \
                                 +x1[i + p + bfsize / 2].real * w[i * 1 << (r - k - 1)].imag;
                x2[i + p + bfsize / 2].real = x1[i + p].real - x1[i + p + bfsize / 2].real * w[i * 1 << (r - k - 1)].real \
                                              +x1[i + p + bfsize / 2].imag * w[i * 1 << (r - k - 1)].imag;
                x2[i + p + bfsize / 2].imag = x1[i + p].imag - x1[i + p + bfsize / 2].imag * w[i * 1 << (r - k - 1)].real \
                                              -x1[i + p + bfsize / 2].real * w[i * 1 << (r - k - 1)].imag;
            }
        }
        x = x1;
        x1 = x2;
        x2 = x;
    }
    for(j = 0; j < count; j++) {
        data[j] = sqrt(x1[j].real * x1[j].real + x1[j].imag * x1[j].imag) / 10;
    }
    free(w);
    free(x1);
    free(x2);
}

FvsError_t FingerprintGetFrequency2(const FvsImage_t image, const FvsFloatField_t direction,
                                    FvsFloatField_t frequency) {
    /* 输入图像的宽度和高度 */
    FvsError_t nRet = FvsOK;
    FvsInt_t w      = ImageGetWidth (image);
    FvsInt_t h      = ImageGetHeight(image);
    FvsInt_t pitchi = ImageGetPitch (image);
    FvsByte_t* p    = ImageGetBuffer(image);
    FvsFloat_t* out;
    FvsFloat_t* freq;
    FvsFloat_t* orientation = FloatFieldGetBuffer(direction);
    FvsInt_t x, y, u, v, d, k;
    size_t size;
    if (p == NULL)
        return FvsMemory;
    /* 输出图像的内存申请 */
    nRet = FloatFieldSetSize(frequency, w, h);
    if (nRet != FvsOK) return nRet;
    (void)FloatFieldClear(frequency);
    freq = FloatFieldGetBuffer(frequency);
    if (freq == NULL)
        return FvsMemory;
    /* 输出的内存申请 */
    size = w * h * sizeof(FvsFloat_t);
    out  = (FvsFloat_t*)malloc(size);
    if (out != NULL) {
        FvsFloat_t dir = 0.0;
        FvsFloat_t cosdir = 0.0;
        FvsFloat_t sindir = 0.0;
        FvsInt_t peak_pos[BLOCK_L];		/* 顶点			*/
        FvsInt_t peak_cnt;				/* 顶点数目		*/
        FvsFloat_t peak_freq;			/* 顶点频率		*/
        FvsFloat_t Xsig[BLOCK_L];		/* x signature	*/
        FvsFloat_t pmin, pmax;
        memset(out,  0, size);
        memset(freq, 0, size);
        /* 1 - 图像分块  BLOCK_W x BLOCK_W - (16 x 16) */
        for (y = BLOCK_L2; y < h - BLOCK_L2; y++)
            for (x = BLOCK_L2; x < w - BLOCK_L2; x++) {
                /* 2 - 脊线方向的窗口 l x w (32 x 16) */
//            dir = orientation[(x+BLOCK_W2) + (y+BLOCK_W2)*w];
                dir = orientation[(x) + (y) * w];
                cosdir = cos(dir);
                sindir = sin(dir);
                /* 3 - 计算 x-signature X[0], X[1], ... X[l-1] */
                for (k = 0; k < BLOCK_L; k++) {
                    Xsig[k] = 0.0;
                    for (d = 0; d < BLOCK_W; d++) {
                        u = (FvsInt_t)(x + (k - BLOCK_L2) * cosdir - (d - BLOCK_W2) * sindir);
                        v = (FvsInt_t)(y + (k - BLOCK_L2) * sindir + (d - BLOCK_W2) * cosdir);
                        /* clipping */
                        if (u < 0) u = 0;
                        else if (u > w - 1) u = w - 1;
                        if (v < 0) v = 0;
                        else if (v > h - 1) v = h - 1;
                        Xsig[k] += p[u + (v * pitchi)];
                    }
                    Xsig[k] /= BLOCK_W;
                }
                if(x == 130 && y == 100)
                    x = x;
//			for(k=0;k<32;k++)
//				Xsig[k]=cos(2*M_PI*7*k/32)+cos(2*M_PI*3*k/32);
                fft(Xsig);
                if (peak_freq > 30.0)
                    out[x + y * w] = 0.0;
                else if (peak_freq < 2.0)
                    out[x + y * w] = 0.0;
                else
                    out[x + y * w] = 1.0 / peak_freq;
            }
        /* 5 - 未知点 */
        for (y = BLOCK_L2; y < h - BLOCK_L2; y++)
            for (x = BLOCK_L2; x < w - BLOCK_L2; x++) {
                if (out[x + y * w] < EPSILON) {
                    if (out[x + (y - 1)*w] > EPSILON) {
                        out[x + (y * w)] = out[x + (y - 1) * w];
                    }
                    else {
                        if (out[x - 1 + (y * w)] > EPSILON)
                            out[x + (y * w)] = out[x - 1 + (y * w)];
                    }
                }
            }
        /* 6 - 频率插值 */
        for (y = BLOCK_L2; y < h - BLOCK_L2; y++)
            for (x = BLOCK_L2; x < w - BLOCK_L2; x++) {
                k = x + y * w;
                peak_freq = 0.0;
                for ( v = -LPSIZE; v <= LPSIZE; v++)
                    for ( u = -LPSIZE; u <= LPSIZE; u++)
                        peak_freq += out[(x + u) + (y + v) * w];
                freq[k] = peak_freq * LPFACTOR;
            }
        free(out);
    }
    return nRet;
}


