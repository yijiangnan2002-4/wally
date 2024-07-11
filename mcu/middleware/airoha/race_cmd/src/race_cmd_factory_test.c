/**
wayne.xiong add for factory air test cmd
20200528

***/


#include "FreeRTOS.h"
#include "task.h"
#include "syslog.h"
#include "race_xport.h"


#include "apps_config_event_list.h"
#include "ui_shell_activity.h"
#include "apps_events_event_group.h"
#include "apps_events_key_event.h"

#include "app_preproc_activity.h"
#include "bt_type.h"
#include "battery_management.h"
#include "app_customer_common.h"
#include "race_cmd.h"
#include "race_cmd_factory_test.h"
#include "bt_sink_srv_a2dp.h"

#include "bt_connection_manager.h"
#include "bt_aws_mce.h"
#include "bt_sink_srv_ami.h"
#include "ui_shell_manager.h"
#include "apps_events_event_group.h"
#include "ui_shell_activity.h"
#include "apps_events_interaction_event.h"
#include "app_smcharger_utils.h"
#include "app_customer_nvkey_operation.h"
#include "bsp_px31bf.h"
#include "app_customer_common_activity.h"
#include "battery_management_HW_JEITA.h"
#include "apps_aws_sync_event.h"
#include "app_preproc_activity.h"
#include "app_hall_sensor_activity.h"
#include "race_util.h"


extern bt_bd_addr_t *bt_sink_srv_cm_get_aws_connected_device(void);

static uint8_t g_channel_id_save = 0;

void race_port_send_debug_data(uint8_t *buffer, uint32_t len)
{
	uint32_t port_handle = race_get_port_handle_by_channel_id(race_get_channel_id_by_port_type(RACE_SERIAL_PORT_TYPE_SPP));

	race_port_send_data(port_handle, buffer, len);
}

void* RACE_FACTORY_TEST_ROLE_HANDOVER_HDR(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
    typedef struct
	{
	    RACE_COMMON_HDR_STRU cmdhdr;
	}PACKED CMD;

	typedef struct
	{
		uint8_t status;
	}PACKED RSP;

	CMD *pCmd = (CMD *)pCmdMsg;
	RSP* pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE, (uint16_t)FACTORY_TEST_BLE_CMD_ROLE_HANDOVER, (uint16_t)sizeof(RSP), channel_id);
    int32_t ret = RACE_ERRCODE_SUCCESS;

    if (pEvt)
    {

        /* Do Role Handover */
        ui_shell_send_event(false, EVENT_PRIORITY_HIGNEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                            APPS_EVENTS_INTERACTION_TRIGGER_RHO, NULL, 0,
                            NULL, 100);

        pEvt->status = ret;
    }

	return pEvt;
}


void* RACE_FACTORY_TEST_EARBUD_READ_OUT_CASE_VERS(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
    typedef struct
	{
	    RACE_COMMON_HDR_STRU cmdhdr;
		uint8_t version_read;
	}PACKED CMD;

	typedef struct
	{
		uint8_t status;
	
		uint8_t case_version[8];
	}PACKED RSP;

	CMD* pCmd = (CMD *)pCmdMsg;
	RSP* pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE, (uint16_t)FACTORY_TEST_BLE_CMD_GET_VERSION, (uint16_t)sizeof(RSP), channel_id);
    int32_t ret = RACE_ERRCODE_SUCCESS;

	RACE_LOG_MSGID_I("RACE_FACTORY_TEST_EARBUD_READ_OUT_CASE_VERS cmd data = %x \r\n",1, pCmd->version_read);

    if (pEvt)
    {
		
    	if(pCmd->version_read == 0x1) 
		{
			app_get_charger_case_version(pEvt->case_version);
		}
		else
		{
			ret = RACE_ERRCODE_FAIL;		
		}

        pEvt->status = ret;
    }
	return pEvt;
}

#if 0
void* RACE_FACTORY_TEST_EARBUD_READ_OUT_AB1571D_VERS(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
	typedef struct
	{
		RACE_COMMON_HDR_STRU cmdhdr;
		uint8_t version_read;
	}PACKED CMD;

	typedef struct
	{
		uint8_t status;
		uint8_t ab1571d_version[4];
	}PACKED RSP;

	CMD* pCmd = (CMD *)pCmdMsg;
	RSP* pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE, (uint16_t)FACTORY_TEST_BLE_CMD_AB1571D_VERSION, (uint16_t)sizeof(RSP), channel_id);
    	int32_t ret = RACE_ERRCODE_SUCCESS;

	RACE_LOG_MSGID_I("RACE_FACTORY_TEST_EARBUD_READ_OUT_AB1571D_VERS cmd data = %x \r\n",1, pCmd->version_read);

	if (pEvt)
	{
		if(pCmd->version_read == 0x1) 
		{
			app_get_ab1571d_version(pEvt->ab1571d_version);
		}
		else
		{
			ret = RACE_ERRCODE_FAIL;		
		}
		pEvt->status = ret;
	}
	
	return pEvt;
}
#endif

#if 0
void* RACE_FACTORY_TEST_BLE_COLOR_ID(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{	
	 typedef struct
	{
	    RACE_COMMON_HDR_STRU cmdhdr;
		uint8_t WR;
		uint8_t color_id;
	}PACKED CMD;

	typedef struct
	{
		uint8_t status;
		uint8_t action;
		uint8_t color_rsp;
	}PACKED RSP;

	CMD* pCmd = (CMD *)pCmdMsg;
	RSP* pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE, (uint16_t)FACTORY_TEST_BLE_CMD_DEVICE_COLOR, (uint16_t)sizeof(RSP), channel_id);
    int32_t ret = RACE_ERRCODE_SUCCESS;

	RACE_LOG_MSGID_I("RACE_FACTORY_TEST_BLE_COLOR_ID cmd data = %x \r\n",1, pCmd->color_id);

    if (pEvt)
    {
    
		pEvt->action = pCmd->WR;
		
    	if(pCmd->WR == 0x00)
		{
			pEvt->color_rsp = app_nvkey_ble_color_id_read();
		}
		else if(pCmd->WR == 0x01)
		{
			pEvt->color_rsp = pCmd->color_id;		
			app_set_ble_device_color_state(pCmd->color_id);
			ui_shell_send_event(false, EVENT_PRIORITY_HIGNEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                            APPS_EVENTS_INTERACTION_REQUEST_REBOOT, NULL, 0,
                                            NULL, 1000);
		}
		else
		{
			ret = RACE_ERRCODE_FAIL;
		}
        pEvt->status = ret;
    }
	return pEvt;
	
}
#endif

void* RACE_FACTORY_TEST_TOUCH_CTR(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{	
	 typedef struct
	{
	    RACE_COMMON_HDR_STRU cmdhdr;
		uint8_t ctr;
	}PACKED CMD;

	typedef struct
	{
		uint8_t status;
		uint8_t action;
	}PACKED RSP;

	CMD* pCmd = (CMD *)pCmdMsg;
	RSP* pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE, (uint16_t)FACTORY_TEST_BLE_CMD_TOUCH_TEST_CTR, (uint16_t)sizeof(RSP), channel_id);
    int32_t ret = RACE_ERRCODE_SUCCESS;

    if (pEvt)
    {
    
		pEvt->action = pCmd->ctr;
		
    	if(pCmd->ctr <= 0x01)
		{
			app_touch_key_test_status_set(pCmd->ctr);
		}
		else
		{
			ret = RACE_ERRCODE_FAIL;
		}
        pEvt->status = ret;
    }
	return pEvt;
	
}

#if 0
void* RACE_FACTORY_TEST_BLE_MINI_UI(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{	
	 typedef struct
	{
	    RACE_COMMON_HDR_STRU cmdhdr;
		uint8_t WR;
		uint8_t mini_ui;
	}PACKED CMD;

	typedef struct
	{
		uint8_t status;
		uint8_t action;
		uint8_t ui_rsp;
	}PACKED RSP;

	CMD* pCmd = (CMD *)pCmdMsg;
	RSP* pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE, (uint16_t)FACTORY_TEST_BLE_CMD_MINI_UI, (uint16_t)sizeof(RSP), channel_id);
    int32_t ret = RACE_ERRCODE_SUCCESS;

    if (pEvt)
    {
    
		pEvt->action = pCmd->WR;
		
    	if(pCmd->WR == 0x00)
		{
			pEvt->ui_rsp = app_nvkey_ble_mini_ui_read();
		}
		else if(pCmd->WR == 0x01)
		{
			pEvt->ui_rsp = pCmd->mini_ui;

			//if(bt_sink_srv_cm_get_aws_connected_device() != 0)
			//{
				app_set_ble_device_mini_ui_state(pCmd->mini_ui);
			//}
			//else
			//	ret = RACE_ERRCODE_FAIL;
		}
		else
		{
			ret = RACE_ERRCODE_FAIL;
		}
        pEvt->status = ret;
    }
	return pEvt;
	
}
#endif
void* RACE_FACTORY_TEST_HX300X_PS_DATA_READ(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{	
	 typedef struct
	{
	    RACE_COMMON_HDR_STRU cmdhdr;
	}PACKED CMD;

	typedef struct
	{
		uint8_t status;
		uint16_t action_result;
	}PACKED RSP;

	CMD* pCmd = (CMD *)pCmdMsg;
	RSP* pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE, (uint16_t)FACTORY_TEST_BLE_CMD_HX300X_PS_DATA_READ, (uint16_t)sizeof(RSP), channel_id);
    int32_t ret = RACE_ERRCODE_SUCCESS;

    if (pEvt)
    {
		pEvt->action_result = hx300x_read_ps_data();

        pEvt->status = ret;
    }
	return pEvt;
	
}

