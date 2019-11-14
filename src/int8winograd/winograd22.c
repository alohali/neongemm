#ifdef __ARM_NEON
#include <arm_neon.h>
#else
#error("arm neon not supported")
#endif

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

/* Block sizes */
#define DEBUG_PACK_SHAPE
#undef DEBUG_PACK_SHAPE
#define DEBUG_PRINT_DATA
#undef DEBUG_PRINT_DATA



#define min(i, j) ((i) < (j) ? (i): (j))
#define max(i, j) ((i) > (j) ? (i): (j))
//oirs to winograd domain o/4 win16 i/16 o4 i16
void weight_convert(const int8_t* src, int8_t* dst, int cin, int cout) {
    int cin16 = (cin + 8) / 16 * 16;
    int8_t w[3][3];
    int8_t mid[4][3];
    int8_t win_w[4][4];
    int8_t g[4][3] = {{2, 0, 0}, {1,1,1}, {1,-1,1}, {0,0,2}};
    for(int o=0; o<cout; o++){
        for(int i=0; i<cin16; i++){
            if(i<cin) {
                //oirs
                for(int r=0; r<3; r++){
                    for(int s=0; s<3; s++){
                        w[r][s] = src[o*cin*9 + i*9 + r*3 + s];
                    }
                }
                for(int r=0; r<4; r++){
                    for(int s=0; s<3; s++){
                        mid[r][s] = g[r][0]*w[0][s] + g[r][1]*w[1][s] + g[r][2]*w[2][s];
                    }
                }
            }

            int ii=i%16;
            int io=i/16;
            int oi=o%4;
            int oo=o/4;
            for(int r=0; r<4; r++){
                for(int s=0; s<4; s++){
                    //o/4 win16 i/16 o4 i16
                    int winrs = r*4+s;
                    if(i<cin) {
                        win_w[r][s] = g[s][0]*mid[r][0] + g[s][1]*mid[r][1] + g[s][2]*mid[r][2];
                        dst[oo*16*cin16*4+winrs*cin16*4+io*16*4+oi*16+ii] = win_w[r][s];
                    }else {
                        dst[oo*16*cin16*4+winrs*cin16*4+io*16*4+oi*16+ii] = 0;
                    }
                }
            }
        }
    }
#ifdef DEBUG_PRINT_DATA
    print_matrix(cin*cout/4,64,dst, 64 );
#endif
}

