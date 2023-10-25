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

#include <string.h>
#include "Component.h"
#include "syslog.h"
#include "ComponentInternal.h"
#include "hal_nvic.h"
#include "memory_map.h"

log_create_module(Mem_Management,PRINT_LEVEL_DEBUG);

extern const uint32_t RegionAddr[];
extern const uint32_t g_sub_cpnt_to_tcb_idx[];
extern const DMM_TcbInternalData_t g_dmm_tcb[];
static bool Initialize_flag = false;

DspMemoryManagerReturnStatus_t DspMemoryManagementInit()
{
   unsigned int RegionCount;
   unsigned int index;
   uint32_t idx;
   uint32_t GroupHeaderCount;
   DspMemoryManagementCheck_t *ptemp;
   //set the free initial value for exclusive , every region head is used for doing exclusive check.
   RegionCount = DMM_PREDEFINED_REGION_NUM;

   DSP_MM_ASSERT(RegionCount>0);

   if(Initialize_flag !=true)
   {
   for (index = 0 ; index < RegionCount ; index ++)
   {
        if((RegionAddr[index]<DRAM_BASE)||(RegionAddr[index])>DRAM_LIMIT)
        {
            continue;
        }

        ptemp = (DspMemoryManagementCheck_t*)RegionAddr[index];

        if(ptemp == NULL)
        {
            DSP_MM_ASSERT(0);
        };
        ptemp->RegionExclusiveValue = FreeStateFixedValue;

        GroupHeaderCount = (RegionHeaderSize/4)-1;
        for(idx = 0 ; idx < GroupHeaderCount;idx ++ )
        {
            ptemp->pGroupExclusiveValue[idx] = FreeStateFixedValue;
        }
     LOG_MSGID_I(Mem_Management, "DspMemoryManagementInit##Region index:%d Addr:0x%x,GroupNum:%d.", 3,index,ptemp,GroupHeaderCount);
   }
    Initialize_flag = true;
   }
   return DSP_MEMORY_MANAGEMENT_PROCESS_OK;

}