void* RACE_FACTORY_TEST_HX300X_IIC_INT_CHECK(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{	
	 typedef struct
	{
	    RACE_COMMON_HDR_STRU cmdhdr;
	}PACKED CMD;

	typedef struct
	{
		uint8_t status;
		uint8_t action_result;
	}PACKED RSP;

	CMD* pCmd = (CMD *)pCmdMsg;
	RSP* pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE, (uint16_t)FACTORY_TEST_BLE_CMD_HX300X_IIC_INT_CHECK, (uint16_t)sizeof(RSP), channel_id);
    int32_t ret = RACE_ERRCODE_SUCCESS;

    if (pEvt)
    {
		pEvt->action_result = Calibration_First_IIC_INT_check();

        pEvt->status = ret;
    }
	return pEvt;
	
}


void* RACE_FACTORY_TEST_HX300X_OPEN_AIR(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{	
	 typedef struct
	{
	    RACE_COMMON_HDR_STRU cmdhdr;
	}PACKED CMD;

	typedef struct
	{
		uint8_t status;
		uint16_t action_result;
	}PACKED RSP;

	CMD* pCmd = (CMD *)pCmdMsg;
	RSP* pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE, (uint16_t)FACTORY_TEST_BLE_CMD_HX300X_OPEN_AIR, (uint16_t)sizeof(RSP), channel_id);
    int32_t ret = RACE_ERRCODE_SUCCESS;

    if (pEvt)
    {
		pEvt->action_result = Calibration_Second_OpenAir();

        pEvt->status = ret;
    }
	return pEvt;
	
}

void* RACE_FACTORY_TEST_HX300X_IN_EAR(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{	
	 typedef struct
	{
	    RACE_COMMON_HDR_STRU cmdhdr;
	}PACKED CMD;

	typedef struct
	{
		uint8_t status;
		uint16_t ps_data;
		uint16_t ps_k;
	}PACKED RSP;

	CMD* pCmd = (CMD *)pCmdMsg;
	RSP* pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE, (uint16_t)FACTORY_TEST_BLE_CMD_HX300X_IN_EAR, (uint16_t)sizeof(RSP), channel_id);
    int32_t ret = RACE_ERRCODE_SUCCESS;

    if (pEvt)
    {		
		Calibration_Third_GreyCard((void*)pEvt);
        pEvt->status = ret;
    }
	return pEvt;
	
}


void* RACE_FACTORY_TEST_HX300X_OUT_EAR(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{	
	 typedef struct
	{
	    RACE_COMMON_HDR_STRU cmdhdr;
	}PACKED CMD;

	typedef struct
	{
		uint8_t status;
		uint8_t data_reg10;
		uint8_t data_reg12;
		uint8_t data_reg13;
		uint16_t data_h_thod;
		uint16_t data_str_diff;
		uint16_t data_read_ps3;
	}PACKED RSP;


	CMD* pCmd = (CMD *)pCmdMsg;
	RSP* pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE, (uint16_t)FACTORY_TEST_BLE_CMD_HX300X_OUT_EAR, (uint16_t)sizeof(RSP), channel_id);
    int32_t ret = RACE_ERRCODE_SUCCESS;

    if (pEvt)
    {
		Calibration_Fourth_RemoveGreyCard((void*)pEvt);
        pEvt->status = ret;
    }
	return pEvt;
	
}

void* RACE_FACTORY_TEST_HX300X_DATA_SAVE(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{	
	 typedef struct
	{
	    RACE_COMMON_HDR_STRU cmdhdr;
	}PACKED CMD;

	typedef struct
	{
		uint8_t status;
		uint8_t action_result;
	}PACKED RSP;

	CMD* pCmd = (CMD *)pCmdMsg;
	RSP* pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE, (uint16_t)FACTORY_TEST_BLE_CMD_HX300X_DATA_SAVE, (uint16_t)sizeof(RSP), channel_id);
    int32_t ret = RACE_ERRCODE_SUCCESS;

    if (pEvt)
    {
		pEvt->action_result = Calibration_Fifth_SaveDataToFlash();
        pEvt->status = ret;
    }
	return pEvt;
	
}


void* RACE_FACTORY_TEST_HX300X_READ_CALI_SETTING(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{	
	 typedef struct
	{
	    RACE_COMMON_HDR_STRU cmdhdr;
	}PACKED CMD;

	typedef struct
	{
		uint8_t status;
		uint8_t data_reg10;
		uint8_t data_reg12;
		uint8_t data_reg13;
		uint16_t data_h_thod;
		uint16_t data_l_thod;
		uint16_t data_read_ps3;
	}PACKED RSP;


	CMD* pCmd = (CMD *)pCmdMsg;
	RSP* pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE, (uint16_t)FACTORY_TEST_BLE_CMD_HX300X_READ_CALI_SETTING, (uint16_t)sizeof(RSP), channel_id);
    int32_t ret = RACE_ERRCODE_SUCCESS;

    if (pEvt)
    {
		app_nvkey_hx300x_read_cali_setting((void*)pEvt);
        pEvt->status = ret;
    }
	return pEvt;
	
}

void* RACE_FACTORY_TEST_HX300X_DEBUG(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
    typedef struct
	{
	    RACE_COMMON_HDR_STRU cmdhdr;
		uint8_t log_enable;
	}PACKED CMD;

	typedef struct
	{
		uint8_t status;
	}PACKED RSP;

	CMD* pCmd = (CMD *)pCmdMsg;
	RSP* pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE, (uint16_t)FACTORY_TEST_BLE_CMD_HX300X_DEBUG_SWITCH, (uint16_t)sizeof(RSP), channel_id);
    int32_t ret = RACE_ERRCODE_SUCCESS;

    if (pEvt)
    {
		app_set_hx300x_debug_state(pCmd->log_enable);

        pEvt->status = ret;
    }
	return pEvt;
}

void* RACE_FACTORY_TEST_HX300X_REG_CTR(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
    typedef struct
	{
	    RACE_COMMON_HDR_STRU cmdhdr;
		uint8_t WR;
		uint8_t reg_addr;
		uint8_t reg_data;
	}PACKED CMD;

	typedef struct
	{
		uint8_t status;
		uint8_t ret_reg_addr;
		uint8_t ret_reg_data;
	}PACKED RSP;

	CMD* pCmd = (CMD *)pCmdMsg;
	RSP* pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE, (uint16_t)FACTORY_TEST_BLE_CMD_HX300X_REG_CTR, (uint16_t)sizeof(RSP), channel_id);
    int32_t ret = RACE_ERRCODE_SUCCESS;

    if (pEvt)
    {
    	if(pCmd->WR)
		{
			pEvt->status = (uint8_t)HX300X_reg_ctr_write(pCmd->reg_addr, pCmd->reg_data);
			if(pEvt->status == RACE_ERRCODE_SUCCESS)
			{
				pEvt->ret_reg_addr = pCmd->reg_addr;
				pEvt->ret_reg_data = pCmd->reg_data;				
			}
			else
			{
				pEvt->ret_reg_addr = 0;
				pEvt->ret_reg_data = 0;				
			}
		}
		else
		{
			pEvt->status = (uint8_t)HX300X_reg_ctr_read(pCmd->reg_addr, &pEvt->ret_reg_data);
			if(pEvt->status == RACE_ERRCODE_SUCCESS)
			{
				pEvt->ret_reg_addr = pCmd->reg_addr;
			}
			else
			{
				pEvt->ret_reg_addr = 0;
				pEvt->ret_reg_data = 0;				
			}			
		}

    }
	return pEvt;
}