void winfeature_convert(const int8_t *src, int8_t *dst, int width, int channel){
    int cstride = (channel+8)/16*16;
    int c=0;
    for(; c<channel-8; c+=16){
        int8x16_t v[4][4];
        for(int h=0; h<4; h++){
            for(int w=0; w<4; w++){
                v[h][w] = vld1q_s8(src+c + w * channel + h * width * channel);
            }
        }
        int8x16_t mid[4][4];
        mid[0][0] = v[0][0] - v[2][0];
        mid[0][1] = v[0][1] - v[2][1];
        mid[0][2] = v[0][2] - v[2][2];
        mid[0][3] = v[0][3] - v[2][3];
        mid[1][0] = v[1][0] + v[2][0];
        mid[1][1] = v[1][1] + v[2][1];
        mid[1][2] = v[1][2] + v[2][2];
        mid[1][3] = v[1][3] + v[2][3];
        mid[2][0] = v[2][0] - v[1][0];
        mid[2][1] = v[2][1] - v[1][1];
        mid[2][2] = v[2][2] - v[1][2];
        mid[2][3] = v[2][3] - v[1][3];
        mid[3][0] = v[3][0] - v[1][0];
        mid[3][1] = v[3][1] - v[1][1];
        mid[3][2] = v[3][2] - v[1][2];
        mid[3][3] = v[3][3] - v[1][3];
        //save to h4 w4 c/16 t4 c16
        //h0w4
        vst1q_s8(dst + 0*cstride*4 + c*4 + 0*cstride*4*4, mid[0][0]-mid[0][2]);
        vst1q_s8(dst + 1*cstride*4 + c*4 + 0*cstride*4*4, mid[0][1]+mid[0][2]);
        vst1q_s8(dst + 2*cstride*4 + c*4 + 0*cstride*4*4, mid[0][2]-mid[0][1]);
        vst1q_s8(dst + 3*cstride*4 + c*4 + 0*cstride*4*4, mid[0][3]-mid[0][1]);
        //h1w4
        vst1q_s8(dst + 0*cstride*4 + c*4 + 1*cstride*4*4, mid[1][0]-mid[1][2]);
        vst1q_s8(dst + 1*cstride*4 + c*4 + 1*cstride*4*4, mid[1][1]+mid[1][2]);
        vst1q_s8(dst + 2*cstride*4 + c*4 + 1*cstride*4*4, mid[1][2]-mid[1][1]);
        vst1q_s8(dst + 3*cstride*4 + c*4 + 1*cstride*4*4, mid[1][3]-mid[1][1]);

        vst1q_s8(dst + 0*cstride*4 + c*4 + 2*cstride*4*4, mid[2][0]-mid[2][2]);
        vst1q_s8(dst + 1*cstride*4 + c*4 + 2*cstride*4*4, mid[2][1]+mid[2][2]);
        vst1q_s8(dst + 2*cstride*4 + c*4 + 2*cstride*4*4, mid[2][2]-mid[2][1]);
        vst1q_s8(dst + 3*cstride*4 + c*4 + 2*cstride*4*4, mid[2][3]-mid[2][1]);

        vst1q_s8(dst + 0*cstride*4 + c*4 + 3*cstride*4*4, mid[3][0]-mid[3][2]);
        vst1q_s8(dst + 1*cstride*4 + c*4 + 3*cstride*4*4, mid[3][1]+mid[3][2]);
        vst1q_s8(dst + 2*cstride*4 + c*4 + 3*cstride*4*4, mid[3][2]-mid[3][1]);
        vst1q_s8(dst + 3*cstride*4 + c*4 + 3*cstride*4*4, mid[3][3]-mid[3][1]);
    }
    if(channel!=cstride){
        int8x8_t v[4][4];
        for(int h=0; h<4; h++){
            for(int w=0; w<4; w++){
                v[h][w] = vld1_s8(src+c + w * channel + h * width * channel);
            }
        }
        int8x8_t mid[4][4];
        mid[0][0] = v[0][0] - v[2][0];
        mid[0][1] = v[0][1] - v[2][1];
        mid[0][2] = v[0][2] - v[2][2];
        mid[0][3] = v[0][3] - v[2][3];
        mid[1][0] = v[1][0] + v[2][0];
        mid[1][1] = v[1][1] + v[2][1];
        mid[1][2] = v[1][2] + v[2][2];
        mid[1][3] = v[1][3] + v[2][3];
        mid[2][0] = v[2][0] - v[1][0];
        mid[2][1] = v[2][1] - v[1][1];
        mid[2][2] = v[2][2] - v[1][2];
        mid[2][3] = v[2][3] - v[1][3];
        mid[3][0] = v[3][0] - v[1][0];
        mid[3][1] = v[3][1] - v[1][1];
        mid[3][2] = v[3][2] - v[1][2];
        mid[3][3] = v[3][3] - v[1][3];
        v[0][0] = vdup_n_s8(0);
        //save to h4 w4 c/16 t4 c16
        //h0w4
        vst1_s8(dst + 0*cstride*4 + c*4 + 0*cstride*4*4, mid[0][0]-mid[0][2]);
        vst1_s8(dst + 1*cstride*4 + c*4 + 0*cstride*4*4, mid[0][1]+mid[0][2]);
        vst1_s8(dst + 2*cstride*4 + c*4 + 0*cstride*4*4, mid[0][2]-mid[0][1]);
        vst1_s8(dst + 3*cstride*4 + c*4 + 0*cstride*4*4, mid[0][3]-mid[0][1]);
        vst1_s8(dst + 0*cstride*4 + c*4 + 0*cstride*4*4+8, v[0][0]);
        vst1_s8(dst + 1*cstride*4 + c*4 + 0*cstride*4*4+8, v[0][0]);
        vst1_s8(dst + 2*cstride*4 + c*4 + 0*cstride*4*4+8, v[0][0]);
        vst1_s8(dst + 3*cstride*4 + c*4 + 0*cstride*4*4+8, v[0][0]);
        //h14
        vst1_s8(dst + 0*cstride*4 + c*4 + 1*cstride*4*4, mid[1][0]-mid[1][2]);
        vst1_s8(dst + 1*cstride*4 + c*4 + 1*cstride*4*4, mid[1][1]+mid[1][2]);
        vst1_s8(dst + 2*cstride*4 + c*4 + 1*cstride*4*4, mid[1][2]-mid[1][1]);
        vst1_s8(dst + 3*cstride*4 + c*4 + 1*cstride*4*4, mid[1][3]-mid[1][1]);
        vst1_s8(dst + 0*cstride*4 + c*4 + 1*cstride*4*4+8, v[0][0]);
        vst1_s8(dst + 1*cstride*4 + c*4 + 1*cstride*4*4+8, v[0][0]);
        vst1_s8(dst + 2*cstride*4 + c*4 + 1*cstride*4*4+8, v[0][0]);
        vst1_s8(dst + 3*cstride*4 + c*4 + 1*cstride*4*4+8, v[0][0]);

        vst1_s8(dst + 0*cstride*4 + c*4 + 2*cstride*4*4, mid[2][0]-mid[2][2]);
        vst1_s8(dst + 1*cstride*4 + c*4 + 2*cstride*4*4, mid[2][1]+mid[2][2]);
        vst1_s8(dst + 2*cstride*4 + c*4 + 2*cstride*4*4, mid[2][2]-mid[2][1]);
        vst1_s8(dst + 3*cstride*4 + c*4 + 2*cstride*4*4, mid[2][3]-mid[2][1]);
        vst1_s8(dst + 0*cstride*4 + c*4 + 2*cstride*4*4+8, v[0][0]);
        vst1_s8(dst + 1*cstride*4 + c*4 + 2*cstride*4*4+8, v[0][0]);
        vst1_s8(dst + 2*cstride*4 + c*4 + 2*cstride*4*4+8, v[0][0]);
        vst1_s8(dst + 3*cstride*4 + c*4 + 2*cstride*4*4+8, v[0][0]);

        vst1_s8(dst + 0*cstride*4 + c*4 + 3*cstride*4*4, mid[3][0]-mid[3][2]);
        vst1_s8(dst + 1*cstride*4 + c*4 + 3*cstride*4*4, mid[3][1]+mid[3][2]);
        vst1_s8(dst + 2*cstride*4 + c*4 + 3*cstride*4*4, mid[3][2]-mid[3][1]);
        vst1_s8(dst + 3*cstride*4 + c*4 + 3*cstride*4*4, mid[3][3]-mid[3][1]);
        vst1_s8(dst + 0*cstride*4 + c*4 + 3*cstride*4*4+8, v[0][0]);
        vst1_s8(dst + 1*cstride*4 + c*4 + 3*cstride*4*4+8, v[0][0]);
        vst1_s8(dst + 2*cstride*4 + c*4 + 3*cstride*4*4+8, v[0][0]);
        vst1_s8(dst + 3*cstride*4 + c*4 + 3*cstride*4*4+8, v[0][0]);
    }
}

