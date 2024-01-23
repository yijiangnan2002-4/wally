
Build/fft7p5ms/pisplit_lc3i_fft7p5ms_position_independent.splitload.lib:     file format elf32-xtensa-le


Disassembly of section .text:

00000000 <_init-0xa4>:
       0:	000000f4 00001cc4 0000202c 00000000     ........, ......
      10:	00001ff8 00000000 00002014 000020cc     ......... ... ..
      20:	0000205c 00000170 0000206f 00000d08     \ ..p...o ......
      30:	0000207f 00001154 0000208f 00001518     . ..T.... ......
      40:	0000209f 0000177c 000020ad 000018a0     . ..|.... ......
      50:	000020bb 00001b04 00002040 000024a4     . ......@ ...$..
      60:	6ed9eba1 000001ec 478dde64 b0000001     ...n....d..G....
      70:	79bc3854 9d839db0 d18053ce 0000084c     T8.y.....S..L...
      80:	00000a80 5a82799a a57d8666 00001779     .....y.Zf.}.y...
      90:	00001b00 00000548 00000304 00002000     ....H........ ..
      a0:	000000bc                                ....

000000a4 <_init>:
      a4:	008136                                    	entry	a1, 64
      a7:	ffd681                                    	l32r	a8, 0 (f4 <frame_dummy>)
      aa:	f03d                                    	nop.n
      ac:	0008e0                                    	callx8	a8
      af:	ffd581                                    	l32r	a8, 4 (1cc4 <__do_global_ctors_aux>)
      b2:	f03d                                    	nop.n
      b4:	0008e0                                    	callx8	a8
      b7:	f01d                                    	retw.n

000000b9 <_init+0x15>:
      b9:	000000                                        ...

000000bc <__do_global_dtors_aux>:
      bc:	004136                                    	entry	a1, 32
      bf:	ffd221                                    	l32r	a2, 8 (202c <__do_global_dtors_aux.completed>)
      c2:	000232                                    	l8ui	a3, a2, 0
      c5:	63ec                                    	bnez.n	a3, ef <__do_global_dtors_aux+0x33>

000000c7 <__do_global_dtors_aux+0xb>:
      c7:	1248                                    	l32i.n	a4, a2, 4
      c9:	0438                                    	l32i.n	a3, a4, 0
      cb:	04028424000163ae                  	{ beqz.w15	a3, dc <__do_global_dtors_aux+0x20>; addi	a4, a4, 4 }
      d3:	1249                                    	s32i.n	a4, a2, 4
      d5:	0003e0                                    	callx8	a3
      d8:	fffac6                                    	j	c7 <__do_global_dtors_aux+0xb>

000000db <__do_global_dtors_aux+0x1f>:
	...

000000dc <__do_global_dtors_aux+0x20>:
      dc:	ffcc31                                    	l32r	a3, c (0 <_text_start>)
      df:	ffcca1                                    	l32r	a10, 10 (1ff8 <__FRAME_END__>)
      e2:	438c                                    	beqz.n	a3, ea <__do_global_dtors_aux+0x2e>
      e4:	ffca81                                    	l32r	a8, c (0 <_text_start>)
      e7:	0008e0                                    	callx8	a8
      ea:	130c                                    	movi.n	a3, 1
      ec:	004232                                    	s8i	a3, a2, 0
      ef:	f01d                                    	retw.n

000000f1 <__do_global_dtors_aux+0x35>:
      f1:	000000                                        ...

000000f4 <frame_dummy>:
      f4:	004136                                    	entry	a1, 32
      f7:	ffc721                                    	l32r	a2, 14 (0 <_text_start>)
      fa:	ffc5a1                                    	l32r	a10, 10 (1ff8 <__FRAME_END__>)
      fd:	ffc6b1                                    	l32r	a11, 18 (2014 <frame_dummy.object>)
     100:	428c                                    	beqz.n	a2, 108 <frame_dummy+0x14>
     102:	ffc481                                    	l32r	a8, 14 (0 <_text_start>)
     105:	0008e0                                    	callx8	a8

00000108 <frame_dummy+0x14>:
     108:	f01d                                    	retw.n

0000010a <frame_dummy+0x16>:
	...

0000010c <_start>:
     10c:	004136                                    	entry	a1, 32
     10f:	ffc331                                    	l32r	a3, 1c (20cc <printf_ptr>)
     112:	0228                                    	l32i.n	a2, a2, 0
     114:	ffc3a1                                    	l32r	a10, 20 (205c <export_parameter_array+0x1c>)
     117:	ffc3b1                                    	l32r	a11, 24 (170 <fix_fft_Init>)
     11a:	0329                                    	s32i.n	a2, a3, 0
     11c:	0002e0                                    	callx8	a2
     11f:	0328                                    	l32i.n	a2, a3, 0
     121:	ffc1a1                                    	l32r	a10, 28 (206f <export_parameter_array+0x2f>)
     124:	ffc2b1                                    	l32r	a11, 2c (d08 <fix_fft15>)
     127:	0002e0                                    	callx8	a2
     12a:	0328                                    	l32i.n	a2, a3, 0
     12c:	ffc1a1                                    	l32r	a10, 30 (207f <export_parameter_array+0x3f>)
     12f:	ffc1b1                                    	l32r	a11, 34 (1154 <fix_fft30>)
     132:	0002e0                                    	callx8	a2
     135:	0328                                    	l32i.n	a2, a3, 0
     137:	ffc0a1                                    	l32r	a10, 38 (208f <export_parameter_array+0x4f>)
     13a:	ffc0b1                                    	l32r	a11, 3c (1518 <fix_fft40>)
     13d:	0002e0                                    	callx8	a2
     140:	0328                                    	l32i.n	a2, a3, 0
     142:	ffbfa1                                    	l32r	a10, 40 (209f <export_parameter_array+0x5f>)
     145:	ffbfb1                                    	l32r	a11, 44 (177c <FFT4N>)
     148:	0002e0                                    	callx8	a2
     14b:	0328                                    	l32i.n	a2, a3, 0
     14d:	ffbea1                                    	l32r	a10, 48 (20ad <export_parameter_array+0x6d>)
     150:	ffbfb1                                    	l32r	a11, 4c (18a0 <FFT8N>)
     153:	0002e0                                    	callx8	a2
     156:	0328                                    	l32i.n	a2, a3, 0
     158:	ffbea1                                    	l32r	a10, 50 (20bb <export_parameter_array+0x7b>)
     15b:	ffbeb1                                    	l32r	a11, 54 (1b04 <FFT12N>)
     15e:	0002e0                                    	callx8	a2
     161:	ffbd21                                    	l32r	a2, 58 (2040 <export_parameter_array>)
     164:	f01d                                    	retw.n

00000166 <_start+0x5a>:
	...

00000170 <fix_fft_Init>:
     170:	004136                                    	entry	a1, 32
     173:	ffba41                                    	l32r	a4, 5c (24a4 <RotVector_40_32>)
     176:	a8a032                                    	movi	a3, 168
     179:	48d332                                    	addmi	a3, a3, 0x4800
     17c:	223a                                    	add.n	a2, a2, a3
     17e:	0429                                    	s32i.n	a2, a4, 0
     180:	f01d                                    	retw.n

00000182 <fix_fft_Init+0x12>:
	...

00000184 <fix_fft2_stage>:
     184:	004136                                    	entry	a1, 32
     187:	170c                                    	movi.n	a7, 1
     189:	110526c518a0072e                  	{ slli	a7, a7, 14; ae_l32x2f24.i	aed0, a6, 0 }
     191:	230700c00ca5f77e                  	{ ae_cvtp24a16x2.ll	aed1, a7, a7; nop }
     199:	0e1d20fb0d4c0c07c52f16000c01015f 	{ nop; nop; ae_mulzasfd32s.hh.ll	aed0, aed0, aed1; ae_mulzaafd32s.hh.ll	aed2, aed0, aed1 }
     1a9:	000003f3008300fed62303003460b3cf 	{ ae_trunca32q48	a12, aed2; ae_trunca32q48	a13, aed0; nop }
     1b9:	02c9                                    	s32i.n	a12, a2, 0
     1bb:	110526c0009e13de                  	{ s32i	a13, a3, 0; ae_l32x2f24.i	aed30, a6, 8 }
     1c3:	0e1d20fb0d4c0cf7c72f16f80fc1015f 	{ nop; nop; ae_mulzasfd32s.hh.ll	aed31, aed30, aed1; ae_mulzaafd32s.hh.ll	aed3, aed30, aed1 }
     1d3:	000003f3788300fff62303003c60b3ef 	{ ae_trunca32q48	a14, aed3; ae_trunca32q48	a15, aed31; nop }
     1e3:	04e9                                    	s32i.n	a14, a4, 0
     1e5:	05f9                                    	s32i.n	a15, a5, 0
     1e7:	f01d                                    	retw.n

000001e9 <fix_fft2_stage+0x65>:
     1e9:	000000                                        ...

000001ec <fix_fft5_stage>:
     1ec:	004136                                    	entry	a1, 32
     1ef:	8ea082                                    	movi	a8, 142
     1f2:	b97c                                    	movi.n	a9, -5
     1f4:	140702c000e1164e                  	{ ae_l32x2.i	aed0, a4, 8; neg	a5, a5 }
     1fc:	1305144018e1024e                  	{ ae_l32x2.i	aed3, a4, 32; ae_l32x2.i	aed1, a4, 16 }
     204:	1205544218821c3e                  	{ movi	a3, 188; ae_l32x2.i	aed2, a4, 24 }
     20c:	84a062                                    	movi	a6, 132
     20f:	47d882                                    	addmi	a8, a8, 0x4700
     212:	1406894100a6050e                  	{ ae_slaa32s	aed0, aed0, a5; slli	a9, a9, 12 }
     21a:	260403c100bed53e                  	{ ae_slaa32s	aed3, aed3, a5; movi	a7, -93 }
     222:	0e94e68bc90233d0c580801a1460751f 	{ ae_slaa32s	aed1, aed1, a5; addmi	a3, a3, 0x7900; ae_addandsub32s	aed0, aed3, aed0, aed3; nop }
     232:	160296c100a7852e                  	{ ae_slaa32s	aed2, aed2, a5; addmi	a6, a6, 0xffff9d00 }
     23a:	0e95468b31e793d0c48081110041586f 	{ ae_movda16	aed5, a8; ae_movda16	aed6, a9; ae_addandsub32s	aed1, aed2, aed1, aed2; nop }
     24a:	0e9563ab252733084c003d3004a5779f 	{ slli	a7, a7, 7; ae_movda16	aed4, a3; ae_sel16i.n	aed5, aed5, aed6, 1; ae_addandsub32s	aed0, aed1, aed0, aed1 }
     25a:	0de5640b3d2f6309424c0317011a4a4f 	{ ae_l32x2.i	aed25, a4, 0; ae_movda16	aed7, a6; ae_add32s	aed26, aed3, aed2; ae_mulf2p32x16x4ras	aed1, aed5, aed1, aed0, aed5 }
     26a:	0e9567abd92773d0c5809d3a1484b59f 	{ ae_slaa32s	aed6, aed25, a5; ae_movda16	aed27, a7; ae_sel16i.n	aed4, aed4, aed7, 1; nop }
     27a:	0dece57005001fd107cc61380001806f 	{ ae_add32s	aed0, aed6, aed0; movi	a15, 1; nop; ae_mulf2p32x16x4ras	aed3, aed4, aed26, aed3, aed4 }
     28a:	0ded397b0d3d0c11c44c41391c01012f 	{ ae_s32x2.ip	aed0, a2, 8; nop; nop; ae_mulf2p32x16x4ras	aed2, aed7, aed2, aed2, aed27 }
     29a:	220700c100a59f5e                  	{ ae_slaa32s	aed2, aed5, a15; nop }
     2a2:	0ea5201b09240cc0c188433a151c3f4f 	{ ae_slaa32s	aed4, aed4, a15; nop; ae_add32s	aed28, aed3, aed7; ae_add32s	aed2, aed0, aed2 }
     2b2:	0e65301b0a240c07c47383200903412f 	{ ae_addandsub32s	aed1, aed2, aed2, aed1; nop; ae_add32s	aed3, aed3, aed4; ae_sel16i	aed31, aed28, aed28, 0 }
     2c2:	0ea5041b0be40cd0c40081e01e1ed03f 	{ ae_sel16i	aed29, aed3, aed3, 0; nop; ae_addsub32s_hl_lh	aed30, aed1, aed28; nop }
     2d2:	0ea5203b0d350c10c20862eb1ce08d2f 	{ ae_s32x2.ip	aed30, a2, 8; nop; ae_subadd32s	aed0, aed2, aed29; ae_addsub32s_hl_lh	aed2, aed2, aed3 }
     2e2:	0ea5243b0d250cd0c40081f91ce0012f 	{ ae_s32x2.ip	aed0, a2, 8; nop; ae_subadd32s	aed0, aed1, aed31; nop }
     2f2:	230700c014e5992e                  	{ ae_s32x2.ip	aed2, a2, 8; nop }
     2fa:	230700c004e5992e                  	{ ae_s32x2.ip	aed0, a2, 8; nop }
     302:	f01d                                    	retw.n

