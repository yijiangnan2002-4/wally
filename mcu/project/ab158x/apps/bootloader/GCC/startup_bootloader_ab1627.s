/* Copyright Statement:
 *
 * (C) 2017  Airoha Technology Corp. All rights reserved.
 *
 * This software/firmware and related documentation ("Airoha Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to Airoha Technology Corp. ("Airoha") and/or its licensors.
 * Without the prior written permission of Airoha and/or its licensors,
 * any reproduction, modification, use or disclosure of Airoha Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute (as applicable) Airoha Software
 * if you have agreed to and been bound by the applicable license agreement with
 * Airoha ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
 * please cease any access or use of Airoha Software immediately.
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT AIROHA SOFTWARE RECEIVED FROM AIROHA AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES AIROHA PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH AIROHA SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN AIROHA SOFTWARE. AIROHA SHALL ALSO NOT BE RESPONSIBLE FOR ANY AIROHA
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AIROHA'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO AIROHA SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT AIROHA'S OPTION, TO REVISE OR REPLACE AIROHA SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * AIROHA FOR SUCH AIROHA SOFTWARE AT ISSUE.
 */

.file "startup_bootloader.s"
.syntax unified
.cpu cortex-m4
.fpu softvfp
.thumb

.equ    WDT1_Base,          0xA2090030
.equ    WDT1_Disable,       0x40
.equ    Cache_Ctrl_Base,    0xE0180000
.equ    Cache_Disable,      0x0
.equ    Cache_Invalid_All,  0x3


/* start address for the initialization values of the .data section.
defined in linker script */
.word  _sidata
/* start address for the .data section. defined in linker script */
.word  _sdata
/* end address for the .data section. defined in linker script */
.word  _edata
/* start address for the .bss section. defined in linker script */
.word  _sbss
/* end address for the .bss section. defined in linker script */
.word  _ebss


.globl _start
_start:
  b Reset_Handler


/**
 * @brief  This is the code that gets called when the processor first
 *          starts execution following a reset event. Only the absolutely
 *          necessary set is performed, after which the application
 *          supplied main() routine is called.
 * @param  None
 * @retval : None
*/

.section  .reset_handler
.weak  Reset_Handler
.type  Reset_Handler, %function
Reset_Handler:
  ldr  sp, =_estack    		 /* set stack pointer */

/* disable interrupt */
  CPSID  I

/* watch dog1 disable */
  ldr  r0, =WDT1_Base
  ldr  r1, =WDT1_Disable
  str  r1, [r0, #0]

/* latch gpt start time for total bl boot time */
  movs r0, #0
  ldr r1, =bl_latch_start_gpt
  blx r1

/* cache disable, make region init safer */
  movs  r0, #Cache_Disable
  movs  r1, #Cache_Invalid_All
  ldr  r2, =Cache_Ctrl_Base
  str  r0, [r2], #4
  str  r1, [r2, #0]

/* latch gpt start time for total region init time */
  movs r0, #1
  ldr r1, =bl_latch_start_gpt
  blx r1

/* Zero initialization.  */
  ldr  r2, =_sysram_sbss
  ldr  r3, =_sysram_ebss
  bl  ZeroBssLoop

  ldr  r2, =_sbss
  ldr  r3, =_ebss
  bl  ZeroBssLoop

/* system init before jumping to main() */
  ldr r0, =bl_system_init
  blx r0

/* latch gpt start time for region init end time */
  movs r0, #1
  ldr r1, =bl_latch_end_gpt
  blx r1

/* Call the bootloader main function.*/
  bl  main


end_bootloader:
  //bl bootloader_error_hanbler
  bl end_bootloader

/*utility function*/
.globl JumpCmd
JumpCmd:
  ORR  r0, #0x01
  BX  r0

ZeroBssLoop:
  cmp     r2, r3
  ittt    lo
  movlo   r0, #0
  strlo   r0, [r2], #4
  blo     ZeroBssLoop
  bx  lr

/* vector table */
.section  .isr_vector,"a",%progbits
.type  g_pfnVectors, %object
.size  g_pfnVectors, .-g_pfnVectors

g_pfnVectors:
  .word  _estack
  .word  Reset_Handler