DspMemoryManagerReturnStatus_t DspMemoryManager_AcquireGroupMemory(TotalComponentType_t Component, SubComponentType_t SubComponentId , GroupMemoryInfo_t *GroupMemoryInfo)
{
    DMM_TcbInternalData_t *pInfo;
    GroupMemoryInfo_t *pGroupMemoryInfo = GroupMemoryInfo;
    DspMemoryManagerReturnStatus_t ReturnStatus;
    uint32_t int_mask;
    uint32_t temp_index = 0 ;
    GroupMemoryAddressInfo_t *p_temp;

    if(Initialize_flag != true)
    {
        LOG_MSGID_E(Mem_Management, "DspMemoryManager_AcquireGroupMemory## DO NOT Initialize DSP Memory", 0);
        return DSP_MEMORY_MANAGEMENT_NOT_INITIALIZED;
    }

    if((Component<= Component_Start)||(Component>=Component_Max)||(SubComponentId<= SubComponent_Start)||(SubComponentId>=SubComponent_Max)||(GroupMemoryInfo==NULL))
    {
        LOG_MSGID_E(Mem_Management, "DspMemoryManager_AcquireGroupMemory## Pass Parameter Invalid", 0);
        return DSP_MEMORY_MANAGEMENT_PASS_PARAMETER_INVALID;
    }

    if((Component == Component_No_Used_Memory)||(SubComponentId == SubComponent_No_Used_Memory))
    {
        //No Use Memory Manager
        memset(GroupMemoryInfo, 0, sizeof(GroupMemoryInfo_t));
        return DSP_MEMORY_MANAGEMENT_PROCESS_NO_USE;
    }

    LOG_MSGID_I(Mem_Management, "DspMemoryManager_AcquireGroupMemory##Com:0x%x, SubComponentId:0x%x", 2, Component, SubComponentId);

    /*get relate subcomponent addr*/
    temp_index = (uint32_t)(SubComponentId - SubComponent_Start-1);
    temp_index = g_sub_cpnt_to_tcb_idx[temp_index];
    pInfo =(DMM_TcbInternalData_t*) &g_dmm_tcb[temp_index];


    if(pInfo->componentGroup.component != Component)
    {
        LOG_MSGID_E(Mem_Management, "DspMemoryManager_AcquireGroupMemory##Component is not match,default component =0x%x , pass Component=0x%x", 2,pInfo->componentGroup.component,Component);
        return DSP_MEMORY_MANAGEMENT_PASS_PARAMETER_INVALID;
    }

    /*exclusive check */
    p_temp = &(pInfo->dram);
    if(p_temp->regionStartAddr != NULL)
    {
      hal_nvic_save_and_set_interrupt_mask(&int_mask);
      ReturnStatus = ComponentRegionAndGroupExclusiveSet(Component,pInfo->componentGroup.groupId,SubComponentId,p_temp->regionStartAddr);
      if (ReturnStatus == DSP_MEMORY_MANAGEMENT_CHECK_EXCLUSIVE_SUCCESS) {
          if (pInfo->dram.groupStartAddr) {
              memset(pInfo->dram.groupStartAddr, 0, pInfo->dram.groupEndAddr-pInfo->dram.groupStartAddr);
          }
      } else if (ReturnStatus == DSP_MEMORY_MANAGEMENT_SAME_SUBCOMPONENT) {

      } else {
        hal_nvic_restore_interrupt_mask(int_mask);
        LOG_MSGID_E(Mem_Management, "DspMemoryManager_AcquireGroupMemory##Component ID:[0x%x]Exclusive Check fail,Return value = %d.", 2,Component,pInfo->componentGroup.groupId,(int)ReturnStatus);
        return DSP_MEMORY_MANAGEMENT_CHECK_EXCLUSIVE_FAIL;
      }
      hal_nvic_restore_interrupt_mask(int_mask);
    }else{
    /*fatal error*/
    LOG_MSGID_E(Mem_Management, "DspMemoryManager_AcquireGroupMemory## fatal error",0);
    return DSP_MEMORY_MANAGEMENT_FATAL_ERROR;
    }
    /*update the return addr and update some group header for check overflow */
    DspMemoryManager_InternalReturnAddr(pInfo->iram.groupStartAddr,pInfo->iram.groupEndAddr,(uint32_t*)&pGroupMemoryInfo->IramAddr,&pGroupMemoryInfo->IramSizeInBytes);
    DspMemoryManager_InternalReturnAddr(pInfo->dram.groupStartAddr,pInfo->dram.groupEndAddr,(uint32_t*)&pGroupMemoryInfo->DramAddr,&pGroupMemoryInfo->DramSizeInBytes);
    DspMemoryManager_InternalReturnAddr(pInfo->sysram.groupStartAddr,pInfo->sysram.groupEndAddr,(uint32_t*)&pGroupMemoryInfo->SysramAddr,&pGroupMemoryInfo->SysramSizeInBytes);


    LOG_MSGID_I(Mem_Management, "DspMemoryManager_AcquireGroupMemory##Iramaddr=0x%x Dramaddr = 0x%x Sysram = 0x%x", 3, (uint32_t)pGroupMemoryInfo->IramAddr,(uint32_t)pGroupMemoryInfo->DramAddr,(uint32_t)pGroupMemoryInfo->SysramAddr);
    LOG_MSGID_I(Mem_Management, "DspMemoryManager_AcquireGroupMemory##Iramlength=0x%x Dramlength = 0x%x Sysramlength = 0x%x", 3, pGroupMemoryInfo->IramSizeInBytes,pGroupMemoryInfo->DramSizeInBytes,pGroupMemoryInfo->SysramSizeInBytes);
    return DSP_MEMORY_MANAGEMENT_PROCESS_OK;

}