00000304 <fix_fft6>:
     304:	004136                                    	entry	a1, 32
     307:	1174e0                                    	slli	a7, a4, 2
     30a:	025d                                    	mov.n	a5, a2
     30c:	0804244004c0772e                  	{ ae_l32.xp	aed0, a2, a7; movi	a9, 4 }
     314:	0eace57e090027d0c58081390401390f 	{ ae_sraa32rs	aed0, aed0, a9; ae_l32.xp	aed1, a2, a7; nop; nop }
     324:	0eace57e110027d0c58081390401791f 	{ ae_sraa32rs	aed1, aed1, a9; ae_l32.xp	aed2, a2, a7; nop; nop }
     334:	0eace57e190027d0c58081390401b92f 	{ ae_sraa32rs	aed2, aed2, a9; ae_l32.xp	aed3, a2, a7; nop; nop }
     344:	0eace57e210027d0c58081390401f93f 	{ ae_sraa32rs	aed3, aed3, a9; ae_l32.xp	aed4, a2, a7; nop; nop }
     354:	0eace57c292620d0c58081390401394f 	{ ae_sraa32rs	aed4, aed4, a9; ae_l32.i	aed5, a2, 0; nop; nop }
     364:	0ea5640691224458c58842210506795f 	{ ae_sraa32rs	aed5, aed5, a9; addx2	a2, a4, a4; ae_add32s	aed6, aed2, aed4; ae_sub32s	aed2, aed4, aed2 }
     374:	0ea56005953f2ec0c01ca0300508b4df 	{ slli	a11, a4, 3; slli	a2, a2, 2; ae_add32s	aed8, aed0, aed6; ae_add32s	aed7, aed3, aed5 }
     384:	0ea54027e1ee33c0c0a4e519006346bf 	{ neg	a4, a11; or	a12, a3, a3; ae_sub32s	aed3, aed5, aed3; ae_add32s	aed9, aed1, aed7 }
     394:	0e94c69001e118d0c400884f05283f6f 	{ l32r	a6, 60 (6ed9eba1 <_end+0x6ed9c6d5>); movi	a8, 1; ae_addandsub32s	aed8, aed9, aed8, aed9; nop }
     3a4:	3107064200bb225e                  	{ ae_s32.l.xp	aed8, a5, a2; ae_movda32	aed27, a6 }
     3ac:	0e1cfb3004780f17c5ab4e18147b585f 	{ ae_s32.l.xp	aed9, a5, a4; movi	a15, 0; ae_mulfp32x2ras	aed3, aed3, aed27; ae_mulfp32x2ras	aed2, aed2, aed27 }
     3bc:	0eace57e492037d0c58081390401b86f 	{ ae_sraa32rs	aed6, aed6, a8; ae_l32.xp	aed9, a3, a7; nop; nop }
     3cc:	0ea4f43e51f037c8c5b040310460387f 	{ ae_sraa32rs	aed28, aed7, a8; ae_l32.xp	aed10, a3, a7; ae_sub32s	aed0, aed0, aed6; ae_neg32	aed12, aed2 }
     3dc:	0ea4f43ee95037c8c5c061e10461799f 	{ ae_sraa32rs	aed9, aed9, a9; ae_l32.xp	aed29, a3, a7; ae_sub32s	aed1, aed1, aed28; ae_neg32	aed16, aed3 }
     3ec:	0eace57e596137d0c5808139040179df 	{ ae_sraa32rs	aed13, aed29, a9; ae_l32.xp	aed11, a3, a7; nop; nop }
     3fc:	0eace57ef96037d0c58081390401f9bf 	{ ae_sraa32rs	aed15, aed11, a9; ae_l32.xp	aed31, a3, a7; nop; nop }
     40c:	0eace57c712730d0c58081390401b9ff 	{ ae_sraa32rs	aed6, aed31, a9; ae_l32.i	aed14, a3, 0; nop; nop }
     41c:	0e95269b09240cd0c5808d3104c4f9ef 	{ ae_sraa32rs	aed7, aed14, a9; nop; ae_addandsub32s	aed4, aed6, aed13, aed6; nop }
     42c:	0ea5241b0e3c0cc0c009e9231111fd2f 	{ ae_movad32.l	a13, aed2; nop; ae_add32s	aed17, aed9, aed4; ae_add32s	aed2, aed7, aed15 }
     43c:	000000cd320000f991e6ee31201b784f 	{ ae_sraa32rs	aed4, aed4, a8; ae_movad32.l	a3, aed12; ae_mulfp32x2ras	aed6, aed6, aed27 }
     44c:	0ea52c3b09fc0c684db0e9210464b9af 	{ ae_sraa32rs	aed30, aed10, a9; nop; ae_sub32s	aed4, aed9, aed4; ae_addandsub32s	aed12, aed13, aed15, aed7 }
     45c:	000001ad120000ff11e6ee98301b782f 	{ ae_sraa32rs	aed2, aed2, a8; ae_movad32.l	a14, aed4; ae_mulfp32x2ras	aed19, aed13, aed27 }
     46c:	0ea5203b0d050cb8c1dcde1300628aef 	{ ae_add32s	aed18, aed30, aed12; nop; ae_sub32s	aed2, aed30, aed2; ae_add32	aed23, aed0, aed6 }
     47c:	0e95229b0d1d0cc0c18c41981281820f 	{ ae_add32s	aed2, aed16, aed2; nop; ae_addandsub32s	aed1, aed20, aed1, aed19; ae_add32s	aed3, aed3, aed2 }
     48c:	0e1d7b2639baed1fedab4e09043b781f 	{ ae_sraa32rs	aed21, aed1, a8; add	a7, a14, a13; ae_mulfp32x2ras	aed1, aed1, aed27; ae_mulfp32x2ras	aed22, aed3, aed27 }
     49c:	0e95628bd1007450c580d1910567f83f 	{ ae_sraa32rs	aed3, aed3, a8; ae_movda32	aed26, a7; ae_addandsub32s	aed7, aed11, aed17, aed18; ae_sub32	aed0, aed0, aed6 }
     4ac:	0e9d772619cae3c0c192ce11045b782f 	{ ae_sraa32rs	aed25, aed2, a8; add	a3, a14, a3; ae_mulfp32x2ras	aed2, aed2, aed27; ae_add32s	aed4, aed21, aed22 }
     4bc:	0e25782bf47834a7f7ab43081461c6cf 	{ ae_s32.l.xp	aed7, a12, a2; ae_movda32	aed30, a3; ae_sub32s	aed1, aed3, aed1; ae_mulfp32x2ras	aed27, aed20, aed27 }
     4cc:	0e95369b09dd0c484d909a090781384f 	{ ae_sraa32rs	aed24, aed20, a8; nop; ae_addandsub32s	aed1, aed28, aed26, aed1; ae_addandsub32s	aed4, aed9, aed23, aed4 }
     4dc:	0ea5201b0c740c58c58b1bc8151dd8cf 	{ ae_s32.l.xp	aed11, a12, a4; nop; ae_add32s	aed29, aed27, aed25; ae_sub32s	aed2, aed2, aed24 }
     4ec:	0e95269b0c640cd0c58080101440065f 	{ ae_s32.l.xp	aed4, a5, a2; nop; ae_addandsub32s	aed0, aed2, aed0, aed2; nop }
     4fc:	230700c208a5345e                  	{ ae_s32.l.xp	aed9, a5, a4; nop }
     504:	0ead245b0c640cd0c58080e8178142cf 	{ ae_s32.l.xp	aed1, a12, a2; nop; ae_neg32s	aed1, aed29; nop }
     514:	0e95269b0c640cd0c5809e0a143f1ccf 	{ ae_s32.l.xp	aed28, a12, a4; nop; ae_addandsub32s	aed31, aed1, aed30, aed1; nop }
     524:	230700c000a5325e                  	{ ae_s32.l.xp	aed0, a5, a2; nop }
     52c:	230700c010a53f5e                  	{ ae_s32.l.xp	aed2, a5, a15; nop }
     534:	230700c718a532ce                  	{ ae_s32.l.xp	aed31, a12, a2; nop }
     53c:	230700c008a53fce                  	{ ae_s32.l.xp	aed1, a12, a15; nop }
     544:	f01d                                    	retw.n

00000546 <fix_fft6+0x242>:
	...

00000548 <fix_fft12>:
     548:	012136                                    	entry	a1, 144
     54b:	20a052                                    	movi	a5, 32
     54e:	40a0b2                                    	movi	a11, 64
     551:	20d220                                    	or	a13, a2, a2
     554:	26a0f2                                    	movi	a15, 38
     557:	00a0c2                                    	movi	a12, 0
     55a:	02a072                                    	movi	a7, 2
     55d:	2a05324008c2f52e                  	{ ae_l32x2.x	aed1, a2, a5; ae_l32x2.x	aed2, a2, a11 }
     565:	2d0532401ca0c9de                  	{ ae_l32x2.ip	aed3, a13, 8; ae_l32x2.x	aed0, a2, a12 }
     56d:	0eace56c8902ffd0c58081390401771f 	{ ae_sraa32rs	aed1, aed1, a7; addmi	a15, a15, 0xffff9100; nop; nop }
     57d:	0eace57b2104dbd0c58081390401b72f 	{ ae_sraa32rs	aed2, aed2, a7; ae_l32x2.x	aed4, a13, a11; nop; nop }
     58d:	0e94e69bb904d5d0c58081110441f70f 	{ ae_sraa32rs	aed3, aed0, a7; ae_l32x2.x	aed23, a13, a5; ae_addandsub32s	aed1, aed2, aed1, aed2; nop }
     59d:	0ea4c41001e11ed0c480830901190f6f 	{ ae_movda16	aed0, a15; movi	a14, 1; ae_add32s	aed25, aed3, aed1; nop }
     5ad:	0dece1608102131685d4413904017e1f 	{ ae_sraa32rs	aed1, aed1, a14; addi	a3, a1, 16; nop; ae_mulf2p32x16x4s	aed2, aed26, aed2, aed2, aed0 }
     5bd:	0ea4e43bc125dcd0c58083090461777f 	{ ae_sraa32rs	aed5, aed23, a7; ae_l32x2.x	aed24, a13, a12; ae_sub32s	aed1, aed3, aed1; nop }
     5cd:	0ead6567e12633d0c58081390401374f 	{ ae_sraa32rs	aed4, aed4, a7; or	a12, a3, a3; nop; nop }
     5dd:	0e94e69001211ad0c58085210482fe9f 	{ ae_sraa32rs	aed7, aed25, a14; movi	a10, 1; ae_addandsub32s	aed2, aed4, aed5, aed4; nop }
     5ed:	0dece17069c0a42749d481390401fe1f 	{ ae_sraa32rs	aed27, aed1, a14; movi	a4, 218; nop; ae_mulf2p32x16x4s	aed4, aed29, aed4, aed4, aed0 }
     5fd:	0eace56b710344d0c58081390401feaf 	{ ae_sraa32rs	aed3, aed26, a14; addmi	a4, a4, 0x6e00; nop; nop }
     60d:	0e956665d527a2d0c4009b191ca4c5cf 	{ ae_s32x2.ip	aed7, a12, 8; slli	a10, a10, 14; ae_addandsub32js	aed4, aed5, aed27, aed3; nop }
     61d:	0eace57001212bd0c58081390401b78f 	{ ae_sraa32rs	aed6, aed24, a7; movi	a11, 2; nop; nop }
     62d:	0eace57021e009d0c580813904013e2f 	{ ae_sraa32rs	aed28, aed2, a14; movi	a9, 64; nop; nop }
     63d:	0ea4e41005310858c40786111d0605cf 	{ ae_s32x2.ip	aed4, a12, 8; movi	a8, 0; ae_add32s	aed6, aed6, aed2; ae_sub32s	aed1, aed6, aed28 }
     64d:	0ead6567b10644d0c580813904017e1f 	{ ae_sraa32rs	aed1, aed1, a14; or	a6, a4, a4; nop; nop }
     65d:	0eace57001012fd0c58081390401fedf 	{ ae_sraa32rs	aed3, aed29, a14; movi	a15, 2; nop; nop }
     66d:	0e95267b0d250cd0c40081191fc445cf 	{ ae_s32x2.ip	aed5, a12, 8; nop; ae_addandsub32js	aed4, aed30, aed1, aed3; nop }
     67d:	0ead257b09e40cd0c58081390401fe6f 	{ ae_sraa32rs	aed31, aed6, a14; nop; nop; nop }
     68d:	2e833090300044ae                  	{ ae_movda16x2	aed2, a4, a10; nop; nop }
     695:	0b069a471ce989ce                  	{ ae_s32x2.ip	aed31, a12, 8; or	a4, a10, a10 }
     69d:	cb8b76                                    	loop	a11, 76c <fix_fft12+0x224>
     6a0:	0e94e3a7f46bff10c2047c221c84ff5f 	{ addi	a5, a15, -1; addi	a15, a15, -2; ae_sel16i.n	aed4, aed4, aed4, 0; ae_addsub32s_hl_lh	aed1, aed1, aed3 }
     6b0:	0dd563a92d37d92088789c0e1421a6ff 	{ moveqz	a10, a6, a15; ae_l32x2.ip	aed5, a13, 8; ae_sel16i.n	aed1, aed1, aed1, 0; ae_mulfpc32x16x2ras	aed4, aed2, aed4, aed4, aed2 }
     6c0:	0f2c                                    	movi.n	a15, 32
     6c2:	3b0c                                    	movi.n	a11, 3
     6c4:	28053d4108c6ffde                  	{ ae_l32x2.x	aed5, a13, a15; ae_l32x2.x	aed6, a13, a9 }
     6cc:	0e9567a5d926b2d0c5809c210484775f 	{ ae_sraa32rs	aed5, aed5, a7; slli	a11, a11, 14; ae_sel16i.n	aed4, aed4, aed4, 0; nop }
     6dc:	0ead6567592645d0c58081390401b76f 	{ ae_sraa32rs	aed6, aed6, a7; movnez	a11, a4, a5; nop; nop }
     6ec:	0e94e69b3d25d8d0c40085311ca605cf 	{ ae_s32x2.ip	aed4, a12, 8; ae_l32x2.x	aed7, a13, a8; ae_addandsub32s	aed6, aed5, aed5, aed6; nop }
     6fc:	2e83309030004bae                  	{ ae_movda16x2	aed2, a11, a10; nop; nop }
     704:	0ded6167f926552a0bd4a1390401f77f 	{ ae_sraa32rs	aed7, aed7, a7; or	a15, a5, a5; nop; ae_mulf2p32x16x4s	aed5, aed8, aed5, aed5, aed0 }
     714:	0de5201b09f40c084bf827310506be6f 	{ ae_sraa32rs	aed30, aed6, a14; nop; ae_add32s	aed6, aed7, aed6; ae_mulfpc32x16x2ras	aed5, aed1, aed1, aed1, aed2 }
     724:	0ea5243b09240cd0c58087f10463be6f 	{ ae_sraa32rs	aed6, aed6, a14; nop; ae_sub32s	aed3, aed7, aed30; nop }
     734:	0e9527bb09040cd0c5809c2907e57e3f 	{ ae_sraa32rs	aed1, aed3, a14; nop; ae_sel16i.n	aed31, aed5, aed5, 0; nop }
     744:	230700c71ce599ce                  	{ ae_s32x2.ip	aed31, a12, 8; nop }
     74c:	0ead257b09040cd0c58081390401fe8f 	{ ae_sraa32rs	aed3, aed8, a14; nop; nop; nop }
     75c:	0e95267b0d250cd0c40081191ca485cf 	{ ae_s32x2.ip	aed6, a12, 8; nop; ae_addandsub32js	aed4, aed5, aed1, aed3; nop }

0000076c <fix_fft12+0x224>:
     76c:	0eacf4501d3804c8c404a020038003af 	{ movi	a10, 3; movi	a4, 48; ae_neg32s	aed0, aed4; ae_neg32s	aed1, aed5 }
     77c:	000042400080005e                  	{ movi	a5, 0; ae_sel16i	aed0, aed4, aed0, 0 }
     784:	260c                                    	movi.n	a6, 2
     786:	1704084004e099ce                  	{ ae_s32x2.ip	aed0, a12, 8; movi	a7, 24 }
     78e:	884c                                    	movi.n	a8, 72
     790:	090401400ce099ce                  	{ ae_s32x2.ip	aed1, a12, 8; movi	a9, 1 }
     798:	0020f0                                    	nop
     79b:	f03d                                    	nop.n
     79d:	a78a76                                    	loop	a10, 848 <fix_fft12+0x300>
     7a0:	2605334000c3f53e                  	{ ae_l32x2.x	aed0, a3, a5; ae_l32x2.x	aed3, a3, a7 }
     7a8:	2405334008c2e83e                  	{ ae_l32x2.x	aed1, a3, a8; ae_l32x2.x	aed2, a3, a4 }
     7b0:	0ead6569f92639d0c58081390401362f 	{ ae_sraa32rs	aed4, aed2, a6; ae_l32x2.ip	aed31, a3, 8; nop; nop }
     7c0:	0ead257b09040cd0c58081390401360f 	{ ae_sraa32rs	aed0, aed0, a6; nop; nop; nop }
     7d0:	0ea5241b09240cd0c58080210500761f 	{ ae_sraa32rs	aed5, aed1, a6; nop; ae_add32s	aed0, aed0, aed4; nop }
     7e0:	0ead257b09040cd0c58081390401f63f 	{ ae_sraa32rs	aed3, aed3, a6; nop; nop; nop }
     7f0:	0ea5241b09040cd0c58083290503791f 	{ ae_sraa32rs	aed1, aed1, a9; nop; ae_add32s	aed3, aed3, aed5; nop }
     800:	0ea5203b09040c284d9063090461b92f 	{ ae_sraa32rs	aed2, aed2, a9; nop; ae_sub32s	aed1, aed3, aed1; ae_addandsub32s	aed4, aed5, aed0, aed3 }
     810:	0ea5243b0c440cd0c58080101460152f 	{ ae_s32x2.x	aed4, a2, a5; nop; ae_sub32s	aed0, aed0, aed2; nop }
     820:	0e95227b0c440c10c38820081420542f 	{ ae_s32x2.x	aed5, a2, a4; nop; ae_addandsub32js	aed0, aed1, aed0, aed1; ae_addsub32s_hl_lh	aed2, aed0, aed1 }
     830:	220760c014c5572e                  	{ ae_s32x2.x	aed2, a2, a7; nop }
     838:	220760c004c5582e                  	{ ae_s32x2.x	aed0, a2, a8; nop }
     840:	230720c004a5d92e                  	{ ae_l32x2.ip	aed0, a2, 8; nop }
     848:	f01d                                    	retw.n

