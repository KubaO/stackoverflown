/* Copyright (C) 2007  Julien Pommier
  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.
  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:
  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution. */

#include <emmintrin.h>
#include <math.h>
#include <assert.h>

#ifdef _MSC_VER /* visual c++ */
# define ALIGN16_BEG __declspec(align(16))
# define ALIGN16_END
#else /* gcc or icc */
# define ALIGN16_BEG
# define ALIGN16_END __attribute__((aligned(16)))
#endif

typedef __m128 v4sf;  // vector of 4 float (sse1)
typedef __m128i v4si; // vector of 4 int (sse2)

#define _PS_CONST(Name, Val)                                            \
    static const ALIGN16_BEG float _ps_##Name[4] ALIGN16_END = { Val, Val, Val, Val }
#define _PI32_CONST(Name, Val)                                            \
    static const ALIGN16_BEG int _pi32_##Name[4] ALIGN16_END = { Val, Val, Val, Val }

_PS_CONST(1  , 1.0f);
_PS_CONST(0p5, 0.5f);
_PI32_CONST(0x7f, 0x7f);
_PS_CONST(exp_hi,	88.3762626647949f);
_PS_CONST(exp_lo,	-88.3762626647949f);
_PS_CONST(cephes_LOG2EF, 1.44269504088896341);
_PS_CONST(cephes_exp_C1, 0.693359375);
_PS_CONST(cephes_exp_C2, -2.12194440e-4);
_PS_CONST(cephes_exp_p0, 1.9875691500E-4);
_PS_CONST(cephes_exp_p1, 1.3981999507E-3);
_PS_CONST(cephes_exp_p2, 8.3334519073E-3);
_PS_CONST(cephes_exp_p3, 4.1665795894E-2);
_PS_CONST(cephes_exp_p4, 1.6666665459E-1);
_PS_CONST(cephes_exp_p5, 5.0000001201E-1);

v4sf exp_ps(v4sf x) {
    v4sf tmp = _mm_setzero_ps(), fx;
    v4si emm0;
    v4sf one = *(v4sf*)_ps_1;

    x = _mm_min_ps(x, *(v4sf*)_ps_exp_hi);
    x = _mm_max_ps(x, *(v4sf*)_ps_exp_lo);

    fx = _mm_mul_ps(x, *(v4sf*)_ps_cephes_LOG2EF);
    fx = _mm_add_ps(fx, *(v4sf*)_ps_0p5);

    emm0 = _mm_cvttps_epi32(fx);
    tmp  = _mm_cvtepi32_ps(emm0);

    v4sf mask = _mm_cmpgt_ps(tmp, fx);
    mask = _mm_and_ps(mask, one);
    fx = _mm_sub_ps(tmp, mask);

    tmp = _mm_mul_ps(fx, *(v4sf*)_ps_cephes_exp_C1);
    v4sf z = _mm_mul_ps(fx, *(v4sf*)_ps_cephes_exp_C2);
    x = _mm_sub_ps(x, tmp);
    x = _mm_sub_ps(x, z);

    z = _mm_mul_ps(x,x);

    v4sf y = *(v4sf*)_ps_cephes_exp_p0;
    y = _mm_mul_ps(y, x);
    y = _mm_add_ps(y, *(v4sf*)_ps_cephes_exp_p1);
    y = _mm_mul_ps(y, x);
    y = _mm_add_ps(y, *(v4sf*)_ps_cephes_exp_p2);
    y = _mm_mul_ps(y, x);
    y = _mm_add_ps(y, *(v4sf*)_ps_cephes_exp_p3);
    y = _mm_mul_ps(y, x);
    y = _mm_add_ps(y, *(v4sf*)_ps_cephes_exp_p4);
    y = _mm_mul_ps(y, x);
    y = _mm_add_ps(y, *(v4sf*)_ps_cephes_exp_p5);
    y = _mm_mul_ps(y, z);
    y = _mm_add_ps(y, x);
    y = _mm_add_ps(y, one);

    emm0 = _mm_cvttps_epi32(fx);
    emm0 = _mm_add_epi32(emm0, *(v4si*)_pi32_0x7f);
    emm0 = _mm_slli_epi32(emm0, 23);
    v4sf pow2n = _mm_castsi128_ps(emm0);
    y = _mm_mul_ps(y, pow2n);
    return y;
}

