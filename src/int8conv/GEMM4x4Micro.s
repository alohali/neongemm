//
//
#ifdef __aarch64__

.text



.macro asm_function fname
.global \fname
#ifdef __ELF__
.hidden \fname
.type \fname, %function
#endif
\fname:
.endm

.align 5
asm_function GEMM4x4Micro 
//void GEMM4x4Micro(int8_t* a, const int8_t* b, int8_t* c, int cin, int cout, int cdiv8, float *scale, int32_t*bias)
//x0(a),
//x1(b),
//x2(c),
//x3(cin),
//x4(cout),
//x5(cdiv8),
//x6(scale),
//x7(bias)

sub sp, sp, #128
st1 {v8.4s, v9.4s, v10.4s, v11.4s}, [sp], #64
st1 {v12.4s, v13.4s, v14.4s, v15.4s}, [sp], #64


//prefetch data
//assume buffer c>=16 even c==8
ld1 {v12.16b, v13.16b}, [x1], #32 
add x11, x0, x3 
add x12, x11, x3 
add x13, x12, x3 

cmp x5, #2
ld1 {v14.16b, v15.16b}, [x1], #32 
ld1 {v8.16b}, [x0], #16 
ld1 {v9.16b}, [x11], #16 

blt C8First 

C16Start:
     sub x5, x5, #2 
     
     smull v0.8h, v12.8b, v8.8b 
     smull v1.8h, v13.8b, v8.8b 
     smlal2 v0.8h, v12.16b, v8.16b 
     smlal2 v1.8h, v13.16b, v8.16b 
     saddlp v16.4s, v0.8h 
     saddlp v17.4s, v1.8h 
      
     smull v2.8h, v14.8b, v8.8b 
     smull v3.8h, v15.8b, v8.8b 
     smull v4.8h, v12.8b, v9.8b 
     ld1 {v10.16b}, [x12], #16 
     smull v5.8h, v13.8b, v9.8b 
     smull v6.8h, v14.8b, v9.8b 
     smull v7.8h, v15.8b, v9.8b 
     smlal2 v2.8h, v14.16b, v8.16b 
     ld1 {v11.16b}, [x13], #16 
     smlal2 v3.8h, v15.16b, v8.16b 
     smlal2 v4.8h, v12.16b, v9.16b 
     smlal2 v5.8h, v13.16b, v9.16b 
     smlal2 v6.8h, v14.16b, v9.16b 
     smlal2 v7.8h, v15.16b, v9.16b 
     saddlp v18.4s, v2.8h 
     saddlp v19.4s, v3.8h 
     saddlp v20.4s, v4.8h 
     saddlp v21.4s, v5.8h 
     saddlp v22.4s, v6.8h 
     saddlp v23.4s, v7.8h 
     
     cmp x5, #2
     
     smull v0.8h, v12.8b, v10.8b 
     smull v1.8h, v13.8b, v10.8b 
     smull v2.8h, v14.8b, v10.8b 
     smull v3.8h, v15.8b, v10.8b 
     smlal2 v0.8h, v12.16b, v10.16b 
     smlal2 v1.8h, v13.16b, v10.16b 
     smlal2 v2.8h, v14.16b, v10.16b 
     smlal2 v3.8h, v15.16b, v10.16b 
         ld1 {v8.16b}, [x0], #16 
     saddlp v24.4s, v0.8h 
     saddlp v25.4s, v1.8h 
         ld1 {v9.16b}, [x11], #16 
     saddlp v26.4s, v2.8h 
     saddlp v27.4s, v3.8h 
      
     smull v4.8h, v12.8b, v11.8b 
     smull v5.8h, v13.8b, v11.8b 
     smull v6.8h, v14.8b, v11.8b 
     smull v7.8h, v15.8b, v11.8b 
     smlal2 v4.8h, v12.16b, v11.16b 
     smlal2 v5.8h, v13.16b, v11.16b 
     ld1 {v12.16b, v13.16b}, [x1], #32 
     smlal2 v6.8h, v14.16b, v11.16b 
     smlal2 v7.8h, v15.16b, v11.16b 
     saddlp v28.4s, v4.8h 
     saddlp v29.4s, v5.8h 
         ld1 {v14.16b, v15.16b}, [x1], #32 
     saddlp v30.4s, v6.8h 
     saddlp v31.4s, v7.8h 
      
     blt C8Last 
      
     C16Loop: 
         smull v0.8h, v12.8b, v8.8b 
         ld1 {v10.16b}, [x12], #16 
         smull v1.8h, v13.8b, v8.8b 
         smull v2.8h, v14.8b, v8.8b 
         smull v3.8h, v15.8b, v8.8b 
         smlal2 v0.8h, v12.16b, v8.16b 
         ld1 {v11.16b}, [x13], #16 
         smlal2 v1.8h, v13.16b, v8.16b 
         smlal2 v2.8h, v14.16b, v8.16b 
         smlal2 v3.8h, v15.16b, v8.16b 
         sadalp v16.4s, v0.8h 
         smull v4.8h, v12.8b, v9.8b 
         sadalp v17.4s, v1.8h 
         smull v5.8h, v13.8b, v9.8b 
         sadalp v18.4s, v2.8h 
         smull v6.8h, v14.8b, v9.8b 
         sadalp v19.4s, v3.8h 
         smull v7.8h, v15.8b, v9.8b 
      
         smlal2 v4.8h, v12.16b, v9.16b 
         ld1 {v8.16b}, [x0], #16 
         smlal2 v5.8h, v13.16b, v9.16b 
         smlal2 v6.8h, v14.16b, v9.16b 
         sub x5, x5, #2 
         smlal2 v7.8h, v15.16b, v9.16b 
         sadalp v20.4s, v4.8h 
         ld1 {v9.16b}, [x11], #16 
         smull v0.8h, v12.8b, v10.8b 
         sadalp v21.4s, v5.8h 
         smull v1.8h, v13.8b, v10.8b 
         sadalp v22.4s, v6.8h 
         smull v2.8h, v14.8b, v10.8b 
         sadalp v23.4s, v7.8h 
         smull v3.8h, v15.8b, v10.8b 
      
         smlal2 v0.8h, v12.16b, v10.16b 
         smlal2 v1.8h, v13.16b, v10.16b 
         smlal2 v2.8h, v14.16b, v10.16b 
         smlal2 v3.8h, v15.16b, v10.16b 
         sadalp v24.4s, v0.8h 
         smull v4.8h, v12.8b, v11.8b 
         sadalp v25.4s, v1.8h 
         smull v5.8h, v13.8b, v11.8b 
         sadalp v26.4s, v2.8h 
         smull v6.8h, v14.8b, v11.8b 
         sadalp v27.4s, v3.8h 
         smull v7.8h, v15.8b, v11.8b 
      
         smlal2 v4.8h, v12.16b, v11.16b 
         smlal2 v5.8h, v13.16b, v11.16b 
         cmp x5, #2 
         smlal2 v6.8h, v14.16b, v11.16b 
         ld1 {v12.16b, v13.16b}, [x1], #32 
         smlal2 v7.8h, v15.16b, v11.16b 
         sadalp v28.4s, v4.8h 
         ld1 {v14.16b, v15.16b}, [x1], #32 
         sadalp v29.4s, v5.8h 
         sadalp v30.4s, v6.8h 
         sadalp v31.4s, v7.8h 
         bge C16Loop 
 