0000084a <fix_fft12+0x302>:
	...

0000084c <fix_fft15_stage>:
     84c:	006136                                    	entry	a1, 48
     84f:	fe0581                                    	l32r	a8, 64 (1ec <fix_fft5_stage>)
     852:	067d                                    	mov.n	a7, a6
     854:	04ad                                    	mov.n	a10, a4
     856:	04cd                                    	mov.n	a12, a4
     858:	06dd                                    	mov.n	a13, a6
     85a:	28c462                                    	addi	a6, a4, 40
     85d:	50c432                                    	addi	a3, a4, 80
     860:	0008e0                                    	callx8	a8
     863:	3169                                    	s32i.n	a6, a1, 12
     865:	06ad                                    	mov.n	a10, a6
     867:	06cd                                    	mov.n	a12, a6
     869:	fdfe61                                    	l32r	a6, 64 (1ec <fix_fft5_stage>)
     86c:	07dd                                    	mov.n	a13, a7
     86e:	0006e0                                    	callx8	a6
     871:	03ad                                    	mov.n	a10, a3
     873:	03cd                                    	mov.n	a12, a3
     875:	07dd                                    	mov.n	a13, a7
     877:	f77c                                    	movi.n	a7, -1
     879:	0006e0                                    	callx8	a6
     87c:	140554401080168e                  	{ movi	a8, 38; ae_l32x2.i	aed0, a4, 40 }
     884:	180298c008e4023e                  	{ ae_l32x2.i	aed1, a3, 0; addmi	a8, a8, 0xffff9100 }
     88c:	0e95668b152783d0c400800d0020c24f 	{ ae_l32x2.i	aed3, a4, 0; ae_movda16	aed2, a8; ae_addandsub32s	aed0, aed1, aed0, aed1; nop }
     89c:	0de560064934550943cc23021500370f 	{ ae_slaa32s	aed4, aed0, a7; addx4	a9, a5, a5; ae_add32s	aed0, aed3, aed0; ae_mulf2p32x16x4ras	aed1, aed5, aed1, aed1, aed2 }
     8ac:	0e0460400880543e                  	{ ae_sub32s	aed1, aed3, aed4; movi	a15, 0 }
     8b4:	11a9d0                                    	slli	a10, a9, 3
     8b7:	0e956665cc469ed0c58081281434332f 	{ ae_s32x2.x	aed0, a2, a15; slli	a9, a9, 2; ae_addandsub32js	aed20, aed1, aed1, aed5; nop }
     8c7:	26046c4504c04a2e                  	{ ae_s32x2.x	aed20, a2, a10; movi	a6, 44 }
     8cf:	0406e5400cd6592e                  	{ ae_s32x2.x	aed1, a2, a9; addx2	a8, a5, a5 }
     8d7:	3d06884000e61a4e                  	{ ae_l32x2.i	aed0, a4, 48; slli	a11, a8, 3 }
     8df:	1005144008f5163e                  	{ ae_l32x2.i	aed1, a3, 8; ae_l32x2.i	aed21, a4, 8 }
     8e7:	0e956685e5275ed0c40080080420d8cf 	{ slli	a13, a8, 4; slli	a12, a5, 2; ae_addandsub32s	aed0, aed1, aed0, aed1; nop }
     8f7:	0dece1701d314a0d824c21380401f5df 	{ slli	a15, a5, 3; movi	a10, 52; nop; ae_mulf2p32x16x4ras	aed1, aed22, aed1, aed1, aed2 }
     907:	0ea56405c1068ed0c58095021500770f 	{ ae_slaa32s	aed1, aed0, a7; slli	a8, a8, 2; ae_add32s	aed0, aed21, aed0; nop }
     917:	0ea4e4281c441bd0c58095081461232f 	{ ae_s32x2.x	aed0, a2, a11; l32i	a11, a1, 12; ae_sub32s	aed1, aed21, aed1; nop }
     927:	d16560                                    	mul16s	a6, a5, a6
     92a:	0ead257b0a040cd0c400813d1001bb1f 	{ ae_addandsub32js	aed23, aed1, aed1, aed22; nop; nop; nop }
     93a:	0a06e5451cc75c2e                  	{ ae_s32x2.x	aed23, a2, a12; slli	a12, a5, 5 }
     942:	2406f5c00cc5562e                  	{ ae_s32x2.x	aed1, a2, a6; subx8	a6, a5, a5 }
     94a:	2f06864000e70e4e                  	{ ae_l32x2.i	aed0, a4, 56; slli	a14, a6, 2 }
     952:	1205144008f80a3e                  	{ ae_l32x2.i	aed1, a3, 16; ae_l32x2.i	aed24, a4, 16 }
     95a:	0ead257b0a240cd0c40081380801200f 	{ ae_addandsub32s	aed0, aed1, aed0, aed1; nop; nop; nop }
     96a:	0ded217b0d540c0e434c21380c01015f 	{ nop; nop; nop; ae_mulf2p32x16x4ras	aed1, aed25, aed1, aed1, aed2 }
     97a:	0ea5241b09040cd0c58098021500770f 	{ ae_slaa32s	aed1, aed0, a7; nop; ae_add32s	aed0, aed24, aed0; nop }
     98a:	0ea56426ec4455d0c58098081461312f 	{ ae_s32x2.x	aed0, a2, a13; addx8	a13, a5, a5; ae_sub32s	aed1, aed24, aed1; nop }
     99a:	0ead257b0a040cd0c400813e0801ae1f 	{ ae_addandsub32js	aed26, aed1, aed1, aed25; nop; nop; nop }
     9aa:	2c06e54614c74e2e                  	{ ae_s32x2.x	aed26, a2, a14; slli	a14, a5, 4 }
     9b2:	3c06e6400cc75f2e                  	{ ae_s32x2.x	aed1, a2, a15; slli	a15, a6, 3 }
     9ba:	1205134000e11ebe                  	{ ae_l32x2.i	aed0, a11, 24; ae_l32x2.i	aed1, a3, 24 }
     9c2:	0ead2563da204ed0c40081380801200f 	{ ae_addandsub32s	aed0, aed1, aed0, aed1; ae_l32x2.i	aed27, a4, 24; nop; nop }
     9d2:	0ded217b0d540c0f034c21380c01015f 	{ nop; nop; nop; ae_mulf2p32x16x4ras	aed1, aed28, aed1, aed1, aed2 }
     9e2:	0ea5241b09040cd0c5809b021500770f 	{ ae_slaa32s	aed1, aed0, a7; nop; ae_add32s	aed0, aed27, aed0; nop }
     9f2:	220760c0088571be                  	{ ae_sub32s	aed1, aed27, aed1; nop }
     9fa:	d195a0                                    	mul16s	a9, a5, a10
     9fd:	0e95267b0c440cd0c58081e0143d202f 	{ ae_s32x2.x	aed0, a2, a8; nop; ae_addandsub32js	aed29, aed1, aed1, aed28; nop }
     a0d:	220760c70cc5592e                  	{ ae_s32x2.x	aed29, a2, a9; nop }
     a15:	220760c00cc55c2e                  	{ ae_s32x2.x	aed1, a2, a12; nop }
     a1d:	15051b4008e0023e                  	{ ae_l32x2.i	aed1, a3, 32; ae_l32x2.i	aed0, a11, 32 }
     a25:	0e9566859d27ded0c400800f00209e4f 	{ ae_l32x2.i	aed30, a4, 32; slli	a3, a13, 2; ae_addandsub32s	aed0, aed1, aed0, aed1; nop }
     a35:	0ded217b0d540c08834c21380c01015f 	{ nop; nop; nop; ae_mulf2p32x16x4ras	aed1, aed2, aed1, aed1, aed2 }
     a45:	0ea5241b09040cd0c5809e021500770f 	{ ae_slaa32s	aed1, aed0, a7; nop; ae_add32s	aed0, aed30, aed0; nop }
     a55:	220760c0088571ee                  	{ ae_sub32s	aed1, aed30, aed1; nop }
     a5d:	0e95267b0c440cd0c5808110143f032f 	{ ae_s32x2.x	aed0, a2, a3; nop; ae_addandsub32js	aed31, aed1, aed1, aed2; nop }
     a6d:	220760c71cc55e2e                  	{ ae_s32x2.x	aed31, a2, a14; nop }
     a75:	220760c00cc55f2e                  	{ ae_s32x2.x	aed1, a2, a15; nop }
     a7d:	f01d                                    	retw.n

00000a7f <fix_fft15_stage+0x233>:
	...

00000a80 <fix_fft5_8K>:
     a80:	004136                                    	entry	a1, 32
     a83:	90d440                                    	addx2	a13, a4, a4
     a86:	1194d0                                    	slli	a9, a4, 3
     a89:	11a4c0                                    	slli	a10, a4, 4
     a8c:	11b4e0                                    	slli	a11, a4, 2
     a8f:	2905324500a01dee                  	{ slli	a4, a13, 2; ae_l32x2.x	aed0, a2, a9 }
     a97:	2a055240008104ce                  	{ movi	a12, 4; ae_l32x2.x	aed1, a2, a10 }
     a9f:	2405324010c3eb2e                  	{ ae_l32x2.x	aed2, a2, a11; ae_l32x2.x	aed3, a2, a4 }
     aa7:	0e04004310a08c2e                  	{ ae_sraa32s	aed2, aed2, a12; movi	a14, 0 }
     aaf:	2e05124310a84c1e                  	{ ae_sraa32s	aed1, aed1, a12; ae_l32x2.x	aed8, a2, a14 }
     ab7:	0e94e69001001fd0c580820b04412c0f 	{ ae_sraa32s	aed0, aed0, a12; movi	a15, 1; ae_addandsub32s	aed1, aed2, aed2, aed1; nop }
     ac7:	220700c310a5dc3e                  	{ ae_sraa32s	aed3, aed3, a12; nop }
     acf:	0e95069b09e50cd0c400801f0c606b5f 	{ l32r	a5, 6c (b0000001 <_end+0xafffdb35>); nop; ae_addandsub32s	aed0, aed3, aed0, aed3; nop }
     adf:	0e95468b39e154d0c40081070c20697f 	{ l32r	a7, 74 (9d839db0 <_end+0x9d8378e4>); ae_movda32	aed7, a5; ae_addandsub32s	aed0, aed1, aed1, aed0; nop }
     aef:	fd5e31                                    	l32r	a3, 68 (478dde64 <_end+0x478db998>)
     af2:	248327392007675e                  	{ ae_movda32	aed4, a7; nop; ae_mulfp32x2ts	aed7, aed0, aed7 }
     afa:	0e9d434b29f134c0c0086c271c44596f 	{ l32r	a6, 70 (79bc3854 <_end+0x79bc1388>); ae_movda32	aed5, a3; ae_mulfp32x2ts	aed4, aed2, aed4; ae_add32s	aed2, aed2, aed3 }
     b0a:	fd5b81                                    	l32r	a8, 78 (d18053ce <_end+0xd1802f02>)
     b0d:	2483270e3025665e                  	{ ae_movda32	aed26, a6; nop; ae_mulfp32x2ts	aed1, aed1, aed5 }
     b15:	0e9d674b31c084d0c5808c13045aec8f 	{ ae_sraa32s	aed27, aed8, a12; ae_movda32	aed6, a8; ae_mulfp32x2ts	aed2, aed2, aed26; nop }
     b25:	0e9d3b5b09fc0cc0c1800c3214663f7f 	{ ae_slaa32s	aed28, aed7, a15; nop; ae_mulfp32x2ts	aed6, aed3, aed6; ae_add32s	aed0, aed27, aed0 }
     b35:	0ea5241b09240cd0c58080e215033f4f 	{ ae_slaa32s	aed4, aed4, a15; nop; ae_add32s	aed3, aed0, aed28; nop }
     b45:	0ea5201b0a340cc0c01082300902613f 	{ ae_addandsub32s	aed1, aed3, aed3, aed1; nop; ae_add32s	aed2, aed2, aed6; ae_add32s	aed4, aed2, aed4 }
     b55:	0e9523bb0c4c0c10c3f85c2107a4232f 	{ ae_s32x2.xp	aed0, a2, a11; nop; ae_sel16i.n	aed29, aed4, aed4, 0; ae_addsub32s_hl_lh	aed30, aed1, aed2 }
     b65:	0ea5203b0c5c0c10c38c83eb04e0af2f 	{ ae_s32x2.xp	aed30, a2, a11; nop; ae_subadd32s	aed0, aed3, aed29; ae_addsub32s_hl_lh	aed3, aed3, aed4 }
     b75:	0500614004df6b2e                  	{ ae_s32x2.xp	aed0, a2, a11; ae_sel16i	aed31, aed2, aed2, 0 }
     b7d:	0ea5243b0c440cd0c58081f904e0e32f 	{ ae_s32x2.xp	aed3, a2, a11; nop; ae_subadd32s	aed0, aed1, aed31; nop }
     b8d:	230760c004c57e2e                  	{ ae_s32x2.xp	aed0, a2, a14; nop }
     b95:	f01d                                    	retw.n

00000b97 <fix_fft5_8K+0x117>:
	...