DspMemoryManagerReturnStatus_t DspMemoryManager_ReleaseGroupMemory(TotalComponentType_t Component , SubComponentType_t SubComponentId,GroupMemoryInfo_t *GroupMemoryInfo)
{

    DMM_TcbInternalData_t *pInfo;
    GroupMemoryAddressInfo_t *p_temp;
    uint32_t temp_index = 0 ;
    uint32_t TempRegionValue;
    uint32_t TempGroupValue;
    uint32_t UsedGroupNum;
    uint32_t int_mask;
    GroupMemoryInfo_t *p_temp_group = GroupMemoryInfo;

    if(Initialize_flag != true)
    {
        LOG_MSGID_E(Mem_Management, "DspMemoryManager_ReleaseGroupMemory## DO NOT Initialize DSP Memory", 0);
        return DSP_MEMORY_MANAGEMENT_NOT_INITIALIZED;
    }

    if((Component<= Component_Start)||(Component>=Component_Max)||(SubComponentId<= SubComponent_Start)||(SubComponentId>=SubComponent_Max)||(GroupMemoryInfo==NULL))
    {
        LOG_MSGID_E(Mem_Management, "DspMemoryManager_ReleaseGroupMemory## Pass Parameter Invalid", 0);
        return DSP_MEMORY_MANAGEMENT_PASS_PARAMETER_INVALID;
    }

    if((Component == Component_No_Used_Memory)||(SubComponentId == SubComponent_No_Used_Memory))
    {
        //No Use Memory Manager
        return DSP_MEMORY_MANAGEMENT_PROCESS_NO_USE;
    }


    temp_index = (uint32_t)(SubComponentId - SubComponent_Start-1);
    temp_index = g_sub_cpnt_to_tcb_idx[temp_index];
    pInfo = (DMM_TcbInternalData_t*)&g_dmm_tcb[temp_index];
    if(pInfo->componentGroup.component != Component)
    {
        LOG_MSGID_E(Mem_Management, "DspMemoryManager_ReleaseGroupMemory##Component is not match", 0);
        return DSP_MEMORY_MANAGEMENT_PASS_PARAMETER_INVALID;
    }

    p_temp = &(pInfo->dram);
    if(p_temp->regionStartAddr !=NULL)
    {

       if(p_temp_group->IramSizeInBytes != 0)
       ComponentGroupHeaderPatchCheck((GroupHeader_t*)pInfo->iram.groupStartAddr,p_temp_group->IramAddr);
       if(p_temp_group->DramSizeInBytes !=0)
       ComponentGroupHeaderPatchCheck((GroupHeader_t*)pInfo->dram.groupStartAddr,p_temp_group->DramAddr);
       if(p_temp_group->SysramSizeInBytes != 0)
       ComponentGroupHeaderPatchCheck((GroupHeader_t*)pInfo->sysram.groupStartAddr,p_temp_group->SysramAddr);

       hal_nvic_save_and_set_interrupt_mask(&int_mask);

       TempRegionValue = ComponentGetRegionExclusiveValue((DspMemoryManagementCheck_t*)p_temp->regionStartAddr);
       TempGroupValue = ComponentGetGroupExclusiveValue((DspMemoryManagementCheck_t*)p_temp->regionStartAddr,pInfo->componentGroup.groupId);
       if((TempRegionValue != Component)||(TempGroupValue != SubComponentId))
       {
         hal_nvic_restore_interrupt_mask(int_mask);
         LOG_MSGID_E(Mem_Management, "DspMemoryManager_ReleaseGroupMemory## Header_check fail,Component = 0x%x , Sub_id = 0x%x",2,Component,SubComponentId);
         return DSP_MEMORY_MANAGEMENT_CHECK_EXCLUSIVE_FAIL;
       }
        CompoentGroupExclusiveClear((DspMemoryManagementCheck_t*)p_temp->regionStartAddr,pInfo->componentGroup.groupId);
        hal_nvic_restore_interrupt_mask(int_mask);

        LOG_MSGID_I(Mem_Management, "DspMemoryManager_ReleaseGroupMemory## Free Group OK, Component = 0x%x Groupid=%d Sub_id = 0x%x",3,Component,pInfo->componentGroup.groupId,SubComponentId);
        hal_nvic_save_and_set_interrupt_mask(&int_mask);
        UsedGroupNum = ComponentGetUsedGroupNumber((DspMemoryManagementCheck_t*)p_temp->regionStartAddr);
        if(UsedGroupNum==0)
        {
            CompoentRegionExclusiveClear((DspMemoryManagementCheck_t*)p_temp->regionStartAddr);
        }
        hal_nvic_restore_interrupt_mask(int_mask);

        return DSP_MEMORY_MANAGEMENT_PROCESS_OK;
    }else{
        /*fatal error*/
        LOG_MSGID_E(Mem_Management, "DspMemoryManager_ReleaseGroupMemory## fatal error",0);
        return DSP_MEMORY_MANAGEMENT_FATAL_ERROR;
    }

}