C8Last:
     cmp x5, #1
     blt LoopEnd 
     smull v0.8h, v12.8b, v8.8b 
     ld1 {v10.16b}, [x12], #16 
     smull v1.8h, v13.8b, v8.8b 
     smull v2.8h, v14.8b, v8.8b 
     smull v3.8h, v15.8b, v8.8b 
     ld1 {v11.16b}, [x13], #16 
     sadalp v16.4s, v0.8h 
     smull v4.8h, v12.8b, v9.8b 
     sadalp v17.4s, v1.8h 
     smull v5.8h, v13.8b, v9.8b 
     sadalp v18.4s, v2.8h 
     smull v6.8h, v14.8b, v9.8b 
     sadalp v19.4s, v3.8h 
     smull v7.8h, v15.8b, v9.8b 
     
     sadalp v20.4s, v4.8h 
     smull v0.8h, v12.8b, v10.8b 
     sadalp v21.4s, v5.8h 
     smull v1.8h, v13.8b, v10.8b 
     sadalp v22.4s, v6.8h 
     smull v2.8h, v14.8b, v10.8b 
     sadalp v23.4s, v7.8h 
     smull v3.8h, v15.8b, v10.8b 
     
     sadalp v24.4s, v0.8h 
     smull v4.8h, v12.8b, v11.8b 
     sadalp v25.4s, v1.8h 
     smull v5.8h, v13.8b, v11.8b 
     sadalp v26.4s, v2.8h 
     smull v6.8h, v14.8b, v11.8b 
     sadalp v27.4s, v3.8h 
     smull v7.8h, v15.8b, v11.8b 
     
     sadalp v28.4s, v4.8h 
     sadalp v29.4s, v5.8h 
     sadalp v30.4s, v6.8h 
     sadalp v31.4s, v7.8h 
     b LoopEnd
      