00000b98 <fix_fft10>:
     b98:	014136                                    	entry	a1, 160
     b9b:	907440                                    	addx2	a7, a4, a4
     b9e:	50c152                                    	addi	a5, a1, 80
     ba1:	030c                                    	movi.n	a3, 0
     ba3:	1187d0                                    	slli	a8, a7, 3
     ba6:	1164b0                                    	slli	a6, a4, 5
     ba9:	1194d0                                    	slli	a9, a4, 3
     bac:	3406b54000cbf32e                  	{ ae_l32x2.x	aed0, a2, a3; or	a15, a5, a5 }
     bb4:	2c06a44010c6e82e                  	{ ae_l32x2.x	aed2, a2, a8; slli	a10, a4, 4 }
     bbc:	6189                                    	s32i.n	a8, a1, 24
     bbe:	00008139d4e426004003032ab00080ff 	{ ae_s32x2x2.ip	aed0, aed2, a15, 16; ae_l32x2.x	aed30, a2, a6; nop }
     bce:	fd2581                                    	l32r	a8, 64 (1ec <fix_fft5_stage>)
     bd1:	280572400461313e                  	{ addi	a3, a1, 32; ae_l32x2.x	aed1, a2, a9 }
     bd9:	7169                                    	s32i.n	a6, a1, 28
     bdb:	1164e0                                    	slli	a6, a4, 2
     bde:	2b053241089f01ae                  	{ s32i	a10, a1, 20; ae_l32x2.x	aed31, a2, a10 }
     be6:	0000813054e05d0041830321b00001ff 	{ ae_s32x2x2.ip	aed1, aed30, a15, 16; movi	a13, 5; nop }
     bf6:	05cd                                    	mov.n	a12, a5
     bf8:	4199                                    	s32i.n	a9, a1, 16
     bfa:	3169                                    	s32i.n	a6, a1, 12
     bfc:	2206934718ea92fe                  	{ ae_s32x2.i	aed31, a15, 0; or	a10, a3, a3 }
     c04:	0008e0                                    	callx8	a8
     c07:	0406a44000dae62e                  	{ ae_l32x2.x	aed0, a2, a6; addx4	a8, a4, a4 }
     c0f:	1168e0                                    	slli	a6, a8, 2
     c12:	f09440                                    	subx8	a9, a4, a4
     c15:	11a7e0                                    	slli	a10, a7, 2
     c18:	b0f440                                    	addx8	a15, a4, a4
     c1b:	05ed                                    	mov.n	a14, a5
     c1d:	1179e0                                    	slli	a7, a9, 2
     c20:	0e06af4008c5e62e                  	{ ae_l32x2.x	aed1, a2, a6; slli	a4, a15, 2 }
     c28:	00008139d4e42a004003032aa00041ef 	{ ae_s32x2x2.ip	aed1, aed0, a14, 16; ae_l32x2.x	aed29, a2, a10; nop }
     c38:	27053240109e11ae                  	{ s32i	a10, a1, 8; ae_l32x2.x	aed30, a2, a7 }
     c40:	fd0981                                    	l32r	a8, 64 (1ec <fix_fft5_stage>)
     c43:	5d0c                                    	movi.n	a13, 5
     c45:	00008139d4e424004183032ba800deef 	{ ae_s32x2x2.ip	aed30, aed29, a14, 16; ae_l32x2.x	aed31, a2, a4; nop }
     c55:	05ad                                    	mov.n	a10, a5
     c57:	0406954718eb92ee                  	{ ae_s32x2.i	aed31, a14, 0; or	a12, a5, a5 }
     c5f:	0008e0                                    	callx8	a8
     c62:	1005154000e1023e                  	{ ae_l32x2.i	aed0, a3, 0; ae_l32x2.i	aed1, a5, 0 }
     c6a:	0e94e68825251ad0c400800d0820823f 	{ ae_l32x2.i	aed2, a3, 8; l32i	a10, a1, 16; ae_addandsub32s	aed0, aed1, aed0, aed1; nop }
     c7a:	0804004608e0065e                  	{ ae_l32x2.i	aed25, a5, 8; movi	a8, 0 }
     c82:	0e94e6882c441bd0c58082c81440202f 	{ ae_s32x2.x	aed0, a2, a8; l32i	a11, a1, 20; ae_addandsub32s	aed0, aed2, aed2, aed25; nop }
     c92:	1205154610fb0a3e                  	{ ae_l32x2.i	aed26, a3, 16; ae_l32x2.i	aed27, a5, 16 }
     c9a:	2c03e1400cc1462e                  	{ ae_s32x2.x	aed1, a2, a6; l32i	a12, a1, 24 }
     ca2:	0e952683e4403ed0c5809ad81420222f 	{ ae_s32x2.x	aed0, a2, a10; ae_l32x2.i	aed28, a3, 24; ae_addandsub32s	aed0, aed1, aed26, aed27; nop }
     cb2:	3c03814708e01e5e                  	{ ae_l32x2.i	aed29, a5, 24; l32i	a13, a1, 12 }
     cba:	71e8                                    	l32i.n	a14, a1, 28
     cbc:	1405734014de472e                  	{ ae_s32x2.x	aed2, a2, a7; ae_l32x2.i	aed30, a3, 32 }
     cc4:	0e952684fc405ed0c5809ce81440232f 	{ ae_s32x2.x	aed0, a2, a11; ae_l32x2.i	aed31, a5, 32; ae_addandsub32s	aed0, aed2, aed28, aed29; nop }
     cd4:	2e03e1400cc0542e                  	{ ae_s32x2.x	aed1, a2, a4; l32i	a15, a1, 8 }
     cdc:	0e95269b0c440cd0c5809ef81420302f 	{ ae_s32x2.x	aed0, a2, a12; nop; ae_addandsub32s	aed0, aed1, aed30, aed31; nop }
     cec:	220760c014c55d2e                  	{ ae_s32x2.x	aed2, a2, a13; nop }
     cf4:	220760c004c55e2e                  	{ ae_s32x2.x	aed0, a2, a14; nop }
     cfc:	220760c00cc55f2e                  	{ ae_s32x2.x	aed1, a2, a15; nop }
     d04:	f01d                                    	retw.n

00000d06 <fix_fft10+0x16e>:
	...

00000d08 <fix_fft15>:
     d08:	016136                                    	entry	a1, 176
     d0b:	030c                                    	movi.n	a3, 0
     d0d:	b06440                                    	addx8	a6, a4, a4
     d10:	3406a44000d4e32e                  	{ ae_l32x2.x	aed0, a2, a3; addx2	a3, a4, a4 }
     d18:	11a3e0                                    	slli	a10, a3, 2
     d1b:	10c1c2                                    	addi	a12, a1, 16
     d1e:	c52c                                    	movi.n	a5, 44
     d20:	11d3d0                                    	slli	a13, a3, 3
     d23:	0c06a44100c6fa2e                  	{ ae_l32x2.x	aed4, a2, a10; slli	a8, a4, 3 }
     d2b:	1166e0                                    	slli	a6, a6, 2
     d2e:	0cad                                    	mov.n	a10, a12
     d30:	2406a44010dbe82e                  	{ ae_l32x2.x	aed2, a2, a8; addx4	a14, a4, a4 }
     d38:	d15450                                    	mul16s	a5, a4, a5
     d3b:	3c06a34108c4ed2e                  	{ ae_l32x2.x	aed5, a2, a13; slli	a3, a3, 4 }
     d43:	0e06ae4110c6e62e                  	{ ae_l32x2.x	aed6, a2, a6; slli	a8, a14, 2 }
     d4b:	00008144d4e04b0040030323a000c0af 	{ ae_s32x2x2.ip	aed0, aed4, a10, 16; slli	a7, a4, 5; nop }
     d5b:	1194e0                                    	slli	a9, a4, 2
     d5e:	f0f440                                    	subx8	a15, a4, a4
     d61:	3c04244118c0f32e                  	{ ae_l32x2.x	aed7, a2, a3; movi	a13, 52 }
     d69:	0000813954e4280040030329b00011af 	{ ae_s32x2x2.ip	aed5, aed6, a10, 16; ae_l32x2.x	aed8, a2, a8; nop }
     d79:	d134d0                                    	mul16s	a3, a4, a13
     d7c:	2605324018c1f92e                  	{ ae_l32x2.x	aed3, a2, a9; ae_l32x2.x	aed1, a2, a7 }
     d84:	1c06af4208c6f52e                  	{ ae_l32x2.x	aed9, a2, a5; slli	a9, a15, 3 }
     d8c:	00008144d4e0ed0040830322a000d3af 	{ ae_s32x2x2.ip	aed7, aed8, a10, 16; slli	a7, a14, 3; nop }
     d9c:	0000813954e4290040830328a80081af 	{ ae_s32x2x2.ip	aed1, aed9, a10, 16; ae_l32x2.x	aed10, a2, a9; nop }
     dac:	2c06a44708c7e32e                  	{ ae_l32x2.x	aed29, a2, a3; slli	a14, a4, 4 }
     db4:	00008139d4e427004003032ab00006af 	{ ae_s32x2x2.ip	aed10, aed2, a10, 16; ae_l32x2.x	aed28, a2, a7; nop }
     dc4:	00008139d4e42e004183032ba8009caf 	{ ae_s32x2x2.ip	aed28, aed29, a10, 16; ae_l32x2.x	aed30, a2, a14; nop }
     dd4:	00008144d4e0fe004183032bb000c3af 	{ ae_s32x2x2.ip	aed3, aed30, a10, 16; slli	a15, a15, 2; nop }
     de4:	0e04254718c0ef2e                  	{ ae_l32x2.x	aed31, a2, a15; movi	a14, 5 }
     dec:	fca431                                    	l32r	a3, 7c (84c <fix_fft15_stage>)
     def:	04dd                                    	mov.n	a13, a4
     df1:	2206924718ea82ae                  	{ ae_s32x2.i	aed31, a10, 0; or	a10, a2, a2 }
     df9:	0003e0                                    	callx8	a3
     dfc:	f01d                                    	retw.n

00000dfe <fix_fft15+0xf6>:
	...

00000e00 <fix_fft20>:
     e00:	026136                                    	entry	a1, 0x130
     e03:	01d162                                    	addmi	a6, a1, 0x100
     e06:	11e4a0                                    	slli	a14, a4, 6
     e09:	e0c662                                    	addi	a6, a6, -32
     e0c:	030c                                    	movi.n	a3, 0
     e0e:	905440                                    	addx2	a5, a4, a4
     e11:	1184b0                                    	slli	a8, a4, 5
     e14:	1159                                    	s32i.n	a5, a1, 4
     e16:	1c06a54008c5ee2e                  	{ ae_l32x2.x	aed1, a2, a14; slli	a5, a5, 4 }
     e1e:	06fd                                    	mov.n	a15, a6
     e20:	2305324608a014ce                  	{ slli	a9, a4, 4; ae_l32x2.x	aed0, a2, a3 }
     e28:	290572400c6201ae                  	{ addmi	a10, a1, 0x100; ae_l32x2.x	aed2, a2, a8 }
     e30:	00008139d4e425004003032aa80080ff 	{ ae_s32x2x2.ip	aed0, aed1, a15, 16; ae_l32x2.x	aed30, a2, a5; nop }
     e40:	fc8931                                    	l32r	a3, 64 (1ec <fix_fft5_stage>)
     e43:	b0caa2                                    	addi	a10, a10, -80
     e46:	126152                                    	s32i	a5, a1, 72
     e49:	1154e0                                    	slli	a5, a4, 2
     e4c:	00008139d4e429004003032ab000deff 	{ ae_s32x2x2.ip	aed30, aed2, a15, 16; ae_l32x2.x	aed31, a2, a9; nop }
     e5c:	5d0c                                    	movi.n	a13, 5
     e5e:	06cd                                    	mov.n	a12, a6
     e60:	f189                                    	s32i.n	a8, a1, 60
     e62:	d199                                    	s32i.n	a9, a1, 52
     e64:	1161e2                                    	s32i	a14, a1, 68
     e67:	136152                                    	s32i	a5, a1, 76
     e6a:	3e069e4718e982fe                  	{ ae_s32x2.i	aed31, a15, 0; or	a7, a14, a14 }
     e72:	61a9                                    	s32i.n	a10, a1, 24
     e74:	0003e0                                    	callx8	a3
     e77:	03bd                                    	mov.n	a11, a3
     e79:	a03440                                    	addx4	a3, a4, a4
     e7c:	4c3c                                    	movi.n	a12, 52
     e7e:	11a3e0                                    	slli	a10, a3, 2
     e81:	d194c0                                    	mul16s	a9, a4, a12
     e84:	2505124204e0347e                  	{ addx4	a8, a4, a7; ae_l32x2.x	aed0, a2, a5 }
     e8c:	b07440                                    	addx8	a7, a4, a4
     e8f:	06dd                                    	mov.n	a13, a6
     e91:	2e06a74008c7ea2e                  	{ ae_l32x2.x	aed1, a2, a10; slli	a14, a7, 2 }
     e99:	00008139d4e428004003032aa00041df 	{ ae_s32x2x2.ip	aed1, aed0, a13, 16; ae_l32x2.x	aed29, a2, a8; nop }
     ea9:	2e05324710dfe92e                  	{ ae_l32x2.x	aed30, a2, a9; ae_l32x2.x	aed31, a2, a14 }
     eb1:	0000812654e01f0041830321b0005ddf 	{ ae_s32x2x2.ip	aed29, aed30, a13, 16; addmi	a15, a1, 0x100; nop }
     ec1:	51a9                                    	s32i.n	a10, a1, 20
     ec3:	80cfa2                                    	addi	a10, a15, -128
     ec6:	0c04054718e092de                  	{ ae_s32x2.i	aed31, a13, 0; movi	a13, 5 }
     ece:	06cd                                    	mov.n	a12, a6
     ed0:	b199                                    	s32i.n	a9, a1, 44
     ed2:	106182                                    	s32i	a8, a1, 64
     ed5:	0b5d                                    	mov.n	a5, a11
     ed7:	91e9                                    	s32i.n	a14, a1, 36
     ed9:	21a9                                    	s32i.n	a10, a1, 8
     edb:	000be0                                    	callx8	a11
     ede:	11c8                                    	l32i.n	a12, a1, 4
     ee0:	11a3d0                                    	slli	a10, a3, 3
     ee3:	1197d0                                    	slli	a9, a7, 3
     ee6:	1184d0                                    	slli	a8, a4, 3
     ee9:	f07440                                    	subx8	a7, a4, a4
     eec:	06ed                                    	mov.n	a14, a6
     eee:	2b05324708a00cde                  	{ slli	a13, a12, 3; ae_l32x2.x	aed0, a2, a10 }
     ef6:	3406b54008cafd2e                  	{ ae_l32x2.x	aed1, a2, a13; or	a11, a5, a5 }
     efe:	00008144d4e07d004003032aa800c0ef 	{ ae_s32x2x2.ip	aed0, aed1, a14, 16; slli	a15, a7, 3; nop }
     f0e:	2805324708def82e                  	{ ae_l32x2.x	aed29, a2, a8; ae_l32x2.x	aed30, a2, a9 }
     f16:	50c152                                    	addi	a5, a1, 80
     f19:	31a9                                    	s32i.n	a10, a1, 12
     f1b:	2f053243109f11de                  	{ s32i	a13, a1, 56; ae_l32x2.x	aed31, a2, a15 }
     f23:	0000813054e05d0041830321b0001def 	{ ae_s32x2x2.ip	aed29, aed30, a14, 16; movi	a13, 5; nop }
     f33:	06cd                                    	mov.n	a12, a6
     f35:	05ad                                    	mov.n	a10, a5
     f37:	c189                                    	s32i.n	a8, a1, 48
     f39:	8199                                    	s32i.n	a9, a1, 32
     f3b:	41f9                                    	s32i.n	a15, a1, 16
     f3d:	3a069b4718e892ee                  	{ ae_s32x2.i	aed31, a14, 0; or	a3, a11, a11 }
     f45:	000be0                                    	callx8	a11
     f48:	11d8                                    	l32i.n	a13, a1, 4
     f4a:	c82c                                    	movi.n	a8, 44
     f4c:	cc3c                                    	movi.n	a12, 60
     f4e:	03bd                                    	mov.n	a11, a3
     f50:	d134c0                                    	mul16s	a3, a4, a12
     f53:	d18480                                    	mul16s	a8, a4, a8
     f56:	cf4c                                    	movi.n	a15, 76
     f58:	11ade0                                    	slli	a10, a13, 2
     f5b:	1197e0                                    	slli	a9, a7, 2
     f5e:	06ed                                    	mov.n	a14, a6
     f60:	d144f0                                    	mul16s	a4, a4, a15
     f63:	2805324000c1e32e                  	{ ae_l32x2.x	aed0, a2, a3; ae_l32x2.x	aed1, a2, a8 }
     f6b:	00008139d4e429004003032aa80040ef 	{ ae_s32x2x2.ip	aed0, aed1, a14, 16; ae_l32x2.x	aed29, a2, a9; nop }
     f7b:	2b053240089e01ae                  	{ s32i	a10, a1, 4; ae_l32x2.x	aed30, a2, a10 }
     f83:	00008139d4e424004183032bb000ddef 	{ ae_s32x2x2.ip	aed29, aed30, a14, 16; ae_l32x2.x	aed31, a2, a4; nop }
     f93:	5d0c                                    	movi.n	a13, 5
     f95:	06ad                                    	mov.n	a10, a6
     f97:	a189                                    	s32i.n	a8, a1, 40
     f99:	7199                                    	s32i.n	a9, a1, 28
     f9b:	0606964718eb82ee                  	{ ae_s32x2.i	aed31, a14, 0; or	a12, a6, a6 }
     fa3:	000be0                                    	callx8	a11
     fa6:	6178                                    	l32i.n	a7, a1, 24
     fa8:	2188                                    	l32i.n	a8, a1, 8
     faa:	1005164010e3025e                  	{ ae_l32x2.i	aed2, a5, 0; ae_l32x2.i	aed3, a6, 0 }
     fb2:	31e8                                    	l32i.n	a14, a1, 12
     fb4:	090c                                    	movi.n	a9, 0
     fb6:	1e03814000e1127e                  	{ ae_l32x2.i	aed0, a7, 0; l32i	a15, a1, 20 }
     fbe:	0e952681d5217ed0c40080150040428f 	{ ae_l32x2.i	aed1, a8, 0; ae_l32x2.i	aed26, a7, 8; ae_addandsub32s	aed0, aed2, aed0, aed2; nop }
     fce:	0e5533a1dd008e0045847c0818a3901f 	{ ae_add32s	aed4, aed1, aed3; ae_l32x2.i	aed27, a8, 8; ae_sel16i.n	aed5, aed3, aed1, 0; ae_sel16i	aed1, aed1, aed3, 0 }
     fde:	0ea52021e5215ec04c00850f08794e6f 	{ ae_l32x2.i	aed29, a6, 8; ae_l32x2.i	aed28, a5, 8; ae_sub32s	aed25, aed5, aed1; ae_addandsub32s	aed0, aed24, aed0, aed4 }
     fee:	0ea4e0086c541a084d833be8151f212f 	{ ae_s32x2.x	aed0, a2, a9; l32i	a10, a1, 52; ae_add32s	aed31, aed27, aed29; ae_addandsub32s	aed0, aed1, aed2, aed25 }
     ffe:	0e54f3a824441b0085efbcda14dd3a2f 	{ ae_s32x2.x	aed24, a2, a14; l32i	a11, a1, 16; ae_sel16i.n	aed6, aed29, aed27, 0; ae_sel16i	aed2, aed27, aed29, 0 }
    100e:	0ea4f8284c541c084dfb86101467732f 	{ ae_s32x2.x	aed1, a2, a15; l32i	a12, a1, 36; ae_sub32s	aed7, aed6, aed2; ae_addandsub32s	aed30, aed1, aed26, aed28 }
    101e:	0e95268244407ed0c5809ef81460032f 	{ ae_s32x2.x	aed0, a2, a3; ae_l32x2.i	aed8, a7, 16; ae_addandsub32s	aed0, aed3, aed30, aed31; nop }
    102e:	0e9526824c408ed0c58081381420222f 	{ ae_s32x2.x	aed0, a2, a10; ae_l32x2.i	aed9, a8, 16; ae_addandsub32s	aed0, aed1, aed1, aed7; nop }
    103e:	1205154218ea0a6e                  	{ ae_l32x2.i	aed11, a6, 16; ae_l32x2.i	aed10, a5, 16 }
    1046:	0e64f0087c441d0385ad2958150de32f 	{ ae_s32x2.x	aed3, a2, a11; l32i	a13, a1, 60; ae_add32s	aed13, aed9, aed11; ae_sel16i	aed14, aed11, aed9, 0 }
    1056:	0e94eba844441e084db15c581449702f 	{ ae_s32x2.x	aed1, a2, a12; l32i	a14, a1, 32; ae_sel16i.n	aed2, aed9, aed11, 0; ae_addandsub32s	aed12, aed1, aed8, aed10 }
    1066:	0ea4ec285c441f184d81ae10146f102f 	{ ae_s32x2.x	aed0, a2, a4; l32i	a15, a1, 44; ae_sub32s	aed15, aed14, aed2; ae_addandsub32s	aed0, aed3, aed12, aed13 }
    1076:	0e94e6880c4414d0c58081781420312f 	{ ae_s32x2.x	aed0, a2, a13; l32i	a4, a1, 4; ae_addandsub32s	aed0, aed1, aed1, aed15; nop }
    1086:	1205184400f11e7e                  	{ ae_l32x2.i	aed16, a7, 24; ae_l32x2.i	aed17, a8, 24 }
    108e:	1305164510f3127e                  	{ ae_l32x2.i	aed22, a7, 32; ae_l32x2.i	aed19, a6, 24 }
    1096:	0e64f00895251701844e319f1915825f 	{ ae_l32x2.i	aed18, a5, 24; l32i	a7, a1, 72; ae_add32s	aed21, aed17, aed19; ae_sel16i	aed6, aed19, aed17, 0 }
    10a6:	0e9527a4bc408ed0c5809c981451f22f 	{ ae_s32x2.x	aed3, a2, a14; ae_l32x2.i	aed23, a8, 32; ae_sel16i.n	aed2, aed17, aed19, 0; nop }
    10b6:	0ea53024cc406e084dd246101462732f 	{ ae_s32x2.x	aed1, a2, a15; ae_l32x2.i	aed25, a6, 32; ae_sub32s	aed2, aed6, aed2; ae_addandsub32s	aed20, aed1, aed16, aed18 }
    10c6:	0e54f2886444190745e6f4a81460102f 	{ ae_s32x2.x	aed0, a2, a4; l32i	a9, a1, 48; ae_addandsub32s	aed0, aed3, aed20, aed21; ae_sel16i	aed29, aed25, aed23, 0 }
    10d6:	0ea4e008844c1a104d8457c8151c132f 	{ ae_s32x2.x	aed0, a2, a7; l32i	a10, a1, 64; ae_add32s	aed28, aed23, aed25; ae_addandsub32s	aed1, aed2, aed1, aed2 }
    10e6:	33000b4600e0125e                  	{ ae_l32x2.i	aed24, a5, 32; ae_sel16i	aed0, aed23, aed25, 0 }
    10ee:	71b8                                    	l32i.n	a11, a1, 28
    10f0:	0ea4d42851f41fd04d6f1d000c6041cf 	{ l32i	a12, a1, 68; l32i	a15, a1, 40; ae_sub32s	aed0, aed29, aed0; ae_addandsub32s	aed27, aed26, aed22, aed24 }
    1100:	0e94c68899e41ed0c5009a0017e031df 	{ l32i	a13, a1, 56; l32i	a14, a1, 76; ae_addandsub32s	aed0, aed31, aed26, aed0; nop }
    1110:	220760c01cc5592e                  	{ ae_s32x2.x	aed3, a2, a9; nop }
    1118:	220760c014c55a2e                  	{ ae_s32x2.x	aed2, a2, a10; nop }
    1120:	0e95269b0c440cd0c5809be017c1632f 	{ ae_s32x2.x	aed1, a2, a11; nop; ae_addandsub32s	aed1, aed30, aed27, aed28; nop }
    1130:	220760c00cc55c2e                  	{ ae_s32x2.x	aed1, a2, a12; nop }
    1138:	220760c714c55d2e                  	{ ae_s32x2.x	aed30, a2, a13; nop }
    1140:	220760c71cc55e2e                  	{ ae_s32x2.x	aed31, a2, a14; nop }
    1148:	220760c004c55f2e                  	{ ae_s32x2.x	aed0, a2, a15; nop }
    1150:	f01d                                    	retw.n

