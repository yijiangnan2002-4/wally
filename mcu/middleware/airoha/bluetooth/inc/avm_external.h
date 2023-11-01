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

#ifndef _AVM_EXTERNAL_H_
#define _AVM_EXTERNAL_H_

//#include "types.h"
//#include "bt_os_manager.h"
//#include "config.h"
//#include "bt_types.h"

//<<<=====================================================================>>>//
//<<                            INCLUDE HEADER FILES                       >>//
//<<<=====================================================================>>>//
#if 1

//<<<=====================================================================>>>//
//<<                            Constant                                  >>//
//<<<=====================================================================>>>//

/**
 * @brief    Bluetooth memory buffer types.
 * @{
 */
#if 0
typedef enum {
    BT_MEMORY_TX_BUFFER = 0,     /**< TX packet buffer, a buffer type for the Memory Management module.*/
    BT_MEMORY_RX_BUFFER,         /**< RX packet buffer, a buffer type for the Memory Management module.*/
    BT_MEMORY_CONTROLLER_BUFFER  /**< Controller buffer, a buffer type for the Memory Management module.*/
} bt_memory_packet_t;

typedef enum {
    BT_QUEUE_TYPE_RX = 0,
    BT_QUEUE_TYPE_TX_ACL,
    BT_QUEUE_TYPE_TX_CMD,
    BT_QUEUE_TYPE_TX_IF_PACKET
} bt_hb_queue_type_t;
#endif

typedef uint32_t BT_MON_MEMORY_TYPE;


/*structure for getting BT clock*/
typedef struct bt_stru_bttime {
    uint32_t period;
    uint16_t phase;
} BT_TIME_STRU, *BT_TIME_STRU_PTR;

#define     BT_MEMORY_TYPE_TX_BUFFER          0      /**< TX packet buffer, a buffer type for the Memory Management module.*/
#define     BT_MEMORY_TYPE_RX_BUFFER          1      /**< RX packet buffer, a buffer type for the Memory Management module.*/
#define     BT_MEMORY_TYPE_CONTROLLER_BUFFER  2      /**< Controller buffer, a buffer type for the Memory Management module.*/
#define     BT_MEMORY_TYPE_RR_BUFFER          3      /**< RR buffer, a buffer type for the Memory Management module.*/
#define     BT_MEMORY_TYPE_STATIC_BUFFER0     4      /**< Static buffer for controller RX buffer0, a buffer type for the Memory Management module.*/
#define     BT_MEMORY_TYPE_STATIC_BUFFER1     5      /**< Static buffer for controller RX buffer1, a buffer type for the Memory Management module.*/
#define     BT_MEMORY_TYPE_STATIC_BUFFER2     6      /**< Static buffer for controller RX buffer2, a buffer type for the Memory Management module.*/
#define     BT_MEMORY_TYPE_TX_BUFFER_POSTPONE 7      /**< TX packet buffer, a buffer type for the Memory Management module.*/
#define     BT_MEMORY_TYPE_CONTROLLER_TCM_BUFFER    8      /**< Controller buffer, a buffer type for the Memory Management module.*/

#define     BT_QUEUE_RX_TYPE                  0
#define     BT_QUEUE_TX_ACL_TYPE              1
#define     BT_QUEUE_TX_CMD_TYPE              2
#define     BT_QUEUE_TX_IF_PACKET_TYPE        3
#define     BT_QUEUE_TX_ISO_TYPE              4

/* GPT SYNC FEATURE */

typedef enum {
    BT_SYNC_USER_LED = 0,
    BT_SYNC_USER_SENSOR,
} bt_sync_user_type_t;

typedef enum {
    BT_SYNC_GPT_COUNT_32K = 1,
    BT_SYNC_GPT_COUNT_1M,
} bt_sync_gpt_type_t;

#define BT_SYNC_DEFAULT_HCI_HANDLE 0xFF

#define MAX_BT_SYNC_USER_NUM 2
#define MAX_BT_SYNC_EMP_LINK 3

#define PKA_BT_SYNC_TYPE_START   1
#define PKA_BT_SYNC_TYPE_INFO    2
#define PKA_BT_SYNC_TYPE_STOP    3
#define PKA_BT_SYNC_TYPE_REGULAR 4