void dst_convert(int32_t *src, int8_t *dst, int ws, int hs,float *scale, int32_t *bias){
    int32_t *dst_wr = (int32_t *)dst;
    int32x4_t v[4][4];
    int32x4_t vmid[2][4];
    for(int h=0; h<4; h++){
        for(int w=0; w<4; w++){
            //ws = 4o * 4t 
            //hs = 4o * 4t * 4w
            v[h][w] = vld1q_s32(src+h*64+w*16);
        }
    }

    vmid[0][0] = v[0][0] + v[1][0] + v[2][0];
    vmid[0][1] = v[0][1] + v[1][1] + v[2][1];
    vmid[0][2] = v[0][2] + v[1][2] + v[2][2];
    vmid[0][3] = v[0][3] + v[1][3] + v[2][3];

    vmid[1][0] = v[1][0] - v[2][0] + v[3][0];
    vmid[1][1] = v[1][1] - v[2][1] + v[3][1];
    vmid[1][2] = v[1][2] - v[2][2] + v[3][2];
    vmid[1][3] = v[1][3] - v[2][3] + v[3][3];

    //dst , reuse v
    int32x4_t b4 = vld1q_s32(bias);
    float32x4_t s4 = vld1q_f32(scale);
    v[0][0] = vcvtnq_s32_f32(vcvtq_f32_s32(vmid[0][0] + vmid[0][1] + vmid[0][2] + b4) * s4);
    v[0][1] = vcvtnq_s32_f32(vcvtq_f32_s32(vmid[0][1] - vmid[0][2] + vmid[0][3] + b4) * s4);
    v[1][0] = vcvtnq_s32_f32(vcvtq_f32_s32(vmid[1][0] + vmid[1][1] + vmid[1][2] + b4) * s4);
    v[1][1] = vcvtnq_s32_f32(vcvtq_f32_s32(vmid[1][1] - vmid[1][2] + vmid[1][3] + b4) * s4);
    int8x8_t rmid[2];
    rmid[0] = vqmovn_s16( vqmovn_high_s32(vqmovn_s32(v[0][0]), v[0][1]));
    rmid[1] = vqmovn_s16( vqmovn_high_s32(vqmovn_s32(v[1][0]), v[1][1]));
    vst1_lane_s32(dst_wr, vreinterpret_s32_s8(rmid[0]), 0);//*(int32_t*)&rmid[0];
    vst1_lane_s32(dst_wr + ws/4, vreinterpret_s32_s8(rmid[0]), 1);//*(int32_t*)&rmid[0];
    vst1_lane_s32(dst_wr + hs/4, vreinterpret_s32_s8(rmid[1]), 0);//*(int32_t*)&rmid[0];
    vst1_lane_s32(dst_wr + hs/4 + ws/4, vreinterpret_s32_s8(rmid[1]), 1);//*(int32_t*)&rmid[0];
}


