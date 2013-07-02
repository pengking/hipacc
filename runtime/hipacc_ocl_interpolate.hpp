//
// Copyright (c) 2012, University of Erlangen-Nuremberg
// Copyright (c) 2012, Siemens AG
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
// ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

#ifndef __HIPACC_OCL_INTERPOLATE_HPP__
#define __HIPACC_OCL_INTERPOLATE_HPP__

__constant sampler_t interpolationSampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;

#define IMG_PARM(TYPE) __global const TYPE *img
#define ARR_PARM(TYPE) __read_only image2d_t img
#define CONST_PARM(TYPE) , const TYPE const_val
#define NO_PARM(TYPE)
#define IMG(idx_x, idx_y, stride, const_val, method) img[(idx_x) + (idx_y)*(stride)]
#define ARR(idx_x, idx_y, stride, const_val, method) method(img, interpolationSampler, (int2)(idx_x, idx_y)).x
#define IMG_CONST(idx_x, idx_y, stride, const_val, method) ((idx_x)<0||(idx_y)<0)<0?const_val:img[(idx_x) + (idx_y)*(stride)]
#define ARR_CONST(idx_x, idx_y, stride, const_val, method) ((idx_x)<0||(idx_y)<0)<0?const_val:method(img, interpolationSampler, (int2)(idx_x, idx_y)).x

// border handling: CLAMP
#define BH_CLAMP_LOWER(idx, lower, stride) bh_clamp_lower(idx, lower)
#define BH_CLAMP_UPPER(idx, upper, stride) bh_clamp_upper(idx, upper)
inline int bh_clamp_lower(int idx, int lower) {
    if (idx < lower) idx = lower;
    return idx;
}
inline int bh_clamp_upper(int idx, int upper) {
    if (idx >= upper) idx = upper-1;
    return idx;
}

// border handling: REPEAT
#define BH_REPEAT_LOWER(idx, lower, stride) bh_repeat_lower(idx, lower, stride)
#define BH_REPEAT_UPPER(idx, upper, stride) bh_repeat_upper(idx, upper, stride)
inline int bh_repeat_lower(int idx, int lower, int stride) {
    while (idx < lower) idx += stride;
    return idx;
}
inline int bh_repeat_upper(int idx, int upper, int stride) {
    while (idx >= upper) idx -= stride;
    return idx;
}

// border handling: MIRROR
#define BH_MIRROR_LOWER(idx, lower, stride) bh_mirror_lower(idx, lower)
#define BH_MIRROR_UPPER(idx, upper, stride) bh_mirror_upper(idx, upper)
inline int bh_mirror_lower(int idx, int lower) {
    if (idx < lower) idx = lower + (lower - idx-1);
    return idx;
}
inline int bh_mirror_upper(int idx, int upper) {
    if (idx >= upper) idx = upper - (idx+1 - upper);
    return idx;
}

// border handling: CONSTANT
#define BH_CONSTANT_LOWER(idx, lower, stride) bh_constant_lower(idx, lower)
#define BH_CONSTANT_UPPER(idx, upper, stride) bh_constant_upper(idx, upper)
inline int bh_constant_lower(int idx, int lower) {
    if (idx < lower) return -1;
    return idx;
}
inline int bh_constant_upper(int idx, int upper) {
    if (idx >= upper) return -1;
    return idx;
}

// border handling: UNDEFINED
#define NO_BH(idx, limit, stride) (idx)
inline int no_bh(int idx) {
    return idx;
}


