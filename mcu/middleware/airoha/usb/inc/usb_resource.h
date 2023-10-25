/* Copyright Statement:
 *
 * (C) 2005-2016  MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. ("MediaTek") and/or its licensors.
 * Without the prior written permission of MediaTek and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute (as applicable) MediaTek Software
 * if you have agreed to and been bound by the applicable license agreement with
 * MediaTek ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
 * please cease any access or use of MediaTek Software immediately.
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT MEDIATEK SOFTWARE RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */

#ifndef USB_RESOURCE_H
#define USB_RESOURCE_H

#include <stdbool.h>
#include <stdint.h>
#include "usb.h"

/***********************************************
    implement enum and structure
************************************************/

/* interface create infomration structrue, use this information to create interface */
typedef struct {
    char        *if_name_ptr;
    usb_create_if_func_ptr  if_create_func;
    usb_void_func       if_init_func;
    usb_void_func       if_enable_func;
    usb_speed_if_func_ptr   if_speed_func;
    usb_void_func       if_resume_func;
} Usb_IfCreate_Info;


typedef struct {
    uint8_t     device_code;    /*device class*/
    uint8_t     subclass_code;
    uint8_t     protocol_code;
    uint16_t    product_code;
    bool    b_registerd;
} Usb_Device_Code;


/***********************************************
    function and global variable
************************************************/

extern void USB_Software_Create(void);
extern void USB_Software_PrevInit(void);
extern void USB_Software_Speed_Init(bool b_other_speed);
extern void USB_Software_Init(void);
extern void USB_Software_Resume(void);
extern void USB_Software_Enable(void);
extern void USB_DeRegister_CreateFunc(void);
extern void USB_Register_CreateFunc(char *if_name, usb_create_if_func_ptr if_create_func, usb_void_func if_init_func,
                                    usb_void_func if_enable_func, usb_speed_if_func_ptr if_speed_func, usb_void_func if_resume_func);
extern Usb_Interface_Info* USB_Get_Interface_Number(uint8_t *p_num);
extern Usb_Ep_Info *USB_Get_Intr_Tx_Ep(uint8_t *p_num);
extern Usb_Ep_Info *USB_Get_Intr_Rx_Ep(uint8_t *p_num);
extern Usb_Ep_Info *USB_Get_Bulk_Tx_Ep(uint8_t *p_num);
extern Usb_Ep_Info *USB_Get_Bulk_Rx_Ep(uint8_t *p_num);
extern Usb_Ep_Info *USB_Get_Iso_Tx_Ep(uint8_t *p_num);
extern Usb_Ep_Info *USB_Get_Iso_Rx_Ep(uint8_t *p_num);

extern Usb_IAD_Dscr *USB_Get_IAD_Number(void);

uint8_t USB_String_Create(usb_string_usage_t usage, uint8_t id, const char *str);
uint8_t USB_String_Get_Id_By_Usage(usb_string_usage_t usage);
usb_string_t *USB_String_Get_By_Usage(usb_string_usage_t usage);
usb_string_t *USB_String_Get_By_Id(uint8_t id);
void USB_String_Nvkey_Write(void);
void USB_String_Nvkey_Read(void);

extern bool        g_USB_Software_Speed_skip_enable;

#endif /* USB_RESOURCE_H */