int8_t  *win_midbuffer = NULL;
int8_t  *src_midbuffer = NULL;
int32_t *dst_midbuffer = NULL;
void kernel4x4(int cin, int hin, int win, int cout, int hout, 
                int wout, int8_t* sa, int8_t * sb, int8_t* sc, float *scale, 
                int32_t *bias, int pad) 
{
    int8_t *restrict b = sb, *restrict c = sc;
    int cin16 = (cin+8)/16*16;
    if(!win_midbuffer){
        win_midbuffer = (int8_t *)malloc(1024 * 32);
        src_midbuffer = (int8_t *)malloc(1024 * 32);
        dst_midbuffer = (int32_t *)malloc(1024*2);
    }

    for(int h = 0; h < hout; h += 4) {
        for(int w = 0; w < wout; w += 4) {
            for(int ht=0; ht<2; ht++){
                for(int wt=0; wt<2; wt++){
                    int srch = h + ht * 2 - pad;
                    int srcw = w + wt * 2 - pad;
                    int8_t *apos = sa + srch*cin*win + srcw*cin;
                    if(srch<0 || srcw<0 || srch+4>hin || srcw+4>win){
                        memset(src_midbuffer, 0, cin * 16);

                        int sy    = max(0, srch) - srch;
                        int ey    = min(srch + 4, hin) - srch;
                        int sx    = max(0, srcw) - srcw;
                        int ex    = min(srcw + 4, win) - srcw;
                        int count = cin * (ex - sx);
                        
                        for(int yy=sy; yy<ey; yy++){
                            int8_t* src_yy = apos + sx * cin + yy * cin * win;
                            int8_t* dst_yy = src_midbuffer + yy * cin * 4 + sx * cin;
                            memcpy(dst_yy, src_yy, count);
                        }
                        winfeature_convert(src_midbuffer, win_midbuffer+(ht*2+wt)*16, 4, cin);
                    }else{
                        winfeature_convert(apos, win_midbuffer+(ht*2+wt)*16, win, cin);
                    }
                }
            }
#ifdef DEBUG_PRINT_DATA
            print_matrix(16*4,cin16, win_midbuffer, cin16);
#endif
            for(int j = 0; j < cout; j += 4) {
                int8_t  *win_temp = win_midbuffer;
                int32_t *dst_temp = dst_midbuffer;
                for(int wintile=0; wintile<16; wintile++) {
                    
            	    asm volatile (

                    "mov x10, %1\n"
                    "ld1 {v12.16b, v13.16b}, [x10], #32\n"
                    "mov x8, %0\n"
                    "ld1 {v14.16b, v15.16b}, [x10], #32\n"
                    "ld1 {v8.16b, v9.16b}, [x8], #32\n"
                    "subs x9, %5, #16\n"
                    
                    "smull v0.8h, v12.8b, v8.8b\n"
                    "smull v1.8h, v13.8b, v8.8b\n"
                    "smlal2 v0.8h, v12.16b, v8.16b\n"
                    "smlal2 v1.8h, v13.16b, v8.16b\n"
                    "saddlp v16.4s, v0.8h\n"
                    "saddlp v17.4s, v1.8h\n"
                     
                    "smull v2.8h, v14.8b, v8.8b\n"
                    "smull v3.8h, v15.8b, v8.8b\n"
                    "smull v4.8h, v12.8b, v9.8b\n"
                    "ld1 {v10.16b}, [x8], #16\n"
                    "smull v5.8h, v13.8b, v9.8b\n"
                    "smull v6.8h, v14.8b, v9.8b\n"
                    "smull v7.8h, v15.8b, v9.8b\n"
                    "smlal2 v2.8h, v14.16b, v8.16b\n"
                    "ld1 {v11.16b}, [x8], #16\n"
                    "smlal2 v3.8h, v15.16b, v8.16b\n"
                    "smlal2 v4.8h, v12.16b, v9.16b\n"
                    "smlal2 v5.8h, v13.16b, v9.16b\n"
                    "smlal2 v6.8h, v14.16b, v9.16b\n"
                    "smlal2 v7.8h, v15.16b, v9.16b\n"
                    "saddlp v18.4s, v2.8h\n"
                    "saddlp v19.4s, v3.8h\n"
                    "saddlp v20.4s, v4.8h\n"
                    "saddlp v21.4s, v5.8h\n"
                    "saddlp v22.4s, v6.8h\n"
                    "saddlp v23.4s, v7.8h\n"
                     
                    
                    "smull v0.8h, v12.8b, v10.8b\n"
                    "smull v1.8h, v13.8b, v10.8b\n"
                    "smull v2.8h, v14.8b, v10.8b\n"
                    "smull v3.8h, v15.8b, v10.8b\n"
                    "smlal2 v0.8h, v12.16b, v10.16b\n"
                    "smlal2 v1.8h, v13.16b, v10.16b\n"
                    "smlal2 v2.8h, v14.16b, v10.16b\n"
                    "smlal2 v3.8h, v15.16b, v10.16b\n"
                    "    ld1 {v8.16b}, [x8], #16\n"
                    "saddlp v24.4s, v0.8h\n"
                    "saddlp v25.4s, v1.8h\n"
                    "    ld1 {v9.16b}, [x8], #16\n"
                    "saddlp v26.4s, v2.8h\n"
                    "saddlp v27.4s, v3.8h\n"
                     
                    "smull v4.8h, v12.8b, v11.8b\n"
                    "smull v5.8h, v13.8b, v11.8b\n"
                    "smull v6.8h, v14.8b, v11.8b\n"
                    "smull v7.8h, v15.8b, v11.8b\n"
                    "smlal2 v4.8h, v12.16b, v11.16b\n"
                    "smlal2 v5.8h, v13.16b, v11.16b\n"
                    "ld1 {v12.16b, v13.16b}, [x10], #32\n"
                    "smlal2 v6.8h, v14.16b, v11.16b\n"
                    "smlal2 v7.8h, v15.16b, v11.16b\n"
                    "saddlp v28.4s, v4.8h\n"
                    "saddlp v29.4s, v5.8h\n"
                    "    ld1 {v14.16b, v15.16b}, [x10], #32\n"
                    "saddlp v30.4s, v6.8h\n"
                    "saddlp v31.4s, v7.8h\n"
                     
                    "beq L4LoopSzEnd\n"
                     
                    "L4LoopSz:\n"
                    "    smull v0.8h, v12.8b, v8.8b\n"
                    "    ld1 {v10.16b}, [x8], #16\n"
                    "    smull v1.8h, v13.8b, v8.8b\n"
                    "    smull v2.8h, v14.8b, v8.8b\n"
                    "    smull v3.8h, v15.8b, v8.8b\n"
                    "    smlal2 v0.8h, v12.16b, v8.16b\n"
                    "    ld1 {v11.16b}, [x8], #16\n"
                    "    smlal2 v1.8h, v13.16b, v8.16b\n"
                    "    smlal2 v2.8h, v14.16b, v8.16b\n"
                    "    smlal2 v3.8h, v15.16b, v8.16b\n"
                    "    sadalp v16.4s, v0.8h\n"
                    "    smull v4.8h, v12.8b, v9.8b\n"
                    "    sadalp v17.4s, v1.8h\n"
                    "    smull v5.8h, v13.8b, v9.8b\n"
                    "    sadalp v18.4s, v2.8h\n"
                    "    smull v6.8h, v14.8b, v9.8b\n"
                    "    sadalp v19.4s, v3.8h\n"
                    "    smull v7.8h, v15.8b, v9.8b\n"
                     
                    "    smlal2 v4.8h, v12.16b, v9.16b\n"
                    "    ld1 {v8.16b}, [x8], #16\n"
                    "    smlal2 v5.8h, v13.16b, v9.16b\n"
                    "    smlal2 v6.8h, v14.16b, v9.16b\n"
                    "    smlal2 v7.8h, v15.16b, v9.16b\n"
                    "    sadalp v20.4s, v4.8h\n"
                    "    ld1 {v9.16b}, [x8], #16\n"
                    "    smull v0.8h, v12.8b, v10.8b\n"
                    "    sadalp v21.4s, v5.8h\n"
                    "    smull v1.8h, v13.8b, v10.8b\n"
                    "    sadalp v22.4s, v6.8h\n"
                    "    smull v2.8h, v14.8b, v10.8b\n"
                    "    sadalp v23.4s, v7.8h\n"
                    "    smull v3.8h, v15.8b, v10.8b\n"
                     
                    "    smlal2 v0.8h, v12.16b, v10.16b\n"
                    "    smlal2 v1.8h, v13.16b, v10.16b\n"
                    "    smlal2 v2.8h, v14.16b, v10.16b\n"
                    "    smlal2 v3.8h, v15.16b, v10.16b\n"
                    "    sadalp v24.4s, v0.8h\n"
                    "    smull v4.8h, v12.8b, v11.8b\n"
                    "    sadalp v25.4s, v1.8h\n"
                    "    smull v5.8h, v13.8b, v11.8b\n"
                    "    sadalp v26.4s, v2.8h\n"
                    "    smull v6.8h, v14.8b, v11.8b\n"
                    "    sadalp v27.4s, v3.8h\n"
                    "    smull v7.8h, v15.8b, v11.8b\n"
                     
                    "    smlal2 v4.8h, v12.16b, v11.16b\n"
                    "    subs x9, x9, #16\n"
                    "    smlal2 v5.8h, v13.16b, v11.16b\n"
                    "    smlal2 v6.8h, v14.16b, v11.16b\n"
                    "    ld1 {v12.16b, v13.16b}, [x10], #32\n"
                    "    smlal2 v7.8h, v15.16b, v11.16b\n"
                    "    sadalp v28.4s, v4.8h\n"
                    "    ld1 {v14.16b, v15.16b}, [x10], #32\n"
                    "    sadalp v29.4s, v5.8h\n"
                    "    sadalp v30.4s, v6.8h\n"
                    "    sadalp v31.4s, v7.8h\n"
                    "    bne L4LoopSz\n"
                     
                    "L4LoopSzEnd:\n"
                     
                    "addp v4.4s, v16.4s, v17.4s\n"
                    "addp v5.4s, v18.4s, v19.4s\n"
                    "addp v6.4s, v20.4s, v21.4s\n"
                    "addp v7.4s, v22.4s, v23.4s\n"
                    "addp v8.4s, v24.4s, v25.4s\n"
                    "addp v9.4s, v26.4s, v27.4s\n"
                    "addp v10.4s, v28.4s, v29.4s\n"
                    "addp v11.4s, v30.4s, v31.4s\n"
                     
                    "addp v12.4s, v4.4s, v5.4s\n"
                    "addp v13.4s, v6.4s, v7.4s\n"
                    "addp v14.4s, v8.4s, v9.4s\n"
                    "addp v15.4s, v10.4s, v11.4s\n"
                    "st1 {v12.4s, v13.4s, v14.4s, v15.4s}, [%2]\n"

                    : "=r"(win_temp),
                      "=r"(b),
                      "=r"(dst_temp),
                      "=r"(cin),
                      "=r"(cout),
                      "=r"(cin16)
                    : "0"(win_temp),
                      "1"(b),
                      "2"(dst_temp),
                      "3"(cin),
                      "4"(cout),
                      "5"(cin16)
                    : "memory", "cc", "x8", "x9","x10", 
                    "v0", "v1", "v2", "v3", "v4","v5","v6","v7", 
                    "v8", "v9", "v10", "v11", "v12", "v13", "v14", "v15",  
                    "v16", "v17", "v18", "v19", "v20", "v21", "v22", "v23", 
                    "v24", "v25", "v26", "v27", "v28", "v29", "v30", "v31"
            	    );  

                    b += 4 * cin16;
                    win_temp += 4 * cin16;
                    dst_temp += 16;
                }// end wintile
                for(int ht=0; ht<2; ht++){
                    for(int wt=0; wt<2; wt++){
                        c = sc + (h+ht*2) * cout * wout + (w+wt*2) * cout + j;
                        //dst_midbuffer: wintile16, h2, w2, o4
                        dst_convert(dst_midbuffer+ht*8+wt*4, c, cout, cout * wout, scale + j, bias + j);
                    }
                }
#ifdef DEBUG_PRINT_DATA
            print_matrix1(16*4,4, dst_midbuffer, 4);
#endif
            } // endo
            b = sb;
        } //endw
    }// endh
}