typedef enum {
    BT_MON_MEMORY_TYPE_TX_BUFFER         = (BT_MON_MEMORY_TYPE)BT_MEMORY_TYPE_TX_BUFFER,
    BT_MON_MEMORY_TYPE_RX_BUFFER         = (BT_MON_MEMORY_TYPE)BT_MEMORY_TYPE_RX_BUFFER,
    BT_MON_MEMORY_TYPE_CONTROLLER_BUFFER = (BT_MON_MEMORY_TYPE)BT_MEMORY_TYPE_CONTROLLER_BUFFER,
    BT_MON_MEMORY_TYPE_RR_BUFFER         = (BT_MON_MEMORY_TYPE)BT_MEMORY_TYPE_RR_BUFFER,
    BT_MON_MEMORY_TYPE_STATIC_BUFFER0    = (BT_MON_MEMORY_TYPE)BT_MEMORY_TYPE_STATIC_BUFFER0,
    BT_MON_MEMORY_TYPE_STATIC_BUFFER1    = (BT_MON_MEMORY_TYPE)BT_MEMORY_TYPE_STATIC_BUFFER1,
    BT_MON_MEMORY_TYPE_STATIC_BUFFER2    = (BT_MON_MEMORY_TYPE)BT_MEMORY_TYPE_STATIC_BUFFER2,
    BT_MON_MEMORY_TYPE_TX_BUFFER_POSTPONE = (BT_MON_MEMORY_TYPE)BT_MEMORY_TYPE_TX_BUFFER_POSTPONE,
    BT_MON_MEMORY_TYPE_CONTROLLER_TCM_BUFFER = (BT_MON_MEMORY_TYPE)BT_MEMORY_TYPE_CONTROLLER_TCM_BUFFER,
    BT_MON_MEMORY_TYPE_NUM,
} bt_mon_memory_type_t;


typedef struct {
    uint16_t race_id;
    uint16_t tag;
    uint32_t native_clock;
    uint32_t native_phase;
    uint32_t dl_src_clock_offset;
    uint32_t dl_src_phase_offset;
    uint32_t valid;
} dchs_pka_latch_info_t;

enum dchs_role_t {
    DCHS_ROLE_NONE,
    DCHS_ROLE_MASTER,
    DCHS_ROLE_SLAVE,
};

//<<<=====================================================================>>>//
//<<                            Data Type                                 >>//
//<<<=====================================================================>>>//
typedef struct avm_pka_callbacks {
    unsigned char *(*bt_hb_mm_allocate)(uint8_t type, unsigned int size);
    void (*bt_hb_mm_free)(uint8_t type, unsigned char *ptr);
    void (*bt_hb_rx_enqueue)(unsigned char *hb_header);
    void *(*bt_hb_tx_dequeue)(uint8_t type, unsigned char *q_header);
    unsigned short (*bt_get_hb_header_size)(void);
    void (*bt_rx_notify_hb)(void);
} avm_pka_callbacks_t;

typedef struct stru_gpt_sync_request_info {
    uint8_t  type;
    uint8_t  lens;
    uint16_t reserved;
    uint8_t  data[32];
} GPT_SYNC_REQUEST_INFO_STRU, *GPT_SYNC_REQUEST_INFO_STRU_PTR;

/* GPT sync mechanism for LED */
typedef struct stru_bt_sync_timer_info {
    uint32_t            interval[MAX_BT_SYNC_EMP_LINK];           /* the interval between each SYNC_INFO */
    BT_TIME_STRU        n_clk[MAX_BT_SYNC_EMP_LINK];              /* native timer */
    BT_TIME_STRU        n_clk_regular[MAX_BT_SYNC_EMP_LINK];      /* native timer since register, callback regularly */
    uint16_t            hci_handle[MAX_BT_SYNC_EMP_LINK];         /* hci handle */
} BT_SYNC_TIMER_INFO_STRU, *BT_SYNC_TIMER_INFO_STRU_PTR;

typedef struct s_esco_a2dp_statistic {
    uint32_t esco_packet_cnt;
    uint32_t esco_bad_packet_cnt;
    uint32_t iso_packet_cnt;
    uint32_t iso_bad_packet_cnt;
} ESCO_ISO_STATISTIC_STRU, *pESCO_ISO_STATISTIC_STRU;

typedef void (*bt_afh_notification_callback)(unsigned char *avoid_map, uint8_t length);

/* GPT sync callback function pointer
    type : BT_SYNC_TYPE_START, BT_SYNC_TYPE_INFO, BT_SYNC_TYPE_STOP
    aws_flag : gLC_LinkCtrl.AwsCtrl.isSAwsLink
    hci_handle : hci handle
*/

