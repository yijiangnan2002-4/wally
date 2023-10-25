/* Copyright Statement:
 *
 * (C) 2019  Airoha Technology Corp. All rights reserved.
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

#ifndef __COMPONENT__
#define __COMPONENT__


#include "ComponentPreDefine.h"
#ifdef __cplusplus
extern "C" {
#endif



#define FreeStateFixedValue  (0xA7A7A7A7)
#define RegionHeaderSize  (128)

/*
demo code:
   DspMemoryManagementInit();   // The function should only call one times after reboot.

    SubComponentType_t GetId = SubComponent_A2DP_SBC;
    GroupMemoryInfo_t GetAddr;  // The obtained memory address and length.



    DspMemoryManagerReturnStatus_t ret_status;
    ret_status = DspMemoryManager_AcquireGroupMemory(ComA , GetId, &GetAddr);
    if(ret_status != DSP_MEMORY_MANAGEMENT_PROCESS_OK )
    {
       //error handle;
    }
    LOG_MSGID_I(main, "Get_ComponentA_info_1 Iramaddr=0x%x Dramaddr = 0x%x Sysram = 0x%x", 3, GetAddr.IramAddr,GetAddr.DramAddr,GetAddr.SysramAddr);
    LOG_MSGID_I(main, "Get_ComponentA_info_1 Iramlength=0x%x Dramlength = 0x%x Sysramlength = 0x%x", 3, GetAddr.IramLength,GetAddr.DramLength,GetAddr.SysramLength);

    ret_status = DspMemoryManager_ReleaseGroupMemory(ComA, GetId);
    if(ret_status != DSP_MEMORY_MANAGEMENT_PROCESS_OK )
    {
        //error handle;
    }

*/





typedef struct {
    void *IramAddr;
    uint32_t IramSizeInBytes;
    void *DramAddr;
    uint32_t DramSizeInBytes;
    void *SysramAddr;
    uint32_t SysramSizeInBytes;
} GroupMemoryInfo_t;


typedef enum
{
DSP_MEMORY_MANAGEMENT_NOT_INITIALIZED = -5,
DSP_MEMORY_MANAGEMENT_FATAL_ERROR = -4,
DSP_MEMORY_MANAGEMENT_CHECK_EXCLUSIVE_FAIL =-3,
DSP_MEMORY_MANAGEMENT_PASS_PARAMETER_INVALID = -2,
DSP_MEMORY_MANAGEMENT_DSP_MEMORY_INIT_FAIL = -1,
DSP_MEMORY_MANAGEMENT_PROCESS_OK = 0,
DSP_MEMORY_MANAGEMENT_PROCESS_NO_USE = 1,
DSP_MEMORY_MANAGEMENT_SAME_SUBCOMPONENT = 2,
DSP_MEMORY_MANAGEMENT_CHECK_EXCLUSIVE_SUCCESS = 3,

}DspMemoryManagerReturnStatus_t;


typedef struct
{
    uint32_t RegionExclusiveValue;
    uint32_t pGroupExclusiveValue[RegionHeaderSize/sizeof(uint32_t)-2];
}DspMemoryManagementCheck_t;


typedef struct {
    TotalComponentType_t component;
    uint32_t groupId;
} ComponentGroupType_t;

typedef struct {
    /* region start address */
    void *regionStartAddr;
    /* group start address */
    void *groupStartAddr;
    /* group end address */
    void *groupEndAddr;
} GroupMemoryAddressInfo_t;



typedef struct {
    ComponentGroupType_t componentGroup;
    GroupMemoryAddressInfo_t iram;
    GroupMemoryAddressInfo_t dram;
    GroupMemoryAddressInfo_t sysram;
} DMM_TcbInternalData_t;




/**
  * @}
  */

/**
 * @brief      This function init the DSP memory header.
 * @return     #COMPONENT_PROCESS_OK, if the operation is successful.\n
 *             #COMPONENT_DSP_MEMORY_INIT_FAIL, if the handle is invalid. \n
 */
DspMemoryManagerReturnStatus_t DspMemoryManagementInit();

/**
 * @brief      This function is used to get the memory address of the specified sub_component.
 * @param[in]  Component is used to specify the ownership of the sub_component that is currently acquiring memory.
 * @param[in]  SubComponentId is an struct, is the sub component id .
 * @param[in]  GroupMemoryInfo is as the return value, include the Iram address, Dram address ,sysram address.
 * @return     #COMPONENT_PROCESS_OK, if the operation is successful.\n
 *             #COMPONENT_PASS_PARAMETER_INVALIDE, if the parameter is invalid. \n
 *             #COMPONENT_CHECK_EXCLUSIVE_FAIL, for the current memory is used by other users.
 */
DspMemoryManagerReturnStatus_t DspMemoryManager_AcquireGroupMemory(TotalComponentType_t Component, SubComponentType_t SubComponentId , GroupMemoryInfo_t *GroupMemoryInfo);

/**
 * @brief      This function is used to get the memory address of the specified sub_component.
 * @param[in]  Component is used to specify the ownership of the sub_component that is currently free memory.
 * @param[in]  SubComponentId is an struct, is the sub component id .
 * @param[in]  GroupMemoryInfo is the free address of Iram, Dram,sysram.
 * @return     #COMPONENT_PROCESS_OK, if the operation is successful.\n
 *             #COMPONENT_PASS_PARAMETER_INVALIDE, if the parameter is invalid. \n
 *             #COMPONENT_FREE_REFERENCE_MEMORY_ERROR, for the current memory is used by other users.
 */
DspMemoryManagerReturnStatus_t DspMemoryManager_ReleaseGroupMemory(TotalComponentType_t Component , SubComponentType_t SubComponentId,GroupMemoryInfo_t *GroupMemoryInfo);

#ifdef __cplusplus
}
#endif

#endif /* _COMPONENT_ */



