
Build/lib/pisplit_skew_ctrl_position_independent.splitload.lib:     file format elf32-xtensa-le


Disassembly of section .text:

00000000 <_init-0x7c>:
       0:	000000cc 00001578 00001b8c 00000000     ....x...........
      10:	00001b5c 00000000 00001b74 00001c1c     \.......t.......
      20:	00001bb0 00000130 00001bcb 00000138     ....0.......8...
      30:	00001be0 0000016c 00001c04 00000bf8     ....l...........
      40:	00001ba0 23022501 000015a0 000015c0     .....%.#........
      50:	0000142c 000007e0 002aaaaa 55555555     ,.........*.UUUU
      60:	0ccd0000 00001138 00000560 000009e0     ....8...`.......
      70:	2aaaaaaa 00001b64 00000094              ...*d.......

0000007c <_init>:
      7c:	008136                                    	entry	a1, 64
      7f:	ffe081                                    	l32r	a8, 0 (cc <frame_dummy>)
      82:	f03d                                    	nop.n
      84:	0008e0                                    	callx8	a8
      87:	ffdf81                                    	l32r	a8, 4 (1578 <__do_global_ctors_aux>)
      8a:	f03d                                    	nop.n
      8c:	0008e0                                    	callx8	a8
      8f:	f01d                                    	retw.n

00000091 <_init+0x15>:
      91:	000000                                        ...

00000094 <__do_global_dtors_aux>:
      94:	004136                                    	entry	a1, 32
      97:	ffdc21                                    	l32r	a2, 8 (1b8c <__do_global_dtors_aux.completed>)
      9a:	000232                                    	l8ui	a3, a2, 0
      9d:	63ec                                    	bnez.n	a3, c7 <__do_global_dtors_aux+0x33>

0000009f <__do_global_dtors_aux+0xb>:
      9f:	1248                                    	l32i.n	a4, a2, 4
      a1:	0438                                    	l32i.n	a3, a4, 0
      a3:	04028424000163ae                  	{ beqz.w15	a3, b4 <__do_global_dtors_aux+0x20>; addi	a4, a4, 4 }
      ab:	1249                                    	s32i.n	a4, a2, 4
      ad:	0003e0                                    	callx8	a3
      b0:	fffac6                                    	j	9f <__do_global_dtors_aux+0xb>

000000b3 <__do_global_dtors_aux+0x1f>:
	...

000000b4 <__do_global_dtors_aux+0x20>:
      b4:	ffd631                                    	l32r	a3, c (0 <_text_start>)
      b7:	ffd6a1                                    	l32r	a10, 10 (1b5c <__FRAME_END__>)
      ba:	438c                                    	beqz.n	a3, c2 <__do_global_dtors_aux+0x2e>
      bc:	ffd481                                    	l32r	a8, c (0 <_text_start>)
      bf:	0008e0                                    	callx8	a8
      c2:	130c                                    	movi.n	a3, 1
      c4:	004232                                    	s8i	a3, a2, 0
      c7:	f01d                                    	retw.n

000000c9 <__do_global_dtors_aux+0x35>:
      c9:	000000                                        ...

000000cc <frame_dummy>:
      cc:	004136                                    	entry	a1, 32
      cf:	ffd121                                    	l32r	a2, 14 (0 <_text_start>)
      d2:	ffcfa1                                    	l32r	a10, 10 (1b5c <__FRAME_END__>)
      d5:	ffd0b1                                    	l32r	a11, 18 (1b74 <frame_dummy.object>)
      d8:	428c                                    	beqz.n	a2, e0 <frame_dummy+0x14>
      da:	ffce81                                    	l32r	a8, 14 (0 <_text_start>)
      dd:	0008e0                                    	callx8	a8

000000e0 <frame_dummy+0x14>:
      e0:	f01d                                    	retw.n

000000e2 <frame_dummy+0x16>:
	...

000000e4 <_start>:
      e4:	004136                                    	entry	a1, 32
      e7:	ffcd31                                    	l32r	a3, 1c (1c1c <printf_ptr>)
      ea:	0228                                    	l32i.n	a2, a2, 0
      ec:	ffcda1                                    	l32r	a10, 20 (1bb0 <export_parameter_array+0x10>)
      ef:	ffcdb1                                    	l32r	a11, 24 (130 <get_skewctrl_version>)
      f2:	006322                                    	s32i	a2, a3, 0
      f5:	0002e0                                    	callx8	a2
      f8:	002322                                    	l32i	a2, a3, 0
      fb:	ffcba1                                    	l32r	a10, 28 (1bcb <export_parameter_array+0x2b>)
      fe:	ffcbb1                                    	l32r	a11, 2c (138 <skew_ctrl_init>)
     101:	0002e0                                    	callx8	a2
     104:	002322                                    	l32i	a2, a3, 0
     107:	ffcaa1                                    	l32r	a10, 30 (1be0 <export_parameter_array+0x40>)
     10a:	ffcab1                                    	l32r	a11, 34 (16c <skew_ctrl_set_input_framesize>)
     10d:	0002e0                                    	callx8	a2
     110:	002322                                    	l32i	a2, a3, 0
     113:	ffc9a1                                    	l32r	a10, 38 (1c04 <export_parameter_array+0x64>)
     116:	ffc9b1                                    	l32r	a11, 3c (bf8 <skew_ctrl_process>)
     119:	0002e0                                    	callx8	a2
     11c:	ffc921                                    	l32r	a2, 40 (1ba0 <export_parameter_array>)
     11f:	f01d                                    	retw.n

00000121 <_start+0x3d>:
	...

00000130 <get_skewctrl_version>:
     130:	004136                                    	entry	a1, 32
     133:	ffc421                                    	l32r	a2, 44 (23022501 <_end+0x230208e1>)
     136:	f01d                                    	retw.n

00000138 <skew_ctrl_init>:
     138:	004136                                    	entry	a1, 32
     13b:	0e0441400080106e                  	{ movi	a6, 0; movi	a15, 1 }
     143:	623d                                    	ae_s16i.n	a3, a2, 0
     145:	0a0420410880125e                  	{ s16i	a5, a2, 10; movi	a11, 0 }
     14d:	1c0426410081026e                  	{ s16i	a6, a2, 8; movi	a12, 86 }
     155:	0a02a2411890024e                  	{ s16i	a4, a2, 14; addi	a10, a2, 64 }
     15d:	0652f2                                    	s16i	a15, a2, 12
     160:	12f9                                    	s32i.n	a15, a2, 4
     162:	ffb921                                    	l32r	a2, 48 (15a0 <memset>)
     165:	0002e0                                    	callx8	a2
     168:	f01d                                    	retw.n

0000016a <skew_ctrl_init+0x32>:
	...

0000016c <skew_ctrl_set_input_framesize>:
     16c:	008136                                    	entry	a1, 64
     16f:	070401400880024e                  	{ l16ui	a4, a2, 2; movi	a6, 1 }
     177:	230691c00097128e                  	{ l16ui	a8, a2, 0; srli	a14, a3, 1 }
     17f:	24035240008010fe                  	{ movi	a15, 0; l16ui	a5, a2, 4 }
     187:	360312000861f43e                  	{ beq.w15	a4, a3, 4a7 <skew_ctrl_set_input_framesize+0x33b>; l16ui	a7, a2, 14 }
     18f:	0402a8c00898023e                  	{ s16i	a3, a2, 2; addi	a4, a8, -32 }
     197:	0706b441009102fe                  	{ s32i	a15, a2, 16; saltu	a4, a4, a6 }
     19f:	0d04204318e0746e                  	{ ssr	a4; movi	a13, 0 }
     1a7:	9130e0                                    	srl	a3, a14
     1aa:	c15350                                    	mul16u	a5, a3, a5
     1ad:	230720a21c25376e                  	{ bnei.w15	a7, 16, 317 <skew_ctrl_set_input_framesize+0x1ab>; nop }
     1b5:	0c0460471c60f53e                  	{ addi	a3, a5, -1; movi	a13, 0 }
     1bd:	3306b2c210b6132e                  	{ slli	a10, a3, 30; srli	a11, a3, 2 }
     1c5:	0506f546140bf13e                  	{ l32r	a3, 4c (15c0 <__udivdi3>); or	a12, a5, a5 }
     1cd:	0020f0                                    	nop
     1d0:	f03d                                    	nop.n
     1d2:	f03d                                    	nop.n
     1d4:	0003e0                                    	callx8	a3
     1d7:	0a06fa400c6905fe                  	{ addi	a15, a5, 1; or	a4, a10, a10 }
     1df:	2306b34210a91f2e                  	{ slli	a10, a15, 30; or	a6, a3, a3 }
     1e7:	0d0420421ca052fe                  	{ srli	a11, a15, 2; movi	a13, 0 }
     1ef:	0506b541188b114e                  	{ s32i	a4, a1, 28; or	a12, a5, a5 }
     1f7:	35042fc1109f024e                  	{ s32i	a4, a2, 24; movi	a4, -1 }
     1ff:	0003e0                                    	callx8	a3
     202:	3d06c54718847f7e                  	{ movi	a7, -1; slli	a3, a5, 3 }
     20a:	3306874504e1254e                  	{ addx8	a4, a5, a4; slli	a7, a7, 30 }
     212:	3506b2c1109601ae                  	{ s32i	a10, a1, 24; srli	a11, a4, 2 }
     21a:	0d042042188012ae                  	{ s32i	a10, a2, 44; movi	a13, 0 }
     222:	2606b74700eaf33e                  	{ or	a12, a3, a3; or	a10, a7, a7 }
     22a:	0006e0                                    	callx8	a6
     22d:	3206d2c0009611fe                  	{ movi	a15, 1; srli	a11, a3, 2 }
     235:	0d042041088011ae                  	{ s32i	a10, a1, 20; movi	a13, 0 }
     23d:	2306af41188202ae                  	{ s32i	a10, a2, 28; slli	a10, a15, 30 }
     245:	0a06a54700e5e33e                  	{ or	a12, a3, a3; slli	a4, a5, 6 }
     24d:	0006e0                                    	callx8	a6
     250:	3302a4c1009f11ae                  	{ s32i	a10, a1, 16; addi	a3, a4, -1 }
     258:	3306b2c3009612ae                  	{ s32i	a10, a2, 48; srli	a11, a3, 2 }
     260:	2606d740008a10de                  	{ movi	a13, 0; or	a10, a7, a7 }
     268:	0204214700e0f44e                  	{ or	a12, a4, a4; movi	a3, 1 }
     270:	0006e0                                    	callx8	a6
     273:	3506b2c018b6032e                  	{ slli	a3, a3, 30; srli	a11, a4, 2 }
     27b:	0d042040188011ae                  	{ s32i	a10, a1, 12; movi	a13, 0 }
     283:	2306b342008a12ae                  	{ s32i	a10, a2, 32; or	a10, a3, a3 }
     28b:	3806a54700e5e44e                  	{ or	a12, a4, a4; slli	a7, a5, 8 }
     293:	0006e0                                    	callx8	a6
     296:	3e046fc71c7ff76e                  	{ addi	a6, a7, -1; movi	a15, -1 }
     29e:	0b06ba421ca9426e                  	{ srli	a11, a6, 2; or	a4, a10, a10 }
     2a6:	0d0460451400f96e                  	{ l32r	a6, 4c (15c0 <__udivdi3>); movi	a13, 0 }
     2ae:	2306af43088202ae                  	{ s32i	a10, a2, 52; slli	a10, a15, 30 }
     2b6:	07cd                                    	mov.n	a12, a7
     2b8:	0006e0                                    	callx8	a6
     2bb:	2306b340108a11ae                  	{ s32i	a10, a1, 8; or	a10, a3, a3 }
     2c3:	3706f2c51416f23e                  	{ l32r	a3, 4c (15c0 <__udivdi3>); srli	a11, a7, 2 }
     2cb:	0606d740008b10de                  	{ movi	a13, 0; or	a12, a7, a7 }
     2d3:	0003e0                                    	callx8	a3
     2d6:	3f042fc510bf157e                  	{ slli	a6, a5, 9; movi	a15, -1 }
     2de:	3a06fa471c69e65e                  	{ addi	a5, a6, -1; or	a7, a10, a10 }
     2e6:	0d0420421ca0525e                  	{ srli	a11, a5, 2; movi	a13, 0 }
     2ee:	0706b64210ab0f2e                  	{ slli	a10, a15, 30; or	a12, a6, a6 }
     2f6:	0003e0                                    	callx8	a3
     2f9:	1a06da40008901fe                  	{ movi	a15, 1; or	a5, a10, a10 }
     301:	0d0420421ca0526e                  	{ srli	a11, a6, 2; movi	a13, 0 }
     309:	06cd                                    	mov.n	a12, a6
     30b:	22068f400862093e                  	{ j	448 <skew_ctrl_set_input_framesize+0x2dc>; slli	a10, a15, 30 }

00000313 <skew_ctrl_set_input_framesize+0x1a7>:
     313:	00000000                                ....

00000317 <skew_ctrl_set_input_framesize+0x1ab>:
     317:	0506b2c018b5152e                  	{ slli	a3, a5, 30; srli	a4, a5, 2 }
     31f:	1d02e5450400eb7e                  	{ l32r	a7, 4c (15c0 <__udivdi3>); addi	a12, a5, 1 }
     327:	3406b44610eae33e                  	{ or	a10, a3, a3; or	a11, a4, a4 }
     32f:	f03d                                    	nop.n
     331:	f03d                                    	nop.n
     333:	0007e0                                    	callx8	a7
     336:	3d02a5c1189f01ae                  	{ s32i	a10, a1, 28; addi	a12, a5, -1 }
     33e:	0d042041108012ae                  	{ s32i	a10, a2, 24; movi	a13, 0 }
     346:	3406b44610eae33e                  	{ or	a10, a3, a3; or	a11, a4, a4 }
     34e:	0007e0                                    	callx8	a7
     351:	0706a54418bf05de                  	{ slli	a3, a5, 3; addx8	a12, a5, a6 }
     359:	3306b2c1109411ae                  	{ s32i	a10, a1, 24; srli	a3, a3, 2 }
     361:	0b042042188002ae                  	{ s32i	a10, a2, 44; movi	a10, 0 }
     369:	3206d340008a10de                  	{ movi	a13, 0; or	a11, a3, a3 }
     371:	0007e0                                    	callx8	a7
     374:	35042fc1089f01ae                  	{ s32i	a10, a1, 20; movi	a4, -1 }
     37c:	0506a541189f02ae                  	{ s32i	a10, a2, 28; addx8	a12, a5, a4 }
     384:	0c044040008010ae                  	{ movi	a10, 0; movi	a13, 0 }
     38c:	2a06a54618e5e33e                  	{ or	a11, a3, a3; slli	a6, a5, 6 }
     394:	0007e0                                    	callx8	a7
     397:	0706b2c1009501ae                  	{ s32i	a10, a1, 16; srli	a4, a6, 2 }
     39f:	1d02a643008002ae                  	{ s32i	a10, a2, 48; addi	a12, a6, 1 }
     3a7:	0c044040008010ae                  	{ movi	a10, 0; movi	a13, 0 }
     3af:	3606b74618e8f44e                  	{ or	a11, a4, a4; or	a3, a7, a7 }
     3b7:	0007e0                                    	callx8	a7
     3ba:	3d02a6c0189f01ae                  	{ s32i	a10, a1, 12; addi	a12, a6, -1 }
     3c2:	0b042042008002ae                  	{ s32i	a10, a2, 32; movi	a10, 0 }
     3ca:	3406d440008a00de                  	{ movi	a13, 0; or	a11, a4, a4 }
     3d2:	0007e0                                    	callx8	a7
     3d5:	0b06ba4518a9058e                  	{ slli	a7, a5, 8; or	a4, a10, a10 }
     3dd:	2706b2c3089512ae                  	{ s32i	a10, a2, 52; srli	a6, a7, 2 }
     3e5:	0a0460400c6007ce                  	{ addi	a12, a7, 1; movi	a10, 0 }
     3ed:	3606d640008a00de                  	{ movi	a13, 0; or	a11, a6, a6 }
     3f5:	0003e0                                    	callx8	a3
     3f8:	3d02a7c0109f01ae                  	{ s32i	a10, a1, 8; addi	a12, a7, -1 }
     400:	0c044040008010ae                  	{ movi	a10, 0; movi	a13, 0 }
     408:	06bd                                    	mov.n	a11, a6
     40a:	0003e0                                    	callx8	a3
     40d:	3706e5440404ff5e                  	{ l32r	a5, 4c (15c0 <__udivdi3>); slli	a3, a5, 9 }
     415:	2206b2c518f5faae                  	{ or	a7, a10, a10; srli	a6, a3, 2 }
     41d:	0a0460400c6003ce                  	{ addi	a12, a3, 1; movi	a10, 0 }
     425:	3606d640008a00de                  	{ movi	a13, 0; or	a11, a6, a6 }
     42d:	0005e0                                    	callx8	a5
     430:	3d02e3c4041fe73e                  	{ l32r	a3, 4c (15c0 <__udivdi3>); addi	a12, a3, -1 }
     438:	0a04204508e0eaae                  	{ or	a5, a10, a10; movi	a10, 0 }
     440:	3606d640008a00de                  	{ movi	a13, 0; or	a11, a6, a6 }
     448:	0003e0                                    	callx8	a3
     44b:	380381411c80013e                  	{ l32i	a3, a1, 28; l32i	a8, a1, 12 }
     453:	3a0285c10c9f116e                  	{ l32i	a6, a1, 20; addi	a11, a5, -1 }
     45b:	2c0381401481019e                  	{ l32i	a9, a1, 8; l32i	a12, a1, 24 }
     463:	0c03e1471c61f33e                  	{ addi	a3, a3, -1; l32i	a13, a1, 16 }
     46b:	3702a6c1109f023e                  	{ s32i	a3, a2, 24; addi	a6, a6, -1 }
     473:	3302a8c1189f126e                  	{ s32i	a6, a2, 28; addi	a3, a8, -1 }
     47b:	3702a9c2009f023e                  	{ s32i	a3, a2, 32; addi	a6, a9, -1 }
     483:	3202edc71c7ffc5e                  	{ addi	a5, a12, -1; addi	a3, a13, -1 }
     48b:	3e02e7c71c7fe44e                  	{ addi	a4, a4, -1; addi	a14, a7, -1 }
     493:	3f02aac2089f126e                  	{ s32i	a6, a2, 36; addi	a15, a10, -1 }
     49b:	a2b9                                    	s32i.n	a11, a2, 40
     49d:	b259                                    	s32i.n	a5, a2, 44
     49f:	c239                                    	s32i.n	a3, a2, 48
     4a1:	d249                                    	s32i.n	a4, a2, 52
     4a3:	e2e9                                    	s32i.n	a14, a2, 56
     4a5:	f2f9                                    	s32i.n	a15, a2, 60
     4a7:	f01d                                    	retw.n

000004a9 <skew_ctrl_set_input_framesize+0x33d>:
     4a9:	000000                                        ...

000004ac <Get_Src_Len>:
     4ac:	004136                                    	entry	a1, 32
     4af:	070409411089025e                  	{ l16ui	a5, a2, 12; movi	a6, 0x249 }
     4b7:	0704294518a906be                  	{ slli	a7, a6, 5; movi	a6, 0x249 }
     4bf:	0704204600a106ae                  	{ slli	a8, a6, 6; movi	a6, 64 }
     4c7:	280302250c12b5ce                  	{ blti.w15	a5, 5, 500 <Get_Src_Len+0x54>; l16si	a9, a2, 148 }
     4cf:	0f0426261800f58e                  	{ bgeui.w15	a5, 16, 532 <Get_Src_Len+0x86>; movi	a15, 6 }
     4d7:	070405031000d75e                  	{ bbs.w15	a7, a5, 4f5 <Get_Src_Len+0x49>; movi	a7, 5 }
     4df:	0b0404021400685e                  	{ bbc.w15	a8, a5, 511 <Get_Src_Len+0x65>; movi	a10, 4 }
     4e7:	2906a7c718feb79e                  	{ movgez	a15, a7, a9; movgez	a10, a7, a9 }
     4ef:	63fd                                    	ae_s16i.n	a15, a3, 0
     4f1:	64ad                                    	ae_s16i.n	a10, a4, 0
     4f3:	f01d                                    	retw.n

000004f5 <Get_Src_Len+0x49>:
     4f5:	2203024000720d4e                  	{ j	546 <Get_Src_Len+0x9a>; l16si	a2, a2, 148 }

000004fd <Get_Src_Len+0x51>:
     4fd:	000000                                        ...

00000500 <Get_Src_Len+0x54>:
     500:	070408241000850e                  	{ beqi.w15	a5, 2, 517 <Get_Src_Len+0x6b>; movi	a6, 8 }
     508:	153526                                    	beqi	a5, 3, 521 <Get_Src_Len+0x75>
     50b:	314526                                    	beqi	a5, 4, 540 <Get_Src_Len+0x94>
     50e:	000806                                    	j	532 <Get_Src_Len+0x86>

00000511 <Get_Src_Len+0x65>:
     511:	1d7566                                    	bnei	a5, 7, 532 <Get_Src_Len+0x86>
     514:	000a06                                    	j	540 <Get_Src_Len+0x94>

00000517 <Get_Src_Len+0x6b>:
     517:	520c                                    	movi.n	a2, 5
     519:	0404054000601b3e                  	{ j	558 <Get_Src_Len+0xac>; movi	a5, 5 }

00000521 <Get_Src_Len+0x75>:
     521:	620c                                    	movi.n	a2, 6
     523:	040404400060113e                  	{ j	558 <Get_Src_Len+0xac>; movi	a5, 4 }

0000052b <Get_Src_Len+0x7f>:
     52b:	00000000                                 .......

00000532 <Get_Src_Len+0x86>:
     532:	060460451468f55e                  	{ addi	a5, a5, -10; movi	a7, 0x200 }
     53a:	00a162                                    	movi	a6, 0x100
     53d:	936750                                    	movnez	a6, a7, a5
     540:	4a9222                                    	l16si	a2, a2, 148
     543:	f22260                                    	rems	a2, a2, a6
     546:	050c                                    	movi.n	a5, 0
     548:	0f04264508e0f52e                  	{ salt	a5, a5, a2; movi	a15, 6 }
     550:	0502854014e15f5e                  	{ sub	a2, a15, a5; addi	a5, a5, 4 }
     558:	632d                                    	ae_s16i.n	a2, a3, 0
     55a:	645d                                    	ae_s16i.n	a5, a4, 0
     55c:	f01d                                    	retw.n

0000055e <Get_Src_Len+0xb2>:
	...

00000560 <skew_ctrl_32b>:
     560:	008136                                    	entry	a1, 64
     563:	12069240048a03ee                  	{ l32i	a14, a3, 0; or	a9, a2, a2 }
     56b:	3306b345088912ee                  	{ s32i	a14, a2, 84; or	a7, a3, a3 }
     573:	1303a3401880016e                  	{ s32i	a6, a1, 12; l32i	a2, a3, 4 }
     57b:	166922                                    	s32i	a2, a9, 88
     57e:	2303a3411080015e                  	{ s32i	a5, a1, 24; l32i	a2, a3, 8 }
     586:	050429451889192e                  	{ s32i	a2, a9, 92; movi	a5, 0x249 }
     58e:	2a0685401c85032e                  	{ l32i	a2, a3, 12; slli	a6, a5, 6 }
     596:	230339460081092e                  	{ s32i	a2, a9, 96; l16ui	a2, a9, 12 }
     59e:	02040941048913fe                  	{ l32i	a15, a3, 16; movi	a3, 0x249 }
     5a6:	3b06a346088419fe                  	{ s32i	a15, a9, 100; slli	a3, a3, 5 }
     5ae:	290309210c12a2ce                  	{ blti.w15	a2, 5, 5e8 <skew_ctrl_32b+0x88>; l16si	a8, a9, 148 }
     5b6:	050425221800f28e                  	{ bgeui.w15	a2, 16, 618 <skew_ctrl_32b+0xb8>; movi	a5, 5 }
     5be:	030400031001f32e                  	{ bbs.w15	a3, a2, 5e0 <skew_ctrl_32b+0x80>; movi	a3, 64 }
     5c6:	070404021400a62e                  	{ bbc.w15	a6, a2, 600 <skew_ctrl_32b+0xa0>; movi	a6, 4 }
     5ce:	2806c5c0009d062e                  	{ movi	a2, 6; movgez	a6, a5, a8 }
     5d6:	280685c0007c066e                  	{ j	640 <skew_ctrl_32b+0xe0>; movgez	a2, a5, a8 }

000005de <skew_ctrl_32b+0x7e>:
	...

000005e0 <skew_ctrl_32b+0x80>:
     5e0:	2203094000720a4e                  	{ j	62e <skew_ctrl_32b+0xce>; l16si	a2, a9, 148 }

000005e8 <skew_ctrl_32b+0x88>:
     5e8:	030408201000d20e                  	{ beqi.w15	a2, 2, 606 <skew_ctrl_32b+0xa6>; movi	a3, 8 }
     5f0:	060404201800e20e                  	{ beqi.w15	a2, 3, 610 <skew_ctrl_32b+0xb0>; movi	a6, 4 }
     5f8:	2a4226                                    	beqi	a2, 4, 626 <skew_ctrl_32b+0xc6>
     5fb:	000646                                    	j	618 <skew_ctrl_32b+0xb8>

000005fe <skew_ctrl_32b+0x9e>:
	...

00000600 <skew_ctrl_32b+0xa0>:
     600:	147266                                    	bnei	a2, 7, 618 <skew_ctrl_32b+0xb8>
     603:	0007c6                                    	j	626 <skew_ctrl_32b+0xc6>

00000606 <skew_ctrl_32b+0xa6>:
     606:	520c                                    	movi.n	a2, 5
     608:	060405400060043e                  	{ j	640 <skew_ctrl_32b+0xe0>; movi	a6, 5 }

00000610 <skew_ctrl_32b+0xb0>:
     610:	0204064000600c2e                  	{ j	640 <skew_ctrl_32b+0xe0>; movi	a2, 6 }

00000618 <skew_ctrl_32b+0xb8>:
     618:	040460451468f22e                  	{ addi	a2, a2, -10; movi	a5, 0x200 }
     620:	00a132                                    	movi	a3, 0x100
     623:	933520                                    	movnez	a3, a5, a2
     626:	4a9922                                    	l16si	a2, a9, 148
     629:	f03d                                    	nop.n
     62b:	f22230                                    	rems	a2, a2, a3
     62e:	030c                                    	movi.n	a3, 0
     630:	0f04264418e0f32e                  	{ salt	a3, a3, a2; movi	a15, 6 }
     638:	0702834014e14f3e                  	{ sub	a2, a15, a3; addi	a6, a3, 4 }
     640:	1303194014e1329e                  	{ addx4	a2, a2, a9; l16ui	a3, a9, 10 }
     648:	0202e2440470075e                  	{ addi	a5, a7, 16; addi	a2, a2, 64 }
     650:	3b042fc1089f016e                  	{ s32i	a6, a1, 20; movi	a10, -1 }
     658:	070420411880017e                  	{ s32i	a7, a1, 28; movi	a6, 0 }
     660:	2b06b2c00896019e                  	{ s32i	a9, a1, 4; srli	a10, a10, 2 }
     668:	3f042fc0109f115e                  	{ s32i	a5, a1, 8; movi	a15, -1 }
     670:	0203c9201401136e                  	{ bnei.w15	a3, 2, 714 <skew_ctrl_32b+0x1b4>; l32i	a3, a9, 16 }
     678:	040400410c8019ae                  	{ l32i	a10, a9, 20; movi	a5, 0 }
     680:	1f06f2c11c16f5ce                  	{ l32r	a12, 54 (7e0 <IN_LINE_32b_5ord>); srli	a9, a15, 2 }
     688:	41a9                                    	s32i.n	a10, a1, 16
     68a:	280699401ce933ae                  	{ add	a3, a3, a10; or	a6, a9, a9 }
     692:	3906e3c600641e3e                  	{ extui	a8, a3, 30, 2; and	a3, a3, a9 }
     69a:	3306934204ea382e                  	{ addx4	a8, a8, a2; or	a11, a3, a3 }
     6a2:	3d06bc4410e9c85e                  	{ movnez	a2, a8, a5; or	a7, a12, a12 }
     6aa:	1402a54610e0f22e                  	{ or	a10, a2, a2; addi	a5, a5, 1 }
     6b2:	000ce0                                    	callx8	a12
     6b5:	1903a140008104ae                  	{ s32i	a10, a4, 0; l32i	a8, a1, 20 }
     6bd:	06069741048b11ae                  	{ l32i	a10, a1, 16; or	a12, a7, a7 }
     6c5:	069d                                    	mov.n	a9, a6
     6c7:	0502c41e1fe1e85e                  	{ bne.w15	a8, a5, 68a <skew_ctrl_32b+0x12a>; addi	a4, a4, 4 }
     6cf:	3128                                    	l32i.n	a2, a1, 12
     6d1:	2158                                    	l32i.n	a5, a1, 8
     6d3:	290692050c18882e                  	{ bgeu.w15	a8, a2, 70a <skew_ctrl_32b+0x1aa>; sub	a2, a2, a8 }
     6db:	320282c01cff23ae                  	{ add	a3, a3, a10; addi	a2, a2, -1 }
     6e3:	3906e3c600641e3e                  	{ extui	a8, a3, 30, 2; and	a3, a3, a9 }
     6eb:	330693410cea385e                  	{ addx4	a5, a8, a5; or	a11, a3, a3 }
     6f3:	05ad                                    	mov.n	a10, a5
     6f5:	0007e0                                    	callx8	a7
     6f8:	0b03a140008104ae                  	{ s32i	a10, a4, 0; l32i	a10, a1, 16 }
     700:	069d                                    	mov.n	a9, a6
     702:	0402e43d03e1a2ee                  	{ bnez.w15	a2, 6db <skew_ctrl_32b+0x17b>; addi	a4, a4, 4 }
     70a:	1198                                    	l32i.n	a9, a1, 4
     70c:	4939                                    	s32i.n	a3, a9, 16
     70e:	59a9                                    	s32i.n	a10, a9, 20
     710:	002906                                    	j	7b8 <skew_ctrl_32b+0x258>

00000713 <skew_ctrl_32b+0x1b3>:
	...

00000714 <skew_ctrl_32b+0x1b4>:
     714:	fe4fd1                                    	l32r	a13, 50 (142c <intp_filter_32b>)
     717:	080389410c81095e                  	{ l32i	a5, a9, 20; l32i	a8, a9, 16 }
     71f:	0c04214418e0eaae                  	{ or	a3, a10, a10; movi	a12, 1 }
     727:	207dd0                                    	or	a7, a13, a13
     72a:	805850                                    	add	a5, a8, a5
     72d:	1b06e5c600650e5e                  	{ extui	a8, a5, 30, 2; and	a5, a5, a10 }
     735:	1b03814204e0282e                  	{ addx4	a8, a8, a2; l32i	a10, a1, 4 }
     73d:	2706b8410084095e                  	{ s32i	a5, a9, 16; movnez	a2, a8, a6 }
     745:	1602a64618e0e22e                  	{ or	a11, a2, a2; addi	a6, a6, 1 }
     74d:	000de0                                    	callx8	a13
     750:	180381410c80115e                  	{ l32i	a5, a1, 20; l32i	a9, a1, 4 }
     758:	1706b740008b14ae                  	{ s32i	a10, a4, 0; or	a13, a7, a7 }
     760:	03ad                                    	mov.n	a10, a3
     762:	0402c41e1fe1856e                  	{ bne.w15	a5, a6, 717 <skew_ctrl_32b+0x1b7>; addi	a4, a4, 4 }
     76a:	3128                                    	l32i.n	a2, a1, 12
     76c:	0221b2                                    	l32i	a11, a1, 8
     76f:	2406b2050818352e                  	{ bgeu.w15	a5, a2, 7b8 <skew_ctrl_32b+0x258>; sub	a2, a2, a5 }
     777:	060389410c81095e                  	{ l32i	a5, a9, 20; l32i	a6, a9, 16 }
     77f:	1c0c                                    	movi.n	a12, 1
     781:	220b                                    	addi.n	a2, a2, -1
     783:	565a                                    	add.n	a5, a6, a5
     785:	1b06e5c510650e5e                  	{ extui	a6, a5, 30, 2; and	a5, a5, a10 }
     78d:	1b0381421ce026be                  	{ addx4	a11, a6, a11; l32i	a10, a1, 4 }
     795:	1b06bb410089195e                  	{ s32i	a5, a9, 16; or	a5, a11, a11 }
     79d:	0007e0                                    	callx8	a7
     7a0:	1903a140008014ae                  	{ s32i	a10, a4, 0; l32i	a9, a1, 4 }
     7a8:	2206b34618eaf55e                  	{ or	a11, a5, a5; or	a10, a3, a3 }
     7b0:	0502e43d03e102ee                  	{ bnez.w15	a2, 777 <skew_ctrl_32b+0x217>; addi	a4, a4, 4 }
     7b8:	7128                                    	l32i.n	a2, a1, 28
     7ba:	6138                                    	l32i.n	a3, a1, 24
     7bc:	a02320                                    	addx4	a2, a3, a2
     7bf:	ecc222                                    	addi	a2, a2, -20
     7c2:	02f8                                    	l32i.n	a15, a2, 0
     7c4:	1069f2                                    	s32i	a15, a9, 64
     7c7:	1238                                    	l32i.n	a3, a2, 4
     7c9:	116932                                    	s32i	a3, a9, 68
     7cc:	2238                                    	l32i.n	a3, a2, 8
     7ce:	126932                                    	s32i	a3, a9, 72
     7d1:	3238                                    	l32i.n	a3, a2, 12
     7d3:	136932                                    	s32i	a3, a9, 76
     7d6:	4228                                    	l32i.n	a2, a2, 16
     7d8:	146922                                    	s32i	a2, a9, 80
     7db:	f01d                                    	retw.n

000007dd <skew_ctrl_32b+0x27d>:
     7dd:	000000                                        ...

000007e0 <IN_LINE_32b_5ord>:
     7e0:	006136                                    	entry	a1, 48
     7e3:	050461400c00fd6e                  	{ l32r	a6, 58 (2aaaaa <_end+0x2a8e8a>); movi	a5, 1 }
     7eb:	0704214108a015ae                  	{ slli	a5, a5, 22; movi	a7, 1 }
     7f3:	050636c508a013fe                  	{ slli	a5, a3, 1; ae_movda32x2	aed0, a6, a5 }
     7fb:	3306a7401081015e                  	{ s32i	a5, a1, 8; slli	a7, a7, 30 }
     803:	2107274018c1021e                  	{ ae_l32f24.i	aed3, a1, 8; ae_cvt48a32	aed1, a7 }
     80b:	310765401ccac31e                  	{ ae_s32.l.i	aed3, a1, 12; ae_movda32	aed10, a5 }
     813:	3b046dc00c1fe28e                  	{ l32r	a8, 5c (55555555 <_end+0x55553935>); movi	a10, -3 }
     81b:	200728401cc2231e                  	{ ae_l32.i	aed3, a1, 12; ae_cvt48a32	aed2, a8 }
     823:	0e5cd71001f11821840c2b161c600f9f 	{ l32r	a9, 60 (ccd0000 <_end+0xccce3e0>); movi	a8, 1; ae_mulaf32s.lh	aed2, aed3, aed0; ae_cvtq56p32s.l	aed6, aed3 }
     833:	0e9d672b392094d0c5808b081c60281f 	{ ae_slaaq56	aed4, aed1, a8; ae_movda32	aed7, a9; ae_mulaf32s.ll	aed1, aed3, aed0; nop }
     843:	0e5d544550fea321459030201060022f 	{ ae_round24x2f48ssym	aed2, aed2, aed2; slli	a10, a10, 29; ae_mulsf32s.lh	aed4, aed3, aed0; ae_mov	aed5, aed4 }
     853:	0e9d444b40e8a460c598d1280860011f 	{ ae_round24x2f48ssym	aed1, aed1, aed1; ae_movda32	aed8, a10; ae_mulsf32s.ll	aed5, aed3, aed0; ae_sub64	aed6, aed5, aed6 }
     863:	0e9cc657f8e0c3a0c3908d491022126f 	{ ae_round24x2f48ssym	aed6, aed6, aed6; movi	a3, -4; ae_mulf32s.ll	aed9, aed1, aed2; ae_round24x2f48ssym	aed4, aed4, aed4 }
     873:	0e1cc65e58f023246dab6d110862115f 	{ ae_round24x2f48ssym	aed5, aed5, aed5; ae_l32.xp	aed11, a2, a3; ae_mulf32s.ll	aed2, aed3, aed2; ae_mulf32s.ll	aed22, aed4, aed6 }
     883:	0e1d265b0d340c2c702b6d230c85699f 	{ ae_trunca32q48	a9, aed9; nop; ae_mulf32s.ll	aed4, aed4, aed5; ae_mulf32s.ll	aed24, aed5, aed6 }
     893:	0e9d664bbd2194d0c4008d0b0c616a6f 	{ ae_trunca32q48	a10, aed22; ae_movda32	aed23, a9; ae_mulf32s.ll	aed1, aed3, aed1; nop }
     8a3:	0e9d662bcd21a4a0c2708cd30eea6d8f 	{ ae_trunca32q48	a13, aed24; ae_movda32	aed25, a10; ae_mulf32r.hh	aed26, aed23, aed10; ae_round24x2f48ssym	aed28, aed4, aed4 }
     8b3:	0e9d664bed20d4d0c4008d330f866b2f 	{ ae_trunca32q48	a11, aed2; ae_movda32	aed29, a13; ae_mulf32s.ll	aed6, aed28, aed6; nop }
     8c3:	000003f3008300fe762303063460b3ef 	{ ae_trunca32q48	a14, aed26; ae_trunca32q48	a12, aed1; nop }
     8d3:	0ead796b6530e4a0c207413b0c016f6f 	{ ae_trunca32q48	a15, aed6; ae_movda32	aed12, a14; nop; ae_round24x2f48ssym	aed1, aed26, aed26 }
     8e3:	0e1d4e2b11e1b4cbcaab4c2103acdf8f 	{ ae_movda32	aed13, a15; ae_movda32	aed2, a11; ae_mulf32r.hh	aed4, aed29, aed12; ae_mulf32r.hh	aed5, aed25, aed12 }
     8f3:	0e1cce5e51e92313c4ab4d010781bc8f 	{ ae_movda32	aed27, a12; ae_l32.xp	aed10, a2, a3; ae_mulf32s.ll	aed0, aed28, aed1; ae_mulf32r.hh	aed2, aed2, aed13 }
     903:	000002f3108300fbf6260619240db36f 	{ ae_trunca32q48	a6, aed4; ae_trunca32q48	a7, aed5; ae_mulf32r.hh	aed3, aed23, aed13 }
     913:	0e9d462b91e174d0c4808c49076d168f 	{ ae_movda32	aed17, a6; ae_movda32	aed18, a7; ae_mulf32r.hh	aed9, aed27, aed13; nop }
     923:	000002330c0000fc568606982007d8ff 	{ ae_slaaq56	aed15, aed0, a8; ae_trunca32q48	a8, aed2; ae_mulf32r.hh	aed19, aed17, aed7 }
     933:	0e5d622ba508844c07806c131e48c00f 	{ ae_add64	aed0, aed0, aed15; ae_movda32	aed20, a8; ae_mulf32r.hh	aed2, aed18, aed8; ae_slai64s	aed16, aed3, 1 }
     943:	00000293208300fcf62606bc3c08b3bf 	{ ae_trunca32q48	a11, aed19; ae_trunca32q48	a9, aed9; ae_mulf32r.hh	aed23, aed20, aed8 }
     953:	0ea5600bc520b4d0c40043830d836c2f 	{ ae_trunca32q48	a12, aed2; ae_movda32	aed24, a11; ae_add64s	aed3, aed3, aed16; ae_roundsq32f48asym	aed0, aed0 }
     963:	0e9d622bad3894d0c4584cdb0d786a0f 	{ ae_trunca32q48	a10, aed0; ae_movda32	aed21, a9; ae_mulf32r.hh	aed27, aed11, aed24; ae_roundsq32f48asym	aed22, aed3 }
     973:	1305c24610fe1cee                  	{ ae_movda32	aed26, a12; ae_l32.xp	aed30, a2, a3 }
     97b:	0e1d668be539a4abf22b4bdb0d5a6d6f 	{ ae_trunca32q48	a13, aed22; ae_movda32	aed28, a10; ae_mulaf32r.hh	aed27, aed10, aed26; ae_mulf32r.hh	aed25, aed21, aed7 }
     98b:	0e9ce69efd2123d0c4008bdb0fdc6e7f 	{ ae_trunca32q48	a14, aed23; ae_l32.xp	aed31, a2, a3; ae_mulaf32r.hh	aed27, aed30, aed28; nop }
     99b:	1305c24708ee1dee                  	{ ae_movda32	aed29, a13; ae_l32.xp	aed14, a2, a3 }
     9a3:	0e9d668b1d21e4d0c4008bdb0ffd6f9f 	{ ae_trunca32q48	a15, aed25; ae_movda32	aed3, a14; ae_mulaf32r.hh	aed27, aed31, aed29; nop }
     9b3:	0e9d668bf981f4d0c5808bdb1dc38c2f 	{ ae_l32.i	aed30, a2, 0; ae_movda32	aed31, a15; ae_mulaf32r.hh	aed27, aed14, aed3; nop }
     9c3:	1e8346db37dfb13e                  	{ nop; nop; ae_mulaf32r.hh	aed27, aed30, aed31 }
     9cb:	0ead257b0d250cd0c400813b0c0162bf 	{ ae_trunca32q48	a2, aed27; nop; nop; nop }
     9db:	f01d                                    	retw.n

000009dd <IN_LINE_32b_5ord+0x1fd>:
     9dd:	000000                                        ...

000009e0 <skew_ctrl_16b>:
     9e0:	008136                                    	entry	a1, 64
     9e3:	0f0323411080015e                  	{ s32i	a5, a1, 24; l16si	a14, a3, 0 }
     9eb:	3206b241088a22ee                  	{ s16i	a14, a2, 74; or	a11, a2, a2 }
     9f3:	150323410880116e                  	{ s32i	a6, a1, 20; l16si	a5, a3, 2 }
     9fb:	265252                                    	s16i	a5, a2, 76
     9fe:	060409401089035e                  	{ l16si	a5, a3, 4; movi	a6, 0x249 }
     a06:	2a06a6411885325e                  	{ s16i	a5, a2, 78; slli	a6, a6, 5 }
     a0e:	060409401889135e                  	{ l16si	a5, a3, 6; movi	a7, 0x249 }
     a16:	240332420081325e                  	{ s16i	a5, a2, 80; l16ui	a5, a2, 12 }
     a1e:	0a068741008603fe                  	{ l16si	a15, a3, 8; slli	a8, a7, 6 }
     a26:	0202a242089022fe                  	{ s16i	a15, a2, 82; addi	a2, a2, 64 }
     a2e:	27030b210c12b5ce                  	{ blti.w15	a5, 5, a68 <skew_ctrl_16b+0x88>; l16si	a7, a11, 148 }
     a36:	090425221800f58e                  	{ bgeui.w15	a5, 16, a98 <skew_ctrl_16b+0xb8>; movi	a9, 5 }
     a3e:	070400031001e65e                  	{ bbs.w15	a6, a5, a60 <skew_ctrl_16b+0x80>; movi	a6, 64 }
     a46:	2f5857                                    	bbc	a8, a5, a79 <skew_ctrl_16b+0x99>
     a49:	060444400080065e                  	{ movi	a5, 6; movi	a6, 4 }
     a51:	2706a9c508fdb97e                  	{ movgez	a5, a9, a7; movgez	a6, a9, a7 }
     a59:	0018c6                                    	j	ac0 <skew_ctrl_16b+0xe0>

00000a5c <skew_ctrl_16b+0x7c>:
     a5c:	00000000                                ....

00000a60 <skew_ctrl_16b+0x80>:
     a60:	24030b4000721a4e                  	{ j	aae <skew_ctrl_16b+0xce>; l16si	a5, a11, 148 }

00000a68 <skew_ctrl_16b+0x88>:
     a68:	060408201000a50e                  	{ beqi.w15	a5, 2, a80 <skew_ctrl_16b+0xa0>; movi	a6, 8 }
     a70:	163526                                    	beqi	a5, 3, a8a <skew_ctrl_16b+0xaa>
     a73:	2f4526                                    	beqi	a5, 4, aa6 <skew_ctrl_16b+0xc6>
     a76:	000786                                    	j	a98 <skew_ctrl_16b+0xb8>

00000a79 <skew_ctrl_16b+0x99>:
     a79:	1b7566                                    	bnei	a5, 7, a98 <skew_ctrl_16b+0xb8>
     a7c:	000986                                    	j	aa6 <skew_ctrl_16b+0xc6>

00000a7f <skew_ctrl_16b+0x9f>:
	...

00000a80 <skew_ctrl_16b+0xa0>:
     a80:	550c                                    	movi.n	a5, 5
     a82:	0604054000600a3e                  	{ j	ac0 <skew_ctrl_16b+0xe0>; movi	a6, 5 }

00000a8a <skew_ctrl_16b+0xaa>:
     a8a:	650c                                    	movi.n	a5, 6
     a8c:	060404400060003e                  	{ j	ac0 <skew_ctrl_16b+0xe0>; movi	a6, 4 }

00000a94 <skew_ctrl_16b+0xb4>:
     a94:	00000000                                ....

00000a98 <skew_ctrl_16b+0xb8>:
     a98:	060460451468f55e                  	{ addi	a5, a5, -10; movi	a7, 0x200 }
     aa0:	00a162                                    	movi	a6, 0x100
     aa3:	936750                                    	movnez	a6, a7, a5
     aa6:	4a9b52                                    	l16si	a5, a11, 148
     aa9:	f03d                                    	nop.n
     aab:	f25560                                    	rems	a5, a5, a6
     aae:	060c                                    	movi.n	a6, 0
     ab0:	0f04264510e0f65e                  	{ salt	a6, a6, a5; movi	a15, 6 }
     ab8:	070286410ce14f6e                  	{ sub	a5, a15, a6; addi	a6, a6, 4 }
     ac0:	3a046fc5141fe9de                  	{ l32r	a13, 64 (1138 <intp_filter_16b>); movi	a10, -1 }
     ac8:	0e02834414e2352e                  	{ addx2	a2, a5, a2; addi	a15, a3, 8 }
     ad0:	050420411880113e                  	{ s32i	a3, a1, 28; movi	a5, 0 }
     ad8:	2b06b2c1009601fe                  	{ s32i	a15, a1, 16; srli	a10, a10, 2 }
     ae0:	08038b410c811b8e                  	{ l32i	a8, a11, 20; l32i	a9, a11, 16 }
     ae8:	0c04214518e0ebbe                  	{ or	a7, a11, a11; movi	a12, 1 }
     af0:	0d3d                                    	mov.n	a3, a13
     af2:	898a                                    	add.n	a8, a9, a8
     af4:	0b06e8c608660e8e                  	{ extui	a9, a8, 30, 2; and	a8, a8, a10 }
     afc:	1306a94100960b8e                  	{ s32i	a8, a11, 16; addx2	a9, a9, a2 }
     b04:	2b06bb4410ead95e                  	{ movnez	a2, a9, a5; or	a10, a11, a11 }
     b0c:	1402a54618e0f22e                  	{ or	a11, a2, a2; addi	a5, a5, 1 }
     b14:	000de0                                    	callx8	a13
     b17:	3e042fc0009f14ae                  	{ s16i	a10, a4, 0; movi	a15, -1 }
     b1f:	20b770                                    	or	a11, a7, a7
     b22:	1306b34214ab52fe                  	{ srli	a10, a15, 2; or	a13, a3, a3 }
     b2a:	2502c41a1fe0865e                  	{ bne.w15	a6, a5, ae0 <skew_ctrl_16b+0x100>; addi	a4, a4, 2 }
     b32:	052122                                    	l32i	a2, a1, 20
     b35:	042152                                    	l32i	a5, a1, 16
     b38:	2606b2050818262e                  	{ bgeu.w15	a6, a2, b81 <skew_ctrl_16b+0x1a1>; sub	a2, a2, a6 }
     b40:	06038b410c810b8e                  	{ l32i	a8, a11, 20; l32i	a6, a11, 16 }
     b48:	1c0c                                    	movi.n	a12, 1
     b4a:	220b                                    	addi.n	a2, a2, -1
     b4c:	868a                                    	add.n	a8, a6, a8
     b4e:	0b06e8c510660e8e                  	{ extui	a6, a8, 30, 2; and	a8, a8, a10 }
     b56:	1506a64100951b8e                  	{ s32i	a8, a11, 16; addx2	a5, a6, a5 }
     b5e:	3406b54610eafbbe                  	{ or	a10, a11, a11; or	a11, a5, a5 }
     b66:	0003e0                                    	callx8	a3
     b69:	3e042fc0009f14ae                  	{ s16i	a10, a4, 0; movi	a15, -1 }
     b71:	2e06b2c618f6f77e                  	{ or	a11, a7, a7; srli	a10, a15, 2 }
     b79:	2502e43d03e002ee                  	{ bnez.w15	a2, b40 <skew_ctrl_16b+0x160>; addi	a4, a4, 2 }
     b81:	7128                                    	l32i.n	a2, a1, 28
     b83:	6148                                    	l32i.n	a4, a1, 24
     b85:	902420                                    	addx2	a2, a4, a2
     b88:	f6c222                                    	addi	a2, a2, -10
     b8b:	22fd                                    	ae_l16si.n	a15, a2, 0
     b8d:	205bf2                                    	s16i	a15, a11, 64
     b90:	324d                                    	ae_l16si.n	a4, a2, 2
     b92:	215b42                                    	s16i	a4, a11, 66
     b95:	029242                                    	l16si	a4, a2, 4
     b98:	225b42                                    	s16i	a4, a11, 68
     b9b:	039242                                    	l16si	a4, a2, 6
     b9e:	235b42                                    	s16i	a4, a11, 70
     ba1:	049222                                    	l16si	a2, a2, 8
     ba4:	245b22                                    	s16i	a2, a11, 72
     ba7:	f01d                                    	retw.n

00000ba9 <skew_ctrl_16b+0x1c9>:
     ba9:	000000                                        ...

00000bac <Check_Phase_Frac>:
     bac:	004136                                    	entry	a1, 32
     baf:	170282a0081fe20e                  	{ beqi.w15	a2, 1, bd1 <Check_Phase_Frac+0x25>; addi	a6, a2, -3 }
     bb7:	050440400084007e                  	{ movi	a7, 0x200; movi	a4, 0x100 }
     bbf:	054c                                    	movi.n	a5, 64
     bc1:	2f02a2c500ffd76e                  	{ movnez	a4, a7, a6; addi	a15, a2, -2 }
     bc9:	0e0685c00079160e                  	{ j	bd3 <Check_Phase_Frac+0x27>; moveqz	a4, a5, a15 }

00000bd1 <Check_Phase_Frac+0x25>:
     bd1:	840c                                    	movi.n	a4, 8
     bd3:	235d                                    	ae_l16si.n	a5, a3, 0
     bd5:	020c                                    	movi.n	a2, 0
     bd7:	640b                                    	addi.n	a6, a4, -1
     bd9:	150700831801356e                  	{ bnone.w15	a5, a6, be3 <Check_Phase_Frac+0x37>; abs	a5, a5 }
     be1:	f01d                                    	retw.n

00000be3 <Check_Phase_Frac+0x37>:
     be3:	855d                                    	ae_sext16	a5, a5
     be5:	060460181fe0c54e                  	{ blt.w15	a5, a4, be1 <Check_Phase_Frac+0x35>; movi	a6, 0 }
     bed:	020421400080036e                  	{ s16i	a6, a3, 0; movi	a2, 1 }
     bf5:	f01d                                    	retw.n

00000bf7 <Check_Phase_Frac+0x4b>:
	...

00000bf8 <skew_ctrl_process>:
     bf8:	004136                                    	entry	a1, 32
     bfb:	0e04284718e0e22e                  	{ or	a15, a2, a2; movi	a14, 8 }
     c03:	3206b341108a127e                  	{ s16i	a7, a2, 12; or	a11, a3, a3 }
     c0b:	02040e041c000e7e                  	{ blt.w15	a14, a7, c30 <skew_ctrl_process+0x38>; movi	a2, 14 }
     c13:	3957a6                                    	blti	a7, 5, c50 <skew_ctrl_process+0x58>
     c16:	665726                                    	beqi	a7, 5, c80 <skew_ctrl_process+0x88>
     c19:	716726                                    	beqi	a7, 6, c8e <skew_ctrl_process+0x96>
     c1c:	230760a20065376e                  	{ bnei.w15	a7, 8, fe6 <skew_ctrl_process+0x3ee>; nop }
     c24:	02038f40006202ae                  	{ j	cca <skew_ctrl_process+0xd2>; l32i	a2, a15, 32 }

00000c2c <skew_ctrl_process+0x34>:
     c2c:	00000000                                ....

00000c30 <skew_ctrl_process+0x38>:
     c30:	03040f26140077ce                  	{ blti.w15	a7, 12, c63 <skew_ctrl_process+0x6b>; movi	a3, 15 }
     c38:	62a726                                    	beqi	a7, 12, c9e <skew_ctrl_process+0xa6>
     c3b:	6f1727                                    	beq	a7, a2, cae <skew_ctrl_process+0xb6>
     c3e:	220740821c65373e                  	{ bne.w15	a7, a3, fe6 <skew_ctrl_process+0x3ee>; nop }
     c46:	32038f400063066e                  	{ j	cb0 <skew_ctrl_process+0xb8>; l32i	a2, a15, 60 }

00000c4e <skew_ctrl_process+0x56>:
	...

00000c50 <skew_ctrl_process+0x58>:
     c50:	6a2726                                    	beqi	a7, 2, cbe <skew_ctrl_process+0xc6>
     c53:	230740a41865776e                  	{ bnei.w15	a7, 3, fe6 <skew_ctrl_process+0x3ee>; nop }
     c5b:	32038f400062016e                  	{ j	cc0 <skew_ctrl_process+0xc8>; l32i	a2, a15, 44 }

00000c63 <skew_ctrl_process+0x6b>:
     c63:	920c                                    	movi.n	a2, 9
     c65:	03042b040800f72e                  	{ beq.w15	a7, a2, cc8 <skew_ctrl_process+0xd0>; movi	a3, 11 }
     c6d:	220720861c65b73e                  	{ bne.w15	a7, a3, fe6 <skew_ctrl_process+0x3ee>; nop }
     c75:	12038f400062072e                  	{ j	ca0 <skew_ctrl_process+0xa8>; l32i	a2, a15, 36 }

00000c7d <skew_ctrl_process+0x85>:
     c7d:	000000                                        ...

00000c80 <skew_ctrl_process+0x88>:
     c80:	32038f4000610c0e                  	{ j	c90 <skew_ctrl_process+0x98>; l32i	a2, a15, 28 }

00000c88 <skew_ctrl_process+0x90>:
     c88:	00000000                                 ......

00000c8e <skew_ctrl_process+0x96>:
     c8e:	cf28                                    	l32i.n	a2, a15, 48
     c90:	080401400060183e                  	{ j	ccc <skew_ctrl_process+0xd4>; movi	a9, 1 }

00000c98 <skew_ctrl_process+0xa0>:
     c98:	00000000                                 ......

00000c9e <skew_ctrl_process+0xa6>:
     c9e:	ef28                                    	l32i.n	a2, a15, 56
     ca0:	080403400060182e                  	{ j	ccc <skew_ctrl_process+0xd4>; movi	a9, 3 }

00000ca8 <skew_ctrl_process+0xb0>:
     ca8:	00000000                                 ......

00000cae <skew_ctrl_process+0xb6>:
     cae:	af28                                    	l32i.n	a2, a15, 40
     cb0:	080404400060181e                  	{ j	ccc <skew_ctrl_process+0xd4>; movi	a9, 4 }

00000cb8 <skew_ctrl_process+0xc0>:
     cb8:	00000000                                 ......

00000cbe <skew_ctrl_process+0xc6>:
     cbe:	6f28                                    	l32i.n	a2, a15, 24
     cc0:	080400400060180e                  	{ j	ccc <skew_ctrl_process+0xd4>; movi	a9, 0 }

00000cc8 <skew_ctrl_process+0xd0>:
     cc8:	df28                                    	l32i.n	a2, a15, 52
     cca:	290c                                    	movi.n	a9, 2
     ccc:	5f29                                    	s32i.n	a2, a15, 20
     cce:	12033f4200804f9e                  	{ s16i	a9, a15, 144; l16ui	a2, a15, 2 }
     cd6:	3b031f4000810f3e                  	{ l16ui	a3, a15, 0; l16ui	a10, a15, 14 }
     cde:	08044440008002ce                  	{ movi	a12, 2; movi	a8, 4 }
     ce6:	1d1c                                    	movi.n	a13, 17
     ce8:	1e037f400472e33e                  	{ addi	a3, a3, -32; l16ui	a14, a15, 146 }
     cf0:	0304214600e0dc3e                  	{ movnez	a8, a12, a3; movi	a3, 1 }
     cf8:	23069202180aaade                  	{ bne.w15	a10, a13, d12 <skew_ctrl_process+0x11a>; or	a10, a2, a2 }
     d00:	020461201420776e                  	{ bnei.w15	a7, 2, ef0 <skew_ctrl_process+0x2f8>; movi	a3, 1 }
     d08:	a82a                                    	add.n	a10, a8, a2
     d0a:	020401400060181e                  	{ j	d26 <skew_ctrl_process+0x12e>; movi	a3, 1 }

00000d12 <skew_ctrl_process+0x11a>:
     d12:	0d0409201449676e                  	{ bnei.w15	a7, 2, f44 <skew_ctrl_process+0x34c>; movi	a12, 0x249 }
     d1a:	230692411cea428e                  	{ sub	a7, a2, a8; or	a10, a2, a2 }
     d22:	130c                                    	movi.n	a3, 1
     d24:	072d                                    	mov.n	a2, a7
     d26:	3c042ec0009f142e                  	{ s16i	a2, a4, 0; movi	a13, -2 }
     d2e:	0c0230c0009916ae                  	{ s16i	a10, a6, 0; extui	a4, a13, 0, 16 }
     d36:	0f04014000800f7e                  	{ l16ui	a7, a15, 0; movi	a14, 1 }
     d3e:	05068ac108860fce                  	{ l16ui	a12, a15, 10; and	a8, a10, a4 }
     d46:	0906b1c410b6424e                  	{ and	a2, a2, a4; srli	a8, a8, 1 }
     d4e:	2206f1c00474e79e                  	{ addi	a9, a7, -32; srli	a2, a2, 1 }
     d56:	38028cc114ff59ee                  	{ saltu	a6, a9, a14; addi	a9, a12, -1 }
     d5e:	400600                                    	ssr	a6
     d61:	290744c70ca3612e                  	{ srl	a13, a2; srl	a14, a8 }
     d69:	d38c                                    	beqz.n	a3, d7a <skew_ctrl_process+0x182>
     d6b:	230740a30005b76e                  	{ bnei.w15	a7, 32, e05 <skew_ctrl_process+0x20d>; nop }
     d73:	fcbd21                                    	l32r	a2, 68 (560 <skew_ctrl_32b>)
     d76:	002386                                    	j	e08 <skew_ctrl_process+0x210>

00000d79 <skew_ctrl_process+0x181>:
	...

00000d7a <skew_ctrl_process+0x182>:
     d7a:	240c                                    	movi.n	a4, 2
     d7c:	2c06bc4500e8ac9e                  	{ moveqz	a4, a12, a9; or	a2, a12, a12 }
     d84:	834cc0                                    	moveqz	a4, a12, a12
     d87:	2506be4418b904fe                  	{ slli	a3, a4, 1; sub	a6, a14, a4 }
     d8f:	0602cf230410976e                  	{ bnei.w15	a7, 32, e43 <skew_ctrl_process+0x24b>; addi	a7, a15, 64 }
     d97:	2e0684240004e2ae                  	{ beqz.w15	a2, db8 <skew_ctrl_process+0x1c0>; slli	a2, a4, 2 }
     d9f:	c02f20                                    	sub	a2, a15, a2
     da2:	54c232                                    	addi	a3, a2, 84
     da5:	0f8476                                    	loop	a4, db8 <skew_ctrl_process+0x1c0>
     da8:	020283400481132e                  	{ l32i	a2, a3, 0; addi	a3, a3, 4 }
     db0:	0502a5400081152e                  	{ s32i	a2, a5, 0; addi	a5, a5, 4 }
     db8:	112600                                    	slli	a2, a6, 16
     dbb:	3b069b240808d2ce                  	{ blti.w15	a2, 1, dda <skew_ctrl_process+0x1e2>; or	a3, a11, a11 }
     dc3:	862d                                    	ae_sext16	a2, a6
     dc5:	0f8276                                    	loop	a2, dd8 <skew_ctrl_process+0x1e0>
     dc8:	020283400481132e                  	{ l32i	a2, a3, 0; addi	a3, a3, 4 }
     dd0:	0502a5400081152e                  	{ s32i	a2, a5, 0; addi	a5, a5, 4 }

00000dd8 <skew_ctrl_process+0x1e0>:
     dd8:	03bd                                    	mov.n	a11, a3

00000dda <skew_ctrl_process+0x1e2>:
     dda:	a024b0                                    	addx4	a2, a4, a11
     ddd:	ecc222                                    	addi	a2, a2, -20
     de0:	0238                                    	l32i.n	a3, a2, 0
     de2:	106f32                                    	s32i	a3, a15, 64
     de5:	1238                                    	l32i.n	a3, a2, 4
     de7:	116f32                                    	s32i	a3, a15, 68
     dea:	2238                                    	l32i.n	a3, a2, 8
     dec:	126f32                                    	s32i	a3, a15, 72
     def:	3238                                    	l32i.n	a3, a2, 12
     df1:	136f32                                    	s32i	a3, a15, 76
     df4:	4228                                    	l32i.n	a2, a2, 16
     df6:	03033f4500920f2e                  	{ s32i	a2, a15, 80; l16ui	a2, a15, 144 }
     dfe:	1e1266                                    	bnei	a2, 1, e20 <skew_ctrl_process+0x228>
     e01:	002b06                                    	j	eb1 <skew_ctrl_process+0x2b9>

00000e04 <skew_ctrl_process+0x20c>:
	...

00000e05 <skew_ctrl_process+0x20d>:
     e05:	fc9921                                    	l32r	a2, 6c (9e0 <skew_ctrl_16b>)
     e08:	0406b54610ebfffe                  	{ or	a10, a15, a15; or	a12, a5, a5 }
     e10:	0f3d                                    	mov.n	a3, a15
     e12:	0002e0                                    	callx8	a2
     e15:	481322                                    	l16ui	a2, a3, 144
     e18:	3206d324080bb20e                  	{ beqi.w15	a2, 1, eb1 <skew_ctrl_process+0x2b9>; or	a15, a3, a3 }
     e20:	120282a0101f726e                  	{ bnei.w15	a2, 2, e30 <skew_ctrl_process+0x238>; addi	a3, a2, -3 }
     e28:	020400400061078e                  	{ j	eb3 <skew_ctrl_process+0x2bb>; movi	a2, 64 }

00000e30 <skew_ctrl_process+0x238>:
     e30:	030440400084004e                  	{ movi	a4, 0x200; movi	a2, 0x100 }
     e38:	220694400064177e                  	{ j	eb3 <skew_ctrl_process+0x2bb>; movnez	a2, a4, a3 }

00000e40 <skew_ctrl_process+0x248>:
     e40:	000000                                        ...

00000e43 <skew_ctrl_process+0x24b>:
     e43:	220697240018d2ae                  	{ beqz.w15	a2, e60 <skew_ctrl_process+0x268>; sub	a2, a7, a3 }
     e4b:	32ab                                    	addi.n	a3, a2, 10
     e4d:	0f8476                                    	loop	a4, e60 <skew_ctrl_process+0x268>
     e50:	220283400080132e                  	{ l16si	a2, a3, 0; addi	a3, a3, 2 }
     e58:	2402a5400080152e                  	{ s16i	a2, a5, 0; addi	a5, a5, 2 }

00000e60 <skew_ctrl_process+0x268>:
     e60:	112600                                    	slli	a2, a6, 16
     e63:	3b069b240808d2ce                  	{ blti.w15	a2, 1, e82 <skew_ctrl_process+0x28a>; or	a3, a11, a11 }
     e6b:	862d                                    	ae_sext16	a2, a6
     e6d:	0f8276                                    	loop	a2, e80 <skew_ctrl_process+0x288>
     e70:	220283400080132e                  	{ l16si	a2, a3, 0; addi	a3, a3, 2 }
     e78:	2402a5400080152e                  	{ s16i	a2, a5, 0; addi	a5, a5, 2 }

00000e80 <skew_ctrl_process+0x288>:
     e80:	03bd                                    	mov.n	a11, a3

00000e82 <skew_ctrl_process+0x28a>:
     e82:	9024b0                                    	addx2	a2, a4, a11
     e85:	f6c222                                    	addi	a2, a2, -10
     e88:	223d                                    	ae_l16si.n	a3, a2, 0
     e8a:	205f32                                    	s16i	a3, a15, 64
     e8d:	323d                                    	ae_l16si.n	a3, a2, 2
     e8f:	215f32                                    	s16i	a3, a15, 66
     e92:	029232                                    	l16si	a3, a2, 4
     e95:	225f32                                    	s16i	a3, a15, 68
     e98:	039232                                    	l16si	a3, a2, 6
     e9b:	235f32                                    	s16i	a3, a15, 70
     e9e:	049222                                    	l16si	a2, a2, 8
     ea1:	02033f4100922f2e                  	{ s16i	a2, a15, 72; l16ui	a2, a15, 144 }
     ea9:	230720bc0fe5926e                  	{ bnei.w15	a2, 1, e20 <skew_ctrl_process+0x228>; nop }
     eb1:	820c                                    	movi.n	a2, 8
     eb3:	340282c2089f4f3e                  	{ l16si	a3, a15, 146; addi	a4, a2, -1 }
     ebb:	4a9f52                                    	l16si	a5, a15, 148
     ebe:	060c                                    	movi.n	a6, 0
     ec0:	060400071800934e                  	{ bnone.w15	a3, a4, ed5 <skew_ctrl_process+0x2dd>; movi	a7, 0 }
     ec8:	604150                                    	abs	a4, a5
     ecb:	844d                                    	ae_sext16	a4, a4
     ecd:	722420                                    	salt	a2, a4, a2
     ed0:	932520                                    	movnez	a2, a5, a2
     ed3:	f01d                                    	retw.n

00000ed5 <skew_ctrl_process+0x2dd>:
     ed5:	864d                                    	ae_sext16	a4, a6
     ed7:	2306b441008c0f7e                  	{ s32i	a7, a15, 16; salt	a2, a4, a2 }
     edf:	2206b74210844f7e                  	{ s16i	a7, a15, 148; movnez	a2, a7, a2 }
     ee7:	f01d                                    	retw.n

00000ee9 <skew_ctrl_process+0x2f1>:
     ee9:	00000000                                 .......

00000ef0 <skew_ctrl_process+0x2f8>:
     ef0:	2306923a1fca878e                  	{ bgeui.w15	a7, 16, d26 <skew_ctrl_process+0x12e>; or	a10, a2, a2 }
     ef8:	0d044941008909ae                  	{ movi	a10, 0x249; movi	a12, 0x249 }
     f00:	1b06ac4610a70abe                  	{ slli	a10, a10, 5; slli	a13, a12, 6 }
     f08:	2206d203100a2a7e                  	{ bbs.w15	a10, a7, f90 <skew_ctrl_process+0x398>; or	a10, a2, a2 }
     f10:	1e037f021412ed7e                  	{ bbc.w15	a13, a7, 1010 <skew_ctrl_process+0x418>; l16ui	a14, a15, 146 }
     f18:	ae0b                                    	addi.n	a10, a14, -1
     f1a:	8acd                                    	ae_sext16	a12, a10
     f1c:	3e06be4208894fae                  	{ s16i	a10, a15, 146; or	a7, a14, a14 }
     f24:	1a02a9c2109f4fae                  	{ s16i	a10, a15, 148; addi	a10, a9, -3 }
     f2c:	0c0400200828190e                  	{ beqi.w15	a9, 1, 1030 <skew_ctrl_process+0x438>; movi	a13, 0x200 }
     f34:	080400241024396e                  	{ bnei.w15	a9, 2, 103d <skew_ctrl_process+0x445>; movi	a9, 0x100 }
     f3c:	080400400861100e                  	{ j	1040 <skew_ctrl_process+0x448>; movi	a9, 64 }

00000f44 <skew_ctrl_process+0x34c>:
     f44:	1b06ec3a1ba7f78e                  	{ bgeui.w15	a7, 16, d26 <skew_ctrl_process+0x12e>; slli	a13, a12, 5 }
     f4c:	0b04290314094d7e                  	{ bbs.w15	a13, a7, fba <skew_ctrl_process+0x3c2>; movi	a10, 0x249 }
     f54:	2b068a4208864fde                  	{ l16ui	a13, a15, 146; slli	a10, a10, 6 }
     f5c:	2206f202100a0a7e                  	{ bbc.w15	a10, a7, 1020 <skew_ctrl_process+0x428>; or	a10, a2, a2 }
     f64:	ad0b                                    	addi.n	a10, a13, -1
     f66:	8acd                                    	ae_sext16	a12, a10
     f68:	3c06bd4208895fae                  	{ s16i	a10, a15, 146; or	a7, a13, a13 }
     f70:	1a02a9c2109f4fae                  	{ s16i	a10, a15, 148; addi	a10, a9, -3 }
     f78:	0c0460200c08b90e                  	{ beqi.w15	a9, 1, 1070 <skew_ctrl_process+0x478>; movi	a13, 0x200 }
     f80:	080460241404d96e                  	{ bnei.w15	a9, 2, 107d <skew_ctrl_process+0x485>; movi	a9, 0x100 }
     f88:	08040040006114fe                  	{ j	1080 <skew_ctrl_process+0x488>; movi	a9, 64 }

00000f90 <skew_ctrl_process+0x398>:
     f90:	491f72                                    	l16ui	a7, a15, 146
     f93:	a71b                                    	addi.n	a10, a7, 1
     f95:	8acd                                    	ae_sext16	a12, a10
     f97:	495fa2                                    	s16i	a10, a15, 146
     f9a:	1a02a9c2109f4fae                  	{ s16i	a10, a15, 148; addi	a10, a9, -3 }
     fa2:	0d0400200828590e                  	{ beqi.w15	a9, 1, 10b0 <skew_ctrl_process+0x4b8>; movi	a13, 0x200 }
     faa:	090400241024796e                  	{ bnei.w15	a9, 2, 10bd <skew_ctrl_process+0x4c5>; movi	a9, 0x100 }
     fb2:	0804004008611a0e                  	{ j	10c0 <skew_ctrl_process+0x4c8>; movi	a9, 64 }

00000fba <skew_ctrl_process+0x3c2>:
     fba:	ae1b                                    	addi.n	a10, a14, 1
     fbc:	8acd                                    	ae_sext16	a12, a10
     fbe:	3e06be4208894fae                  	{ s16i	a10, a15, 146; or	a7, a14, a14 }
     fc6:	1a02a9c2109f4fae                  	{ s16i	a10, a15, 148; addi	a10, a9, -3 }
     fce:	0d0400200828f90e                  	{ beqi.w15	a9, 1, 10f0 <skew_ctrl_process+0x4f8>; movi	a13, 0x200 }
     fd6:	090400241424196e                  	{ bnei.w15	a9, 2, 10fd <skew_ctrl_process+0x505>; movi	a9, 0x100 }
     fde:	0804004008611e1e                  	{ j	1100 <skew_ctrl_process+0x508>; movi	a9, 64 }

00000fe6 <skew_ctrl_process+0x3ee>:
     fe6:	12035f400080018e                  	{ movi	a8, 1; l16ui	a2, a15, 2 }
     fee:	0206c8400082009e                  	{ movi	a9, 0; slli	a8, a8, 30 }
     ff6:	0204204200805f9e                  	{ s16i	a9, a15, 144; movi	a3, 0 }
     ffe:	5f89                                    	s32i.n	a8, a15, 20
    1000:	2306923817aa072e                  	{ bltui.w15	a7, 2, d26 <skew_ctrl_process+0x12e>; or	a10, a2, a2 }
    1008:	ff3286                                    	j	cd6 <skew_ctrl_process+0xde>

0000100b <skew_ctrl_process+0x413>:
    100b:	00000000                                 .....

00001010 <skew_ctrl_process+0x418>:
    1010:	230700b81ba5976e                  	{ bnei.w15	a7, 3, d26 <skew_ctrl_process+0x12e>; nop }
    1018:	c0a280                                    	sub	a10, a2, a8
    101b:	ff41c6                                    	j	d26 <skew_ctrl_process+0x12e>

0000101e <skew_ctrl_process+0x426>:
	...

00001020 <skew_ctrl_process+0x428>:
    1020:	330688381bb1076e                  	{ bnei.w15	a7, 3, d26 <skew_ctrl_process+0x12e>; add	a7, a8, a2 }
    1028:	23069247046ae8fe                  	{ j	d24 <skew_ctrl_process+0x12c>; or	a10, a2, a2 }

00001030 <skew_ctrl_process+0x438>:
    1030:	0804084000601c0e                  	{ j	1040 <skew_ctrl_process+0x448>; movi	a9, 8 }

00001038 <skew_ctrl_process+0x440>:
    1038:	00000000                                 .....

0000103d <skew_ctrl_process+0x445>:
    103d:	939da0                                    	movnez	a9, a13, a10
    1040:	2206f2471c6ae9de                  	{ addi	a13, a9, -1; or	a10, a2, a2 }
    1048:	0d0460191380dcde                  	{ bany.w15	a12, a13, d26 <skew_ctrl_process+0x12e>; movi	a13, 0 }
    1050:	130c                                    	movi.n	a3, 1
    1052:	c07370                                    	sub	a7, a3, a7
    1055:	a3c7c0                                    	movltz	a12, a7, a12
    1058:	8c7d                                    	ae_sext16	a7, a12
    105a:	2206f2181b8a479e                  	{ blt.w15	a7, a9, d26 <skew_ctrl_process+0x12e>; or	a10, a2, a2 }
    1062:	2806b242089a4fde                  	{ s16i	a13, a15, 146; sub	a10, a2, a8 }
    106a:	ff2e06                                    	j	d26 <skew_ctrl_process+0x12e>

0000106d <skew_ctrl_process+0x475>:
    106d:	000000                                        ...

00001070 <skew_ctrl_process+0x478>:
    1070:	0804084000601c0e                  	{ j	1080 <skew_ctrl_process+0x488>; movi	a9, 8 }

00001078 <skew_ctrl_process+0x480>:
    1078:	00000000                                 .....

0000107d <skew_ctrl_process+0x485>:
    107d:	939da0                                    	movnez	a9, a13, a10
    1080:	2206f2471c6ae9de                  	{ addi	a13, a9, -1; or	a10, a2, a2 }
    1088:	0306c8191392ccde                  	{ bany.w15	a12, a13, d26 <skew_ctrl_process+0x12e>; add	a8, a8, a2 }
    1090:	130c                                    	movi.n	a3, 1
    1092:	230692411cea437e                  	{ sub	a7, a3, a7; or	a10, a2, a2 }
    109a:	a3c7c0                                    	movltz	a12, a7, a12
    109d:	8c7d                                    	ae_sext16	a7, a12
    109f:	0704401c1b80179e                  	{ blt.w15	a7, a9, d26 <skew_ctrl_process+0x12e>; movi	a7, 0 }
    10a7:	001fc6                                    	j	112a <skew_ctrl_process+0x532>

000010aa <skew_ctrl_process+0x4b2>:
    10aa:	00000000                                 ......

000010b0 <skew_ctrl_process+0x4b8>:
    10b0:	0804084000601c0e                  	{ j	10c0 <skew_ctrl_process+0x4c8>; movi	a9, 8 }

000010b8 <skew_ctrl_process+0x4c0>:
    10b8:	00000000                                 .....

000010bd <skew_ctrl_process+0x4c5>:
    10bd:	939da0                                    	movnez	a9, a13, a10
    10c0:	2206f2471c6ae9de                  	{ addi	a13, a9, -1; or	a10, a2, a2 }
    10c8:	3f042f99139fccde                  	{ bany.w15	a12, a13, d26 <skew_ctrl_process+0x12e>; movi	a14, -1 }
    10d0:	230692411cea67ee                  	{ xor	a7, a7, a14; or	a10, a2, a2 }
    10d8:	a3c7c0                                    	movltz	a12, a7, a12
    10db:	8c7d                                    	ae_sext16	a7, a12
    10dd:	0c04201c1b80379e                  	{ blt.w15	a7, a9, d26 <skew_ctrl_process+0x12e>; movi	a13, 0 }
    10e5:	2206a84208924fde                  	{ s16i	a13, a15, 146; add	a10, a8, a2 }
    10ed:	ff0d46                                    	j	d26 <skew_ctrl_process+0x12e>

000010f0 <skew_ctrl_process+0x4f8>:
    10f0:	0804084000601c0e                  	{ j	1100 <skew_ctrl_process+0x508>; movi	a9, 8 }

000010f8 <skew_ctrl_process+0x500>:
    10f8:	00000000                                 .....

000010fd <skew_ctrl_process+0x505>:
    10fd:	939da0                                    	movnez	a9, a13, a10
    1100:	2206f2471c6ae9de                  	{ addi	a13, a9, -1; or	a10, a2, a2 }
    1108:	3f040f99139fccde                  	{ bany.w15	a12, a13, d26 <skew_ctrl_process+0x12e>; movi	a14, -1 }
    1110:	230692411cea67ee                  	{ xor	a7, a7, a14; or	a10, a2, a2 }
    1118:	0806b24700fac7ce                  	{ movltz	a12, a7, a12; sub	a8, a2, a8 }
    1120:	8c7d                                    	ae_sext16	a7, a12
    1122:	060400181b80179e                  	{ blt.w15	a7, a9, d26 <skew_ctrl_process+0x12e>; movi	a7, 0 }
    112a:	02ad                                    	mov.n	a10, a2
    112c:	2806b84208884f7e                  	{ s16i	a7, a15, 146; or	a2, a8, a8 }
    1134:	fefb86                                    	j	d26 <skew_ctrl_process+0x12e>

00001137 <skew_ctrl_process+0x53f>:
	...

00001138 <intp_filter_16b>:
    1138:	008136                                    	entry	a1, 64
    113b:	4268                                    	l32i.n	a6, a2, 16
    113d:	0904214510a006fe                  	{ slli	a6, a6, 1; movi	a8, 1 }
    1145:	070421411080016e                  	{ s32i	a6, a1, 24; movi	a6, 1 }
    114d:	0306a84000c2061e                  	{ ae_l32f24.i	aed0, a1, 24; slli	a8, a8, 30 }
    1155:	2306e64004c5c41e                  	{ ae_s32.l.i	aed0, a1, 16; slli	a6, a6, 14 }
    115d:	150511c10880022e                  	{ l16ui	a2, a2, 10; ae_l32.i	aed0, a1, 16 }
    1165:	210768470403c25e                  	{ l32r	a5, 70 (2aaaaaaa <_end+0x2aaa8e8a>); ae_cvt48a32	aed3, a8 }
    116d:	0e6cd51001f11d260400212714e3b7bf 	{ l32r	a11, 5c (55555555 <_end+0x55553935>); movi	a13, 1; ae_mov	aed7, aed3; ae_cvtq56p32s.l	aed24, aed0 }
    117d:	21072b240804820e                  	{ beqi.w15	a2, 1, 11d4 <intp_filter_16b+0x9c>; ae_cvt48a32	aed4, a11 }
    1185:	0d040d25002342ee                  	{ bnez.w15	a2, 1294 <intp_filter_16b+0x15c>; movi	a12, 205 }
    118d:	2f06a44410a314ee                  	{ slli	a2, a4, 2; slli	a14, a4, 17 }
    1195:	3f06804014ef432e                  	{ sub	a2, a3, a2; srai	a15, a14, 16 }
    119d:	220760c00c85bf2e                  	{ ae_l16m.xu	aed1, a2, a15; nop }
    11a5:	0e0452c010ff112e                  	{ ae_cvtq56p32s.l	aed2, aed1; ae_l16m.xu	aed31, a2, a15 }
    11ad:	268352133420b13e                  	{ nop; nop; ae_mulsf32s.ll	aed2, aed1, aed0 }
    11b5:	1e834b1337e0b13e                  	{ nop; nop; ae_mulaf32s.ll	aed2, aed31, aed0 }
    11bd:	220740c00485d22e                  	{ ae_round24x2f48ssym	aed0, aed2, aed2; nop }
    11c5:	220740c004c5161e                  	{ ae_s16m.l.i	aed0, a1, 12; nop }
    11cd:	069122                                    	l16si	a2, a1, 12
    11d0:	f01d                                    	retw.n

000011d2 <intp_filter_16b+0x9a>:
	...

000011d4 <intp_filter_16b+0x9c>:
    11d4:	0a0441400080112e                  	{ movi	a2, 1; movi	a11, 1 }
    11dc:	0504254410a0123e                  	{ slli	a2, a2, 13; movi	a5, 5 }
    11e4:	23068b400ca0e22e                  	{ ae_cvtp24a16x2.ll	aed1, a2, a2; slli	a2, a11, 30 }
    11ec:	0e9cc65001e11cd0c4808d090020420f 	{ ae_cvt48a32	aed4, a2; movi	a12, 1; ae_mulf32s.ll	aed1, aed1, aed0; nop }
    11fc:	1206854304a51c4e                  	{ ae_slaaq56	aed4, aed4, a12; slli	a5, a5, 13 }
    1204:	0ead6165752f4fa0c20c21380401d4df 	{ slli	a13, a4, 3; slli	a14, a4, 17; nop; ae_round24x2f48ssym	aed3, aed1, aed1 }
    1214:	22832a192c6023de                  	{ sub	a2, a3, a13; nop; ae_mulf32s.ll	aed3, aed3, aed0 }
    121c:	3f06804014afe55e                  	{ ae_cvtp24a16x2.ll	aed2, a5, a5; srai	a15, a14, 16 }
    1224:	0ea4c035c8ea2f60c58463081885f04f 	{ ae_sub64	aed4, aed4, aed3; ae_l16m.xu	aed25, a2, a15; ae_sub64	aed5, aed3, aed1; ae_sub64	aed1, aed1, aed3 }
    1234:	0e9cdb35d8e22fd8c1802b2108400e5f 	{ ae_round24x2f48ssym	aed26, aed5, aed5; ae_l16m.xu	aed27, a2, a15; ae_mulaf32s.ll	aed4, aed2, aed0; ae_add64	aed0, aed24, aed1 }
    1244:	0e9cc255e9e22fa0c3800d100b3a216f 	{ ae_neg64	aed1, aed1; ae_l16m.xu	aed29, a2, a15; ae_mulf32s.ll	aed2, aed25, aed26; ae_round24x2f48ssym	aed0, aed0, aed0 }
    1254:	0eacc575f0e22fa0c3f081380801011f 	{ ae_round24x2f48ssym	aed1, aed1, aed1; ae_l16m.xu	aed30, a2, a15; nop; ae_round24x2f48ssym	aed28, aed4, aed4 }
    1264:	1e834b13377cb13e                  	{ nop; nop; ae_mulaf32s.ll	aed2, aed27, aed28 }
    126c:	1e834b1337a0b13e                  	{ nop; nop; ae_mulaf32s.ll	aed2, aed29, aed0 }
    1274:	1e834b1337c1b13e                  	{ nop; nop; ae_mulaf32s.ll	aed2, aed30, aed1 }
    127c:	220740c71c85d22e                  	{ ae_round24x2f48ssym	aed31, aed2, aed2; nop }
    1284:	220740c71cc5161e                  	{ ae_s16m.l.i	aed31, a1, 12; nop }
    128c:	069122                                    	l16si	a2, a1, 12
    128f:	f01d                                    	retw.n

00001291 <intp_filter_16b+0x159>:
    1291:	000000                                        ...

00001294 <intp_filter_16b+0x15c>:
    1294:	070425410880115e                  	{ s32i	a5, a1, 20; movi	a7, 5 }
    129c:	3504914014a1f66e                  	{ ae_cvtp24a16x2.ll	aed2, a6, a6; ae_l32f24.i	aed1, a1, 20 }
    12a4:	1e82eb3c3002171e                  	{ ae_s32.l.i	aed1, a1, 28; nop; ae_mulaf32s.ll	aed7, aed0, aed2 }
    12ac:	0e9ce45c292617d0c58091181c02ad3f 	{ ae_slaaq56	aed6, aed3, a13; ae_l32.i	aed5, a1, 28; ae_mulsf32s.ll	aed3, aed0, aed2; nop }
    12bc:	0e5d5725b8fe732205982b211805167f 	{ ae_round24x2f48ssym	aed14, aed7, aed7; slli	a7, a7, 13; ae_mulaf32s.ll	aed4, aed0, aed5; ae_mov	aed8, aed6 }
    12cc:	0cdc62                                    	addmi	a6, a12, 0xc00
    12cf:	0e9d644571664fa0c39091301c056d3f 	{ ae_slaaq56	aed13, aed3, a13; slli	a14, a4, 17; ae_mulsf32s.ll	aed6, aed0, aed5; ae_round24x2f48ssym	aed4, aed4, aed4 }
    12df:	0e1d444698e243744fab7143080202df 	{ ae_round24x2f48ssym	aed2, aed13, aed13; addx2	a3, a4, a3; ae_mulsf32s.ll	aed8, aed0, aed2; ae_mulf32s.ll	aed7, aed14, aed4 }
    12ef:	0e1d4e46a0f00e0447ab6d891004126f 	{ ae_round24x2f48ssym	aed6, aed6, aed6; srai	a4, a14, 16; ae_mulf32s.ll	aed17, aed0, aed4; ae_mulf32s.ll	aed3, aed0, aed14 }
    12ff:	0ead4570a0f824a0c39ce13a0001178f 	{ ae_round24x2f48ssym	aed15, aed8, aed8; neg	a4, a4; nop; ae_round24x2f48ssym	aed7, aed7, aed7 }
    130f:	0e1ce255bd23343c552b6d4d04cf066f 	{ ae_cvtp24a16x2.ll	aed16, a6, a6; ae_l16m.xu	aed23, a3, a4; ae_mulf32s.ll	aed9, aed6, aed15; ae_mulf32s.ll	aed10, aed7, aed0 }
    131f:	0e1cc255c0f2347c4bab6d3018c2033f 	{ ae_round24x2f48ssym	aed3, aed3, aed3; ae_l16m.xu	aed24, a3, a4; ae_mulf32s.ll	aed6, aed6, aed2; ae_mulf32s.ll	aed5, aed15, aed2 }
    132f:	0eacc975c8f334a0c3cd413c0801001f 	{ ae_round24x2f48ssym	aed0, aed17, aed17; ae_l16m.xu	aed25, a3, a4; nop; ae_round24x2f48ssym	aed19, aed10, aed10 }
    133f:	0eacc575d8ea34a0c394a13a0801059f 	{ ae_round24x2f48ssym	aed9, aed9, aed9; ae_l16m.xu	aed27, a3, a4; nop; ae_round24x2f48ssym	aed5, aed5, aed5 }
    134f:	0e1cc255e8f2344c45ab6d2910b30a6f 	{ ae_round24x2f48ssym	aed18, aed6, aed6; ae_l16m.xu	aed29, a3, a4; ae_mulf32s.ll	aed5, aed5, aed19; ae_mulf32s.ll	aed2, aed9, aed2 }
    135f:	0e1cf255f53b3494492b6d350133c77f 	{ ae_cvtp24a16x2.ll	aed12, a7, a7; ae_l16m.xu	aed30, a3, a4; ae_mulf32s.ll	aed6, aed9, aed19; ae_mulf32s.ll	aed4, aed18, aed19 }
    136f:	0ead017b08f40ca0c38841390801185f 	{ ae_round24x2f48ssym	aed20, aed5, aed5; nop; nop; ae_round24x2f48ssym	aed2, aed2, aed2 }
    137f:	0e1d025b08e40c1441ab6d390047104f 	{ ae_round24x2f48ssym	aed4, aed4, aed4; nop; ae_mulf32s.ll	aed7, aed2, aed7; ae_mulf32s.ll	aed0, aed2, aed0 }
    138f:	0e1d225b09bc0c1445ab6d181e906d6f 	{ ae_slaaq56	aed21, aed6, a13; nop; ae_mulf32s.ll	aed3, aed20, aed16; ae_mulf32s.ll	aed2, aed2, aed3 }
    139f:	0e9d225b09a40ca0c3800d201c8cad7f 	{ ae_slaaq56	aed22, aed7, a13; nop; ae_mulf32s.ll	aed4, aed4, aed12; ae_round24x2f48ssym	aed0, aed0, aed0 }
    13af:	0ea5001b08fc0ca0c38c66a81165022f 	{ ae_round24x2f48ssym	aed2, aed2, aed2; nop; ae_add64	aed5, aed6, aed21; ae_round24x2f48ssym	aed3, aed3, aed3 }
    13bf:	0e1d025b08fc0cbc47ab6d01000c104f 	{ ae_round24x2f48ssym	aed4, aed4, aed4; nop; ae_mulf32s.ll	aed0, aed0, aed12; ae_mulf32s.ll	aed3, aed23, aed3 }
    13cf:	0e9d273b0d0c0ca0c3e8ab1d1704d37f 	{ ae_add64	aed7, aed7, aed22; nop; ae_mulaf32s.ll	aed3, aed24, aed4; ae_round24x2f48ssym	aed26, aed5, aed5 }
    13df:	0e1d133b08e40c1445ab6b191b3a1c7f 	{ ae_round24x2f48ssym	aed28, aed7, aed7; nop; ae_mulaf32s.ll	aed3, aed25, aed26; ae_mulf32s.ll	aed2, aed2, aed16 }
    13ef:	0e9d073b08e40cd0c5808b18037c000f 	{ ae_round24x2f48ssym	aed0, aed0, aed0; nop; ae_mulaf32s.ll	aed3, aed27, aed28; nop }
    13ff:	0e9d073b08e40cd0c5808b1813a0022f 	{ ae_round24x2f48ssym	aed2, aed2, aed2; nop; ae_mulaf32s.ll	aed3, aed29, aed0; nop }
    140f:	1e834b1b37c2b13e                  	{ nop; nop; ae_mulaf32s.ll	aed3, aed30, aed2 }
    1417:	220740c71c85d33e                  	{ ae_round24x2f48ssym	aed31, aed3, aed3; nop }
    141f:	220740c71cc5161e                  	{ ae_s16m.l.i	aed31, a1, 12; nop }
    1427:	069122                                    	l16si	a2, a1, 12
    142a:	f01d                                    	retw.n

0000142c <intp_filter_32b>:
    142c:	006136                                    	entry	a1, 48
    142f:	080401410480025e                  	{ l32i	a5, a2, 16; movi	a8, 1 }
    1437:	230688410886127e                  	{ l16ui	a7, a2, 10; slli	a10, a8, 13 }
    143f:	1f06a44510a505fe                  	{ slli	a6, a5, 1; slli	a5, a4, 2 }
    1447:	310726401082016e                  	{ s32i	a6, a1, 8; ae_movda32	aed2, a6 }
    144f:	3204b1240800c70e                  	{ beqi.w15	a7, 1, 14ac <intp_filter_32b+0x80>; ae_l32f24.i	aed0, a1, 8 }
    1457:	040693250039d7ee                  	{ bnez.w15	a7, 1574 <intp_filter_32b+0x148>; sub	a4, a3, a5 }
    145f:	220720c004c5754e                  	{ ae_l32.xp	aed0, a4, a5; nop }
    1467:	0eace57cfd2640d0c400813c0001192f 	{ ae_cvt64f32.h	aed1, aed0; ae_l32.i	aed31, a4, 0; nop; nop }
    1477:	0ead257b09040cd0c580813a0c01441f 	{ ae_srai64	aed1, aed1, 16; nop; nop; nop }
    1487:	26834d0b3402b13e                  	{ nop; nop; ae_mulsf32r.hh	aed1, aed0, aed2 }
    148f:	1e83460b37e2b13e                  	{ nop; nop; ae_mulaf32r.hh	aed1, aed31, aed2 }
    1497:	0ead257b0d240cd0c400813b0c01621f 	{ ae_trunca32q48	a2, aed1; nop; nop; nop }
    14a7:	f01d                                    	retw.n

000014a9 <intp_filter_32b+0x7d>:
    14a9:	000000                                        ...

000014ac <intp_filter_32b+0x80>:
    14ac:	0904614004c0d31e                  	{ ae_s32.l.i	aed0, a1, 12; movi	a9, 1 }
    14b4:	130511c51ca1faae                  	{ ae_cvtp24a16x2.ll	aed23, a10, a10; ae_l32.i	aed1, a1, 12 }
    14bc:	0e5cf6500530572604042d0006e1292f 	{ slli	a2, a9, 30; movi	a7, 5; ae_mulf32s.ll	aed0, aed23, aed1; ae_cvtq56p32s.l	aed24, aed1 }
    14cc:	20074240008401be                  	{ movi	a11, 1; ae_cvt48a32	aed4, a2 }
    14d4:	0ead6165b92673a0c38801381c012b4f 	{ ae_slaaq56	aed4, aed4, a11; slli	a7, a7, 13; nop; ae_round24x2f48ssym	aed2, aed0, aed0 }
    14e4:	22830a122441a4ce                  	{ slli	a10, a4, 4; nop; ae_mulf32s.ll	aed2, aed2, aed1 }
    14ec:	0b0694c01ca1e77e                  	{ ae_cvtp24a16x2.ll	aed3, a7, a7; subx4	a4, a4, a10 }
    14f4:	0ea5402618e23460c58042001085f04f 	{ ae_sub64	aed4, aed4, aed2; add	a3, a3, a4; ae_sub64	aed5, aed2, aed0; ae_sub64	aed0, aed0, aed2 }
    1504:	0e9cfb3e352035d8c0080b230c616c5f 	{ ae_trunca32q48	a12, aed5; ae_l32.xp	aed6, a3, a5; ae_mulaf32s.ll	aed4, aed3, aed1; ae_add64	aed2, aed24, aed0 }
    1514:	0eacc45ec9e135d0c480800107c0bc8f 	{ ae_movda32	aed27, a12; ae_l32.xp	aed25, a3, a5; ae_neg64	aed0, aed0; nop }
    1524:	000000d3088300fed6260611241bb3ef 	{ ae_trunca32q48	a14, aed4; ae_trunca32q48	a13, aed2; ae_mulf32r.hh	aed2, aed6, aed27 }
    1534:	1505c34708fa1eee                  	{ ae_movda32	aed29, a14; ae_l32.xp	aed26, a3, a5 }
    153c:	0e9d668bf520d4d0c4008b130f3d6f0f 	{ ae_trunca32q48	a15, aed0; ae_movda32	aed30, a13; ae_mulaf32r.hh	aed2, aed25, aed29; nop }
    154c:	0e9d668bf981f4d0c5808b131f5e0c3f 	{ ae_l32.i	aed28, a3, 0; ae_movda32	aed31, a15; ae_mulaf32r.hh	aed2, aed26, aed30; nop }
    155c:	1e834613379fb13e                  	{ nop; nop; ae_mulaf32r.hh	aed2, aed28, aed31 }
    1564:	0ead257b0d240cd0c400813b0c01622f 	{ ae_trunca32q48	a2, aed2; nop; nop; nop }
    1574:	f01d                                    	retw.n

00001576 <intp_filter_32b+0x14a>:
	...

00001578 <__do_global_ctors_aux>:
    1578:	004136                                    	entry	a1, 32
    157b:	fabe31                                    	l32r	a3, 74 (1b64 <__CTOR_END__>)
    157e:	fcc322                                    	addi	a2, a3, -4
    1581:	002222                                    	l32i	a2, a2, 0
    1584:	020283a4001e920e                  	{ beqi.w15	a2, -1, 1599 <__do_global_ctors_aux+0x21>; addi	a3, a3, -8 }

0000158c <__do_global_ctors_aux+0x14>:
    158c:	0002e0                                    	callx8	a2
    158f:	0328                                    	l32i.n	a2, a3, 0
    1591:	0302e3bc07ffb26e                  	{ bnei.w15	a2, -1, 158c <__do_global_ctors_aux+0x14>; addi	a3, a3, -4 }
    1599:	f01d                                    	retw.n

0000159b <__do_global_ctors_aux+0x23>:
    159b:	00000000                                 .....

000015a0 <memset>:
    15a0:	004136                                    	entry	a1, 32
    15a3:	025d                                    	mov.n	a5, a2
    15a5:	24069524000892ae                  	{ beqz.w15	a2, 15ba <memset+0x1a>; or	a2, a5, a5 }
    15ad:	948c                                    	beqz.n	a4, 15ba <memset+0x1a>
    15af:	078476                                    	loop	a4, 15ba <memset+0x1a>
    15b2:	1202a2400480023e                  	{ s8i	a3, a2, 0; addi	a2, a2, 1 }

000015ba <memset+0x1a>:
    15ba:	052d                                    	mov.n	a2, a5
    15bc:	f01d                                    	retw.n

000015be <memset+0x1e>:
	...

000015c0 <__udivdi3>:
    15c0:	004136                                    	entry	a1, 32
    15c3:	3206b24510e9e33e                  	{ or	a6, a3, a3; or	a7, a2, a2 }
    15cb:	03046020040055ae                  	{ beqz.w15	a5, 16b9 <__udivdi3+0xf9>; movi	a3, 0 }
    15d3:	020460030c00065e                  	{ bltu.w15	a6, a5, 16b7 <__udivdi3+0xf7>; movi	a2, 0 }
    15db:	40f5a0                                    	nsau	a10, a5
    15de:	2304202000402aae                  	{ beqz.w15	a10, 1828 <__udivdi3+0x268>; movi	a2, 32 }
    15e6:	3b06b24318fa6a5e                  	{ ssl	a10; sub	a11, a2, a10 }
    15ee:	33074bc308a435ce                  	{ sll	a13, a5; ssr	a11 }
    15f6:	23074ac714a4714e                  	{ srl	a14, a4; ssl	a10 }
    15fe:	000724c410e5eede                  	{ or	a2, a14, a13; sll	a4, a4 }
    1606:	030230c318fe6b6e                  	{ ssr	a11; extui	a8, a2, 16, 16 }
    160e:	23074ac50ca4716e                  	{ srl	a5, a6; ssl	a10 }
    1616:	33074bc318a436ce                  	{ sll	a15, a6; ssr	a11 }
    161e:	c2b580                                    	quou	a11, a5, a8
    1621:	82eb80                                    	mull	a14, a11, a8
    1624:	91d070                                    	srl	a13, a7
    1627:	1e06b54510f9edfe                  	{ or	a6, a13, a15; sub	a5, a5, a14 }
    162f:	929d                                    	ae_zext16	a9, a2
    1631:	82cb90                                    	mull	a12, a11, a9
    1634:	1106e5470865e06e                  	{ extui	a13, a6, 16, 16; slli	a5, a5, 16 }
    163c:	1a069b430ce935de                  	{ add	a13, a5, a13; or	a5, a11, a11 }
    1644:	23070a810804ddce                  	{ bgeu.w15	a13, a12, 1662 <__udivdi3+0xa2>; ssl	a10 }
    164c:	34028bc30cff3d2e                  	{ add	a13, a13, a2; addi	a5, a11, -1 }
    1654:	0a3d27                                    	bltu	a13, a2, 1662 <__udivdi3+0xa2>
    1657:	07bdc7                                    	bgeu	a13, a12, 1662 <__udivdi3+0xa2>
    165a:	1206ed471473eb5e                  	{ addi	a5, a11, -2; add	a13, a13, a2 }

00001662 <__udivdi3+0xa2>:
    1662:	c0bdc0                                    	sub	a11, a13, a12
    1665:	c2ab80                                    	quou	a10, a11, a8
    1668:	828a80                                    	mull	a8, a10, a8
    166b:	96fd                                    	ae_zext16	a15, a6
    166d:	c08b80                                    	sub	a8, a11, a8
    1670:	118800                                    	slli	a8, a8, 16
    1673:	826a90                                    	mull	a6, a10, a9
    1676:	1a069a4204ea28fe                  	{ add	a8, a8, a15; or	a9, a10, a10 }
    167e:	14b867                                    	bgeu	a8, a6, 1696 <__udivdi3+0xd6>
    1681:	882a                                    	add.n	a8, a8, a2
    1683:	39028a87081f782e                  	{ bltu.w15	a8, a2, 1696 <__udivdi3+0xd6>; addi	a9, a10, -1 }
    168b:	07b867                                    	bgeu	a8, a6, 1696 <__udivdi3+0xd6>
    168e:	28028ac204ff382e                  	{ add	a8, a8, a2; addi	a9, a10, -2 }
    1696:	2706b84410b9050e                  	{ slli	a2, a5, 16; sub	a6, a8, a6 }
    169e:	202920                                    	or	a2, a9, a2
    16a1:	a25240                                    	muluh	a5, a2, a4
    16a4:	824240                                    	mull	a4, a2, a4
    16a7:	310707830805465e                  	{ bltu.w15	a6, a5, 16b5 <__udivdi3+0xf5>; sll	a7, a7 }
    16af:	04b747                                    	bgeu	a7, a4, 16b7 <__udivdi3+0xf7>
    16b2:	019657                                    	bne	a6, a5, 16b7 <__udivdi3+0xf7>
    16b5:	220b                                    	addi.n	a2, a2, -1
    16b7:	f01d                                    	retw.n

000016b9 <__udivdi3+0xf9>:
    16b9:	40f430                                    	nsau	a3, a4
    16bc:	220440010c00664e                  	{ bgeu.w15	a6, a4, 176c <__udivdi3+0x1ac>; movi	a2, 32 }
    16c4:	23040024040023ae                  	{ beqz.w15	a3, 16ef <__udivdi3+0x12f>; movi	a2, 32 }
    16cc:	2306b24318f8735e                  	{ ssl	a3; sub	a2, a2, a3 }
    16d4:	110746c100a524ce                  	{ sll	a4, a4; sll	a5, a6 }
    16dc:	400200                                    	ssr	a2
    16df:	230743c414a4717e                  	{ srl	a2, a7; ssl	a3 }
    16e7:	300727c510e5e25e                  	{ or	a6, a2, a5; sll	a7, a7 }
    16ef:	370270c4107ff04e                  	{ extui	a2, a4, 16, 16; extui	a15, a7, 16, 16 }
    16f7:	c28620                                    	quou	a8, a6, a2
    16fa:	825820                                    	mull	a5, a8, a2
    16fd:	943d                                    	ae_zext16	a3, a4
    16ff:	c05650                                    	sub	a5, a6, a5
    1702:	115500                                    	slli	a5, a5, 16
    1705:	829830                                    	mull	a9, a8, a3
    1708:	1806984114e925fe                  	{ add	a6, a5, a15; or	a5, a8, a8 }
    1710:	15b697                                    	bgeu	a6, a9, 1729 <__udivdi3+0x169>
    1713:	340288c114ff364e                  	{ add	a6, a6, a4; addi	a5, a8, -1 }
    171b:	0a3647                                    	bltu	a6, a4, 1729 <__udivdi3+0x169>
    171e:	07b697                                    	bgeu	a6, a9, 1729 <__udivdi3+0x169>
    1721:	2406e6471471e85e                  	{ addi	a5, a8, -2; add	a6, a6, a4 }
    1729:	c08690                                    	sub	a8, a6, a9
    172c:	c26820                                    	quou	a6, a8, a2
    172f:	82e620                                    	mull	a14, a6, a2
    1732:	977d                                    	ae_zext16	a7, a7
    1734:	3b040fc31cff48ee                  	{ sub	a15, a8, a14; movi	a10, -1 }
    173c:	822630                                    	mull	a2, a6, a3
    173f:	113f00                                    	slli	a3, a15, 16
    1742:	337a                                    	add.n	a3, a3, a7
    1744:	340683010810a32e                  	{ bgeu.w15	a3, a2, 175c <__udivdi3+0x19c>; add	a3, a3, a4 }
    174c:	3406934014f0432e                  	{ saltu	a2, a3, a2; saltu	a3, a3, a4 }
    1754:	c02a20                                    	sub	a2, a10, a2
    1757:	932a30                                    	movnez	a2, a10, a3
    175a:	626a                                    	add.n	a6, a2, a6
    175c:	0304204410a0150e                  	{ slli	a2, a5, 16; movi	a3, 0 }
    1764:	202620                                    	or	a2, a6, a2
    1767:	f01d                                    	retw.n

00001769 <__udivdi3+0x1a9>:
    1769:	000000                                        ...

0000176c <__udivdi3+0x1ac>:
    176c:	0206f220001a93ae                  	{ beqz.w15	a3, 1840 <__udivdi3+0x280>; sub	a8, a2, a3 }
    1774:	35042ec318ff735e                  	{ ssl	a3; movi	a5, -2 }
    177c:	330748c100a434ce                  	{ sll	a4, a4; ssr	a8 }
    1784:	250250c614bc616e                  	{ srl	a10, a6; extui	a2, a4, 16, 16 }
    178c:	150230c318f9735e                  	{ ssl	a3; extui	a5, a5, 0, 16 }
    1794:	c29a20                                    	quou	a9, a10, a2
    1797:	330748c318a436ce                  	{ sll	a15, a6; ssr	a8 }
    179f:	82c920                                    	mull	a12, a9, a2
    17a2:	1506c4c61ca5717e                  	{ srl	a11, a7; and	a5, a4, a5 }
    17aa:	2c06ba4510faebfe                  	{ or	a6, a11, a15; sub	a10, a10, a12 }
    17b2:	828950                                    	mull	a8, a9, a5
    17b5:	2106ea461866e06e                  	{ extui	a11, a6, 16, 16; slli	a10, a10, 16 }
    17bd:	2b06aa4318f2735e                  	{ ssl	a3; add	a10, a10, a11 }
    17c5:	093d                                    	mov.n	a3, a9
    17c7:	15ba87                                    	bgeu	a10, a8, 17e0 <__udivdi3+0x220>
    17ca:	320289c214ff3a4e                  	{ add	a10, a10, a4; addi	a3, a9, -1 }
    17d2:	0a3a47                                    	bltu	a10, a4, 17e0 <__udivdi3+0x220>
    17d5:	07ba87                                    	bgeu	a10, a8, 17e0 <__udivdi3+0x220>
    17d8:	2406ea471472e93e                  	{ addi	a3, a9, -2; add	a10, a10, a4 }
    17e0:	c09a80                                    	sub	a9, a10, a8
    17e3:	c28920                                    	quou	a8, a9, a2
    17e6:	82f820                                    	mull	a15, a8, a2
    17e9:	966d                                    	ae_zext16	a6, a6
    17eb:	c099f0                                    	sub	a9, a9, a15
    17ee:	119900                                    	slli	a9, a9, 16
    17f1:	825850                                    	mull	a5, a8, a5
    17f4:	1806984114ea296e                  	{ add	a6, a9, a6; or	a9, a8, a8 }
    17fc:	14b657                                    	bgeu	a6, a5, 1814 <__udivdi3+0x254>
    17ff:	664a                                    	add.n	a6, a6, a4
    1801:	39028887081f764e                  	{ bltu.w15	a6, a4, 1814 <__udivdi3+0x254>; addi	a9, a8, -1 }
    1809:	07b657                                    	bgeu	a6, a5, 1814 <__udivdi3+0x254>
    180c:	2406e6471471e89e                  	{ addi	a9, a8, -2; add	a6, a6, a4 }
    1814:	310727c418a5030e                  	{ slli	a3, a3, 16; sll	a7, a7 }
    181c:	3306994114e8565e                  	{ sub	a6, a6, a5; or	a3, a9, a3 }
    1824:	000886                                    	j	184a <__udivdi3+0x28a>

00001827 <__udivdi3+0x267>:
	...

00001828 <__udivdi3+0x268>:
    1828:	0e04014104e0574e                  	{ saltu	a4, a7, a4; movi	a15, 1 }
    1830:	0e0694c014e9556e                  	{ saltu	a2, a5, a6; xor	a4, a4, a15 }
    1838:	202420                                    	or	a2, a4, a2
    183b:	f01d                                    	retw.n

0000183d <__udivdi3+0x27d>:
    183d:	000000                                        ...

00001840 <__udivdi3+0x280>:
    1840:	250210c114fc464e                  	{ sub	a6, a6, a4; extui	a2, a4, 16, 16 }
    1848:	130c                                    	movi.n	a3, 1
    184a:	c28620                                    	quou	a8, a6, a2
    184d:	82a820                                    	mull	a10, a8, a2
    1850:	945d                                    	ae_zext16	a5, a4
    1852:	370210c314ff56ae                  	{ sub	a14, a6, a10; extui	a15, a7, 16, 16 }
    185a:	116e00                                    	slli	a6, a14, 16
    185d:	829850                                    	mull	a9, a8, a5
    1860:	2806984214e926fe                  	{ add	a10, a6, a15; or	a6, a8, a8 }
    1868:	15ba97                                    	bgeu	a10, a9, 1881 <__udivdi3+0x2c1>
    186b:	360288c214ff2a4e                  	{ add	a10, a10, a4; addi	a6, a8, -1 }
    1873:	0a3a47                                    	bltu	a10, a4, 1881 <__udivdi3+0x2c1>
    1876:	07ba97                                    	bgeu	a10, a9, 1881 <__udivdi3+0x2c1>
    1879:	2406ea471472e86e                  	{ addi	a6, a8, -2; add	a10, a10, a4 }
    1881:	c08a90                                    	sub	a8, a10, a9
    1884:	97dd                                    	ae_zext16	a13, a7
    1886:	c27820                                    	quou	a7, a8, a2
    1889:	82e720                                    	mull	a14, a7, a2
    188c:	822750                                    	mull	a2, a7, a5
    188f:	39040fc31cff48ee                  	{ sub	a15, a8, a14; movi	a8, -1 }
    1897:	115f00                                    	slli	a5, a15, 16
    189a:	55da                                    	add.n	a5, a5, a13
    189c:	140685010811a52e                  	{ bgeu.w15	a5, a2, 18b4 <__udivdi3+0x2f4>; add	a5, a5, a4 }
    18a4:	0406954014f1452e                  	{ saltu	a2, a5, a2; saltu	a4, a5, a4 }
    18ac:	c02820                                    	sub	a2, a8, a2
    18af:	932840                                    	movnez	a2, a8, a4
    18b2:	727a                                    	add.n	a7, a2, a7
    18b4:	112600                                    	slli	a2, a6, 16
    18b7:	202720                                    	or	a2, a7, a2
    18ba:	f01d                                    	retw.n

000018bc <_fini>:
    18bc:	008136                                    	entry	a1, 64
    18bf:	f9ee81                                    	l32r	a8, 78 (94 <__do_global_dtors_aux>)
    18c2:	f03d                                    	nop.n
    18c4:	0008e0                                    	callx8	a8

000018c7 <_fini+0xb>:
    18c7:	f01d                                    	retw.n