void* RACE_FACTORY_TEST_HX300X_READ_ALL_REG(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{	
	 typedef struct
	{
	    RACE_COMMON_HDR_STRU cmdhdr;
	}PACKED CMD;

	typedef struct
	{
		uint8_t status;
	}PACKED RSP;


	CMD* pCmd = (CMD *)pCmdMsg;
	RSP* pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE, (uint16_t)FACTORY_TEST_BLE_CMD_HX300X_READ_ALL_REG, (uint16_t)sizeof(RSP), channel_id);
    int32_t ret = RACE_ERRCODE_SUCCESS;

    if (pEvt)
    {
		pEvt->status = ret;
		app_read_all_reg();
    }
	return pEvt;
	
}


void* RACE_FACTORY_TEST_LEA_DISABLE(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{	
	 typedef struct
	{
	    RACE_COMMON_HDR_STRU cmdhdr;
		uint8_t WR;
		uint8_t lea_disable;
	}PACKED CMD;

	typedef struct
	{
		uint8_t status;
		uint8_t action;
		uint8_t lea_status_rsp;
	}PACKED RSP;

	CMD* pCmd = (CMD *)pCmdMsg;
	RSP* pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE, (uint16_t)FACTORY_TEST_BLE_CMD_LEA_DISABLE, (uint16_t)sizeof(RSP), channel_id);
    int32_t ret = RACE_ERRCODE_SUCCESS;

    if (pEvt)
    {
    
		pEvt->action = pCmd->WR;
		
    	if(pCmd->WR == 0x00)
		{
			pEvt->lea_status_rsp = app_nvkey_is_lea_disable();
		}
		else if(pCmd->WR == 0x01 && bt_sink_srv_cm_get_aws_connected_device() != NULL)
		{
			pEvt->lea_status_rsp = pCmd->lea_disable;

			app_set_lea_disable_state(pCmd->lea_disable);
		}
		else
		{
			pEvt->lea_status_rsp = 0xFF;
			ret = RACE_ERRCODE_FAIL;
		}
        pEvt->status = ret;
    }
	return pEvt;
	
}


void* RACE_FACTORY_TEST_TOUCH_LIMIT_DISABLE_SET(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{	
	 typedef struct
	{
	    RACE_COMMON_HDR_STRU cmdhdr;
		uint8_t WR;
		uint8_t touch_limit_disable;
	}PACKED CMD;

	typedef struct
	{
		uint8_t status;
		uint8_t action;
		uint8_t touch_limit_disable_rsp;
	}PACKED RSP;

	CMD* pCmd = (CMD *)pCmdMsg;
	RSP* pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE, (uint16_t)FACTORY_TEST_BLE_CMD_TOUCH_LIMIT_DISABLE, (uint16_t)sizeof(RSP), channel_id);
    int32_t ret = RACE_ERRCODE_SUCCESS;

    if (pEvt)
    {
    
		pEvt->action = pCmd->WR;
		
    	if(pCmd->WR == 0x00)
		{
			pEvt->touch_limit_disable_rsp = app_nvkey_ble_touch_limit_disable_read();
		}
		else if(pCmd->WR == 0x01)
		{
			pEvt->touch_limit_disable_rsp = pCmd->touch_limit_disable;

			app_nvkey_ble_touch_limit_disable_set(pCmd->touch_limit_disable);
		}
		else
		{
			ret = RACE_ERRCODE_FAIL;
		}
        pEvt->status = ret;
    }
	return pEvt;
	
}

#if 0
void* RACE_FACTORY_TEST_BAT_HEATHY_PERCENT2TIME_SET(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{	
	 typedef struct
	{
	    RACE_COMMON_HDR_STRU cmdhdr;
		uint8_t WR;
		uint8_t percent2time;
	}PACKED CMD;

	typedef struct
	{
		uint8_t status;
		uint8_t action;
		uint8_t percent2time_rsp;
	}PACKED RSP;

	CMD* pCmd = (CMD *)pCmdMsg;
	RSP* pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE, (uint16_t)FACTORY_TEST_BLE_CMD_BAT_HEATHY_PERCENT2TIME_SET, (uint16_t)sizeof(RSP), channel_id);
    int32_t ret = RACE_ERRCODE_SUCCESS;

    if (pEvt)
    {
    
		pEvt->action = pCmd->WR;
		
    	if(pCmd->WR == 0x00)
		{
			pEvt->percent2time_rsp = app_nvkey_battery_heathy_percent2time_read();
		}
		else if(pCmd->WR == 0x01)
		{
			pEvt->percent2time_rsp = pCmd->percent2time;

			app_nvkey_battery_heathy_percent2time_set(pCmd->percent2time);

		}
		else
		{
			ret = RACE_ERRCODE_FAIL;
		}
        pEvt->status = ret;
    }
	return pEvt;
	
}

void* RACE_FACTORY_TEST_BAT_HEATHY_CTR(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{	
	 typedef struct
	{
	    RACE_COMMON_HDR_STRU cmdhdr;
		uint8_t WR;
	}PACKED CMD;

	typedef struct
	{
		uint8_t status;
		uint8_t action;
		uint16_t bat_heathy_L;
		uint16_t bat_heathy_R;
	}PACKED RSP;

	CMD* pCmd = (CMD *)pCmdMsg;
	RSP* pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE, (uint16_t)FACTORY_TEST_BLE_CMD_BAT_HEATHY_CTR, (uint16_t)sizeof(RSP), channel_id);
    int32_t ret = RACE_ERRCODE_SUCCESS;

    if (pEvt)
    {
    	
    	audio_channel_t channel = ami_get_audio_channel();
		
		pEvt->action = pCmd->WR;
		
    	if(pCmd->WR == 0x00)
		{
			if(AUDIO_CHANNEL_L == channel)
			{
				pEvt->bat_heathy_L = app_nvkey_battery_heathy_read();
				if(bt_sink_srv_cm_get_aws_connected_device())
					pEvt->bat_heathy_R = app_get_peer_battery_heathy_times();
				else
					pEvt->bat_heathy_R = 0;
			}
			else
			{
				pEvt->bat_heathy_R = app_nvkey_battery_heathy_read();
				if(bt_sink_srv_cm_get_aws_connected_device())
					pEvt->bat_heathy_L = app_get_peer_battery_heathy_times();
				else
					pEvt->bat_heathy_L = 0;
			}


		}
		else if(pCmd->WR == 0x01)
		{
			pEvt->bat_heathy_L = 0;
			pEvt->bat_heathy_R = 0;
			app_nvkey_battery_heathy_write(0, true);

		}
		else
		{
			ret = RACE_ERRCODE_FAIL;
		}
        pEvt->status = ret;
    }
	return pEvt;
	
}
#endif

void* RACE_FACTORY_TEST_HW_VERSION(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{	
	 typedef struct
	{
	    RACE_COMMON_HDR_STRU cmdhdr;
		uint8_t WR;
		uint8_t ver[5];
	}PACKED CMD;

	typedef struct
	{
		uint8_t status;
		uint8_t action;
		uint8_t ver_rsp[5];
	}PACKED RSP;

	CMD* pCmd = (CMD *)pCmdMsg;
	RSP* pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE, (uint16_t)FACTORY_TEST_BLE_CMD_HW_VERSION, (uint16_t)sizeof(RSP), channel_id);
    int32_t ret = RACE_ERRCODE_SUCCESS;

    if (pEvt)
    {
    
		pEvt->action = pCmd->WR;
		
    	if(pCmd->WR == 0x00)
		{
			app_nvkey_hw_version_read(pEvt->ver_rsp, 5);
		}
		else if(pCmd->WR == 0x01)
		{
			memmove(pEvt->ver_rsp, pCmd->ver, 5);
			app_set_hw_version(pCmd->ver, 5);
		}
		else
		{
			ret = RACE_ERRCODE_FAIL;
		}
        pEvt->status = ret;
    }
	return pEvt;
	
}
#define SN_LEN  10
void* RACE_FACTORY_TEST_SERIAL_NUMBER(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
	 typedef struct
	{
	    RACE_COMMON_HDR_STRU cmdhdr;
		uint8_t WR;
		uint8_t ver[SN_LEN];
	}PACKED CMD;

	typedef struct
	{
		uint8_t status;
		uint8_t action;
		uint8_t ver_rsp[SN_LEN];
	}PACKED RSP;

	CMD* pCmd = (CMD *)pCmdMsg;
	RSP* pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE, (uint16_t)FACTORY_TEST_BLE_CMD_SERIAL_NUMBER, (uint16_t)sizeof(RSP), channel_id);
    int32_t ret = RACE_ERRCODE_SUCCESS;

    if (pEvt)
    {
    
		pEvt->action = pCmd->WR;
		
    	if(pCmd->WR == 0x00)
		{
			app_nvkey_sn_read(pEvt->ver_rsp, SN_LEN);
		}
		else if(pCmd->WR == 0x01)
		{
			if(bt_sink_srv_cm_get_aws_connected_device() != 0)
			{
				memmove(pEvt->ver_rsp, pCmd->ver, SN_LEN);  
				app_set_ble_write_SN(pCmd->ver, SN_LEN);
			}
			else
			{
				memset(pEvt->ver_rsp, 0x00, SN_LEN);
				ret = RACE_ERRCODE_FAIL;
			}
		}
		else if(pCmd->WR == 0x02)
		{
			memmove(pEvt->ver_rsp, pCmd->ver, SN_LEN);	
			app_nvkey_sn_set(pCmd->ver, SN_LEN);
		}
		else
		{
			ret = RACE_ERRCODE_FAIL;
		}
        pEvt->status = ret;
    }
	return pEvt;

}

