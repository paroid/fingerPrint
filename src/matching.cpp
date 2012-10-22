/*#############################################################################
 * 文件名：matching.cpp
 * 功能：  实现了指纹匹配算法
#############################################################################*/

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "matching.h"

#define REF_X (FvsInt_t) 0
#define REF_Y (FvsInt_t) 0
#define REF_THETA (FvsFloat_t) 0.0
#define PI (double)3.14285714285714285714285714285714


typedef struct Fvs_PolarMinutia_t {
    /* r位置 */
    FvsFloat_t    r;
    /* e位置 */
    FvsFloat_t    e;
    /* 角度  */
    FvsFloat_t    angle;

} Fvs_PolarMinutia_t;

FvsError_t Insertion_Sort(Fvs_PolarMinutia_t*, FvsInt_t) ;
FvsError_t fInsertion_Sort(FvsFloat_t* v, FvsInt_t v_length);


Fvs_PolarMinutia_t p_polar_input[100];
Fvs_PolarMinutia_t p_polar_tmplt[100];

Fvs_PolarMinutia_t s_polar_input[100];
Fvs_PolarMinutia_t s_polar_tmplt[100];


/******************************************************************************
  * 功能：匹配两个指纹
  * 参数：image1      指纹图像1
  *       image2      指纹图像2
  *       pgoodness   匹配度，越高越好
  * 返回：错误编号
******************************************************************************/
FvsError_t MatchingCompareImages
(
    const FvsImage_t image1,
    const FvsImage_t image2,
    FvsInt_t* pgoodness
) {
    /* ！！该算法尚未实现，读者可以自己试着实现！！ */
    return FvsOK;
}


# define ALPHA (float) 1.0
# define BETA (float) 2.0
# define GAMMA (float) 0.1
# define OHM (float) (200.0 * (ALPHA + BETA + GAMMA))
# define ETA (float) 0.5

# define DEL_LO_M_N (float) -8.0
# define DEL_HI_M_N (float) +8.0
# define SI_LO_M_N (float) -7.5
# define SI_HI_M_N (float) +7.5

# define DEL_REF (float) (DEL_HI_M_N - DEL_LO_M_N)
# define SI_REF  (float) (SI_HI_M_N  - SI_LO_M_N )
# define EPS_REF (int) 30
# define EDIT_DIST_THRESHOLD (int) 10


