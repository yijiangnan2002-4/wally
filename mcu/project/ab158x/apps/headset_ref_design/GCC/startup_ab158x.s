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

.file "startup_ab158x.s"
.syntax unified
.cpu cortex-m33
.arch	armv8-m.main
.fpu softvfp
.thumb

#ifdef AIR_CPU_IN_SECURITY_MODE 
.equ    WDT_Base,           0x420B0000
.equ    WDT1_Base,          0x420B0030
#else
.equ    WDT_Base,           0x520B0000
.equ    WDT1_Base,          0x520B0030
#endif

.equ    WDT_Disable,        0x10
.equ    WDT_Enable,         0x110
.equ    WDT_Length_Value,   0x04800012  /* 15.625*N << 16 | 0x12 = 18s */
.equ    WDT_RST_Key,        0x1456789A
.equ    WDT_RST_Mask,       0x01260125  // mask pmic
.equ    WDT_Disable,        0x10
.equ    WDT1_Disable,       0x40

.equ    Remap_Base,         0xE0181000
.equ    Remap_Entry_HI0,    0x24200017
.equ    Remap_Entry_LO0,    0x04200000
.equ    Remap_Entry_HI1,    0x34200017
.equ    Remap_Entry_LO1,    0x14200000

.global  g_pfnVectors
.global  Default_Handler

/**
 * @brief  reset_handler is the entry point that processor starts to boot
 * @param  None
 * @retval : None
*/

  .section  .reset_handler
  .weak  Reset_Handler
  .type  Reset_Handler, %function
Reset_Handler:

/* set stack pointer */
  ldr  sp, =_stack_end

/* set stack limit pointer register, 
    Privileged access only. */
  ldr  r0, =_stack_start
  msr  msplim, r0

/* interrupt disable */
  cpsid i