void* RACE_FACTORY_TEST_PSENSOR_PUW_SET(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{	
	 typedef struct
	{
	    RACE_COMMON_HDR_STRU cmdhdr;
		uint8_t psensor_setting[4];
	}PACKED CMD;

	typedef struct
	{
		uint8_t status;
	}PACKED RSP;

	CMD* pCmd = (CMD *)pCmdMsg;
	RSP* pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE, (uint16_t)FACTORY_TEST_BLE_CMD_PSENSOR_PUW_SET, (uint16_t)sizeof(RSP), channel_id);
    int32_t ret = RACE_ERRCODE_SUCCESS;

    if (pEvt)
    {		
    	if(pCmd->psensor_setting[0] < 0x0F && pCmd->psensor_setting[1] < 0x07 && pCmd->psensor_setting[2] < 0x0F)
		{
			app_nvkey_psensor_setting_set(pCmd->psensor_setting);
		}
		else
		{
			ret = RACE_ERRCODE_FAIL;
		}
        pEvt->status = ret;
    }
	return pEvt;
	
}

#if 0
void* RACE_FACTORY_TEST_SET_ECO_CHARGING_PROFILE_STATE(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
    typedef struct
	{
	    RACE_COMMON_HDR_STRU cmdhdr;
		uint8_t WR;
		uint8_t eco_charging_profile;
	}PACKED CMD;

	typedef struct
	{
		uint8_t status;
		uint8_t action;
		uint8_t profile_set;
	}PACKED RSP;

	CMD* pCmd = (CMD *)pCmdMsg;
	RSP* pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE, (uint16_t)FACTORY_TEST_SET_ECO_PROFILE, (uint16_t)sizeof(RSP), channel_id);
    int32_t ret = RACE_ERRCODE_SUCCESS;

	RACE_LOG_MSGID_I("RACE_FACTORY_TEST_SET_ECO_CHARGING_PROFILE_STATE cmd data = %x \r\n",1, pCmd->eco_charging_profile);

    if (pEvt)
    {
    
		pEvt->action = pCmd->WR;
		
    	if(pCmd->WR == 0x1)
		{
			pEvt->profile_set = pCmd->eco_charging_profile;
			
	    	if(pCmd->eco_charging_profile <= 0x0F) 
			{
				app_set_eco_charing_profile_switch(pCmd->eco_charging_profile);
			}
			else
			{
				ret = RACE_ERRCODE_FAIL;		
			}
		}
		else if(pCmd->WR == 0x0)
		{
			pEvt->profile_set = app_get_eco_charing_profile_switch();
		}
		else
		{
			ret = RACE_ERRCODE_FAIL;
		}

        pEvt->status = ret;
    }
	return pEvt;
}
#endif

void* RACE_FACTORY_TEST_SHIPPING_MODE(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
    typedef struct
	{
	    RACE_COMMON_HDR_STRU cmdhdr;
		uint8_t WR;
		uint8_t shipping_mode_set;
	}PACKED CMD;

	typedef struct
	{
		uint8_t status;
		uint8_t action;
		uint8_t shipping_mode_state;
	}PACKED RSP;

	CMD* pCmd = (CMD *)pCmdMsg;
	RSP* pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE, (uint16_t)FACTORY_TEST_BLE_CMD_SHIPPING_MODE, (uint16_t)sizeof(RSP), channel_id);
    int32_t ret = RACE_ERRCODE_SUCCESS;

	RACE_LOG_MSGID_I("FACTORY_TEST_BLE_CMD_SHIPPING_MODE cmd data = %x \r\n",1, pCmd->shipping_mode_set);

    if (pEvt)
    {
    
		pEvt->action = pCmd->WR;
		
    	if(pCmd->WR == 0x1)
		{
			pEvt->shipping_mode_state = pCmd->shipping_mode_set;
			
	    	if(pCmd->shipping_mode_set <= 0x01) 
			{
				app_set_shipping_mode_state(pCmd->shipping_mode_set);
			}
			else
			{
				ret = RACE_ERRCODE_FAIL;		
			}
		}
		else if(pCmd->WR == 0x0)
		{
			pEvt->shipping_mode_state = app_get_shipping_mode_state();
		}
		else
		{
			ret = RACE_ERRCODE_FAIL;
		}

        pEvt->status = ret;
    }
	return pEvt;
}

void* RACE_FACTORY_TEST_GFP_TX_POWER_SET(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
    typedef struct
	{
	    RACE_COMMON_HDR_STRU cmdhdr;
		int8_t tx_power;
	}PACKED CMD;

	typedef struct
	{
		uint8_t status;
		int8_t rsp_tx_power;
	}PACKED RSP;

	CMD* pCmd = (CMD *)pCmdMsg;
	RSP* pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE, (uint16_t)FACTORY_TEST_BLE_CMD_GFP_TX_POWER_SET, (uint16_t)sizeof(RSP), channel_id);
    int32_t ret = RACE_ERRCODE_SUCCESS;

    if (pEvt)
    {
    		
		pEvt->rsp_tx_power = pCmd->tx_power;
#ifdef AIR_BT_FAST_PAIR_ENABLE		
		app_fast_pair_set_tx_power_level(pCmd->tx_power);
#endif
	    uint16_t *p_key_action = (uint16_t *)pvPortMalloc(sizeof(uint16_t)); /* free by ui shell */

		*p_key_action = KEY_SYSTEM_REBOOT;

	    ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_KEY, INVALID_KEY_EVENT_ID, p_key_action,
	                        sizeof(uint16_t), NULL, 500);		

        pEvt->status = ret;
    }
	return pEvt;
}

void* RACE_FACTORY_TEST_SET_A2DP_VOLUME(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
    typedef struct
	{
	    RACE_COMMON_HDR_STRU cmdhdr;
		uint8_t volume_level;
	}PACKED CMD;

	typedef struct
	{
		uint8_t status;
	
		uint8_t volume_set;
	}PACKED RSP;

	CMD* pCmd = (CMD *)pCmdMsg;
	RSP* pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE, (uint16_t)FACTORY_TEST_SET_A2DP_VOLUME, (uint16_t)sizeof(RSP), channel_id);
    int32_t ret = RACE_ERRCODE_SUCCESS;

	RACE_LOG_MSGID_I("RACE_FACTORY_TEST_SET_A2DP_VOL volume level = %x \r\n",1, pCmd->volume_level);

    if (pEvt)
    {
		pEvt->volume_set = pCmd->volume_level;
		
    	if(pCmd->volume_level >= 0x0 && pCmd->volume_level <= BT_SINK_SRV_A2DP_MAX_VOL_LEV) 
		{
			app_set_a2dp_volume(pCmd->volume_level);
		}
		else
		{
			ret = RACE_ERRCODE_FAIL;		
		}

        pEvt->status = ret;
    }
	return pEvt;
}

extern void key_send_address_proc(void);
void* RACE_FACTORY_TEST_BLE_CMD_BT_ADDR_SET(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
	typedef struct
	{
		RACE_COMMON_HDR_STRU cmdhdr;
		uint8_t set_flag;
	}PACKED CMD;

	typedef struct
	{
		uint8_t status;
		uint8_t set_value;
	}PACKED RSP;

	CMD* pCmd = (CMD *)pCmdMsg;
	RSP* pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE, (uint16_t)FACTORY_TEST_BLE_CMD_BT_ADDR_SET, (uint16_t)sizeof(RSP), channel_id);
	int32_t ret = RACE_ERRCODE_SUCCESS;

	RACE_LOG_MSGID_I("RACE_FACTORY_TEST_BLE_CMD_BT_ADDR_SET set_falg = %x \r\n",1, pCmd->set_flag);

	if (pEvt)
	{
		pEvt->set_value = pCmd->set_flag;

		if(pCmd->set_flag == 0x1) 
		{
			key_send_address_proc();
		}
		else
		{
			ret = RACE_ERRCODE_FAIL;		
		}
		pEvt->status = ret;
	}
	return pEvt;
}