typedef void (*bt_sync_callback_function_t)(uint8_t type, BT_TIME_STRU syncTime, uint32_t gpt_count, uint8_t aws_flag);
typedef void (*bt_sync_callback_function_w_handle_t)(uint8_t type, BT_TIME_STRU syncTime, uint32_t gpt_count, uint8_t aws_flag, uint16_t hci_handle);

//<<<=====================================================================>>>//
//<<                           Function Declaration                        >>//
//<<<=====================================================================>>>/ uint16_t length);
void bt_avm_pka_register_callbacks(avm_pka_callbacks_t callbacks);
unsigned short bt_get_pka_header_size(uint8_t type);
void bt_tx_notify_pka(uint8_t type, unsigned char *q_header);
unsigned char bt_pka_get_bt_clock(uint16_t hci_handle, BT_TIME_STRU_PTR current_bt_clk);
unsigned char bt_pka_get_bt_clock_with_gpt(uint16_t hci_handle, \
                                           BT_TIME_STRU_PTR current_bt_clk, uint32_t *gpt);
void pka_request_sync_gpt(GPT_SYNC_REQUEST_INFO_STRU_PTR sync_req_info_ptr, \
                          uint32_t duration, uint32_t timeout);
unsigned char bt_pka_allow_poweroff(void *data);
unsigned char* bt_pka_get_leaudio_AVM_addr(uint16_t size);
uint16_t bt_pka_get_leaudio_AVM_size(void);
uint16_t bt_pka_get_CIS_Required_AVM_size(uint16_t SDUSize, uint16_t PDUSize, uint8_t FT, uint8_t BN, uint8_t IsUL);
uint16_t bt_pka_get_BIS_Required_AVM_size(uint16_t SDUSize, uint16_t PDUSize, uint8_t BN, uint8_t NSE, uint8_t IRC, uint8_t PTO);
unsigned char* bt_pka_get_esco_forwarder_addr(uint16_t size);
uint8_t bt_pka_get_preferred_rate(uint16_t ConnHdl);
void bt_pka_dual_ant_register_callback(bt_afh_notification_callback callback);
uint16_t bt_pka_get_acl_queue_entries(uint16_t handle);
void bt_pka_set_no_retransmission_mode(uint16_t seq_num, uint16_t length);
uint8_t bt_pka_get_no_retransmission_mode_status();
void bt_pka_enable_power_control(uint8_t enable);
char *bt_pikachu_lib_verno(void);
char *bt_pikachu_lib_lastest_commit(void);
void bt_source_dongle_pka_allow_bqb(uint8_t bqb_status);


// @input : hci handle, a2dp l2cap channel, start_suspend
// @output 1 : success, 0 : fail
uint8_t bt_pka_set_a2dp_handle_channel(uint16_t handle, uint16_t a2dp_channel, uint8_t start_suspend);

// @input : N/A
// @output 1 : success, 0 : fail
uint8_t bt_pka_set_pause_sync(void);

// @input : N/A
// @output 1 : success, 0 : fail
uint8_t bt_pka_set_restart_sync(void);

uint16_t bt_pka_get_acl_queue_entries(uint16_t handle);

// @input : user, 0 for LED, 1 for sensor
// @input : callback
// @input : interval, the interval between sync info callback
// @input : hci_handle, hci handle to check
// @input : gpt_type, 1 for 32K, 2 for 1M
uint8_t bt_pka_sync_register_callback_with_handle(bt_sync_user_type_t user, bt_sync_callback_function_w_handle_t callback_w_handle,
    uint32_t interval, uint16_t hci_handle, bt_sync_gpt_type_t gpt_type);



// @input : user, LED for 0
// @input : callback
// @input : interval, the interval between sync info callback
uint8_t bt_pka_sync_register_callback(uint8_t user, bt_sync_callback_function_t callback, uint32_t interval);

//<<<=====================================================================>>>//
//<<                           External Reference                          >>//
//<<<=====================================================================>>>//
extern avm_pka_callbacks_t avm_callbacks;

//<<<=====================================================================>>>//
//<<                          External Function Reference                 >>//
//<<<=====================================================================>>>//

//<<<=====================================================================>>>//
//<<                            MACRO                                     >>//
//<<<=====================================================================>>>//



#endif
#endif // _AVM_EXTERNAL_H_