/* config watch dog channel-0 */
#if 0
  ldr  r0, =WDT_Base
  ldr  r1, =WDT_Enable
  str  r1, [r0, #0x0]
  ldr  r1, =WDT_Length_Value
  str  r1, [r0, #0x4]
  ldr  r1, =WDT_RST_Mask
  str  r1, [r0, #0x84]
  ldr  r1, =WDT_RST_Key
  str  r1, [r0, #0xC]
#else
  ldr  r0, =WDT_Base
  ldr  r1, =WDT_Disable
  str  r1, [r0, #0]
#endif /* MTK_BOOT_UP_DISABLE_WDT */

/* watch dog channel-1 disable */
  ldr  r0, =WDT1_Base
  ldr  r1, =WDT1_Disable
  str  r1, [r0, #0]

/* preinit cache to accelerate region init progress */
  bl CachePreInit

/* make virtual space available */
#ifdef AIR_CPU_IN_SECURITY_MODE 
  ldr  r0, =Remap_Entry_HI0
  ldr  r1, =Remap_Entry_LO0
  ldr  r3, =Remap_Entry_HI1
  ldr  r4, =Remap_Entry_LO1
  ldr  r2, =Remap_Base
  str  r0, [r2], #4
  str  r1, [r2], #4
  str  r3, [r2], #4
  str  r4, [r2, #0]
#endif

/* stack space zero init */
  movs  r0, #0
  ldr  r1, =_stack_start
  ldr  r2, =_stack_end
FillZero:
  str  r0, [r1], #4
  cmp  r1, r2
  blo  FillZero

/* bss ZI space zero init */
  ldr  r2, =_bss_start
  ldr  r3, =_bss_end
  bl  Bss_Init

/* tcm section init */
  ldr  r1, =_tcm_text_load
  ldr  r2, =_tcm_text_start
  ldr  r3, =_tcm_text_end
  bl  Data_Init

/* tcm rw section init */
  ldr  r1, =_tcm_rw_data_load
  ldr  r2, =_tcm_rw_start
  ldr  r3, =_tcm_rw_end
  bl  Data_Init

  ldr  r2, =_tcm_zi_start
  ldr  r3, =_tcm_zi_end
  bl  Bss_Init

/* sysram_text section init */
  ldr  r1, =_sysram_code_load
  ldr  r2, =_sysram_code_start
  ldr  r3, =_sysram_code_end
  bl  Data_Init

/* cached_sysram_data section init */
  ldr  r1, =_cached_sysram_data_load
  ldr  r2, =_cached_sysram_data_start
  ldr  r3, =_cached_sysram_data_end
  bl  Data_Init

/* noncached_sysram_data section init */
  ldr  r1, =_noncached_sysram_rw_load
  ldr  r2, =_noncached_sysram_rw_start
  ldr  r3, =_noncached_sysram_rw_end
  bl  Data_Init

/* share data section init */
  ldr  r1, =_share_data_load
  ldr  r2, =_share_data_start
  ldr  r3, =_share_data_end
  bl  Data_Init

/* bt share data section init */
  ldr  r1, =_bt_sysram_rw_data_load
  ldr  r2, =_bt_sysram_rw_data_start
  ldr  r3, =_bt_sysram_rw_data_end
  bl  Data_Init

/* noncached_sysram_zidata section init */
  ldr  r2, =_noncached_sysram_zi_start
  ldr  r3, =_noncached_sysram_zi_end
  bl  Bss_Init

/* share zidata section init */
  ldr  r2, =_share_bss_start
  ldr  r3, =_share_bss_end
  bl  Bss_Init

/* bt share zidata section init */
  ldr  r2, =_bt_sysram_zi_data_start
  ldr  r3, =_bt_sysram_zi_data_end
  bl  Bss_Init

/* Call the clock system intitialization function.*/
  ldr r0, =SystemInit
  blx r0

/* Call the application's entry point.*/
  ldr r0, =main
  bx  r0
  bx  lr
.size  Reset_Handler, .-Reset_Handler

/**
 * @brief  This is data init sub-function
 * @param  None
 * @retval None
*/
  .section  .reset_handler.Data_Init,"ax",%progbits
Data_Init:
CopyDataLoop:
  cmp     r2, r3
  ittt    lo
  ldrlo   r0, [r1], #4
  strlo   r0, [r2], #4
  blo     CopyDataLoop
  bx  lr
  .size  Data_Init, .-Data_Init

/**
 * @brief  This is bss init sub-function
 * @param  None
 * @retval None
*/
  .section  .reset_handler.Bss_Init,"ax",%progbits
Bss_Init:
ZeroBssLoop:
  cmp     r2, r3
  ittt    lo
  movlo   r0, #0
  strlo   r0, [r2], #4
  blo     ZeroBssLoop
  bx  lr
  .size  Bss_Init, .-Bss_Init

/**
 * @brief  This is the code that gets called when the processor receives an
 *         unexpected interrupt.  This simply enters an infinite loop, preserving
 *         the system state for examination by a debugger.
 * @param  None
 * @retval None
*/
  .section  .text.Default_Handler,"ax",%progbits
Default_Handler:
Infinite_Loop:
  b  Infinite_Loop
  .size  Default_Handler, .-Default_Handler


/******************************************************************************
*
* The minimal vector table for a Cortex M4. Note that the proper constructs
* must be placed on this to ensure that it ends up at physical address
* 0x0000.0000.
*
*******************************************************************************/
  .section  .isr_vector,"a",%progbits
  .type  g_pfnVectors, %object
  .size  g_pfnVectors, .-g_pfnVectors


g_pfnVectors:
  .word   _stack_end                                      /* Top of Stack */
  .word   Reset_Handler                                   /* Reset Handler */
  .word   NMI_Handler                                     /* NMI Handler*/
  .word   HardFault_Handler                               /* Hard Fault Handler*/
  .word   MemManage_Handler                               /* MPU Fault Handler*/
  .word   BusFault_Handler                                /* Bus Fault Handler*/
  .word   UsageFault_Handler                              /* Usage Fault Handler*/
  .word   0                                               /* Reserved*/
  .word   0                                               /* Reserved*/
  .word   0                                               /* Reserved*/
  .word   0                                               /* Reserved*/
  .word   SVC_Handler                                     /* SVCall Handler*/
  .word   DebugMon_Handler                                /* Debug Monitor Handler*/
  .word   0                                               /* Reserved*/
  .word   PendSV_Handler                                  /* PendSV Handler*/
  .word   SysTick_Handler                                 /* SysTick Handler*/

/* External Interrupts */
  .word     isrC_main     /*16:  OS_GPT_IRQn         */
  .word     isrC_main     /*17:  MCU_DMA0_IRQn       */
  .word     isrC_main     /*18:  MCU_DMA1_IRQn       */
  .word     isrC_main     /*19:  UART_DMA0_IRQn      */
  .word     isrC_main     /*20:  UART_DMA1_IRQn      */
  .word     isrC_main     /*21:  UART_DMA2_IRQn      */
  .word     isrC_main     /*22:  I2C_DMA0_IRQn       */
  .word     isrC_main     /*23:  I2C_DMA1_IRQn       */
  .word     isrC_main     /*24:  I2C_DMA2_IRQn       */
  .word     isrC_main     /*25:  I3C_DMA0_IRQn       */
  .word     isrC_main     /*26:  I3C_DMA1_IRQn       */
  .word     isrC_main     /*27:  SPI_MST0_IRQn       */
  .word     isrC_main     /*28:  SPI_MST1_IRQn       */
  .word     isrC_main     /*29:  SPI_MST2_IRQn       */
  .word     isrC_main     /*30:  SPI_SLV_IRQn        */
  .word     isrC_main     /*31:  SDIO_MST0_IRQn      */
  .word     isrC_main     /*32:  UART0_IRQn          */
  .word     isrC_main     /*33:  UART1_IRQn          */
  .word     isrC_main     /*34:  UART2_IRQn          */
  .word     isrC_main     /*35:  CRYPTO_IRQn         */
  .word     isrC_main     /*36:  TRNG_IRQn           */
  .word     isrC_main     /*37:  I2S_SLAVE_IRQn      */
  .word     isrC_main     /*38:  I2C0_IRQn           */
  .word     isrC_main     /*39:  I2C1_IRQn           */
  .word     isrC_main     /*40:  I2C2_IRQn           */
  .word     isrC_main     /*41:  I2C_AO_IRQn         */
  .word     isrC_main     /*42:  I3C0_IRQn           */
  .word     isrC_main     /*43:  I3C1_IRQn           */
  .word     isrC_main     /*44:  RTC_IRQn            */
  .word     isrC_main     /*45:  GPT_IRQn            */
  .word     isrC_main     /*46:  GPT_SEC_IRQn        */
  .word     isrC_main     /*47:  SPM_IRQn            */
  .word     isrC_main     /*48:  WDT_IRQn            */
  .word     isrC_main     /*49:  EINT_S_IRQn         */
  .word     isrC_main     /*50:  EINT_NS_IRQn        */
  .word     isrC_main     /*51:  SFC_IRQn            */
  .word     isrC_main     /*52:  ESC_IRQn            */
  .word     isrC_main     /*53:  USB_IRQn            */
  .word     isrC_main     /*54:  DSP0_IRQn           */
  .word     isrC_main     /*55:  CAP_TOUCH_IRQn      */
  .word     isrC_main     /*56:  AUDIOSYS0_IRQn      */
  .word     isrC_main     /*57:  AUDIOSYS1_IRQn      */
  .word     isrC_main     /*58:  AUDIOSYS2_IRQn      */
  .word     isrC_main     /*59:  AUDIOSYS3_IRQn      */
  .word     isrC_main     /*60:  ANC_IRQn            */
  .word     isrC_main     /*61:  ANC_RAMP_IRQn       */
  .word     isrC_main     /*62:  ANC_DMA_IRQn        */
  .word     isrC_main     /*63:  VAD_IRQn            */
  .word     isrC_main     /*64:  BT_IRQn             */
  .word     isrC_main     /*65:  BT_AURX_IRQn        */
  .word     isrC_main     /*66:  BT_AUTX_IRQn        */
  .word     isrC_main     /*67:  BT_TIMER_IRQn       */
  .word     isrC_main     /*68:  BT_PLAY_EN_IRQn     */
  .word     isrC_main     /*69:  VOW_SNR_IRQn        */
  .word     isrC_main     /*70:  VOW_FIFO_IRQn       */
  .word     isrC_main     /*71:  SEC_VIOLATION_IRQn  */
  .word     isrC_main     /*72:  MEM_ILLEGAL_IRQn    */
  .word     isrC_main     /*73:  BUS_ERR_IRQn        */
  .word     isrC_main     /*74:  MBX_TX0_IRQn        */
  .word     isrC_main     /*75:  MBX_TX1_IRQn        */
  .word     isrC_main     /*76:  MBX_TX2_IRQn        */
  .word     isrC_main     /*77:  MBX_TX3_IRQn        */
  .word     isrC_main     /*78:  MBX_TX4_IRQn        */
  .word     isrC_main     /*79:  MBX_TX5_IRQn        */
  .word     isrC_main     /*80:  MBX_RX0_IRQn        */
  .word     isrC_main     /*81:  MBX_RX1_IRQn        */
  .word     isrC_main     /*82:  MBX_RX2_IRQn        */
  .word     isrC_main     /*83:  MBX_RX3_IRQn        */
  .word     isrC_main     /*84:  MBX_RX4_IRQn        */
  .word     isrC_main     /*85:  MBX_RX5_IRQn        */
  .word     isrC_main     /*86:  PMU_IRQn            */
  .word     isrC_main     /*87:  IRRX_IRQn           */
  .word     isrC_main     /*88:  DSP_ERR_IRQn        */
  .word     isrC_main     /*89:  SW_IRQn             */
  .word     isrC_main     /*90:  CM33_reserved0_IRQn */


/*******************************************************************************
*
* Provide weak aliases for each Exception handler to the Default_Handler.
* As they are weak aliases, any function with the same name will override
* this definition.
*
*******************************************************************************/