void* RACE_FACTORY_TEST_CHANNEL_CHECK_PARAMETER_HDR(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
    typedef struct
	{
	    RACE_COMMON_HDR_STRU cmdhdr;
		uint8_t param;
	}PACKED CMD;

	typedef struct
	{
		uint8_t status;
	
		uint8_t data[1];
	}PACKED RSP;

	CMD *pCmd = (CMD *)pCmdMsg;
	RSP* pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE, (uint16_t)FACTORY_TEST_BLE_CMD_CHANNEL_CHECK, (uint16_t)sizeof(RSP), channel_id);
    int32_t ret = RACE_ERRCODE_SUCCESS;

	RACE_LOG_MSGID_I("cmd data = %x \r\n",1, pCmd->param);

    if (pEvt)
    {
    	pEvt->data[0] = ami_get_audio_channel();

		if(pEvt->data[0] == 0)
		{
			pEvt->data[0] = 0x01;
		}
		else if(pEvt->data[0] == 1)
		{
			pEvt->data[0] = 0x02;
		}
		else
			ret = RACE_ERRCODE_FAIL;

        pEvt->status = ret;
    }

	return pEvt;
}

void* RACE_FACTORY_TEST_HALL_STATUS_CHECK(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
#if 0
    typedef struct
	{
	    RACE_COMMON_HDR_STRU cmdhdr;
		uint8_t param;
	}PACKED CMD;
#endif

	typedef struct
	{
		uint8_t status;
	
		uint8_t data[1];
	}PACKED RSP;

	//CMD *pCmd = (CMD *)pCmdMsg;
	RSP* pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE, (uint16_t)FACTORY_TEST_BLE_CMD_HALL_CHECK, (uint16_t)sizeof(RSP), channel_id);
    int32_t ret = RACE_ERRCODE_SUCCESS;


    if (pEvt)
    {
    	pEvt->data[0] = get_hall_sensor_status();

        pEvt->status = ret;
    }

	return pEvt;
}

void* RACE_FACTORY_TEST_TOUCH_KEY_READ(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
#if 0
    typedef struct
	{
	    RACE_COMMON_HDR_STRU cmdhdr;
		uint8_t param;
	}PACKED CMD;
#endif

	typedef struct
	{
		uint8_t status;
	
		uint8_t data[1];
	}PACKED RSP;

	//CMD *pCmd = (CMD *)pCmdMsg;
	RSP* pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE, (uint16_t)FACTORY_TEST_BLE_CMD_TOUCH_READ, (uint16_t)sizeof(RSP), channel_id);
    int32_t ret = RACE_ERRCODE_SUCCESS;


    if (pEvt)
    {
    	pEvt->data[0] = app_touch_key_test_read();

		app_touch_key_test_status_set(0x00);

        pEvt->status = ret;
    }

	return pEvt;
}



void* RACE_FACTORY_TEST_SN_WRITE_RESULT_CHECK(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
#if 0
    typedef struct
	{
	    RACE_COMMON_HDR_STRU cmdhdr;
		uint8_t param;
	}PACKED CMD;
#endif

	typedef struct
	{
		uint8_t status;
	
		uint8_t data[1];
	}PACKED RSP;

	//CMD *pCmd = (CMD *)pCmdMsg;
	RSP* pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE, (uint16_t)FACTORY_TEST_BLE_CMD_WRITE_SN_SYNC_RESULT, (uint16_t)sizeof(RSP), channel_id);
    int32_t ret = RACE_ERRCODE_SUCCESS;

    if (pEvt)
    {
    	pEvt->data[0] = app_write_sn_compare_result_read();

        pEvt->status = ret;
    }

	return pEvt;
}

#if 0
void* RACE_FACTORY_TEST_MFI_CHECK(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
#if 0
    typedef struct
	{
	    RACE_COMMON_HDR_STRU cmdhdr;
		uint8_t param;
	}PACKED CMD;
#endif

	typedef struct
	{
		uint8_t status;	
	}PACKED RSP;

	//CMD *pCmd = (CMD *)pCmdMsg;
	RSP* pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE, (uint16_t)FACTORY_TEST_BLE_CMD_MFI_CHECK, (uint16_t)sizeof(RSP), channel_id);
    int32_t ret = RACE_ERRCODE_SUCCESS;

    if (pEvt)
    {
	    g_channel_id_save = channel_id;
		ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_CUSTOMER_COMMON,
							EVENT_ID_DEVICE_MFI_CHECK, NULL, 0, NULL, 0);

        pEvt->status = ret;
    }

	return pEvt;
}

void RACE_FACTORY_TEST_MFI_STATUS_NOTIFY(uint8_t data)
{
	typedef struct
	{
		uint8_t status;
	
	}PACKED RSP;

	RSP* pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_NOTIFICATION, (uint16_t)FACTORY_TEST_BLE_CMD_MFI_CHECK, (uint16_t)sizeof(RSP), g_channel_id_save);

	RACE_LOG_MSGID_I("MFI_STATUS channel id: %d, status info len=%d\r\n",2, g_channel_id_save,data);

	if (pEvt)
	{
		void * ptr = NULL;
		race_send_pkt_t* ptr_send = NULL;
		
		pEvt->status = data;

        ptr = (void *)race_pointer_cnv_pkt_to_send_pkt((void*)pEvt);
        //race_pkt_t      *pret;
        //race_send_pkt_t *psend;
        //psend = (race_send_pkt_t *)ptr;
        //pret = &psend->race_data;
        //race_debug_print((uint8_t *)pret, (uint32_t)(pret->hdr.length + 4), "Race evt:");

		ptr_send = (race_send_pkt_t*)ptr;
		race_port_send_data(race_get_port_handle_by_channel_id(g_channel_id_save), (uint8_t*)&ptr_send->race_data, ptr_send->length);
		race_mem_free(ptr_send);
	}

	
}
#endif


void* RACE_FACTORY_TEST_NTC_STATE_READ(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
    typedef struct
	{
	    RACE_COMMON_HDR_STRU cmdhdr;
		uint8_t param;
	}PACKED CMD;

	typedef struct
	{
		uint8_t status;

		uint8_t ntc_state;
		int16_t temperature;
	}PACKED RSP;

	CMD *pCmd = (CMD *)pCmdMsg;
	RSP* pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE, (uint16_t)FACTORY_TEST_BLE_CMD_NTC_STATE_READ, (uint16_t)sizeof(RSP), channel_id);
    int32_t ret = RACE_ERRCODE_SUCCESS;

	RACE_LOG_MSGID_I("cmd data = %x \r\n",1, pCmd->param);

    if (pEvt)
    {
    	get_battery_NTC_status(&pEvt->temperature, &pEvt->ntc_state);

        pEvt->status = ret;
    }

	return pEvt;
}


void* RACE_FACTORY_TEST_FACTORY_RST_PARAMETER_HDR(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
    typedef struct
	{
	    RACE_COMMON_HDR_STRU cmdhdr;
		uint8_t param;
	}PACKED CMD;

	typedef struct
	{
		uint8_t status;
	
		uint8_t data[1];
	}PACKED RSP;

	CMD *pCmd = (CMD *)pCmdMsg;
	RSP* pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE, (uint16_t)FACTORY_TEST_BLE_CMD_FACTORY_REST, (uint16_t)sizeof(RSP), channel_id);
    int32_t ret = RACE_ERRCODE_SUCCESS;

	RACE_LOG_MSGID_I("cmd data = %x \r\n",1, pCmd->param);

    if (pEvt)
    {
    	pEvt->data[0] = pCmd->param;
		
    	if(pCmd->param >= 0x1 && pCmd->param <= 0x4)
		{
			app_system_factory_reset(pCmd->param);
		}
		else
		{
			ret = RACE_ERRCODE_FAIL;
		}

        pEvt->status = ret;
    }

	return pEvt;
}

void* RACE_FACTORY_TEST_ENTER_SHIPPING_MODE(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
    typedef struct
	{
	    RACE_COMMON_HDR_STRU cmdhdr;
		uint8_t param;
	}PACKED CMD;

	typedef struct
	{
		uint8_t status;	
	}PACKED RSP;

	CMD *pCmd = (CMD *)pCmdMsg;
	RSP* pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE, (uint16_t)FACTORY_TEST_BLE_CMD_ENTER_SHIPPING_MODE, (uint16_t)sizeof(RSP), channel_id);
    int32_t ret = RACE_ERRCODE_SUCCESS;

    if (pEvt)
    {	
		app_set_shipping_mode_state(0x81);

#if 0
		if(pCmd->param == 0x02)
		{

			if(bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_AGENT
				&& bt_sink_srv_cm_get_aws_connected_device() != NULL)
			{
				app_system_factory_reset(0x03);
			}
			else
			{
				app_system_factory_reset(0x01);
			}
			
		}
		else
		{
			app_enter_shipping_mode_flag_set(true);
			//app_system_factory_reset(0x02);
			ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
								APPS_EVENTS_INTERACTION_REQUEST_POWER_OFF, NULL, 0,
								NULL, 500);
			
			
		}