00001152 <fix_fft20+0x352>:
	...

00001154 <fix_fft30>:
    1154:	038136                                    	entry	a1, 0x1c0
    1157:	11e4b0                                    	slli	a14, a4, 5
    115a:	b0d440                                    	addx8	a13, a4, a4
    115d:	3c06a44000c7ee2e                  	{ ae_l32x2.x	aed0, a2, a14; slli	a15, a4, 4 }
    1165:	905440                                    	addx2	a5, a4, a4
    1168:	11edd0                                    	slli	a14, a13, 3
    116b:	01d1c2                                    	addmi	a12, a1, 0x100
    116e:	a0a440                                    	addx4	a10, a4, a4
    1171:	3c06a54010c7ff2e                  	{ ae_l32x2.x	aed2, a2, a15; slli	a15, a5, 3 }
    1179:	2c06a54018c7ee2e                  	{ ae_l32x2.x	aed3, a2, a14; slli	a14, a5, 4 }
    1181:	0c02ac4100c8ef2e                  	{ ae_l32x2.x	aed4, a2, a15; addi	a12, a12, 32 }
    1189:	3c06aa4110c7ee2e                  	{ ae_l32x2.x	aed6, a2, a14; slli	a15, a10, 4 }
    1191:	0e0c                                    	movi.n	a14, 0
    1193:	c33c                                    	movi.n	a3, 60
    1195:	61d9                                    	s32i.n	a13, a1, 24
    1197:	11d5b0                                    	slli	a13, a5, 5
    119a:	2e05324118caef2e                  	{ ae_l32x2.x	aed7, a2, a15; ae_l32x2.x	aed10, a2, a14 }
    11a2:	68a062                                    	movi	a6, 104
    11a5:	0cfd                                    	mov.n	a15, a12
    11a7:	d1b430                                    	mul16s	a11, a4, a3
    11aa:	d13460                                    	mul16s	a3, a4, a6
    11ad:	0000813954e42d0040030322b80046ff 	{ ae_s32x2x2.ip	aed10, aed3, a15, 16; ae_l32x2.x	aed5, a2, a13; nop }
    11bd:	0000814754e0440040030329a80050ff 	{ ae_s32x2x2.ip	aed4, aed5, a15, 16; subx8	a9, a4, a4; nop }
    11cd:	1184d0                                    	slli	a8, a4, 3
    11d0:	11d9d0                                    	slli	a13, a9, 3
    11d3:	00008139d4e423004003032bb80052ff 	{ ae_s32x2x2.ip	aed6, aed7, a15, 16; ae_l32x2.x	aed29, a2, a3; nop }
    11e3:	41a9                                    	s32i.n	a10, a1, 16
    11e5:	875c                                    	movi.n	a7, 88
    11e7:	2905324610a10ade                  	{ slli	a10, a10, 3; ae_l32x2.x	aed1, a2, a8 }
    11ef:	0000813954e42d0041830329a80040ff 	{ ae_s32x2x2.ip	aed0, aed29, a15, 16; ae_l32x2.x	aed9, a2, a13; nop }
    11ff:	11d9c0                                    	slli	a13, a9, 4
    1202:	11e4a0                                    	slli	a14, a4, 6
    1205:	d17470                                    	mul16s	a7, a4, a7
    1208:	2a05324700c8ed2e                  	{ ae_l32x2.x	aed28, a2, a13; ae_l32x2.x	aed8, a2, a10 }
    1210:	00008139d4e42e004003032aa80085ff 	{ ae_s32x2x2.ip	aed9, aed1, a15, 16; ae_l32x2.x	aed30, a2, a14; nop }
    1220:	00008139d4e427004183032ba000c4ff 	{ ae_s32x2x2.ip	aed8, aed28, a15, 16; ae_l32x2.x	aed31, a2, a7; nop }
    1230:	0000814654e244004003032ab0005eff 	{ ae_s32x2x2.ip	aed30, aed2, a15, 16; or	a13, a4, a4; nop }
    1240:	1e02914718e092fe                  	{ ae_s32x2.i	aed31, a15, 0; addmi	a15, a1, 0x100 }
    1248:	a0cf62                                    	addi	a6, a15, -96
    124b:	fb8c81                                    	l32r	a8, 7c (84c <fix_fft15_stage>)
    124e:	51e9                                    	s32i.n	a14, a1, 20
    1250:	5e0c                                    	movi.n	a14, 5
    1252:	06ad                                    	mov.n	a10, a6
    1254:	31c9                                    	s32i.n	a12, a1, 12
    1256:	21b9                                    	s32i.n	a11, a1, 8
    1258:	1174e0                                    	slli	a7, a4, 2
    125b:	7199                                    	s32i.n	a9, a1, 28
    125d:	32ba                                    	add.n	a3, a2, a11
    125f:	0008e0                                    	callx8	a8
    1262:	1155e0                                    	slli	a5, a5, 2
    1265:	2188                                    	l32i.n	a8, a1, 8
    1267:	6ca0c2                                    	movi	a12, 108
    126a:	6198                                    	l32i.n	a9, a1, 24
    126c:	0a03a14008c1e52e                  	{ ae_l32x2.x	aed1, a2, a5; l32i	a10, a1, 16 }
    1274:	51b8                                    	l32i.n	a11, a1, 20
    1276:	4f5c                                    	movi.n	a15, 84
    1278:	d154c0                                    	mul16s	a5, a4, a12
    127b:	3c03a14000c0e82e                  	{ ae_l32x2.x	aed0, a2, a8; l32i	a12, a1, 12 }
    1283:	d184f0                                    	mul16s	a8, a4, a15
    1286:	1199e0                                    	slli	a9, a9, 2
    1289:	11aae0                                    	slli	a10, a10, 2
    128c:	3a06a44110daf52e                  	{ ae_l32x2.x	aed6, a2, a5; addx4	a11, a4, a11 }
    1294:	cd5c                                    	movi.n	a13, 92
    1296:	0c5d                                    	mov.n	a5, a12
    1298:	2e042c4010c0e92e                  	{ ae_l32x2.x	aed2, a2, a9; movi	a14, 44 }
    12a0:	d194d0                                    	mul16s	a9, a4, a13
    12a3:	2a05324100c3eb2e                  	{ ae_l32x2.x	aed4, a2, a11; ae_l32x2.x	aed3, a2, a10 }
    12ab:	3a04244108c1f82e                  	{ ae_l32x2.x	aed5, a2, a8; movi	a11, 116 }
    12b3:	0000813054e04f0040030322a800805f 	{ ae_s32x2x2.ip	aed0, aed1, a5, 16; movi	a15, 100; nop }
    12c3:	d1a4e0                                    	mul16s	a10, a4, a14
    12c6:	0000813054e04d0040030320b000d15f 	{ ae_s32x2x2.ip	aed5, aed2, a5, 16; movi	a13, 52; nop }
    12d6:	d1b4b0                                    	mul16s	a11, a4, a11
    12d9:	d184f0                                    	mul16s	a8, a4, a15
    12dc:	2a05324118c8e92e                  	{ ae_l32x2.x	aed7, a2, a9; ae_l32x2.x	aed8, a2, a10 }
    12e4:	0000813054e0ce0040030322b800125f 	{ ae_s32x2x2.ip	aed6, aed3, a5, 16; movi	a14, 76; nop }
    12f4:	d194d0                                    	mul16s	a9, a4, a13
    12f7:	2a0512411c9b11fe                  	{ l32i	a15, a1, 28; ae_l32x2.x	aed27, a2, a11 }
    12ff:	00008139d4e428004083032aa000135f 	{ ae_s32x2x2.ip	aed7, aed8, a5, 16; ae_l32x2.x	aed28, a2, a8; nop }
    130f:	d1a4e0                                    	mul16s	a10, a4, a14
    1312:	00008139d4e429004003032ba0004f5f 	{ ae_s32x2x2.ip	aed27, aed4, a5, 16; ae_l32x2.x	aed29, a2, a9; nop }
    1322:	0e06af4610c6e72e                  	{ ae_l32x2.x	aed26, a2, a7; slli	a8, a15, 2 }
    132a:	00008139d4e42a004183032ba8009c5f 	{ ae_s32x2x2.ip	aed28, aed29, a5, 16; ae_l32x2.x	aed30, a2, a10; nop }
    133a:	00008139d4e428004183032bb000ce5f 	{ ae_s32x2x2.ip	aed26, aed30, a5, 16; ae_l32x2.x	aed31, a2, a8; nop }
    134a:	0402814718e8925e                  	{ ae_s32x2.i	aed31, a5, 0; addi	a5, a1, 32 }
    1352:	04dd                                    	mov.n	a13, a4
    1354:	fb4a41                                    	l32r	a4, 7c (84c <fix_fft15_stage>)
    1357:	5e0c                                    	movi.n	a14, 5
    1359:	05ad                                    	mov.n	a10, a5
    135b:	0004e0                                    	callx8	a4
    135e:	140c                                    	movi.n	a4, 1
    1360:	080c                                    	movi.n	a8, 0
    1362:	67ed44                                    	wur.ae_sar	a4
    1365:	0a04204000c1e86e                  	{ ae_l32x2.x	aed0, a6, a8; movi	a10, 64 }
    136d:	0a04284008c0f85e                  	{ ae_l32x2.x	aed1, a5, a8; movi	a11, 8 }
    1375:	0e94e6fb29445ad0c58080091c20a26f 	{ ae_l32x2.x	aed2, a6, a10; ae_l32x2.x	aed5, a5, a10; ae_addandsubrng32.l	aed0, aed1, aed0, aed1; nop }
    1385:	0e94e6f024408cd0c58082290440132f 	{ ae_s32x2.xp	aed0, a2, a7; movi	a12, 72; ae_addandsubrng32.l	aed0, aed2, aed2, aed5; nop }
    1395:	2b0576400cc6773e                  	{ ae_s32x2.xp	aed1, a3, a7; ae_l32x2.x	aed6, a6, a11 }
    139d:	2b05754014c7772e                  	{ ae_s32x2.xp	aed2, a2, a7; ae_l32x2.x	aed7, a5, a11 }
    13a5:	0e94e6f00c400dd0c58086390441133f 	{ ae_s32x2.xp	aed0, a3, a7; movi	a13, 16; ae_addandsubrng32.l	aed1, aed2, aed6, aed7; nop }
    13b5:	2c05354200c9ec6e                  	{ ae_l32x2.x	aed8, a6, a12; ae_l32x2.x	aed9, a5, a12 }
    13bd:	0e94e6f02c400ed0c58088490420532f 	{ ae_s32x2.xp	aed1, a2, a7; movi	a14, 80; ae_addandsubrng32.l	aed0, aed1, aed8, aed9; nop }
    13cd:	2d05764014ca773e                  	{ ae_s32x2.xp	aed2, a3, a7; ae_l32x2.x	aed10, a6, a13 }
    13d5:	2d0575400ccb772e                  	{ ae_s32x2.xp	aed1, a2, a7; ae_l32x2.x	aed11, a5, a13 }
    13dd:	0e94e6f00c408fd0c5808a590441133f 	{ ae_s32x2.xp	aed0, a3, a7; movi	a15, 24; ae_addandsubrng32.l	aed1, aed2, aed10, aed11; nop }
    13ed:	2e05354300cdee6e                  	{ ae_l32x2.x	aed12, a6, a14; ae_l32x2.x	aed13, a5, a14 }
    13f5:	0e94e6f02c4089d0c5808c690420532f 	{ ae_s32x2.xp	aed1, a2, a7; movi	a9, 88; ae_addandsubrng32.l	aed0, aed1, aed12, aed13; nop }
    1405:	2f05764014ce773e                  	{ ae_s32x2.xp	aed2, a3, a7; ae_l32x2.x	aed14, a6, a15 }
    140d:	2f0575400ccf772e                  	{ ae_s32x2.xp	aed1, a2, a7; ae_l32x2.x	aed15, a5, a15 }
    1415:	0e94e6f014400ad0c5808e790441133f 	{ ae_s32x2.xp	aed0, a3, a7; movi	a10, 32; ae_addandsubrng32.l	aed1, aed2, aed14, aed15; nop }
    1425:	2805354400d1f96e                  	{ ae_l32x2.x	aed16, a6, a9; ae_l32x2.x	aed17, a5, a9 }
    142d:	0e94e6f034400bd0c58090890420532f 	{ ae_s32x2.xp	aed1, a2, a7; movi	a11, 96; ae_addandsubrng32.l	aed0, aed1, aed16, aed17; nop }
    143d:	2b05764014d2673e                  	{ ae_s32x2.xp	aed2, a3, a7; ae_l32x2.x	aed18, a6, a10 }
    1445:	2804284418c0fa5e                  	{ ae_l32x2.x	aed19, a5, a10; movi	a9, 40 }
    144d:	0e94e6fba4446bd0c58092990441532f 	{ ae_s32x2.xp	aed1, a2, a7; ae_l32x2.x	aed20, a6, a11; ae_addandsubrng32.l	aed1, aed2, aed18, aed19; nop }
    145d:	2b05754004d5773e                  	{ ae_s32x2.xp	aed0, a3, a7; ae_l32x2.x	aed21, a5, a11 }
    1465:	0e94e6f034408cd0c58094a90420532f 	{ ae_s32x2.xp	aed1, a2, a7; movi	a12, 104; ae_addandsubrng32.l	aed0, aed1, aed20, aed21; nop }
    1475:	2905764014d6773e                  	{ ae_s32x2.xp	aed2, a3, a7; ae_l32x2.x	aed22, a6, a9 }
    147d:	290575400cd7772e                  	{ ae_s32x2.xp	aed1, a2, a7; ae_l32x2.x	aed23, a5, a9 }
    1485:	0e94e6f01c400dd0c58096b90441133f 	{ ae_s32x2.xp	aed0, a3, a7; movi	a13, 48; ae_addandsubrng32.l	aed1, aed2, aed22, aed23; nop }
    1495:	2c05354600d9ec6e                  	{ ae_l32x2.x	aed24, a6, a12; ae_l32x2.x	aed25, a5, a12 }
    149d:	0e94e6f03c400ed0c58098c90420532f 	{ ae_s32x2.xp	aed1, a2, a7; movi	a14, 112; ae_addandsubrng32.l	aed0, aed1, aed24, aed25; nop }
    14ad:	2d05764014da773e                  	{ ae_s32x2.xp	aed2, a3, a7; ae_l32x2.x	aed26, a6, a13 }
    14b5:	3e04284618c0fd5e                  	{ ae_l32x2.x	aed27, a5, a13; movi	a15, 56 }
    14bd:	0e94e6fbe4446ed0c5809ad90441532f 	{ ae_s32x2.xp	aed1, a2, a7; ae_l32x2.x	aed28, a6, a14; ae_addandsubrng32.l	aed1, aed2, aed26, aed27; nop }
    14cd:	2f05754004dd673e                  	{ ae_s32x2.xp	aed0, a3, a7; ae_l32x2.x	aed29, a5, a14 }
    14d5:	0e94e6fbf4446fd0c5809ce90420532f 	{ ae_s32x2.xp	aed1, a2, a7; ae_l32x2.x	aed30, a6, a15; ae_addandsubrng32.l	aed0, aed1, aed28, aed29; nop }
    14e5:	2f05754014df773e                  	{ ae_s32x2.xp	aed2, a3, a7; ae_l32x2.x	aed31, a5, a15 }
    14ed:	0e9526fb0c440cd0c5809ef90441532f 	{ ae_s32x2.xp	aed1, a2, a7; nop; ae_addandsubrng32.l	aed1, aed2, aed30, aed31; nop }
    14fd:	230760c004c5773e                  	{ ae_s32x2.xp	aed0, a3, a7; nop }
    1505:	230760c00cc5772e                  	{ ae_s32x2.xp	aed1, a2, a7; nop }
    150d:	230760c014c5773e                  	{ ae_s32x2.xp	aed2, a3, a7; nop }
    1515:	f01d                                    	retw.n