/******************************************************************************
  * 功能：匹配指纹细节点
  * 参数：minutia1      细节点集合1
  *       minutia2      细节点集合2
  *       pgoodness   匹配度，越高越好
  * 返回：错误编号
******************************************************************************/
FvsError_t MatchingCompareMinutiaSets (
    const FvsMinutiaSet_t set1,
    const FvsMinutiaSet_t set2,
    FvsInt_t* pgoodness
) {
#define MM 100
    FvsInt_t i = 0, n = 0, m = 0;
    FvsInt_t ix, tx;
    FvsInt_t iy, ty;
    FvsFloat_t itheta, ttheta;
    FvsFloat_t ir, tr;
    FvsFloat_t ie, te;
    FvsFloat_t ftmp1, ftmp2, ftmp3 = 0.0;
    FvsInt_t ltmp1 = 0, ltmp2 = 0, ltmp3 = 0;
    FvsFloat_t fatmp1[MM];
    FvsFloat_t fatmp2[MM];
    FvsFloat_t edit_dist[MM][MM];
    // 距离计算之用
    FvsFloat_t diff_r, diff_e, diff_theta, ftmp;
    FvsFloat_t window_mn, a;
    FvsFloat_t edit_dist_m1_n;
    FvsFloat_t edit_dist_m_n1;
    FvsFloat_t edit_dist_m1_n1;
    FvsFloat_t edit_dist_m_n = (float) 0.0;
    int window_flag;
    int nb_pair;
    int nb_minutiae;
    float Mpq;
    FvsMinutia_t* input_minutia = MinutiaSetGetBuffer(set1);
    FvsInt_t nb_input_minutia   = MinutiaSetGetCount(set1);
    FvsMinutia_t* tmplt_minutia = MinutiaSetGetBuffer(set2);
    FvsInt_t nb_tmplt_minutia   = MinutiaSetGetCount(set2);
    if (input_minutia == NULL)
        return FvsMemory;
    if (tmplt_minutia == NULL)
        return FvsMemory;
    printf("\nNumber of minutiae in the input image is = %d", nb_input_minutia);
    printf("\nNumber of minutiae in the tmplt image is = %d", nb_tmplt_minutia);
    for (n = 0; n < nb_input_minutia; n++) {
        ix = (FvsInt_t)input_minutia[n].x;
        iy = (FvsInt_t)input_minutia[n].y;
        itheta = (FvsFloat_t) input_minutia[n].angle * (180 / PI);
        ftmp1 = (FvsFloat_t) (ix - REF_X) * (ix - REF_X);
        ftmp2 = (FvsFloat_t) (iy - REF_Y) * (iy - REF_Y);
        p_polar_input[n].r = sqrt (ftmp1 + ftmp2);
        ftmp1 = (FvsFloat_t) (ix - REF_X);
        ftmp2 = (FvsFloat_t) (iy - REF_Y);
        p_polar_input[n].e = (FvsFloat_t) atan (ftmp2 / ftmp1);
        p_polar_input[n].angle = (itheta - REF_THETA);
    }
    for (n = 0; n < nb_tmplt_minutia; n++) {
        tx = (FvsInt_t)tmplt_minutia[n].x;
        ty = (FvsInt_t)tmplt_minutia[n].y;
        ttheta = (FvsFloat_t) tmplt_minutia[n].angle * (180 / PI);
        ftmp1 = (FvsFloat_t) (tx - REF_X) * (tx - REF_X);
        ftmp2 = (FvsFloat_t) (ty - REF_Y) * (ty - REF_Y);
        p_polar_tmplt[n].r = sqrt (ftmp1 + ftmp2);
        ftmp1 = (FvsFloat_t) (tx - REF_X);
        ftmp2 = (FvsFloat_t) (ty - REF_Y);
        p_polar_tmplt[n].e = (FvsFloat_t) atan (ftmp2 / ftmp1);
        p_polar_tmplt[n].angle = (ttheta - REF_THETA);
    }
    /* 升序排列细节点 */
    for (n = 0; n < nb_input_minutia; n++) {
        s_polar_input[n].r = p_polar_input[n].r;
        s_polar_input[n].e = p_polar_input[n].e;
        s_polar_input[n].angle = p_polar_input[n].angle;
        fatmp1[n] = p_polar_input[n].angle;
        fatmp2[n] = fatmp1[n];
    }
    Insertion_Sort (&s_polar_input[0], nb_input_minutia);
    for (n = 0; n < nb_input_minutia; n++) {
        s_polar_tmplt[n].r = p_polar_tmplt[n].r;
        s_polar_tmplt[n].e = p_polar_tmplt[n].e;
        s_polar_tmplt[n].angle = p_polar_tmplt[n].angle;
        fatmp1[n] = p_polar_tmplt[n].angle;
        fatmp2[n] = fatmp1[n];
    }
    Insertion_Sort (&s_polar_tmplt[0], nb_tmplt_minutia);
    //  如果 m = 0 或 n = 0，则edit ditance = 0
    for (m = 0; m < nb_tmplt_minutia; m++)
        edit_dist[m][0] = 0.0;
    for (n = 0; n < nb_input_minutia; n++)
        edit_dist[0][n] = 0.0;
    for (m = 1; m <= nb_tmplt_minutia; m++) {
        tr = s_polar_tmplt[m].r;
        te = s_polar_tmplt[m].e;
        ttheta = s_polar_tmplt[m].angle;
        for (n = 1; n <= nb_input_minutia; n++) {
            ir = s_polar_input[n].r;
            ie = s_polar_input[n].e;
            itheta = s_polar_input[n].angle;
            // 计算窗口函数 w(m,n)
            // 1. 计算差分 r
            diff_r = tr - ir;
            // 2. 计算差分 e
            ftmp = te - ie;
            ftmp += 360.0;
            // a = (ie - te + 360) mod 360
            if (ftmp > 360.0) {
                a = ftmp - 360.0;
            }
            else {
                a = 360.0 - ftmp;
            }
            if (a < 180.0) {
                diff_e = a;
            }
            else {
                diff_e = a - 180.0;
            }
            // 3. 计算差分 theta
            ftmp = ttheta - itheta;
            ftmp += 360.0;
            // a = (itheta - ttheta + 360) mod 360
            if (ftmp > 360.0) {
                a = ftmp - 360.0;
            }
            else {
                a = 360.0 - ftmp;
            }
            if (a < 180.0) {
                diff_theta = a;
            }
            else {
                diff_theta = a - 180.0;
            }
            // 计算窗口函数 w(m,n)
            window_flag = 0;
            if((diff_r < DEL_REF) | (diff_e < SI_REF) | \
                    (diff_theta < EPS_REF)) {
                window_flag = 1;
            }
            if (window_flag == 1) {
                window_mn = (ALPHA * diff_r);
                window_mn += (BETA * diff_e);
                window_mn += (GAMMA * diff_theta);  // w(m,n)
            }
            else {
                window_mn = OHM;                    // w(m,n)
            }
            //=========================
            // 计算 edit distance:
            //=========================
            // edit_dist[m][n] = ?
            //=========================
            edit_dist_m1_n  = edit_dist[m - 1][n + 0] + OHM;
            edit_dist_m_n1  = edit_dist[m + 0][n - 1] + OHM;
            edit_dist_m1_n1 = edit_dist[m - 1][n - 1] + window_mn;
            edit_dist_m_n = 0.0;
            if (edit_dist_m1_n < edit_dist_m_n1)
                edit_dist_m_n = edit_dist_m1_n;
            else
                edit_dist_m_n = edit_dist_m_n1;
            if (edit_dist_m1_n1 < edit_dist_m_n)
                edit_dist_m_n = edit_dist_m1_n1;
            edit_dist[m][n] = edit_dist_m_n;
        }
    }
    nb_pair = 0;
    for (m = 1; m <= nb_tmplt_minutia; m++) {
        if (edit_dist[m][m] < (float) EDIT_DIST_THRESHOLD)
            nb_pair++;
    }
    nb_minutiae = nb_tmplt_minutia;
    if (nb_input_minutia < nb_tmplt_minutia)
        nb_minutiae = nb_input_minutia;
    Mpq = ((float)100.0 * (float)nb_pair) / (float)nb_minutiae;
    if (Mpq > 70.0)// 超过70%的细节点匹配
        printf ("\nTemplated matched with input");
    else
        printf ("\nTemplated not matched with input");
    *pgoodness = Mpq;
    printf ("\nmatching application !!!\n\n\n\n");
    return FvsOK;
}

/* 插入排序函数 */
FvsError_t Insertion_Sort (
    Fvs_PolarMinutia_t* v,
    FvsInt_t v_length
) {
    FvsFloat_t item_to_insert1;
    FvsFloat_t item_to_insert2;
    FvsFloat_t item_to_insert3;
    int still_looking, j, k;
    for (k = 1; k < v_length; ++k) {
        item_to_insert1 = v[k].angle;
        item_to_insert2 = v[k].r;
        item_to_insert3 = v[k].e;
        j = k - 1;
        still_looking = 1;
        while ((j >= 0) && (still_looking == 1))  {
            if (item_to_insert1 < v[j].angle) {
                v[j + 1].angle = v[j].angle;
                v[j + 1].r = v[j].r;
                v[j + 1].e = v[j].e;
                v[j].angle = item_to_insert1;
                v[j].r = item_to_insert2;
                v[j].e = item_to_insert3;
                --j;
            }
            else  {
                still_looking = 0;
                v[j + 1].angle = item_to_insert1;
                v[j + 1].r = item_to_insert2;
                v[j + 1].e = item_to_insert3;
            }
        }
    }
    return FvsOK;
}