typedef struct {
    float x,y,z, re;
} element_t;

void slow(element_t * const elements, const int num_elts, const float a) {
    element_t * elts = elements;
    for (int i = 0; i < num_elts; ++ i) {
        elts->re =
                powf((powf(elts->x, a) + powf(elts->y, a) + powf(elts->z, a)), 1.0/a);
        elts ++;
    }
}

void fast(element_t * const elements, const int num_elts, const float a) {
    element_t * elts = elements;
    float logf_a = logf(a);
    float logf_1_a = logf(1.0/a);
    v4sf log_a = _mm_load1_ps(&logf_a);
    v4sf log_1_a = _mm_load1_ps(&logf_1_a);
    assert(num_elts % 3 == 0); // operates on 3 elements at a time

    // elts->re = powf((powf(elts->x, a) + powf(elts->y, a) + powf(elts->z, a)), 1.0/a);
    for (int i = 0; i < num_elts; i += 3) {
        // transpose
        // we save one operation over _MM_TRANSPOSE4_PS by skipping the last row of output
        v4sf r0 = _mm_load_ps(&elts[0].x); // x1,y1,z1,0
        v4sf r1 = _mm_load_ps(&elts[1].x); // x2,y2,z2,0
        v4sf r2 = _mm_load_ps(&elts[2].x); // x3,y3,z3,0
        v4sf r3 = _mm_setzero_ps();        // 0, 0, 0, 0
        v4sf t0 = _mm_unpacklo_ps(r0, r1); //  x1,x2,y1,y2
        v4sf t1 = _mm_unpacklo_ps(r2, r3); //  x3,0, y3,0
        v4sf t2 = _mm_unpackhi_ps(r0, r1); //  z1,z2,0, 0
        v4sf t3 = _mm_unpackhi_ps(r2, r3); //  z3,0, 0, 0
        r0 = _mm_movelh_ps(t0, t1);        // x1,x2,x3,0
        r1 = _mm_movehl_ps(t1, t0);        // y1,y2,y3,0
        r2 = _mm_movelh_ps(t2, t3);        // z1,z2,z3,0
        // perform pow(x,a),.. using the fact that pow(x,a) = exp(x * log(a))
        v4sf r0a = _mm_mul_ps(r0, log_a); // x1*log(a), x2*log(a), x3*log(a), 0
        v4sf r1a = _mm_mul_ps(r1, log_a); // y1*log(a), y2*log(a), y3*log(a), 0
        v4sf r2a = _mm_mul_ps(r2, log_a); // z1*log(a), z2*log(a), z3*log(a), 0
        v4sf ex0 = exp_ps(r0a); // pow(x1, a), ..., 0
        v4sf ex1 = exp_ps(r1a); // pow(y1, a), ..., 0
        v4sf ex2 = exp_ps(r2a); // pow(z1, a), ..., 0
        // sum
        v4sf s1 = _mm_add_ps(ex0, ex1);
        v4sf s2 = _mm_add_ps(sum1, ex2);
        // pow(sum, 1/a) = exp(sum * log(1/a))
        v4sf ps = _mm_mul_ps(s2, log_1_a);
        v4sf es = exp_ps(ps);
        ALIGN16_BEG float re[4] ALIGN16_END;
        _mm_store_ps(re, es);
        elts[0].re = re[0];
        elts[1].re = re[1];
        elts[2].re = re[2];
        elts += 3;
    }
}

int main(int, char **)
{
}