#endif

        pEvt->status = ret;
    }

	return pEvt;
}

void* RACE_FACTORY_TEST_POWER_OFF(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
#if 0
    typedef struct
	{
	    RACE_COMMON_HDR_STRU cmdhdr;
		uint8_t param;
	}PACKED CMD;
#endif

	typedef struct
	{
		uint8_t status;	
	}PACKED RSP;

	//CMD *pCmd = (CMD *)pCmdMsg;
	RSP* pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE, (uint16_t)FACTORY_TEST_BLE_CMD_POWER_OFF, (uint16_t)sizeof(RSP), channel_id);
    int32_t ret = RACE_ERRCODE_SUCCESS;

    if (pEvt)
    {	
		ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
							APPS_EVENTS_INTERACTION_REQUEST_POWER_OFF, NULL, 0,
							NULL, 500);

        pEvt->status = ret;
    }

	return pEvt;
}




void *RACE_FACTORY_TEST_KEY_CLICK_COUNT_READ(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
    typedef struct
	{
	    RACE_COMMON_HDR_STRU cmdhdr;
		uint8_t param;
	}PACKED CMD;

	typedef struct
	{
		uint8_t status;

        uint8_t key_cnt_L;
        uint8_t key_cnt_H;
	}PACKED RSP;


	CMD *pCmd = (CMD *)pCmdMsg;
	RSP* pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE, (uint16_t)FACTORY_TEST_BLE_CMD_CLICK_CNT, (uint16_t)sizeof(RSP), channel_id);
    int32_t ret = RACE_ERRCODE_SUCCESS;

	RACE_LOG_MSGID_I("cmd data = %x \r\n",1, pCmd->param);

    if (pEvt)
    {
       // pEvt->key_cnt_L = (uint8_t)(get_factory_key_click_count() >> 0);
       // pEvt->key_cnt_H = (uint8_t)(get_factory_key_click_count() >> 8);
        pEvt->status = ret;
    }

	return pEvt;
}


void *RACE_FACTORY_PSENSOR_CALIBRATE(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
    //uint16_t ir_readout = app_proximity_sensor_get_pxs_reading();

    typedef struct
    {
        RACE_COMMON_HDR_STRU cmdhdr;
        uint8_t param;
    } PACKED CMD;

    typedef struct
    {
        uint8_t status;
		uint8_t action;
        uint8_t PsDacCtrl_val;
        uint8_t PsFineCt_Val;
		uint16_t PsData;
    } PACKED RSP;

    CMD *pCmd = (CMD *)pCmdMsg;
    RSP *pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE, (uint16_t)FACTORY_TEST_BLE_CMD_PSENSOR_CALIBRATE, (uint16_t)sizeof(RSP), channel_id);
    int32_t ret = RACE_ERRCODE_SUCCESS;

    RACE_LOG_MSGID_I("cmd data = %x \r\n", 1, pCmd->param);

    if (pEvt)
    {
    	uint8_t cali_status;
		
    	pEvt->action = pCmd->param;
		
        if (pCmd->param == 0x01)
        {
            cali_status = bsp_px31bf_auto_dac();
        }
		else if(pCmd->param == 0x02 && bt_sink_srv_cm_get_aws_connected_device() != 0)
		{
			apps_aws_sync_event_send(EVENT_GROUP_UI_SHELL_CUSTOMER_COMMON, EVENT_ID_PSENSOR_CALIBRATE_ACTION_SYNC);
		}

        if (pCmd->param >= 0x00 && pCmd->param <= 0x02)
        {        
        	uint8_t reg_65 = 0;
			app_nvkey_psensor_calibration_cfg_read(&reg_65, &pEvt->PsFineCt_Val, &pEvt->PsData, &cali_status);
			pEvt->PsDacCtrl_val = (reg_65 & 0x0F);
        }		
        else
        {
            ret = RACE_ERRCODE_FAIL;
        }
    }

    pEvt->status = ret;
    return pEvt;
}


void *RACE_FACTORY_SET_IR_THRESHOLD_READ(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
    typedef struct
    {
        RACE_COMMON_HDR_STRU cmdhdr;
        uint8_t param;
    } PACKED CMD;

    typedef struct
    {
        uint8_t status;
		uint8_t action;
		uint8_t crosstalk;
        uint16_t thresholdHigh;
        uint16_t thresholdLow;
		uint16_t PsData;
    } PACKED RSP;

    CMD *pCmd = (CMD *)pCmdMsg;
    RSP *pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE, (uint16_t)FACTORY_TEST_BLE_CMD_IR_READ, (uint16_t)sizeof(RSP), channel_id);
    int32_t ret = RACE_ERRCODE_SUCCESS;

    RACE_LOG_MSGID_I("cmd data = %x \r\n", 1, pCmd->param);

    if (pEvt)
    {
    	//uint8_t tem_buff[7] = {0};
		
    	pEvt->action = pCmd->param;
		
        if (pCmd->param == 0x00)
        {
        	bsp_px31bf_PsData_read(&pEvt->crosstalk);//read nvkey
        }
        else if (pCmd->param == 0x01)
        {
			bsp_px31bf_PsData_read_reg(&pEvt->crosstalk);// read register
        }
        else
        {
            ret = RACE_ERRCODE_FAIL;
        }
    }

    pEvt->status = ret;
    return pEvt;
}

void* RACE_FACTORY_IR_THD_WIRETE(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
    typedef struct
	{
	    RACE_COMMON_HDR_STRU cmdhdr;
		uint8_t action;
		uint16_t thd;
	}PACKED CMD;

	typedef struct
	{
		uint8_t status;
		uint16_t rsp_thd;
	}PACKED RSP;

	CMD* pCmd = (CMD *)pCmdMsg;
	RSP* pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE, (uint16_t)FACTORY_TEST_BLE_CMD_PSENSOR_THD_WIRTE, (uint16_t)sizeof(RSP), channel_id);
    int32_t ret = RACE_ERRCODE_SUCCESS;

	RACE_LOG_MSGID_I("thd data = %x \r\n",1, pCmd->thd);

    if (pEvt)
    {
    
		if((pCmd->action == 0x1 || pCmd->action == 0x2) && pCmd->thd <0x3ff)
		{
				app_nvkey_psensor_threshold_write(pCmd->thd, pCmd->action);
				pEvt->rsp_thd = pCmd->thd;
		}
		else
		{
			pEvt->rsp_thd = 0;
			ret = RACE_ERRCODE_FAIL;
		}

        pEvt->status = ret;
    }
	return pEvt;
}


void* RACE_FACTORY_IR_THRESHOLD_CALIBRATE(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
    typedef struct
	{
	    RACE_COMMON_HDR_STRU cmdhdr;
		uint8_t action;
	}PACKED CMD;

	typedef struct
	{
		uint8_t status;
		uint8_t rsp_action;
		uint16_t PsData;
	}PACKED RSP;

	CMD* pCmd = (CMD *)pCmdMsg;
	RSP* pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE, (uint16_t)FACTORY_TEST_BLE_CMD_IR_THRESHOLD_CALIBRATE, (uint16_t)sizeof(RSP), channel_id);
    int32_t ret = RACE_ERRCODE_SUCCESS;

	RACE_LOG_MSGID_I("RACE_FACTORY_IR_THRESHOLD_CALIBRATE cmd data = %x \r\n",1, pCmd->action);

    if (pEvt)
    {
    
		pEvt->rsp_action = pCmd->action;
		
    	if(pCmd->action == 0x0 || pCmd->action == 0x1 || pCmd->action == 0x2)
		{
			pEvt->PsData = bsp_px31bf_Threshold_Factory_Calibrate();

			if(pEvt->PsData > 0x3FF)
			{
				ret = RACE_ERRCODE_FAIL;
			}
			else if(pCmd->action == 0x1 || pCmd->action == 0x2)
			{
				app_nvkey_psensor_threshold_write(pEvt->PsData, pCmd->action);
			}
		}		
		else
		{
			ret = RACE_ERRCODE_FAIL;
		}

        pEvt->status = ret;
    }
	return pEvt;
}



void *RACE_FACTORY_TEST_DUT_FOREC_ON_HDR(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
    /*typedef struct
    {
        RACE_COMMON_HDR_STRU cmdhdr;
        uint8_t FF_VOL;
        uint8_t tws;
    } PACKED CMD;*/

    typedef struct
    {
        uint8_t status;
    } PACKED RSP;

    //CMD *pCmd = (CMD *)pCmdMsg;
    RSP *pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE, (uint16_t)FACTORY_TEST_BLE_CMD_DUT_FORCE_ON, (uint16_t)sizeof(RSP), channel_id);
    int32_t ret = RACE_ERRCODE_SUCCESS;

    RACE_LOG_MSGID_I("cmd data = %x \r\n", 1, 0x900b);

    if (pEvt)
    {
        uint16_t *p_key_action = (uint16_t *)pvPortMalloc(sizeof(uint16_t)); /* free by ui shell */

       // *p_key_action = KEY_FACTORY_TEST_DUT_FORCE_ON;

        ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_KEY, INVALID_KEY_EVENT_ID, p_key_action,
                            sizeof(uint16_t), NULL, 0);
    }
    pEvt->status = ret;

    return pEvt;
}

