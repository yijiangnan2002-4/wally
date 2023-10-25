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

#include "Component.h"
#include "hal_nvic.h"
#include "ComponentInternal.h"
//#include <assert.h>


DspMemoryManagerReturnStatus_t ComponentRegionAndGroupExclusiveSet(TotalComponentType_t Component,uint32_t GroupId, uint32_t SubId, DspMemoryManagementCheck_t* HeaderAddr)
{
    DspMemoryManagementCheck_t *pCheck;
    DspMemoryManagerReturnStatus_t ReturnStatus;
    pCheck = HeaderAddr;
    if(pCheck->RegionExclusiveValue == FreeStateFixedValue)
    {
        ReturnStatus = DSP_MEMORY_MANAGEMENT_CHECK_EXCLUSIVE_SUCCESS;
    }else if((pCheck->RegionExclusiveValue == Component)&&(pCheck->pGroupExclusiveValue[GroupId] == FreeStateFixedValue)){
        ReturnStatus = DSP_MEMORY_MANAGEMENT_CHECK_EXCLUSIVE_SUCCESS;
    }else if((pCheck->RegionExclusiveValue == Component)&&(pCheck->pGroupExclusiveValue[GroupId] == SubId)){
        ReturnStatus = DSP_MEMORY_MANAGEMENT_SAME_SUBCOMPONENT;
    }else{
        ReturnStatus = DSP_MEMORY_MANAGEMENT_CHECK_EXCLUSIVE_FAIL;
    }

    if(ReturnStatus==DSP_MEMORY_MANAGEMENT_CHECK_EXCLUSIVE_SUCCESS)
    {
    pCheck->RegionExclusiveValue = Component;
    pCheck->pGroupExclusiveValue[GroupId] = SubId;
    }

    return ReturnStatus;
}


uint32_t ComponentGetRegionExclusiveValue(DspMemoryManagementCheck_t *HeaderAddr)
{
    DspMemoryManagementCheck_t *pGetValue= HeaderAddr;
    return pGetValue->RegionExclusiveValue;
}

uint32_t ComponentGetGroupExclusiveValue(DspMemoryManagementCheck_t *HeaderAddr , uint32_t GroupId)
{
    DspMemoryManagementCheck_t *pGetValue= HeaderAddr;
    return pGetValue->pGroupExclusiveValue[GroupId];

}
void CompoentRegionExclusiveClear(DspMemoryManagementCheck_t *HeaderAddr)
{
    DspMemoryManagementCheck_t *pClearValue=HeaderAddr;
    pClearValue->RegionExclusiveValue = FreeStateFixedValue;
}


void CompoentGroupExclusiveClear(DspMemoryManagementCheck_t *HeaderAddr,uint32_t GroupId)
{
    DspMemoryManagementCheck_t *pClearValue=HeaderAddr;
    pClearValue->pGroupExclusiveValue[GroupId] = FreeStateFixedValue;
}

uint32_t ComponentGetUsedGroupNumber(DspMemoryManagementCheck_t* HeaderAddr)
{
    DspMemoryManagementCheck_t *pGetValue=HeaderAddr;
    uint32_t Count=0 , Index;
    for(Index = 0 ; Index < ((RegionHeaderSize/4)-1) ; Index++)
    {
       if(pGetValue->pGroupExclusiveValue[Index] !=FreeStateFixedValue)
       {
           Count++;
           break;
       }
    }
    return Count;
}

void ComponentGroupHeaderPatchCheck(GroupHeader_t *StartAddr,void *UserStartAddr)
{
  GroupHeader_t* pTemp = StartAddr;

  if((pTemp->HeaderAddr == (void*)StartAddr)&&((*(uint32_t*)(pTemp->GroupEndAddr))== (uint32_t)pTemp->HeaderAddr)&&(pTemp->UserStartAddr == UserStartAddr))
  {
  *(uint32_t *)(pTemp->GroupEndAddr) = 0xF1F2F3F4;
  pTemp->GroupEndAddr = NULL;
  pTemp->BlockSizeInByte &=((~1)<<31);
  }else{
      //LOG_MSGID_I(Mem_Management, "ComponentGroupHeaderPatchCheck## Group Header check fail Start addr:0x%x  User addr: 0x%x", 2,StartAddr,UserStartAddr);
      DSP_MM_ASSERT(0);
  }
}


DspMemoryManagerReturnStatus_t DspMemoryManager_InternalReturnAddr(void* StartAddr, void* EndAddr, uint32_t* Addr, uint32_t *SizeInByte)
{

    uint32_t TempSize = (uint32_t)EndAddr - (uint32_t)StartAddr;
    GroupHeader_t* pTemp;
    uint32_t int_mask;

    if((TempSize!=0 )&&(EndAddr>StartAddr))
    {
      hal_nvic_save_and_set_interrupt_mask(&int_mask);
      pTemp = (GroupHeader_t*)StartAddr;
      pTemp->HeaderAddr = StartAddr;
      pTemp->BlockSizeInByte = TempSize |(1<<31);
      pTemp->GroupEndAddr = (void*)((uint32_t)EndAddr-4);
      *(uint32_t *)(pTemp->GroupEndAddr)= (uint32_t)StartAddr;
      pTemp->UserStartAddr = (void*)((((uint32_t)StartAddr+sizeof(GroupHeader_t))+63) & (~0x3f));   // from audio requirement ensure 16byte align start address.
      hal_nvic_restore_interrupt_mask(int_mask);

     *Addr=(uint32_t)pTemp->UserStartAddr;
     *SizeInByte= (uint32_t)pTemp->GroupEndAddr - (uint32_t)pTemp->UserStartAddr;
    }else
    {
    *Addr = 0;
    *SizeInByte = 0;
    }
    return DSP_MEMORY_MANAGEMENT_PROCESS_OK;
}