00001517 <fix_fft30+0x3c3>:
	...

00001518 <fix_fft40>:
    1518:	006136                                    	entry	a1, 48
    151b:	fad061                                    	l32r	a6, 5c (24a4 <RotVector_40_32>)
    151e:	530c                                    	movi.n	a3, 5
    1520:	1184b0                                    	slli	a8, a4, 5
    1523:	870c                                    	movi.n	a7, 8
    1525:	029d                                    	mov.n	a9, a2
    1527:	11b4e0                                    	slli	a11, a4, 2
    152a:	0668                                    	l32i.n	a6, a6, 0
    152c:	20a550                                    	or	a10, a5, a5
    152f:	026162                                    	s32i	a6, a1, 8
    1532:	206990                                    	or	a6, a9, a9

00001535 <fix_fft40+0x1d>:
    1535:	230720c004c5186e                  	{ ae_l32x2.xp	aed0, a6, a8; nop }
    153d:	0f8376                                    	loop	a3, 1550 <fix_fft40+0x38>
    1540:	230700c004e599ae                  	{ ae_s32x2.ip	aed0, a10, 8; nop }
    1548:	230720c004c5186e                  	{ ae_l32x2.xp	aed0, a6, a8; nop }

00001550 <fix_fft40+0x38>:
    1550:	99ba                                    	add.n	a9, a9, a11
    1552:	770b                                    	addi.n	a7, a7, -1
    1554:	096d                                    	mov.n	a6, a9
    1556:	fdb756                                    	bnez	a7, 1535 <fix_fft40+0x1d>
    1559:	fac961                                    	l32r	a6, 80 (a80 <fix_fft5_8K>)
    155c:	870c                                    	movi.n	a7, 8
    155e:	053d                                    	mov.n	a3, a5
    1560:	31b9                                    	s32i.n	a11, a1, 12

00001562 <fix_fft40+0x4a>:
    1562:	2c0c                                    	movi.n	a12, 2
    1564:	03ad                                    	mov.n	a10, a3
    1566:	0006e0                                    	callx8	a6
    1569:	770b                                    	addi.n	a7, a7, -1
    156b:	28c332                                    	addi	a3, a3, 40
    156e:	ff0756                                    	bnez	a7, 1562 <fix_fft40+0x4a>
    1571:	28a032                                    	movi	a3, 40
    1574:	08c572                                    	addi	a7, a5, 8
    1577:	2f03a14214c0135e                  	{ ae_l32x2.xp	aed10, a5, a3; l32i	a15, a1, 8 }
    157f:	3d03a1421cc0135e                  	{ ae_l32x2.xp	aed11, a5, a3; l32i	a13, a1, 12 }
    1587:	3506a44304da035e                  	{ ae_l32x2.xp	aed12, a5, a3; addx4	a11, a4, a4 }
    158f:	0d04254204c0035e                  	{ ae_l32x2.xp	aed8, a5, a3; movi	a12, 5 }
    1597:	0502afc01cde035e                  	{ ae_l32x2.xp	aed3, a5, a3; addi	a4, a15, -8 }
    159f:	0904254104c0035e                  	{ ae_l32x2.xp	aed4, a5, a3; movi	a8, 5 }
    15a7:	0b0422430cc0035e                  	{ ae_l32x2.xp	aed13, a5, a3; movi	a10, 2 }
    15af:	3e068b4310e6025e                  	{ ae_l32x2.i	aed14, a5, 0; slli	a11, a11, 2 }
    15b7:	fab361                                    	l32r	a6, 84 (5a82799a <_end+0x5a8254ce>)
    15ba:	01a052                                    	movi	a5, 1
    15bd:	310766421c01c29e                  	{ l32r	a9, 88 (a57d8666 <_end+0xa57d619a>); ae_movda32	aed1, a6 }
    15c5:	090656c718807f9e                  	{ movi	a9, -1; ae_movda32x2	aed0, a6, a9 }
    15cd:	178c76                                    	loop	a12, 15e8 <fix_fft40+0xd0>
    15d0:	0301c0                                    	rsr.lend	a12
    15d3:	1300c0                                    	wsr.lbeg	a12
    15d6:	faadc1                                    	l32r	a12, 8c (1779 <fix_fft40+0x261>)
    15d9:	0020f0                                    	nop
    15dc:	1301c0                                    	wsr.lend	a12
    15df:	002000                                    	isync
    15e2:	0302c0                                    	rsr.lcount	a12
    15e5:	01ccc2                                    	addi	a12, a12, 1
    15e8:	220700c104a5598e                  	{ ae_slaa32s	aed5, aed8, a9; nop }
    15f0:	2406b44100a9493e                  	{ ae_slaa32s	aed9, aed3, a9; or	a6, a4, a4 }
    15f8:	220720c100a5194e                  	{ ae_slaa32s	aed8, aed4, a9; nop }
    1600:	220700c100a599ae                  	{ ae_slaa32s	aed2, aed10, a9; nop }
    1608:	220700c104a5d9be                  	{ ae_slaa32s	aed7, aed11, a9; nop }
    1610:	220700c104a599ce                  	{ ae_slaa32s	aed6, aed12, a9; nop }
    1618:	220700c104a519de                  	{ ae_slaa32s	aed4, aed13, a9; nop }
    1620:	220700c100a5d9ee                  	{ ae_slaa32s	aed3, aed14, a9; nop }
    1628:	025866                                    	bnei	a8, 5, 162e <fix_fft40+0x116>
    162b:	0021c6                                    	j	16b6 <fix_fft40+0x19e>
    162e:	230720c214a5dc6e                  	{ ae_l32x2.ip	aed10, a6, 32; nop }
    1636:	0e2d297b0d350c3a8f28e13c1c01ea6f 	{ ae_l32x2.ip	aed11, a6, 32; nop; nop; ae_mulcj32w	aed7, aed10, aed7, aed10 }
    1646:	0e2d297b0d3d0c32cd28e13c1c012e6f 	{ ae_l32x2.ip	aed12, a6, 32; nop; nop; ae_mulcj32w	aed6, aed11, aed6, aed11 }
    1656:	0e2d6d69c9266c2e0aa8e13a1401f57f 	{ ae_trunca32x2f64s	aed7, aed7, aed10, a5; ae_l32x2.ip	aed25, a6, 32; nop; ae_mulcj32w	aed5, aed24, aed5, aed12 }
    1666:	0e2d7969d92e6c4e92a8e13a1c01b56f 	{ ae_trunca32x2f64s	aed6, aed6, aed11, a5; ae_l32x2.ip	aed27, a6, 32; nop; ae_mulcj32w	aed9, aed26, aed9, aed25 }
    1676:	0e2d3960e9386e4710a8e13e0401755f 	{ ae_trunca32x2f64s	aed5, aed5, aed24, a5; ae_l32x2.i	aed29, a6, 0; nop; ae_mulcj32w	aed8, aed28, aed8, aed27 }
    1686:	0e2d3d64f1486e2308a8e13e1401759f 	{ ae_trunca32x2f64s	aed9, aed9, aed26, a5; ae_l32x2.i	aed30, a6, 32; nop; ae_mulcj32w	aed4, aed12, aed4, aed29 }
    1696:	0e2d3d7b09540c1fc6a8e13f0401358f 	{ ae_trunca32x2f64s	aed8, aed8, aed28, a5; nop; nop; ae_mulcj32w	aed3, aed31, aed3, aed30 }
    16a6:	220700c30445154e                  	{ ae_trunca32x2f64s	aed4, aed4, aed12, a5; nop }
    16ae:	220700c71845d53e                  	{ ae_trunca32x2f64s	aed3, aed3, aed31, a5; nop }
    16b6:	67eda4                                    	wur.ae_sar	a10
    16b9:	0ea56427e53f2258c52d051f106c677f 	{ or	a6, a7, a7; or	a12, a2, a2; ae_sub32s	aed12, aed5, aed3; ae_sub32s	aed11, aed7, aed8 }
    16c9:	094ac5ad503b6341471d0960418b184f 	{ addi	a4, a4, 8; ae_l32x2.xp	aed10, a6, a3; ae_addandsubrng32.l	aed12, aed13, aed11, aed12; ae_addandsubrng32.l	aed7, aed8, aed7, aed8; ae_addandsubrng32.l	aed5, aed3, aed5, aed3 }
    16d9:	095ac00d5932634989092173598cec8f 	{ addi	a8, a8, -1; ae_l32x2.xp	aed11, a6, a3; ae_sel16i.n	aed14, aed12, aed12, 0; ae_addandsubrng32.l	aed2, aed9, aed2, aed9; ae_addandsubrng32.l	aed4, aed6, aed6, aed4 }
    16e9:	031ac78d6478633b9b14b81044821b7f 	{ addi	a7, a7, 8; ae_l32x2.xp	aed12, a6, a3; ae_addandsub32s	aed2, aed28, aed2, aed4; ae_addandsub32s	aed5, aed7, aed7, aed5; ae_addandsub32s	aed15, aed16, aed14, aed13 }
    16f9:	0e94ce7c41f46380c5c5a9340126284f 	{ ae_neg32s	aed16, aed16; ae_l32x2.xp	aed8, a6, a3; ae_addandsub32js	aed6, aed9, aed9, aed6; ae_subadd32s	aed17, aed14, aed13 }
    1709:	0b5ae04d183663284e08b878560f82df 	{ add	a2, a2, a13; ae_l32x2.xp	aed3, a6, a3; ae_sel16i.n	aed15, aed15, aed16, 2; ae_addandsub32s	aed2, aed5, aed2, aed5; ae_mul32js	aed7, aed7 }
    1719:	0e1ce33c222d637fdf2b4ef11a2013cf 	{ ae_addandsub32s	aed7, aed16, aed28, aed7; ae_l32x2.xp	aed4, a6, a3; ae_mulfp32x2ras	aed30, aed17, aed0; ae_mulfp32x2ras	aed15, aed15, aed1 }
    1729:	230720c30cc5136e                  	{ ae_l32x2.xp	aed13, a6, a3; nop }
    1731:	0e952a8074486e484df5e6f104dfa3cf 	{ ae_s32x2.xp	aed2, a12, a11; ae_l32x2.i	aed14, a6, 0; ae_addandsub32s	aed31, aed6, aed6, aed30; ae_addandsub32s	aed29, aed9, aed9, aed15 }
    1741:	230760c70cc57bce                  	{ ae_s32x2.xp	aed29, a12, a11; nop }
    1749:	230760c404c57bce                  	{ ae_s32x2.xp	aed16, a12, a11; nop }
    1751:	230760c71cc57bce                  	{ ae_s32x2.xp	aed31, a12, a11; nop }
    1759:	230760c10cc57bce                  	{ ae_s32x2.xp	aed5, a12, a11; nop }
    1761:	230760c20cc57bce                  	{ ae_s32x2.xp	aed9, a12, a11; nop }
    1769:	230760c11cc57bce                  	{ ae_s32x2.xp	aed7, a12, a11; nop }
    1771:	230760c114c57bce                  	{ ae_s32x2.xp	aed6, a12, a11; nop }