void *RACE_FACTORY_TEST_BT_ADDR_READ(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
    typedef struct
	{
	    RACE_COMMON_HDR_STRU cmdhdr;
		uint8_t param;
	}PACKED CMD;

	typedef struct
	{
		uint8_t status;
		uint8_t earbud_mac_addr[BT_BD_ADDR_LEN];

	}PACKED RSP;


	CMD *pCmd = (CMD *)pCmdMsg;
	RSP* pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE, (uint16_t)FACTORY_TEST_BLE_CMD_BT_ADDR_READ, (uint16_t)sizeof(RSP), channel_id);
    int32_t ret = RACE_ERRCODE_SUCCESS;

	RACE_LOG_MSGID_I("cmd data = %x \r\n",1, pCmd->param);

    if (pEvt)
    {
		bt_bd_addr_t* local_addr = NULL;
		uint8_t addr_tem[BT_BD_ADDR_LEN];
		uint8_t i=0;

		if(pCmd->param == 0x00)
			local_addr = bt_device_manager_aws_local_info_get_local_address();
		else if(bt_sink_srv_cm_get_aws_connected_device() != 0 && pCmd->param == 0x01)
			local_addr = bt_device_manager_aws_local_info_get_peer_address();
		
		if(local_addr != NULL)
		{
			memcpy(&addr_tem[0], local_addr,BT_BD_ADDR_LEN);
			for(i=0;i<BT_BD_ADDR_LEN;i++)
				pEvt->earbud_mac_addr[i] = addr_tem[BT_BD_ADDR_LEN-i-1];
		}
		else
		{
			for(i=0;i<BT_BD_ADDR_LEN;i++)
				pEvt->earbud_mac_addr[i] = 0x00;
		
			ret = RACE_ERRCODE_FAIL;
		}

        pEvt->status = ret;
    }

	return pEvt;
}

void *RACE_FACTORY_TEST_BATTERY_LEVEL_READ(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
    typedef struct
	{
	    RACE_COMMON_HDR_STRU cmdhdr;
		uint8_t param;
	}PACKED CMD;

	typedef struct
	{
		uint8_t status;

        uint8_t battery_percent;
		uint8_t peer_battery;
		uint8_t case_battery;
	}PACKED RSP;


	CMD *pCmd = (CMD *)pCmdMsg;
	RSP* pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE, (uint16_t)FACTORY_TEST_BLE_CMD_BATTERY_LEVEL_READ, (uint16_t)sizeof(RSP), channel_id);
    int32_t ret = RACE_ERRCODE_SUCCESS;

	RACE_LOG_MSGID_I("cmd data = %x \r\n",1, pCmd->param);

    if (pEvt)
    {

		pEvt->battery_percent = (uint8_t)battery_management_get_battery_property(BATTERY_PROPERTY_CAPACITY);
#ifdef AIR_SMART_CHARGER_ENABLE
		pEvt->case_battery = app_get_smcharger_context()->case_battery_percent;
#else
		pEvt->case_battery = 0;
#endif

		
		if(bt_sink_srv_cm_get_aws_connected_device())
		{
		#ifdef AIR_SMART_CHARGER_ENABLE
			pEvt->peer_battery = (app_get_smcharger_context()->peer_battery_percent) & 0x7F;
		#else
			pEvt->peer_battery = 0;
		#endif
		}
		else
		{
			pEvt->peer_battery = 0;
		}
		
        pEvt->status = ret;
    }

	return pEvt;
}

void *RACE_FACTORY_TEST_TRACKING_LOG(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
    typedef struct
	{
	    RACE_COMMON_HDR_STRU cmdhdr;
	}PACKED CMD;

	typedef struct
	{
		uint8_t status;

        uint8_t log[32];
	}PACKED RSP;


	CMD *pCmd = (CMD *)pCmdMsg;
	RSP* pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE, (uint16_t)FACTORY_TEST_BLE_CMD_TRACKING_LOG, (uint16_t)sizeof(RSP), channel_id);
    int32_t ret = RACE_ERRCODE_SUCCESS;

    if (pEvt)
    {
		app_common_get_tracking_log(pEvt->log);
        pEvt->status = ret;
    }

	return pEvt;
}

void *RACE_FACTORY_TEST_IR_STATUS_CHECKING(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
    typedef struct
	{
	    RACE_COMMON_HDR_STRU cmdhdr;
	}PACKED CMD;

	typedef struct
	{
		uint8_t status;
		uint8_t ir_status;
	}PACKED RSP;


	CMD *pCmd = (CMD *)pCmdMsg;
	RSP* pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE, (uint16_t)FACTORY_TEST_BLE_CMD_IR_STATUS_CHECKING, (uint16_t)sizeof(RSP), channel_id);
    int32_t ret = RACE_ERRCODE_SUCCESS;

    if (pEvt)
    {
		pEvt->ir_status = app_get_ir_isr_status();
        pEvt->status = ret;
    }

	return pEvt;
}




