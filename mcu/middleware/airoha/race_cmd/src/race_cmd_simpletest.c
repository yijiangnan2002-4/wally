/* Copyright Statement:
*
* (C) 2022  Airoha Technology Corp. All rights reserved.
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
/* Airoha restricted information */

#include "race_cmd_simpletest.h"
#include "race_xport.h"


#define MAX_COMMAND_BUFFER_LEN  264

#if RACE_CMD_SIMPLE_TEST_ENABLE

////////////////////////////////////////////////////////////////////////////////
// Constant Definitions ////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#define PACKET_OPCODE(x) ((int32_t)x[0] & 0xffU)

#define PFID_SimpleTuningInterfaceTest 200

///////////////////////////////////////////////////////////////////////////
// FUNCTION DECLARATIONS /////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
static void ComputeCheckSumPublic(uint32_t * pPacketBuffer)
{
    uint32_t i;
    uint32_t nLen = pPacketBuffer[0] >> 16;
    uint32_t nCRC = 0;

    for (i = 0; i < (nLen - 1); i++)
    {
        nCRC ^= pPacketBuffer[i];
    }

    pPacketBuffer[nLen - 1] = nCRC;
}    // End ComputeCheckSumPublic

#if 0
static void GenerateInstanceTableReply(uint32_t * pPacketBuffer, uint32_t numInstances, uint32_t * pInstanceTable)
{
	pPacketBuffer[0] = ((3 + numInstances) << 16);
	pPacketBuffer[1] = numInstances;
	memcpy(&pPacketBuffer[2], (void *)pInstanceTable, numInstances * sizeof(uint32_t));
	ComputeCheckSumPublic(pPacketBuffer);
}	// End GenerateInstanceTableReply
#endif

static uint32_t* GenerateSimpleTuningInterfaceTestReply(uint32_t *pMsgIn, uint8_t channel_id)
{
    uint32_t *replyPtr = NULL;
    uint32_t msgLen = 0;
    uint32_t nCRC = 0;
    uint32_t i = 0;
    if (pMsgIn == 0) {
        return 0;
    }

    msgLen = pMsgIn[0] >> 16;
    replyPtr = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE, (uint16_t)RACE_SIMPLE_TEST, (uint16_t)sizeof(uint32_t) * msgLen, channel_id);

    replyPtr[0] = pMsgIn[0];
    nCRC ^= replyPtr[0];

    for (i = 1; i < (msgLen - 1); i++)
    {
        replyPtr[i] = ~pMsgIn[i];
        nCRC ^= replyPtr[i];
    }

    replyPtr[msgLen - 1] = nCRC;

    return replyPtr;
}

void* RACE_CmdHandler_SimpleTest(ptr_race_pkt_t pRaceHeaderCmd, uint16_t length, uint8_t channel_id)
{
    void* ptr = NULL;
    uint32_t *msgPtr = NULL;
    uint32_t *replyPtr = NULL;
    uint16_t msgLen = 0;
    int32_t opcode = 0;


    if (pRaceHeaderCmd->hdr.type == RACE_TYPE_COMMAND)
    {

        msgPtr = (uint32_t *)pRaceHeaderCmd->payload;
        msgLen = msgPtr[0] >> 16;
        opcode = PACKET_OPCODE(msgPtr);

        if (msgLen < 2 || msgLen > MAX_COMMAND_BUFFER_LEN)
        {
            RACE_LOG_MSGID_I("incorrect msgLen:%d",1, msgLen);

            // Illegal packet, construct a suitable error reply.
            replyPtr = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE, (uint16_t)RACE_SIMPLE_TEST, (uint16_t)sizeof(uint32_t) * 3, channel_id);

            replyPtr[0] = 3 << 16;
            replyPtr[1] = E_MESSAGE_LENGTH_TOO_LONG;
            ComputeCheckSumPublic(replyPtr);

            race_flush_packet((void *)replyPtr, channel_id);
        }
        else if (opcode == PFID_SimpleTuningInterfaceTest)
        {
            race_flush_packet((uint8_t *)GenerateSimpleTuningInterfaceTestReply(msgPtr, channel_id), channel_id);
        }
        else
        {
            RACE_LOG_MSGID_I("incorrect opcode:%d",1, opcode);
        }
    }

    return ptr;
}

#endif

/* End of file */