00001779 <fix_fft40+0x261>:
    1779:	f01d                                    	retw.n

0000177b <fix_fft40+0x263>:
	...

0000177c <FFT4N>:
    177c:	004136                                    	entry	a1, 32
    177f:	0921c2                                    	l32i	a12, a1, 36
    1782:	0215e6                                    	bgei	a5, 1, 1788 <FFT4N+0xc>
    1785:	004506                                    	j	189d <FFT4N+0x121>
    1788:	082132                                    	l32i	a3, a1, 32
    178b:	90d550                                    	addx2	a13, a5, a5
    178e:	d1e560                                    	mul16s	a14, a5, a6
    1791:	11ccf0                                    	slli	a12, a12, 1
    1794:	1185d0                                    	slli	a8, a5, 3
    1797:	c044c0                                    	sub	a4, a4, a12
    179a:	1195c0                                    	slli	a9, a5, 4
    179d:	11a5b0                                    	slli	a10, a5, 5
    17a0:	1177f0                                    	slli	a7, a7, 1
    17a3:	00a0b2                                    	movi	a11, 0
    17a6:	1166e0                                    	slli	a6, a6, 2
    17a9:	02a0c2                                    	movi	a12, 2
    17ac:	11ddc0                                    	slli	a13, a13, 4
    17af:	20f330                                    	or	a15, a3, a3
    17b2:	11eee0                                    	slli	a14, a14, 2
    17b5:	e48576                                    	loop	a5, 189d <FFT4N+0x121>
    17b8:	1906a4400cd118fe                  	{ ae_l32x2.xp	aed1, a15, a8; add	a5, a4, a9 }
    17c0:	0eace57c1104f8d0c580813904017c1f 	{ ae_sraa32rs	aed1, aed1, a12; ae_l32x2.xp	aed2, a15, a8; nop; nop }
    17d0:	0eace57c1904f8d0c58081390401bc2f 	{ ae_sraa32rs	aed2, aed2, a12; ae_l32x2.xp	aed3, a15, a8; nop; nop }
    17e0:	0ead25602100fed0c58081390401fc3f 	{ ae_sraa32rs	aed3, aed3, a12; ae_l32x2.i	aed4, a15, 0; nop; nop }
    17f0:	0ead656679224ad0c580813904013c4f 	{ ae_sraa32rs	aed4, aed4, a12; add	a15, a4, a10; nop; nop }
    1800:	04fb16                                    	beqz	a11, 1853 <FFT4N+0xd7>
    1803:	230740c614a57c5e                  	{ ae_la64.pp	u0, a5; nop }
    180b:	230700c10ce5b85e                  	{ ae_la16x4.ip	aed5, u0, a5; nop }
    1813:	1d06c44614b17cfe                  	{ ae_la64.pp	u0, a15; add	a5, a4, a13 }
    181b:	230700c114e5b8fe                  	{ ae_la16x4.ip	aed6, u0, a15; nop }
    1823:	230740c614a57c5e                  	{ ae_la64.pp	u0, a5; nop }
    182b:	0d0002c11ce5b85e                  	{ ae_la16x4.ip	aed7, u0, a5; ae_sel16i	aed5, aed5, aed6, 1 }
    1833:	0ded257b0d4c0c10c57861380c01015f 	{ nop; nop; nop; ae_mulfpc32x16x2ras	aed2, aed3, aed2, aed3, aed5 }
    1843:	0ded257b0d5c0c20097801380c01015f 	{ nop; nop; nop; ae_mulfpc32x16x2ras	aed4, aed0, aed4, aed0, aed7 }

00001853 <FFT4N+0xd7>:
    1853:	0e94e2800d33bb204d08811f0861526f 	{ add	a5, a2, a6; addi	a11, a11, 1; ae_addandsub32s	aed1, aed3, aed1, aed3; ae_addandsub32s	aed2, aed4, aed2, aed4 }
    1863:	0e957286247a47c8c41081100441233f 	{ addi	a3, a3, 8; add	a4, a4, a7; ae_addandsub32s	aed1, aed2, aed1, aed2; ae_mul32js	aed4, aed4 }
    1873:	0e956687fc4633d0c5808321047f722f 	{ ae_s32x2.xp	aed1, a2, a14; or	a15, a3, a3; ae_addandsub32s	aed31, aed3, aed3, aed4; nop }
    1883:	230760c01cc57e2e                  	{ ae_s32x2.xp	aed3, a2, a14; nop }
    188b:	230760c014c57e2e                  	{ ae_s32x2.xp	aed2, a2, a14; nop }
    1893:	230760c71cc57e2e                  	{ ae_s32x2.xp	aed31, a2, a14; nop }
    189b:	052d                                    	mov.n	a2, a5

0000189d <FFT4N+0x121>:
    189d:	f01d                                    	retw.n

0000189f <FFT4N+0x123>:
	...

000018a0 <FFT8N>:
    18a0:	008136                                    	entry	a1, 64
    18a3:	6129                                    	s32i.n	a2, a1, 24
    18a5:	1135d0                                    	slli	a3, a5, 3
    18a8:	1021e2                                    	l32i	a14, a1, 64
    18ab:	0215e6                                    	bgei	a5, 1, 18b1 <FFT8N+0x11>
    18ae:	009386                                    	j	1b00 <FFT8N+0x260>
    18b1:	82a082                                    	movi	a8, 130
    18b4:	51e9                                    	s32i.n	a14, a1, 20
    18b6:	5ad882                                    	addmi	a8, a8, 0x5a00
    18b9:	112122                                    	l32i	a2, a1, 68
    18bc:	230728400cc013ee                  	{ ae_l32x2.xp	aed1, a14, a3; ae_movda16	aed0, a8 }
    18c4:	0506b54014ca13ee                  	{ ae_l32x2.xp	aed2, a14, a3; or	a8, a5, a5 }
    18cc:	03053ec10083112e                  	{ s32i	a2, a1, 16; ae_l32x2.xp	aed3, a14, a3 }
    18d4:	39042e4104c113ee                  	{ ae_l32x2.xp	aed4, a14, a3; movi	a9, 126 }
    18dc:	1902b9c10cc913ee                  	{ ae_l32x2.xp	aed5, a14, a3; addmi	a9, a9, 0xffffa500 }
    18e4:	2307294114c913ee                  	{ ae_l32x2.xp	aed6, a14, a3; ae_movda16	aed9, a9 }
    18ec:	1300a0c11cc003ee                  	{ ae_l32x2.xp	aed7, a14, a3; ae_sel16i	aed0, aed0, aed9, 5 }
    18f4:	d1a750                                    	mul16s	a10, a7, a5
    18f7:	3e06874200e512ee                  	{ ae_l32x2.i	aed8, a14, 0; slli	a7, a7, 1 }
    18ff:	11e6e0                                    	slli	a14, a6, 2
    1902:	90daa0                                    	addx2	a13, a10, a10
    1905:	d1f560                                    	mul16s	a15, a5, a6
    1908:	4168                                    	l32i.n	a6, a1, 16
    190a:	f0baa0                                    	subx8	a11, a10, a10
    190d:	a0caa0                                    	addx4	a12, a10, a10
    1910:	115df0                                    	slli	a5, a13, 1
    1913:	71e9                                    	s32i.n	a14, a1, 28
    1915:	11efe0                                    	slli	a14, a15, 2
    1918:	b02a40                                    	addx8	a2, a10, a4
    191b:	51f8                                    	l32i.n	a15, a1, 20
    191d:	a09a40                                    	addx4	a9, a10, a4
    1920:	90aa40                                    	addx2	a10, a10, a4
    1923:	90bb40                                    	addx2	a11, a11, a4
    1926:	90cc40                                    	addx2	a12, a12, a4
    1929:	90d540                                    	addx2	a13, a5, a4
    192c:	445a                                    	add.n	a4, a4, a5
    192e:	1156f0                                    	slli	a5, a6, 1
    1931:	ff8b                                    	addi.n	a15, a15, 8
    1933:	605050                                    	neg	a5, a5
    1936:	5159                                    	s32i.n	a5, a1, 20
    1938:	050c                                    	movi.n	a5, 0
    193a:	178876                                    	loop	a8, 1955 <FFT8N+0xb5>
    193d:	030180                                    	rsr.lend	a8
    1940:	130080                                    	wsr.lbeg	a8
    1943:	f9d381                                    	l32r	a8, 90 (1b00 <FFT8N+0x260>)
    1946:	0020f0                                    	nop
    1949:	130180                                    	wsr.lend	a8
    194c:	002000                                    	isync
    194f:	030280                                    	rsr.lcount	a8
    1952:	01c882                                    	addi	a8, a8, 1
    1955:	4129                                    	s32i.n	a2, a1, 16
    1957:	5128                                    	l32i.n	a2, a1, 20
    1959:	0a5516                                    	beqz	a5, 1a02 <FFT8N+0x162>
    195c:	6a2a                                    	add.n	a6, a10, a2
    195e:	0306c94614b26c6e                  	{ ae_la64.pp	u0, a6; add	a8, a9, a2 }
    1966:	230700c20ce5b86e                  	{ ae_la16x4.ip	aed9, u0, a6; nop }
    196e:	2306c44614b16c8e                  	{ ae_la64.pp	u0, a8; add	a6, a4, a2 }
    1976:	230700c214e5b88e                  	{ ae_la16x4.ip	aed10, u0, a8; nop }
    197e:	100696c1049d018e                  	{ l32i	a8, a1, 16; ae_la64.pp	u0, a6 }
    1986:	150004c21ce9b86e                  	{ ae_la16x4.ip	aed11, u0, a6; ae_sel16i	aed9, aed9, aed10, 1 }
    198e:	6c2a                                    	add.n	a6, a12, a2
    1990:	882a                                    	add.n	a8, a8, a2
    1992:	0ded297b0e2c0c10c478613b1801a28f 	{ ae_la64.pp	u0, a8; nop; nop; ae_mulfpc32x16x2ras	aed2, aed3, aed2, aed3, aed9 }
    19a2:	230700c304e5b88e                  	{ ae_la16x4.ip	aed12, u0, a8; nop }
    19aa:	0306cd4614b26c6e                  	{ ae_la64.pp	u0, a6; add	a8, a13, a2 }
    19b2:	190005c30cfdb86e                  	{ ae_la16x4.ip	aed13, u0, a6; ae_sel16i	aed29, aed11, aed12, 1 }
    19ba:	0ded7d66362ab2214878a13b1801a28f 	{ ae_la64.pp	u0, a8; add	a6, a11, a2; nop; ae_mulfpc32x16x2ras	aed4, aed5, aed4, aed5, aed29 }
    19ca:	230700c314e5b88e                  	{ ae_la16x4.ip	aed14, u0, a8; nop }
    19d2:	230740c614a57c6e                  	{ ae_la64.pp	u0, a6; nop }
    19da:	1d0006c31cfeb86e                  	{ ae_la16x4.ip	aed15, u0, a6; ae_sel16i	aed30, aed13, aed14, 1 }
    19e2:	0ded3d7b0d540c31cd78e1380c01015f 	{ nop; nop; nop; ae_mulfpc32x16x2ras	aed6, aed7, aed6, aed7, aed30 }
    19f2:	0ded2d7b0d5c0c47d17901380c01015f 	{ nop; nop; nop; ae_mulfpc32x16x2ras	aed8, aed31, aed8, aed8, aed15 }