// no border handling
#define DEFINE_BH_VARIANT_NO_BH(METHOD, DATA_TYPE, TS, NAME, BH_LOWER, BH_UPPER, PARM, CPARM, ACC, ACC_ARR) \
METHOD(NAME##TS,        DATA_TYPE, PARM(DATA_TYPE), CPARM(DATA_TYPE), ACC, ACC_ARR, NO_BH, NO_BH, NO_BH, NO_BH)

// border handling
#define DEFINE_BH_VARIANT(METHOD, DATA_TYPE, TS, NAME, BH_LOWER, BH_UPPER, PARM, CPARM, ACC, ACC_ARR) \
METHOD(NAME##_l##TS,    DATA_TYPE, PARM(DATA_TYPE), CPARM(DATA_TYPE), ACC, ACC_ARR, BH_LOWER, NO_BH, NO_BH, NO_BH) \
METHOD(NAME##_r##TS,    DATA_TYPE, PARM(DATA_TYPE), CPARM(DATA_TYPE), ACC, ACC_ARR, NO_BH, BH_UPPER, NO_BH, NO_BH) \
METHOD(NAME##_t##TS,    DATA_TYPE, PARM(DATA_TYPE), CPARM(DATA_TYPE), ACC, ACC_ARR, NO_BH, NO_BH, BH_LOWER, NO_BH) \
METHOD(NAME##_b##TS,    DATA_TYPE, PARM(DATA_TYPE), CPARM(DATA_TYPE), ACC, ACC_ARR, NO_BH, NO_BH, NO_BH, BH_UPPER) \
METHOD(NAME##_tl##TS,   DATA_TYPE, PARM(DATA_TYPE), CPARM(DATA_TYPE), ACC, ACC_ARR, BH_LOWER, NO_BH, BH_LOWER, NO_BH) \
METHOD(NAME##_tr##TS,   DATA_TYPE, PARM(DATA_TYPE), CPARM(DATA_TYPE), ACC, ACC_ARR, NO_BH, BH_UPPER, BH_LOWER, NO_BH) \
METHOD(NAME##_bl##TS,   DATA_TYPE, PARM(DATA_TYPE), CPARM(DATA_TYPE), ACC, ACC_ARR, BH_LOWER, NO_BH, NO_BH, BH_UPPER) \
METHOD(NAME##_br##TS,   DATA_TYPE, PARM(DATA_TYPE), CPARM(DATA_TYPE), ACC, ACC_ARR, NO_BH, BH_UPPER, NO_BH, BH_UPPER) \
METHOD(NAME##_tblr##TS, DATA_TYPE, PARM(DATA_TYPE), CPARM(DATA_TYPE), ACC, ACC_ARR, BH_LOWER, BH_UPPER, BH_LOWER, BH_UPPER)


// Bilinear Interpolation
#define INTERPOLATE_LINEAR_FILTERING_OPENCL(NAME, DATA_TYPE, PARM, CPARM, ACCESS, ACCESS_ARR, BHXL, BHXU, BHYL, BHYU) \
DATA_TYPE NAME(PARM, const int stride, float x_mapped, float y_mapped, const int rwidth, const int rheight, const int global_offset_x, const int global_offset_y CPARM) { \
    float xb = x_mapped - 0.5f; \
    float yb = y_mapped - 0.5f; \
    int x_int = xb; \
    int y_int = yb; \
    float x_frac = xb - x_int; \
    float y_frac = yb - y_int; \
    x_int += global_offset_x; \
    y_int += global_offset_y; \
 \
    return \
        (1.0f-x_frac) * (1.0f-y_frac) * ACCESS(BHXU(BHXL(x_int  , global_offset_x, stride), global_offset_x+rwidth, stride), BHYU(BHYL(y_int  , global_offset_y, rheight), global_offset_y+rheight, rheight), stride, const_val, ACCESS_ARR) + \
              x_frac  * (1.0f-y_frac) * ACCESS(BHXU(BHXL(x_int+1, global_offset_x, stride), global_offset_x+rwidth, stride), BHYU(BHYL(y_int  , global_offset_y, rheight), global_offset_y+rheight, rheight), stride, const_val, ACCESS_ARR) + \
        (1.0f-x_frac) *       y_frac  * ACCESS(BHXU(BHXL(x_int  , global_offset_x, stride), global_offset_x+rwidth, stride), BHYU(BHYL(y_int+1, global_offset_y, rheight), global_offset_y+rheight, rheight), stride, const_val, ACCESS_ARR) + \
              x_frac  *       y_frac  * ACCESS(BHXU(BHXL(x_int+1, global_offset_x, stride), global_offset_x+rwidth, stride), BHYU(BHYL(y_int+1, global_offset_y, rheight), global_offset_y+rheight, rheight), stride, const_val, ACCESS_ARR); \
}


// Cubic Interpolation
float bicubic_spline(float diff) {
    diff = fabs(diff);
    float a = -0.5f;

    if (diff < 1.0f) {
        return (a + 2.0f) *diff*diff*diff - (a + 3.0f)*diff*diff + 1;
    } else if (diff < 2.0f) {
        return a * diff*diff*diff - 5.0f * a * diff*diff + 8.0f * a * diff - 4.0f * a;
    } else return 0.0f;
}

#define INTERPOLATE_CUBIC_FILTERING_OPENCL(NAME, DATA_TYPE, PARM, CPARM, ACCESS, ACCESS_ARR, BHXL, BHXU, BHYL, BHYU) \
DATA_TYPE NAME(PARM, const int stride, float x_mapped, float y_mapped, const int rwidth, const int rheight, const int global_offset_x, const int global_offset_y CPARM) { \
    float xb = x_mapped - 0.5f; \
    float yb = y_mapped - 0.5f; \
    int x_int = xb; \
    int y_int = yb; \
    float x_frac = xb - x_int; \
    float y_frac = yb - y_int; \
    x_int += global_offset_x; \
    y_int += global_offset_y; \
 \
    float y0 = \
        ACCESS(BHXU(BHXL(x_int - 1 + 0, global_offset_x, stride), global_offset_x+rwidth, stride), BHYU(BHYL(y_int - 1 + 0, global_offset_y, rheight), global_offset_y+rheight, rheight), stride, const_val, ACCESS_ARR) * bicubic_spline(x_frac - 1 + 0) + \
        ACCESS(BHXU(BHXL(x_int - 1 + 1, global_offset_x, stride), global_offset_x+rwidth, stride), BHYU(BHYL(y_int - 1 + 0, global_offset_y, rheight), global_offset_y+rheight, rheight), stride, const_val, ACCESS_ARR) * bicubic_spline(x_frac - 1 + 1) + \
        ACCESS(BHXU(BHXL(x_int - 1 + 2, global_offset_x, stride), global_offset_x+rwidth, stride), BHYU(BHYL(y_int - 1 + 0, global_offset_y, rheight), global_offset_y+rheight, rheight), stride, const_val, ACCESS_ARR) * bicubic_spline(x_frac - 1 + 2) + \
        ACCESS(BHXU(BHXL(x_int - 1 + 3, global_offset_x, stride), global_offset_x+rwidth, stride), BHYU(BHYL(y_int - 1 + 0, global_offset_y, rheight), global_offset_y+rheight, rheight), stride, const_val, ACCESS_ARR) * bicubic_spline(x_frac - 1 + 3); \
    float y1 = \
        ACCESS(BHXU(BHXL(x_int - 1 + 0, global_offset_x, stride), global_offset_x+rwidth, stride), BHYU(BHYL(y_int - 1 + 1, global_offset_y, rheight), global_offset_y+rheight, rheight), stride, const_val, ACCESS_ARR) * bicubic_spline(x_frac - 1 + 0) + \
        ACCESS(BHXU(BHXL(x_int - 1 + 1, global_offset_x, stride), global_offset_x+rwidth, stride), BHYU(BHYL(y_int - 1 + 1, global_offset_y, rheight), global_offset_y+rheight, rheight), stride, const_val, ACCESS_ARR) * bicubic_spline(x_frac - 1 + 1) + \
        ACCESS(BHXU(BHXL(x_int - 1 + 2, global_offset_x, stride), global_offset_x+rwidth, stride), BHYU(BHYL(y_int - 1 + 1, global_offset_y, rheight), global_offset_y+rheight, rheight), stride, const_val, ACCESS_ARR) * bicubic_spline(x_frac - 1 + 2) + \
        ACCESS(BHXU(BHXL(x_int - 1 + 3, global_offset_x, stride), global_offset_x+rwidth, stride), BHYU(BHYL(y_int - 1 + 1, global_offset_y, rheight), global_offset_y+rheight, rheight), stride, const_val, ACCESS_ARR) * bicubic_spline(x_frac - 1 + 3); \
    float y2 = \
        ACCESS(BHXU(BHXL(x_int - 1 + 0, global_offset_x, stride), global_offset_x+rwidth, stride), BHYU(BHYL(y_int - 1 + 2, global_offset_y, rheight), global_offset_y+rheight, rheight), stride, const_val, ACCESS_ARR) * bicubic_spline(x_frac - 1 + 0) + \
        ACCESS(BHXU(BHXL(x_int - 1 + 1, global_offset_x, stride), global_offset_x+rwidth, stride), BHYU(BHYL(y_int - 1 + 2, global_offset_y, rheight), global_offset_y+rheight, rheight), stride, const_val, ACCESS_ARR) * bicubic_spline(x_frac - 1 + 1) + \
        ACCESS(BHXU(BHXL(x_int - 1 + 2, global_offset_x, stride), global_offset_x+rwidth, stride), BHYU(BHYL(y_int - 1 + 2, global_offset_y, rheight), global_offset_y+rheight, rheight), stride, const_val, ACCESS_ARR) * bicubic_spline(x_frac - 1 + 2) + \
        ACCESS(BHXU(BHXL(x_int - 1 + 3, global_offset_x, stride), global_offset_x+rwidth, stride), BHYU(BHYL(y_int - 1 + 2, global_offset_y, rheight), global_offset_y+rheight, rheight), stride, const_val, ACCESS_ARR) * bicubic_spline(x_frac - 1 + 3); \
    float y3 = \
        ACCESS(BHXU(BHXL(x_int - 1 + 0, global_offset_x, stride), global_offset_x+rwidth, stride), BHYU(BHYL(y_int - 1 + 3, global_offset_y, rheight), global_offset_y+rheight, rheight), stride, const_val, ACCESS_ARR) * bicubic_spline(x_frac - 1 + 0) + \
        ACCESS(BHXU(BHXL(x_int - 1 + 1, global_offset_x, stride), global_offset_x+rwidth, stride), BHYU(BHYL(y_int - 1 + 3, global_offset_y, rheight), global_offset_y+rheight, rheight), stride, const_val, ACCESS_ARR) * bicubic_spline(x_frac - 1 + 1) + \
        ACCESS(BHXU(BHXL(x_int - 1 + 2, global_offset_x, stride), global_offset_x+rwidth, stride), BHYU(BHYL(y_int - 1 + 3, global_offset_y, rheight), global_offset_y+rheight, rheight), stride, const_val, ACCESS_ARR) * bicubic_spline(x_frac - 1 + 2) + \
        ACCESS(BHXU(BHXL(x_int - 1 + 3, global_offset_x, stride), global_offset_x+rwidth, stride), BHYU(BHYL(y_int - 1 + 3, global_offset_y, rheight), global_offset_y+rheight, rheight), stride, const_val, ACCESS_ARR) * bicubic_spline(x_frac - 1 + 3); \
 \
    return y0*bicubic_spline(y_frac - 1 + 0) + \
        y1*bicubic_spline(y_frac - 1 + 1) + \
        y2*bicubic_spline(y_frac - 1 + 2) + \
        y3*bicubic_spline(y_frac - 1 + 3); \
}


// Lanczos3 Interpolation
#define MY_PI 3.141592654
float lanczos(float diff) {
    diff = fabs(diff);
    float l = 3.0;

    if (diff==0.0) return 1.0;
    else if (diff < l) {
        return l * (sin(MY_PI*diff/l) * sin(MY_PI*diff)) / (MY_PI*MY_PI*diff*diff);
    } else return 0.0;
}

#define INTERPOLATE_LANCZOS_FILTERING_OPENCL(NAME, DATA_TYPE, PARM, CPARM, ACCESS, ACCESS_ARR, BHXL, BHXU, BHYL, BHYU) \
DATA_TYPE NAME(PARM, const int stride, float x_mapped, float y_mapped, const int rwidth, const int rheight, const int global_offset_x, const int global_offset_y CPARM) { \
    float xb = x_mapped - 0.5f; \
    float yb = y_mapped - 0.5f; \
    int x_int = xb; \
    int y_int = yb; \
    float x_frac = xb - x_int; \
    float y_frac = yb - y_int; \
    x_int += global_offset_x; \
    y_int += global_offset_y; \
 \
    float y0 = \
        ACCESS(BHXU(BHXL(x_int - 2 + 0, global_offset_x, stride), global_offset_x+rwidth, stride), BHYU(BHYL(y_int - 1 + 0, global_offset_y, rheight), global_offset_y+rheight, rheight), stride, const_val, ACCESS_ARR) * lanczos(x_frac - 2 + 0) + \
        ACCESS(BHXU(BHXL(x_int - 2 + 1, global_offset_x, stride), global_offset_x+rwidth, stride), BHYU(BHYL(y_int - 1 + 0, global_offset_y, rheight), global_offset_y+rheight, rheight), stride, const_val, ACCESS_ARR) * lanczos(x_frac - 2 + 1) + \
        ACCESS(BHXU(BHXL(x_int - 2 + 2, global_offset_x, stride), global_offset_x+rwidth, stride), BHYU(BHYL(y_int - 1 + 0, global_offset_y, rheight), global_offset_y+rheight, rheight), stride, const_val, ACCESS_ARR) * lanczos(x_frac - 2 + 2) + \
        ACCESS(BHXU(BHXL(x_int - 2 + 3, global_offset_x, stride), global_offset_x+rwidth, stride), BHYU(BHYL(y_int - 1 + 0, global_offset_y, rheight), global_offset_y+rheight, rheight), stride, const_val, ACCESS_ARR) * lanczos(x_frac - 2 + 3) + \
        ACCESS(BHXU(BHXL(x_int - 2 + 4, global_offset_x, stride), global_offset_x+rwidth, stride), BHYU(BHYL(y_int - 1 + 0, global_offset_y, rheight), global_offset_y+rheight, rheight), stride, const_val, ACCESS_ARR) * lanczos(x_frac - 2 + 4) + \
        ACCESS(BHXU(BHXL(x_int - 2 + 5, global_offset_x, stride), global_offset_x+rwidth, stride), BHYU(BHYL(y_int - 1 + 0, global_offset_y, rheight), global_offset_y+rheight, rheight), stride, const_val, ACCESS_ARR) * lanczos(x_frac - 2 + 5); \
    float y1 = \
        ACCESS(BHXU(BHXL(x_int - 2 + 0, global_offset_x, stride), global_offset_x+rwidth, stride), BHYU(BHYL(y_int - 1 + 1, global_offset_y, rheight), global_offset_y+rheight, rheight), stride, const_val, ACCESS_ARR) * lanczos(x_frac - 2 + 0) + \
        ACCESS(BHXU(BHXL(x_int - 2 + 1, global_offset_x, stride), global_offset_x+rwidth, stride), BHYU(BHYL(y_int - 1 + 1, global_offset_y, rheight), global_offset_y+rheight, rheight), stride, const_val, ACCESS_ARR) * lanczos(x_frac - 2 + 1) + \
        ACCESS(BHXU(BHXL(x_int - 2 + 2, global_offset_x, stride), global_offset_x+rwidth, stride), BHYU(BHYL(y_int - 1 + 1, global_offset_y, rheight), global_offset_y+rheight, rheight), stride, const_val, ACCESS_ARR) * lanczos(x_frac - 2 + 2) + \
        ACCESS(BHXU(BHXL(x_int - 2 + 3, global_offset_x, stride), global_offset_x+rwidth, stride), BHYU(BHYL(y_int - 1 + 1, global_offset_y, rheight), global_offset_y+rheight, rheight), stride, const_val, ACCESS_ARR) * lanczos(x_frac - 2 + 3) + \
        ACCESS(BHXU(BHXL(x_int - 2 + 4, global_offset_x, stride), global_offset_x+rwidth, stride), BHYU(BHYL(y_int - 1 + 1, global_offset_y, rheight), global_offset_y+rheight, rheight), stride, const_val, ACCESS_ARR) * lanczos(x_frac - 2 + 5) + \
        ACCESS(BHXU(BHXL(x_int - 2 + 5, global_offset_x, stride), global_offset_x+rwidth, stride), BHYU(BHYL(y_int - 1 + 1, global_offset_y, rheight), global_offset_y+rheight, rheight), stride, const_val, ACCESS_ARR) * lanczos(x_frac - 2 + 5); \
    float y2 = \
        ACCESS(BHXU(BHXL(x_int - 2 + 0, global_offset_x, stride), global_offset_x+rwidth, stride), BHYU(BHYL(y_int - 1 + 2, global_offset_y, rheight), global_offset_y+rheight, rheight), stride, const_val, ACCESS_ARR) * lanczos(x_frac - 2 + 0) + \
        ACCESS(BHXU(BHXL(x_int - 2 + 1, global_offset_x, stride), global_offset_x+rwidth, stride), BHYU(BHYL(y_int - 1 + 2, global_offset_y, rheight), global_offset_y+rheight, rheight), stride, const_val, ACCESS_ARR) * lanczos(x_frac - 2 + 1) + \
        ACCESS(BHXU(BHXL(x_int - 2 + 2, global_offset_x, stride), global_offset_x+rwidth, stride), BHYU(BHYL(y_int - 1 + 2, global_offset_y, rheight), global_offset_y+rheight, rheight), stride, const_val, ACCESS_ARR) * lanczos(x_frac - 2 + 2) + \
        ACCESS(BHXU(BHXL(x_int - 2 + 3, global_offset_x, stride), global_offset_x+rwidth, stride), BHYU(BHYL(y_int - 1 + 2, global_offset_y, rheight), global_offset_y+rheight, rheight), stride, const_val, ACCESS_ARR) * lanczos(x_frac - 2 + 3) + \
        ACCESS(BHXU(BHXL(x_int - 2 + 4, global_offset_x, stride), global_offset_x+rwidth, stride), BHYU(BHYL(y_int - 1 + 2, global_offset_y, rheight), global_offset_y+rheight, rheight), stride, const_val, ACCESS_ARR) * lanczos(x_frac - 2 + 4) + \
        ACCESS(BHXU(BHXL(x_int - 2 + 5, global_offset_x, stride), global_offset_x+rwidth, stride), BHYU(BHYL(y_int - 1 + 2, global_offset_y, rheight), global_offset_y+rheight, rheight), stride, const_val, ACCESS_ARR) * lanczos(x_frac - 2 + 5); \
    float y3 = \
        ACCESS(BHXU(BHXL(x_int - 2 + 0, global_offset_x, stride), global_offset_x+rwidth, stride), BHYU(BHYL(y_int - 1 + 3, global_offset_y, rheight), global_offset_y+rheight, rheight), stride, const_val, ACCESS_ARR) * lanczos(x_frac - 2 + 0) + \
        ACCESS(BHXU(BHXL(x_int - 2 + 1, global_offset_x, stride), global_offset_x+rwidth, stride), BHYU(BHYL(y_int - 1 + 3, global_offset_y, rheight), global_offset_y+rheight, rheight), stride, const_val, ACCESS_ARR) * lanczos(x_frac - 2 + 1) + \
        ACCESS(BHXU(BHXL(x_int - 2 + 2, global_offset_x, stride), global_offset_x+rwidth, stride), BHYU(BHYL(y_int - 1 + 3, global_offset_y, rheight), global_offset_y+rheight, rheight), stride, const_val, ACCESS_ARR) * lanczos(x_frac - 2 + 2) + \
        ACCESS(BHXU(BHXL(x_int - 2 + 3, global_offset_x, stride), global_offset_x+rwidth, stride), BHYU(BHYL(y_int - 1 + 3, global_offset_y, rheight), global_offset_y+rheight, rheight), stride, const_val, ACCESS_ARR) * lanczos(x_frac - 2 + 3) + \
        ACCESS(BHXU(BHXL(x_int - 2 + 4, global_offset_x, stride), global_offset_x+rwidth, stride), BHYU(BHYL(y_int - 1 + 3, global_offset_y, rheight), global_offset_y+rheight, rheight), stride, const_val, ACCESS_ARR) * lanczos(x_frac - 2 + 4) + \
        ACCESS(BHXU(BHXL(x_int - 2 + 5, global_offset_x, stride), global_offset_x+rwidth, stride), BHYU(BHYL(y_int - 1 + 3, global_offset_y, rheight), global_offset_y+rheight, rheight), stride, const_val, ACCESS_ARR) * lanczos(x_frac - 2 + 5); \
    float y4 = \
        ACCESS(BHXU(BHXL(x_int - 2 + 0, global_offset_x, stride), global_offset_x+rwidth, stride), BHYU(BHYL(y_int - 1 + 4, global_offset_y, rheight), global_offset_y+rheight, rheight), stride, const_val, ACCESS_ARR) * lanczos(x_frac - 2 + 0) + \
        ACCESS(BHXU(BHXL(x_int - 2 + 1, global_offset_x, stride), global_offset_x+rwidth, stride), BHYU(BHYL(y_int - 1 + 4, global_offset_y, rheight), global_offset_y+rheight, rheight), stride, const_val, ACCESS_ARR) * lanczos(x_frac - 2 + 1) + \
        ACCESS(BHXU(BHXL(x_int - 2 + 2, global_offset_x, stride), global_offset_x+rwidth, stride), BHYU(BHYL(y_int - 1 + 4, global_offset_y, rheight), global_offset_y+rheight, rheight), stride, const_val, ACCESS_ARR) * lanczos(x_frac - 2 + 2) + \
        ACCESS(BHXU(BHXL(x_int - 2 + 3, global_offset_x, stride), global_offset_x+rwidth, stride), BHYU(BHYL(y_int - 1 + 4, global_offset_y, rheight), global_offset_y+rheight, rheight), stride, const_val, ACCESS_ARR) * lanczos(x_frac - 2 + 3) + \
        ACCESS(BHXU(BHXL(x_int - 2 + 4, global_offset_x, stride), global_offset_x+rwidth, stride), BHYU(BHYL(y_int - 1 + 4, global_offset_y, rheight), global_offset_y+rheight, rheight), stride, const_val, ACCESS_ARR) * lanczos(x_frac - 2 + 4) + \
        ACCESS(BHXU(BHXL(x_int - 2 + 5, global_offset_x, stride), global_offset_x+rwidth, stride), BHYU(BHYL(y_int - 1 + 4, global_offset_y, rheight), global_offset_y+rheight, rheight), stride, const_val, ACCESS_ARR) * lanczos(x_frac - 2 + 5); \
    float y5 = \
        ACCESS(BHXU(BHXL(x_int - 2 + 0, global_offset_x, stride), global_offset_x+rwidth, stride), BHYU(BHYL(y_int - 1 + 5, global_offset_y, rheight), global_offset_y+rheight, rheight), stride, const_val, ACCESS_ARR) * lanczos(x_frac - 2 + 0) + \
        ACCESS(BHXU(BHXL(x_int - 2 + 1, global_offset_x, stride), global_offset_x+rwidth, stride), BHYU(BHYL(y_int - 1 + 5, global_offset_y, rheight), global_offset_y+rheight, rheight), stride, const_val, ACCESS_ARR) * lanczos(x_frac - 2 + 1) + \
        ACCESS(BHXU(BHXL(x_int - 2 + 2, global_offset_x, stride), global_offset_x+rwidth, stride), BHYU(BHYL(y_int - 1 + 5, global_offset_y, rheight), global_offset_y+rheight, rheight), stride, const_val, ACCESS_ARR) * lanczos(x_frac - 2 + 2) + \
        ACCESS(BHXU(BHXL(x_int - 2 + 3, global_offset_x, stride), global_offset_x+rwidth, stride), BHYU(BHYL(y_int - 1 + 5, global_offset_y, rheight), global_offset_y+rheight, rheight), stride, const_val, ACCESS_ARR) * lanczos(x_frac - 2 + 3) + \
        ACCESS(BHXU(BHXL(x_int - 2 + 4, global_offset_x, stride), global_offset_x+rwidth, stride), BHYU(BHYL(y_int - 1 + 5, global_offset_y, rheight), global_offset_y+rheight, rheight), stride, const_val, ACCESS_ARR) * lanczos(x_frac - 2 + 4) + \
        ACCESS(BHXU(BHXL(x_int - 2 + 5, global_offset_x, stride), global_offset_x+rwidth, stride), BHYU(BHYL(y_int - 1 + 5, global_offset_y, rheight), global_offset_y+rheight, rheight), stride, const_val, ACCESS_ARR) * lanczos(x_frac - 2 + 5); \
 \
    return y0*lanczos(y_frac - 2 + 0) + \
        y1*lanczos(y_frac - 2 + 1) + \
        y2*lanczos(y_frac - 2 + 2) + \
        y3*lanczos(y_frac - 2 + 3) + \
        y4*lanczos(y_frac - 2 + 4) + \
        y5*lanczos(y_frac - 2 + 5); \
}

#endif  // __HIPACC_OCL_INTERPOLATE_HPP__