void* RACE_CmdHandler_FACTORY_TEST(ptr_race_pkt_t pRaceHeaderCmd, uint16_t length, uint8_t channel_id)
{
    void* ptr = NULL;

	RACE_LOG_MSGID_I("RACE_CmdHandler_FACTORY_TEST type:%x pRaceHeaderCmd->hdr.id = %d \r\n",2,
               pRaceHeaderCmd->hdr.type,
               (int)pRaceHeaderCmd->hdr.id);
    if (pRaceHeaderCmd->hdr.type == RACE_TYPE_COMMAND ||
        pRaceHeaderCmd->hdr.type == RACE_TYPE_COMMAND_WITHOUT_RSP)
    {
        switch (pRaceHeaderCmd->hdr.id)
        {	
		case FACTORY_TEST_BLE_CMD_GET_VERSION :
		{
          		ptr = RACE_FACTORY_TEST_EARBUD_READ_OUT_CASE_VERS(pRaceHeaderCmd, channel_id);
            	}
            	break;

		case FACTORY_TEST_BLE_CMD_BT_ADDR_SET:
		{
			ptr = RACE_FACTORY_TEST_BLE_CMD_BT_ADDR_SET(pRaceHeaderCmd, channel_id);
		}
		break;
#if 0
		case FACTORY_TEST_BLE_CMD_AB1571D_VERSION :
            	{
                	ptr = RACE_FACTORY_TEST_EARBUD_READ_OUT_AB1571D_VERS(pRaceHeaderCmd, channel_id);
            	}
		break;	
            case FACTORY_TEST_SET_ECO_PROFILE:
            {
                ptr = RACE_FACTORY_TEST_SET_ECO_CHARGING_PROFILE_STATE(pRaceHeaderCmd, channel_id);
            }
            break;
#endif
			case FACTORY_TEST_BLE_CMD_SHIPPING_MODE:
            {
                ptr = RACE_FACTORY_TEST_SHIPPING_MODE(pRaceHeaderCmd, channel_id);
            }				
			break;

			case FACTORY_TEST_BLE_CMD_GFP_TX_POWER_SET:
			{
                ptr = RACE_FACTORY_TEST_GFP_TX_POWER_SET(pRaceHeaderCmd, channel_id);
			}
			break;
#if 0
			case FACTORY_TEST_BLE_CMD_DEVICE_COLOR:
            {
				ptr = RACE_FACTORY_TEST_BLE_COLOR_ID(pRaceHeaderCmd, channel_id);
            }				
			break;

			case FACTORY_TEST_BLE_CMD_MINI_UI:
			{
				ptr = RACE_FACTORY_TEST_BLE_MINI_UI(pRaceHeaderCmd, channel_id);
			}
			break;
#endif
			case FACTORY_TEST_BLE_CMD_LEA_DISABLE:
			{
				ptr = RACE_FACTORY_TEST_LEA_DISABLE(pRaceHeaderCmd, channel_id);
			}
			break;

			case FACTORY_TEST_BLE_CMD_HX300X_PS_DATA_READ:
			{
				ptr = RACE_FACTORY_TEST_HX300X_PS_DATA_READ(pRaceHeaderCmd, channel_id);
			}
			break;

			case FACTORY_TEST_BLE_CMD_HX300X_IIC_INT_CHECK:
			{
				ptr = RACE_FACTORY_TEST_HX300X_IIC_INT_CHECK(pRaceHeaderCmd, channel_id);
			}
			break;

			case FACTORY_TEST_BLE_CMD_HX300X_OPEN_AIR:
			{
				ptr = RACE_FACTORY_TEST_HX300X_OPEN_AIR(pRaceHeaderCmd, channel_id);
			}
			break;				

			case FACTORY_TEST_BLE_CMD_HX300X_IN_EAR:
			{
				ptr = RACE_FACTORY_TEST_HX300X_IN_EAR(pRaceHeaderCmd, channel_id);
			}
			break;	

			case FACTORY_TEST_BLE_CMD_HX300X_OUT_EAR:
			{
				ptr = RACE_FACTORY_TEST_HX300X_OUT_EAR(pRaceHeaderCmd, channel_id);
			}
			break;				

			case FACTORY_TEST_BLE_CMD_HX300X_DATA_SAVE:
			{
				ptr = RACE_FACTORY_TEST_HX300X_DATA_SAVE(pRaceHeaderCmd, channel_id);
			}
			break;

			case FACTORY_TEST_BLE_CMD_HX300X_READ_CALI_SETTING:
			{
				ptr = RACE_FACTORY_TEST_HX300X_READ_CALI_SETTING(pRaceHeaderCmd, channel_id);
			}
			break;	

			case FACTORY_TEST_BLE_CMD_HX300X_DEBUG_SWITCH:
			{
				ptr = RACE_FACTORY_TEST_HX300X_DEBUG(pRaceHeaderCmd, channel_id);
			}
			break;

			case FACTORY_TEST_BLE_CMD_HX300X_REG_CTR:
			{
				ptr = RACE_FACTORY_TEST_HX300X_REG_CTR(pRaceHeaderCmd, channel_id);
			}
			break;	

			case FACTORY_TEST_BLE_CMD_HX300X_READ_ALL_REG:
			{
				ptr = RACE_FACTORY_TEST_HX300X_READ_ALL_REG(pRaceHeaderCmd, channel_id);
			}
			break;			
			case FACTORY_TEST_BLE_CMD_TOUCH_TEST_CTR:
			{
				ptr = RACE_FACTORY_TEST_TOUCH_CTR(pRaceHeaderCmd, channel_id);
			}
			break;			

			case FACTORY_TEST_BLE_CMD_TOUCH_LIMIT_DISABLE:
			{
				ptr = RACE_FACTORY_TEST_TOUCH_LIMIT_DISABLE_SET(pRaceHeaderCmd, channel_id);
			}
			break;			
#if 0
			case FACTORY_TEST_BLE_CMD_BAT_HEATHY_PERCENT2TIME_SET:
			{
				ptr = RACE_FACTORY_TEST_BAT_HEATHY_PERCENT2TIME_SET(pRaceHeaderCmd, channel_id);
			}
			break;

			case FACTORY_TEST_BLE_CMD_BAT_HEATHY_CTR:
			{
				ptr = RACE_FACTORY_TEST_BAT_HEATHY_CTR(pRaceHeaderCmd, channel_id);
			}
			break;			
#endif
			case FACTORY_TEST_BLE_CMD_PSENSOR_PUW_SET:
			{
				ptr = RACE_FACTORY_TEST_PSENSOR_PUW_SET(pRaceHeaderCmd, channel_id);
			}
			break;		

			case FACTORY_TEST_BLE_CMD_HW_VERSION:
			{
				ptr = RACE_FACTORY_TEST_HW_VERSION(pRaceHeaderCmd, channel_id);
			}
			break;

			case FACTORY_TEST_BLE_CMD_SERIAL_NUMBER:
			{
				ptr = RACE_FACTORY_TEST_SERIAL_NUMBER(pRaceHeaderCmd, channel_id);
			}
			break;
			
            case FACTORY_TEST_SET_A2DP_VOLUME:
            {
                ptr = RACE_FACTORY_TEST_SET_A2DP_VOLUME(pRaceHeaderCmd, channel_id);
            }
            break;
			

            case FACTORY_TEST_BLE_CMD_CHANNEL_CHECK :
            {
                ptr = RACE_FACTORY_TEST_CHANNEL_CHECK_PARAMETER_HDR(pRaceHeaderCmd, channel_id);
            }
            break;

            case FACTORY_TEST_BLE_CMD_HALL_CHECK :
            {
                ptr = RACE_FACTORY_TEST_HALL_STATUS_CHECK(pRaceHeaderCmd, channel_id);
            }
            break;

            case FACTORY_TEST_BLE_CMD_TOUCH_READ :
            {
                ptr = RACE_FACTORY_TEST_TOUCH_KEY_READ(pRaceHeaderCmd, channel_id);
            }
            break;			
			
            case FACTORY_TEST_BLE_CMD_WRITE_SN_SYNC_RESULT :
            {
                ptr = RACE_FACTORY_TEST_SN_WRITE_RESULT_CHECK(pRaceHeaderCmd, channel_id);
            }
            break;		
#if 0
            case FACTORY_TEST_BLE_CMD_MFI_CHECK :
            {
                ptr = RACE_FACTORY_TEST_MFI_CHECK(pRaceHeaderCmd, channel_id);
            }
            break;			
#endif
            case FACTORY_TEST_BLE_CMD_NTC_STATE_READ :
            {
                ptr = RACE_FACTORY_TEST_NTC_STATE_READ(pRaceHeaderCmd, channel_id);
            }
            break;			

            case FACTORY_TEST_BLE_CMD_FACTORY_REST :
            {
                ptr = RACE_FACTORY_TEST_FACTORY_RST_PARAMETER_HDR(pRaceHeaderCmd, channel_id);
            }
            break;	

            case FACTORY_TEST_BLE_CMD_ENTER_SHIPPING_MODE :
            {
                ptr = RACE_FACTORY_TEST_ENTER_SHIPPING_MODE(pRaceHeaderCmd, channel_id);
            }
            break;	

            case FACTORY_TEST_BLE_CMD_POWER_OFF :
            {
                ptr = RACE_FACTORY_TEST_POWER_OFF(pRaceHeaderCmd, channel_id);
            }
            break;				
			
            case FACTORY_TEST_BLE_CMD_ROLE_HANDOVER:
            {
                ptr = RACE_FACTORY_TEST_ROLE_HANDOVER_HDR(pRaceHeaderCmd, channel_id);
            }
            break;
            case FACTORY_TEST_BLE_CMD_PSENSOR_CALIBRATE: {
                ptr = RACE_FACTORY_PSENSOR_CALIBRATE(pRaceHeaderCmd, channel_id);
            }
            break;			
            case FACTORY_TEST_BLE_CMD_IR_READ: {
                ptr = RACE_FACTORY_SET_IR_THRESHOLD_READ(pRaceHeaderCmd, channel_id);
            }
            break;
            case FACTORY_TEST_BLE_CMD_IR_THRESHOLD_CALIBRATE: {
                ptr = RACE_FACTORY_IR_THRESHOLD_CALIBRATE(pRaceHeaderCmd, channel_id);
            }
            break;	
            case FACTORY_TEST_BLE_CMD_PSENSOR_THD_WIRTE: {
                ptr = RACE_FACTORY_IR_THD_WIRETE(pRaceHeaderCmd, channel_id);
            }
            break;				
            case FACTORY_TEST_BLE_CMD_DUT_FORCE_ON: {
                ptr = RACE_FACTORY_TEST_DUT_FOREC_ON_HDR(pRaceHeaderCmd, channel_id);
            }
            break;
            case FACTORY_TEST_BLE_CMD_CLICK_CNT: {
                ptr = RACE_FACTORY_TEST_KEY_CLICK_COUNT_READ(pRaceHeaderCmd, channel_id);
            }
            break;
			case FACTORY_TEST_BLE_CMD_BT_ADDR_READ:
			{
				ptr = RACE_FACTORY_TEST_BT_ADDR_READ(pRaceHeaderCmd, channel_id);
			}
			break;
			case FACTORY_TEST_BLE_CMD_BATTERY_LEVEL_READ:
			{
				ptr = RACE_FACTORY_TEST_BATTERY_LEVEL_READ(pRaceHeaderCmd, channel_id);
			}
			break;

			case FACTORY_TEST_BLE_CMD_TRACKING_LOG:
			{
				ptr = RACE_FACTORY_TEST_TRACKING_LOG(pRaceHeaderCmd, channel_id);
			}				
			break;

			case FACTORY_TEST_BLE_CMD_IR_STATUS_CHECKING:
			{
				ptr = RACE_FACTORY_TEST_IR_STATUS_CHECKING(pRaceHeaderCmd, channel_id);
			}
			break;

            default:
            {
                break;
            }
			
        }
    }

    return ptr;
}