00001a02 <FFT8N+0x162>:
    1a02:	0ea4e43005202858c42902300069032f 	{ movi	a2, 3; movi	a8, 2; ae_sub32s	aed9, aed2, aed6; ae_sub32s	aed10, aed4, aed8 }
    1a12:	67ed24                                    	wur.ae_sar	a2
    1a15:	0e6ce168352d12f60701413f10016fff 	{ or	a6, a15, a15; l32i	a2, a1, 24; nop; ae_srai32	aed24, aed10, 1 }
    1a25:	0948c19045b9ff688d1ce15840a1515f 	{ addi	a5, a5, 1; addi	a15, a15, 8; ae_addandsubrng32.l	aed11, aed12, aed1, aed5; ae_addandsubrng32.l	aed7, aed13, aed3, aed7; ae_addandsubrng32.l	aed20, aed21, aed2, aed6 }
    1a35:	0e5562865a4ab7f047812b3a0167fa4f 	{ ae_addandsubrng32.l	aed22, aed23, aed4, aed8; add	a11, a11, a7; ae_addandsub32s	aed7, aed11, aed11, aed7; ae_srai32	aed1, aed9, 1 }
    1a45:	67ed84                                    	wur.ae_sar	a8
    1a48:	0e9576666d23d7e84d22cc6f11ac822f 	{ or	a8, a2, a2; add	a13, a13, a7; ae_addandsub32js	aed12, aed13, aed12, aed13; ae_addandsub32s	aed8, aed29, aed20, aed22 }
    1a58:	0e9566e6619ac7804dbd01c11f22436f 	{ ae_l32x2.xp	aed1, a6, a3; add	a12, a12, a7; ae_addandsubrng32.l	aed2, aed25, aed1, aed24; ae_addandsub32s	aed15, aed16, aed7, aed8 }
    1a68:	0500214014da036e                  	{ ae_l32x2.xp	aed2, a6, a3; ae_sel16i	aed26, aed2, aed2, 0 }
    1a70:	0ea57826499297d84df33ac91ce9c36f 	{ ae_l32x2.xp	aed3, a6, a3; add	a9, a9, a7; ae_subadd32s	aed9, aed26, aed25; ae_addandsub32s	aed28, aed27, aed26, aed25 }
    1a80:	030556c310e41b4e                  	{ ae_neg32s	aed14, aed27; ae_l32x2.xp	aed4, a6, a3 }
    1a88:	0e9573a6519aa7c8c5bbbe711d5c476f 	{ ae_l32x2.xp	aed5, a6, a3; add	a10, a10, a7; ae_sel16i.n	aed10, aed28, aed14, 2; ae_mul32js	aed14, aed29 }
    1a98:	0dd562862182475293d52b711d7e876f 	{ ae_l32x2.xp	aed6, a6, a3; add	a4, a4, a7; ae_addandsub32s	aed30, aed11, aed11, aed14; ae_mulf2p32x16x4s	aed9, aed10, aed10, aed9, aed0 }
    1aa8:	230720c11cc5136e                  	{ ae_l32x2.xp	aed7, a6, a3; nop }
    1ab0:	0e952e8044406ef84da94d4905a9fe8f 	{ ae_s32x2.xp	aed15, a8, a14; ae_l32x2.i	aed8, a6, 0; ae_addandsub32s	aed9, aed13, aed13, aed9; ae_addandsub32s	aed10, aed31, aed12, aed10 }
    1ac0:	3703e1420cc16e8e                  	{ ae_s32x2.xp	aed9, a8, a14; l32i	a6, a1, 28 }
    1ac8:	230760c21cc57e8e                  	{ ae_s32x2.xp	aed11, a8, a14; nop }
    1ad0:	230760c214c57e8e                  	{ ae_s32x2.xp	aed10, a8, a14; nop }
    1ad8:	2706e24404d06e8e                  	{ ae_s32x2.xp	aed16, a8, a14; add	a2, a2, a6 }
    1ae0:	230760c30cc57e8e                  	{ ae_s32x2.xp	aed13, a8, a14; nop }
    1ae8:	230760c714c57e8e                  	{ ae_s32x2.xp	aed30, a8, a14; nop }
    1af0:	230760c71cc57e8e                  	{ ae_s32x2.xp	aed31, a8, a14; nop }
    1af8:	4188                                    	l32i.n	a8, a1, 16
    1afa:	6129                                    	s32i.n	a2, a1, 24
    1afc:	887a                                    	add.n	a8, a8, a7
    1afe:	082d                                    	mov.n	a2, a8

00001b00 <FFT8N+0x260>:
    1b00:	f01d                                    	retw.n

00001b02 <FFT8N+0x262>:
	...

00001b04 <FFT12N>:
    1b04:	01e136                                    	entry	a1, 240
    1b07:	1903a140188f014e                  	{ s32i	a4, a1, 12; l32i	a8, a1, 244 }
    1b0f:	1145d0                                    	slli	a4, a5, 3
    1b12:	4199                                    	s32i.n	a9, a1, 16
    1b14:	2159                                    	s32i.n	a5, a1, 8
    1b16:	30c132                                    	addi	a3, a1, 48
    1b19:	066162                                    	s32i	a6, a1, 24
    1b1c:	205880                                    	or	a5, a8, a8
    1b1f:	056182                                    	s32i	a8, a1, 20
    1b22:	2716a6                                    	blti	a6, 1, 1b4d <FFT12N+0x49>
    1b25:	0f8676                                    	loop	a6, 1b38 <FFT12N+0x34>
    1b28:	230720c004c5145e                  	{ ae_l32x2.xp	aed0, a5, a4; nop }
    1b30:	230700c004e5993e                  	{ ae_s32x2.ip	aed0, a3, 8; nop }

00001b38 <FFT12N+0x34>:
    1b38:	30c1a2                                    	addi	a10, a1, 48
    1b3b:	0ea666                                    	bnei	a6, 12, 1b4d <FFT12N+0x49>
    1b3e:	f95531                                    	l32r	a3, 94 (548 <fix_fft12>)
    1b41:	0003e0                                    	callx8	a3
    1b44:	130c                                    	movi.n	a3, 1
    1b46:	0004c6                                    	j	1b5d <FFT12N+0x59>

00001b49 <FFT12N+0x45>:
    1b49:	00000000                                ....

00001b4d <FFT12N+0x49>:
    1b4d:	f95231                                    	l32r	a3, 98 (304 <fix_fft6>)
    1b50:	34c1b2                                    	addi	a11, a1, 52
    1b53:	30c1a2                                    	addi	a10, a1, 48
    1b56:	2c0c                                    	movi.n	a12, 2
    1b58:	0003e0                                    	callx8	a3
    1b5b:	030c                                    	movi.n	a3, 0
    1b5d:	21f8                                    	l32i.n	a15, a1, 8
    1b5f:	61b8                                    	l32i.n	a11, a1, 24
    1b61:	7139                                    	s32i.n	a3, a1, 28
    1b63:	07ad                                    	mov.n	a10, a7
    1b65:	d13f70                                    	mul16s	a3, a15, a7
    1b68:	02fd                                    	mov.n	a15, a2
    1b6a:	30c152                                    	addi	a5, a1, 48
    1b6d:	0f6d                                    	mov.n	a6, a15
    1b6f:	1133e0                                    	slli	a3, a3, 2
    1b72:	121ba6                                    	blti	a11, 1, 1b88 <FFT12N+0x84>
    1b75:	0f8b76                                    	loop	a11, 1b88 <FFT12N+0x84>
    1b78:	230720c004a5d95e                  	{ ae_l32x2.ip	aed0, a5, 8; nop }
    1b80:	230760c004c5736e                  	{ ae_s32x2.xp	aed0, a6, a3; nop }

00001b88 <FFT12N+0x84>:
    1b88:	2128                                    	l32i.n	a2, a1, 8
    1b8a:	34c172                                    	addi	a7, a1, 52
    1b8d:	21d8                                    	l32i.n	a13, a1, 8
    1b8f:	290c                                    	movi.n	a9, 2
    1b91:	3c2162                                    	l32i	a6, a1, 240
    1b94:	4158                                    	l32i.n	a5, a1, 16
    1b96:	0222e6                                    	bgei	a2, 2, 1b9c <FFT12N+0x98>
    1b99:	004906                                    	j	1cc1 <FFT12N+0x1bd>
    1b9c:	8179                                    	s32i.n	a7, a1, 32
    1b9e:	a07af0                                    	addx4	a7, a10, a15
    1ba1:	11fae0                                    	slli	a15, a10, 2
    1ba4:	5128                                    	l32i.n	a2, a1, 20
    1ba6:	908dd0                                    	addx2	a8, a13, a13
    1ba9:	3e2152                                    	l32i	a5, a1, 248
    1bac:	51f9                                    	s32i.n	a15, a1, 20
    1bae:	a0fd90                                    	addx4	a15, a13, a9
    1bb1:	1188f0                                    	slli	a8, a8, 1
    1bb4:	909d90                                    	addx2	a9, a13, a9
    1bb7:	82af60                                    	mull	a10, a15, a6
    1bba:	882b                                    	addi.n	a8, a8, 2
    1bbc:	31f8                                    	l32i.n	a15, a1, 12
    1bbe:	ed0b                                    	addi.n	a14, a13, -1
    1bc0:	1155f0                                    	slli	a5, a5, 1
    1bc3:	d1d6d0                                    	mul16s	a13, a6, a13
    1bc6:	30c1c2                                    	addi	a12, a1, 48
    1bc9:	228b                                    	addi.n	a2, a2, 8
    1bcb:	828860                                    	mull	a8, a8, a6
    1bce:	829960                                    	mull	a9, a9, a6
    1bd1:	1166f0                                    	slli	a6, a6, 1
    1bd4:	c05f50                                    	sub	a5, a15, a5
    1bd7:	b169                                    	s32i.n	a6, a1, 44
    1bd9:	116de0                                    	slli	a6, a13, 2
    1bdc:	a189                                    	s32i.n	a8, a1, 40
    1bde:	91a9                                    	s32i.n	a10, a1, 36
    1be0:	4199                                    	s32i.n	a9, a1, 16
    1be2:	31c9                                    	s32i.n	a12, a1, 12
    1be4:	4188                                    	l32i.n	a8, a1, 16
    1be6:	000dc6                                    	j	1c21 <FFT12N+0x11d>

00001be9 <FFT12N+0xe5>:
	...

00001bea <FFT12N+0xe6>:
    1bea:	f92b81                                    	l32r	a8, 98 (304 <fix_fft6>)
    1bed:	0008e0                                    	callx8	a8
    1bf0:	61b8                                    	l32i.n	a11, a1, 24
    1bf2:	21e8                                    	l32i.n	a14, a1, 8
    1bf4:	31c8                                    	l32i.n	a12, a1, 12
    1bf6:	078d                                    	mov.n	a8, a7
    1bf8:	141ba6                                    	blti	a11, 1, 1c10 <FFT12N+0x10c>
    1bfb:	0c9d                                    	mov.n	a9, a12
    1bfd:	0f8b76                                    	loop	a11, 1c10 <FFT12N+0x10c>
    1c00:	230720c004a5d99e                  	{ ae_l32x2.ip	aed0, a9, 8; nop }
    1c08:	230760c004c5738e                  	{ ae_s32x2.xp	aed0, a8, a3; nop }

00001c10 <FFT12N+0x10c>:
    1c10:	b188                                    	l32i.n	a8, a1, 44
    1c12:	51f8                                    	l32i.n	a15, a1, 20
    1c14:	ee0b                                    	addi.n	a14, a14, -1
    1c16:	228b                                    	addi.n	a2, a2, 8
    1c18:	558a                                    	add.n	a5, a5, a8
    1c1a:	77fa                                    	add.n	a7, a7, a15
    1c1c:	4188                                    	l32i.n	a8, a1, 16
    1c1e:	09fe16                                    	beqz	a14, 1cc1 <FFT12N+0x1bd>

00001c21 <FFT12N+0x11d>:
    1c21:	91a8                                    	l32i.n	a10, a1, 36
    1c23:	a1f8                                    	l32i.n	a15, a1, 40
    1c25:	db0b                                    	addi.n	a13, a11, -1
    1c27:	958a                                    	add.n	a9, a5, a8
    1c29:	028d                                    	mov.n	a8, a2
    1c2b:	050558c614a06c9e                  	{ ae_la64.pp	u0, a9; ae_l32x2.xp	aed0, a8, a4 }
    1c33:	050518c00ce2a89e                  	{ ae_la16x4.ip	aed1, u0, a9; ae_l32x2.xp	aed2, a8, a4 }
    1c3b:	0c9d                                    	mov.n	a9, a12
    1c3d:	0ded6166552b5a10827841391c01019f 	{ ae_s32x2.ip	aed0, a9, 8; add	a10, a5, a10; nop; ae_mulfpc32x16x2ras	aed1, aed2, aed2, aed2, aed1 }
    1c4d:	0d0691c00cf7999e                  	{ ae_s32x2.ip	aed1, a9, 8; srli	a12, a13, 1 }
    1c55:	513ba6                                    	blti	a11, 3, 1caa <FFT12N+0x1a6>
    1c58:	3f06a54004d2148e                  	{ ae_l32x2.xp	aed0, a8, a4; add	a11, a5, a15 }
    1c60:	050558c110a14c0e                  	{ loop	a12, 1caa <FFT12N+0x1a6>; ae_l32x2.xp	aed1, a8, a4 }
    1c68:	0b06da4614ab6cae                  	{ ae_la64.pp	u0, a10; or	a12, a10, a10 }
    1c70:	1b06db4614ab7dbe                  	{ ae_la64.pp	u1, a11; or	a13, a11, a11 }
    1c78:	15070d4014e3b8ce                  	{ ae_la16x4.ip	aed2, u0, a12; ae_la16x4.ip	aed3, u1, a13 }
    1c80:	aa6a                                    	add.n	a10, a10, a6
    1c82:	060001c21ce22b6e                  	{ add	a11, a11, a6; ae_sel16i	aed2, aed2, aed3, 1 }
    1c8a:	0ded217b09940c0083f821391c01108f 	{ ae_l32x2.xp	aed0, a8, a4; nop; nop; ae_mulfpc32x16x2ras	aed1, aed2, aed0, aed1, aed2 }
    1c9a:	050518c00ce1899e                  	{ ae_s32x2.ip	aed1, a9, 8; ae_l32x2.xp	aed1, a8, a4 }
    1ca2:	230700c014e5999e                  	{ ae_s32x2.ip	aed2, a9, 8; nop }

00001caa <FFT12N+0x1a6>:
    1caa:	7188                                    	l32i.n	a8, a1, 28
    1cac:	30c1a2                                    	addi	a10, a1, 48
    1caf:	21e9                                    	s32i.n	a14, a1, 8
    1cb1:	2c0c                                    	movi.n	a12, 2
    1cb3:	81b8                                    	l32i.n	a11, a1, 32
    1cb5:	f31816                                    	beqz	a8, 1bea <FFT12N+0xe6>
    1cb8:	f8f781                                    	l32r	a8, 94 (548 <fix_fft12>)
    1cbb:	0008e0                                    	callx8	a8
    1cbe:	ffcb86                                    	j	1bf0 <FFT12N+0xec>

00001cc1 <FFT12N+0x1bd>:
    1cc1:	f01d                                    	retw.n

00001cc3 <FFT12N+0x1bf>:
	...

00001cc4 <__do_global_ctors_aux>:
    1cc4:	004136                                    	entry	a1, 32
    1cc7:	f8f531                                    	l32r	a3, 9c (2000 <__CTOR_END__>)
    1cca:	fcc322                                    	addi	a2, a3, -4
    1ccd:	002222                                    	l32i	a2, a2, 0
    1cd0:	020283a4001e920e                  	{ beqi.w15	a2, -1, 1ce5 <__do_global_ctors_aux+0x21>; addi	a3, a3, -8 }

00001cd8 <__do_global_ctors_aux+0x14>:
    1cd8:	0002e0                                    	callx8	a2
    1cdb:	0328                                    	l32i.n	a2, a3, 0
    1cdd:	0302e3bc07ffb26e                  	{ bnei.w15	a2, -1, 1cd8 <__do_global_ctors_aux+0x14>; addi	a3, a3, -4 }
    1ce5:	f01d                                    	retw.n

00001ce7 <__do_global_ctors_aux+0x23>:
	...

00001ce8 <_fini>:
    1ce8:	008136                                    	entry	a1, 64
    1ceb:	f8ed81                                    	l32r	a8, a0 (bc <__do_global_dtors_aux>)
    1cee:	f03d                                    	nop.n
    1cf0:	0008e0                                    	callx8	a8

00001cf3 <_fini+0xb>:
    1cf3:	f01d                                    	retw.n