C8First:
     cmp x5, #1
     blt LoopEnd 
     smull v0.8h, v12.8b, v8.8b 
     ld1 {v10.16b}, [x12], #16 
     smull v1.8h, v13.8b, v8.8b 
     smull v2.8h, v14.8b, v8.8b 
     smull v3.8h, v15.8b, v8.8b 
     ld1 {v11.16b}, [x13], #16 
     saddlp v16.4s, v0.8h 
     smull v4.8h, v12.8b, v9.8b 
     saddlp v17.4s, v1.8h 
     smull v5.8h, v13.8b, v9.8b 
     saddlp v18.4s, v2.8h 
     smull v6.8h, v14.8b, v9.8b 
     saddlp v19.4s, v3.8h 
     smull v7.8h, v15.8b, v9.8b 
     
     saddlp v20.4s, v4.8h 
     smull v0.8h, v12.8b, v10.8b 
     saddlp v21.4s, v5.8h 
     smull v1.8h, v13.8b, v10.8b 
     saddlp v22.4s, v6.8h 
     smull v2.8h, v14.8b, v10.8b 
     saddlp v23.4s, v7.8h 
     smull v3.8h, v15.8b, v10.8b 
     
     saddlp v24.4s, v0.8h 
     smull v4.8h, v12.8b, v11.8b 
     saddlp v25.4s, v1.8h 
     smull v5.8h, v13.8b, v11.8b 
     saddlp v26.4s, v2.8h 
     smull v6.8h, v14.8b, v11.8b 
     saddlp v27.4s, v3.8h 
     smull v7.8h, v15.8b, v11.8b 
     
     saddlp v28.4s, v4.8h 
     saddlp v29.4s, v5.8h 
     saddlp v30.4s, v6.8h 
     saddlp v31.4s, v7.8h 
LoopEnd: 
      
     ld1 {v0.4s}, [x7], #16 
     addp v4.4s, v16.4s, v17.4s 
     addp v5.4s, v18.4s, v19.4s 
     addp v6.4s, v20.4s, v21.4s 
     addp v7.4s, v22.4s, v23.4s 
     addp v8.4s, v24.4s, v25.4s 
     addp v9.4s, v26.4s, v27.4s 
     addp v10.4s, v28.4s, v29.4s 
     addp v11.4s, v30.4s, v31.4s 
      
     addp v12.4s, v4.4s, v5.4s 
     addp v13.4s, v6.4s, v7.4s 
     addp v14.4s, v8.4s, v9.4s 
     addp v15.4s, v10.4s, v11.4s 
     ld1 {v1.4s}, [x6], #16 
     add v16.4s, v12.4s, v0.4s 
     add v17.4s, v13.4s, v0.4s 
     add v18.4s, v14.4s, v0.4s 
     add v19.4s, v15.4s, v0.4s 
      
     scvtf v4.4s, v16.4s 
     scvtf v5.4s, v17.4s 
     scvtf v6.4s, v18.4s 
     scvtf v7.4s, v19.4s 
      
     fmul v12.4s, v4.4s, v1.4s 
     fmul v13.4s, v5.4s, v1.4s 
      
     fcvtns v8.4s, v12.4s 
     fcvtns v9.4s, v13.4s 
      
     sqxtn v0.4h, v8.4s 
     sqxtn2 v0.8h, v9.4s 
     sqxtn v2.8b, v0.8h 
     st1 {v2.s}[0], [x2], x4 
     st1 {v2.s}[1], [x2], x4 

     fmul v14.4s, v6.4s, v1.4s 
     fmul v15.4s, v7.4s, v1.4s 
     fcvtns v10.4s, v14.4s 
     fcvtns v11.4s, v15.4s 
     sqxtn v1.4h, v10.4s 
     sqxtn2 v1.8h, v11.4s 
     sqxtn2 v2.16b, v1.8h 
     st1 {v2.s}[2], [x2], x4 
     st1 {v2.s}[3], [x2]                 


sub sp, sp, #128
ld1 {v8.4s, v9.4s, v10.4s, v11.4s}, [sp], #64
ld1 {v12.4s, v13.4s, v14.4s, v15.4s}, [sp], #64
ret


#endif

