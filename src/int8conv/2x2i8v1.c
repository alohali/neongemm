#ifdef __ARM_NEON
#include <arm_neon.h>
#else
#error("arm neon not supported")
#endif

#include <assert.h>
#include <stdlib.h>


void packB(int cin, int cout, int8_t *from, int8_t *to)
{

    int64_t *src = (int64_t *)from;
    int64_t *dst = (int64_t *)to;
    cin = cin / 8;
    for (int o = 0; o < cout; o += 4)
    {
        int i = 0;
        for (; i < cin; i += 2)
        {
            dst[0] = src[o * cin];
            dst[1] = src[o * cin + 1];
            dst[2] = src[(o + 1) * cin];
            dst[3] = src[(o + 1) * cin + 1];
            dst[4] = src[(o + 2) * cin];
            dst[5] = src[(o + 2) * cin + 1];
            dst[6] = src[(o + 3) * cin];
            dst[7] = src[(o + 3) * cin + 1];
            dst += 8;
            src += 2;
        }
        src -= i;
    }
}

extern void GEMM2x2Micro(int8_t *a, const int8_t *b, int8_t *c, int cin, int cout, int cdiv8, float *scale, int32_t *bias);

void kernel4x4(int cin, int cout, int hout, int wout, int8_t* sa, int8_t * sb, int8_t* sc, float *scale, int32_t *bias) 
{
    int8_t *a = sa, *b = sb, *c = sc;
    int cdiv8 = cin/8;
    int w_cstride = (cdiv8+1)/2*16;
    for(int h = 0; h < hout; h ++) {
        for(int w = 0; w < wout; w += 2) {
            for(int j = 0; j < cout; j += 4) {
                GEMM2x2Micro(a, b, c, cin, cout, cdiv8, scale + j, bias + j);
                c += 4;
                b += 4 * w_cstride;
            } // endo
            b = sb;
            c += cout;
            a += cin * 2;
        } //endw
    }// endh
}


