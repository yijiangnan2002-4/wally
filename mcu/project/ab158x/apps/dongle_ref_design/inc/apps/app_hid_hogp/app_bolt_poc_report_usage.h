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

#ifndef _APP_BOLT_POC_REPORT_USAGE_H
#define _APP_BOLT_POC_REPORT_USAGE_H

#include "app_bolt_poc_report_item.h"

/* Usage Page: Generic Desktop (0x01U)
** Sys: System
*/
#define GD_Pointer                                    0x01U
#define GD_Mouse                                      0x02U
                                             /* Reserved */
#define GD_Joystick                                   0x04U
#define GD_Game_Pad                                   0x05U
#define GD_Keyboard                                   0x06U
#define GD_Keypad                                     0x07U
#define GD_Multiaxis_Controller                       0x08U
#define GD_Tablet_PC_Sys_Controls                     0x09U
                                             /* Reserved */
#define GD_X                                          0x30U
#define GD_Y                                          0x31U
#define GD_Z                                          0x32U
#define GD_Rx                                         0x33U
#define GD_Ry                                         0x34U
#define GD_Rz                                         0x35U
#define GD_Slider                                     0x36U
#define GD_Dial                                       0x37U
#define GD_Wheel                                      0x38U
#define GD_Hat_Switch                                 0x39U
#define GD_Counted_Buffer                             0x3AU
#define GD_Byte_Count                                 0x3BU
#define GD_Motion_Wakeup                              0x3CU
#define GD_Start                                      0x3DU
#define GD_Select                                     0x3EU
                                             /* Reserved */
#define GD_Vx                                         0x40U
#define GD_Vy                                         0x41U
#define GD_Vz                                         0x42U
#define GD_Vbrx                                       0x43U
#define GD_Vbry                                       0x44U
#define GD_Vbrz                                       0x45U
#define GD_Vno                                        0x46U
#define GD_Feature_Notification                       0x47U
#define GD_Resolution_Multiplier                      0x48U
                                             /* Reserved */
#define GD_Sys_Control                                0x80U
#define GD_Sys_Power_Down                             0x81U
#define GD_Sys_Sleep                                  0x82U
#define GD_Sys_Wake_Up                                0x83U
#define GD_Sys_Context_Menu                           0x84U
#define GD_Sys_Main_Menu                              0x85U
#define GD_Sys_App_Menu                               0x86U
#define GD_Sys_Menu_Help                              0x87U
#define GD_Sys_Menu_Exit                              0x88U
#define GD_Sys_Menu_Select                            0x89U
#define GD_Sys_Menu_Right                             0x8AU
#define GD_Sys_Menu_Left                              0x8BU
#define GD_Sys_Menu_Up                                0x8CU
#define GD_Sys_Menu_Down                              0x8DU
#define GD_Sys_Cold_Restart                           0x8EU
#define GD_Sys_Warm_Restart                           0x8FU
#define GD_D_pad_Up                                   0x90U
#define GD_D_pad_Down                                 0x91U
#define GD_D_pad_Right                                0x92U
#define GD_D_pad_Left                                 0x93U
                                             /* Reserved */
#define GD_Sys_Dock                                   0xA0U
#define GD_Sys_Undock                                 0xA1U
#define GD_Sys_Setup                                  0xA2U
#define GD_Sys_Break                                  0xA3U
#define GD_Sys_Debugger_Break                         0xA4U
#define GD_Application_Break                          0xA5U
#define GD_Application_Debugger_Break                 0xA6U
#define GD_Sys_Speaker_Mute                           0xA7U
#define GD_Sys_Hibernate                              0xA8U
                                             /* Reserved */
#define GD_Sys_Display_Invert                         0xB0U
#define GD_Sys_Display_Internal                       0xB1U
#define GD_Sys_Display_External                       0xB2U
#define GD_Sys_Display_Both                           0xB3U
#define GD_Sys_Display_Dual                           0xB4U
#define GD_Sys_Display_Toggle                         0xB5U
#define GD_Sys_Display_Swap                           0xB6U
#define GD_Sys_Display_LCD_Autoscale                  0xB7U
                                             /* Reserved */


/* Usage Page: Simulation Controls Page (0x02U)
** SimuDev: Simulation Device
*/
#define SC_SimuDev_Flight                             0x01U
#define SC_SimuDev_Automobile                         0x02U
#define SC_SimuDev_Tank                               0x03U
#define SC_SimuDev_Spaceship                          0x04U
#define SC_SimuDev_Submarine                          0x05U
#define SC_SimuDev_Sailing                            0x06U
#define SC_SimuDev_Motorcycle                         0x07U
#define SC_SimuDev_Sports                             0x08U
#define SC_SimuDev_Airplane                           0x09U
#define SC_SimuDev_Helicopter                         0x0AU
#define SC_SimuDev_MagicCarpet                        0x0BU
#define SC_SimuDev_Bicycle                            0x0CU
                                          /* Reserved */
#define SC_Flight_Control_Stick                       0x20U
#define SC_Flilght_Stick                              0x21U
#define SC_Cyclic_Control                             0x22U
#define SC_Cyclic_Trim                                0x23U
#define SC_Flight_Yoke                                0x24U
#define SC_Track_Control                              0x25U
                                          /* Reserved */
#define SC_Aileron                                    0xB0U
#define SC_Aileron_Trim                               0xB1U
#define SC_Anti_Torque_Control                        0xB2U
#define SC_Autopilot_Enable                           0xB3U
#define SC_Chaff_Release                              0xB4U
#define SC_Collective_Control                         0xB5U
#define SC_Dive_Brake                                 0xB6U
#define SC_Electronic_Countermeasures                 0xB7U
#define SC_Elevator                                   0xB8U
#define SC_Elevator_Trim                              0xB9U
#define SC_Rudder                                     0xBAU
#define SC_Throttle                                   0xBBU
#define SC_Flight_Communications                      0xBCU
#define SC_Flare_Release                              0xBDU
#define SC_Landing_Gear                               0xBEU
#define SC_Toe_Brake                                  0xBFU
#define SC_Trigger                                    0xC0U
#define SC_Weapons_Arm                                0xC1U
#define SC_Weapons_Select                             0xC2U
#define SC_Wing_Flaps                                 0xC3U
#define SC_Accelerator                                0xC4U
#define SC_Brake                                      0xC5U
#define SC_Clutch                                     0xC6U
#define SC_Shifter                                    0xC7U
#define SC_Steering                                   0xC8U
#define SC_Turret_Direction                           0xC9U
#define SC_Barrel_Elevation                           0xCAU
#define SC_Dive_Plane                                 0xCBU
#define SC_Ballast                                    0xCCU
#define SC_Bicycle_Crank                              0xCDU
#define SC_Handle_Bars                                0xCEU
#define SC_Front_Brake                                0xCFU
#define SC_Rear_Brake                                 0xD0U
                                          /* Reserved */


/* Usage Page: VR Controls Page (0x03)
**
*/
#define VR_Belt                                       0x01U
#define VR_Body_Suit                                  0x02U
#define VR_Flexor                                     0x03U
#define VR_Glove                                      0x04U
#define VR_Head_Tracker                               0x05U
#define VR_Head_Mounted_Display                       0x06U
#define VR_Hand_Tracker                               0x07U
#define VR_Oculometer                                 0x08U
#define VR_Vest                                       0x09U
#define VR_Animatronic_Device                         0x0AU
                                          /* Reserved */
#define VR_Stereo_Enable                              0x20U
#define VR_Display_Enable                             0x21U
                                          /* Reserved */


/* Usage Page: Sport Controls Page (0x04)
**
*/
#define SpC_Baseball_Bat				      0x01U
#define SpC_Golf_Club					      0x02U
#define SpC_Rowing_Machine				      0x03U
#define SpC_Treadmill					      0x04U
                                          /* Reserved */
#define SpC_Oar						      0x30U
#define SpC_Slope						      0x31U
#define SpC_Rate						      0x32U
#define SpC_Stick_Speed				            0x33U
#define SpC_Stick_Face_Angle			            0x34U
#define SpC_Stick_HeelorToe			            0x35U
#define SpC_Stick_Follow_Through		            0x36U
#define SpC_Stick_Tempo				            0x37U
#define SpC_Stick_Type					      0x38U
#define SpC_Stick_Height				      0x39U
                                          /* Reserved */
#define SpC_Putter						0x50U
#define SpC_Iron_1						0x51U
#define SpC_Iron_2						0x52U
#define SpC_Iron_3						0x53U
#define SpC_Iron_4						0x54U
#define SpC_Iron_5						0x55U
#define SpC_Iron_6						0x56U
#define SpC_Iron_7						0x57U
#define SpC_Iron_8						0x58U
#define SpC_Iron_9						0x59U
#define SpC_Iron_10					      0x5AU
#define SpC_Iron_11					      0x5BU
#define SpC_Sand_Wedge					      0x5CU
#define SpC_Loft_Wedge					      0x5DU
#define SpC_Power_Wedge				            0x5EU
#define SpC_Wood_1						0x5FU
#define SpC_Wood_3						0x60U
#define SpC_Wood_5						0x61U
#define SpC_Wood_7						0x62U
#define SpC_Wood_9						0x63U
                                          /* Reserved */


/* Usage Page: Game Controls Page (0x05)
*/
#define GC_3D_Game_Controller			            0x01U
#define GC_Pinball_Device				      0x02U
#define GC_Gun_Device					      0x03U
                                          /* Reserved */
#define GC_Point_of_View				      0x20U
#define GC_Turn_Right_Left				      0x21U
#define GC_Pitch_Forward_Backward		            0x22U
#define GC_Roll_Right_Left				      0x23U
#define GC_Move_Right_Left				      0x24U
#define GC_Move_Forward_Backward		            0x25U
#define GC_Move_Up_Down				            0x26U
#define GC_Lean_Right_Left				      0x27U
#define GC_Lean_Forward_Backward		            0x28U
#define GC_Height_of_POV				      0x29U
#define GC_Flipper						0x2AU
#define GC_Secondary_Flipper			            0x2BU
#define GC_Bump						      0x2CU
#define GC_New_Game					      0x2DU
#define GC_Shoot_Ball					      0x2EU
#define GC_Player						      0x2FU
#define GC_Gun_Bolt					      0x30U
#define GC_Gun_Clip					      0x31U
#define GC_Gun_Selector				            0x32U
#define GC_Gun_Single_Shot				      0x33U
#define GC_Gun_Burst					      0x34U
#define GC_Gun_Automatic				      0x35U
#define GC_Gun_Safety					      0x36U
#define GC_Gamepad_Fire_Jump			            0x37U
                                          /* Reserved */
#define GC_Gamepad_Trigger				      0x39U
                                          /* Reserved */


/* Usage Page: Generic Device Controls (0x06)
** SC: Security Code
*/
                                    /* 0x00 Undefined */
                                          /* Reserved */
#define GDC_Battery_Strength			            0x20U
#define GDC_Wireless_Channel			            0x21U
#define GDC_Wireless_ID				            0x22U
#define GDC_Discover_Wireless_Ctrl		            0x23U
#define GDC_SC_Character_Entered		            0x24U
#define GDC_SC_Character_Cleared		            0x25U
#define GDC_SC_Cleared				            0x26U
                                          /* Reserved */

/* Usage Page: Keyboard/Keypad (0x07)
*/
//TODO:

/* Usage Page: LED (0x08)
*/
#define LED_Num_Lock					      0x01U
#define LED_Caps_Lock				            0x02U
#define LED_Scroll_Lock				            0x03U
#define LED_Compose					      0x04U
#define LED_Kana					            0x05U
#define LED_Power					            0x06U
#define LED_Shift					            0x07U
#define LED_Donot_Disturb				      0x08U
#define LED_Mute				                  0x09U
#define LED_Tone_Enable				            0x0AU
#define LED_High_Cut_Filter			            0x0BU
#define LED_Low_Cut_Filter				      0x0CU
#define LED_Equalizer_Enable			            0x0DU
#define LED_Sound_Field_On				      0x0EU
#define LED_Surround_On				            0x0FU
#define LED_Repeat					      0x10U
#define LED_Stereo					      0x11U
#define LED_Sampling_Rate_Detect	      	      0x12U
#define LED_Spinning					      0x13U
#define LED_CAV					            0x14U
#define LED_CLV					            0x15U
#define LED_Recording_Format_Detect	                  0x16U
#define LED_Off_Hook					      0x17U
#define LED_Ring					            0x18U
#define LED_Message_Waiting			            0x19U
#define LED_Data_Mode				            0x1AU
#define LED_Battery_Operation			            0x1BU
#define LED_Battery_OK				            0x1CU
#define LED_Battery_Low				            0x1DU
#define LED_Speaker					      0x1EU
#define LED_Head_Set					      0x1FU
#define LED_Hold					            0x20U
#define LED_Microphone				            0x21U
#define LED_Coverage					      0x22U
#define LED_Night_Mode				            0x23U
#define LED_Send_Calls				            0x24U
#define LED_Call_Pickup				            0x25U
#define LED_Conference				            0x26U
#define LED_Standby					      0x27U
#define LED_Camera_On				            0x28U
#define LED_Camera_Off				            0x29U
#define LED_On_Line					      0x2AU
#define LED_Off_Line                                  0x2BU
#define LED_Busy					            0x2CU
#define LED_Ready					            0x2DU
#define LED_Paper_Out				            0x2EU
#define LED_Paper_Jam				            0x2FU
#define LED_Remote					      0x30U
#define LED_Forward					      0x31U
#define LED_Reverse					      0x32U
#define LED_Stop					            0x33U
#define LED_Rewind					      0x34U
#define LED_Fast_Forward				      0x35U
#define LED_Play					            0x36U
#define LED_Pause					            0x37U
#define LED_Record					      0x38U
#define LED_Error					            0x39U
#define LED_Selected_Indicator			      0x3AU
#define LED_In_Use_Indicator			            0x3BU
#define LED_Multi_Mode_Indicator 		            0x3CU
#define LED_Indicator_On				      0x3DU
#define LED_Indicator_Flash			            0x3EU
#define LED_Indicator_Slow_Blink		            0x3FU
#define LED_Indicator_Fast_Blink		            0x40U
#define LED_Indicator_Off				      0x41U
#define LED_Flash_On_Time				      0x42U
#define LED_Slow_Blink_On_Time			      0x43U
#define LED_Slow_Blink_Off_Time		            0x44U
#define LED_Fast_Blink_On_Time			      0x45U
#define LED_Fast_Blink_Off_Time		            0x46U
#define LED_Usage_Indicator_Color		            0x47U
#define LED_Indicator_Red				      0x48U
#define LED_Indicator_Green			            0x49U
#define LED_Indicator_Amber			            0x4AU
#define LED_Generic_Indicator			            0x4BU
#define LED_Sys_Suspend				            0x4CU
#define LED_External_Power_Connected	            0x4DU
                                             /* Reserved */


/* Usage Page: Button (0x09)
** ID N is Button N
*/

/* Usage Page: Ordinal (0x0A)
** ID N is Instance N
*/

/* Usage Page: Telephony Device (0x0B)
*/
#define TD_Phone					            0x01U
#define TD_Answering_Machine		                  0x02U
#define TD_Message_Controls		                  0x03U
#define TD_Handset					      0x04U
#define TD_Headset					      0x05U
#define TD_Telephony_Key_Pad		                  0x06U
#define TD_Programmable_Button		            0x07U
                                             /* Reserved */
#define TD_Hook_Switch				            0x20U
#define TD_Flash					            0x21U
#define TD_Feature					      0x22U
#define TD_Hold					            0x23U
#define TD_Redial					            0x24U
#define TD_Transfer				            0x25U
#define TD_Drop					            0x26U
#define TD_Park					            0x27U
#define TD_Forward_Calls			            0x28U
#define TD_Alternate_Function		                  0x29U
#define TD_Line					            0x2AU
#define TD_Speaker_Phone			            0x2BU
#define TD_Conference				            0x2CU
#define TD_Ring_Enable				            0x2DU
#define TD_Ring_Select				            0x2EU
#define TD_Phone_Mute				            0x2FU
#define TD_Caller_ID				            0x30U
#define TD_Send					            0x31U
                                             /* Reserved */
#define TD_Speed_Dial				            0x50U
#define TD_Store_Number			                  0x51U
#define TD_Recall_Number			            0x52U
#define TD_Phone_Directory		      	      0x53U
                                             /* Reserved */
#define TD_Voice_Mail		      	            0x70U
#define TD_Screen_Calls		      	            0x71U
#define TD_Do_Not_Disturb		      	      0x72U
#define TD_Message			      	      0x73U
#define TD_Answer_On_Off		      	      0x74U
                                             /* Reserved */
#define TD_Inside_Dial_Tone	      	            0x90U
#define TD_Outside_Dial_Tone	      	            0x91U
#define TD_Inside_Ring_Tone	      	            0x92U
#define TD_Outside_Ring_Tone	      	            0x93U
#define TD_Priority_Ring_Tone	      	            0x94U
#define TD_Inside_Ringback		      	      0x95U
#define TD_Priority_Ringback	      	            0x96U
#define TD_Line_Busy_Tone		      	      0x97U
#define TD_Reorder_Tone		      	            0x98U
#define TD_Call_Waiting_Tone	      	            0x99U
#define TD_Confirmation_Tone_1	      	      0x9AU
#define TD_Confirmation_Tone_2	      	      0x9BU
#define TD_Tones_Off			      	      0x9CU
#define TD_Outside_Ringback	      	            0x9DU
#define TD_Ringer			      	            0x9EU
                                             /* Reserved */
#define TD_Phone_Key_0                                0xB0U
#define TD_Phone_Key_1                                0xB1U
#define TD_Phone_Key_2                                0xB2U
#define TD_Phone_Key_3                                0xB3U
#define TD_Phone_Key_4                                0xB4U
#define TD_Phone_Key_5                                0xB5U
#define TD_Phone_Key_6                                0xB6U
#define TD_Phone_Key_7                                0xB7U
#define TD_Phone_Key_8                                0xB8U
#define TD_Phone_Key_9                                0xB9U
#define TD_Phone_Key_Star                             0xBAU
#define TD_Phone_Key_Pound                            0xBBU
#define TD_Phone_Key_A                                0xBCU
#define TD_Phone_Key_B                                0xBDU
#define TD_Phone_Key_C                                0xBEU
#define TD_Phone_Key_D                                0xBFU
                                             /* Reserved */

/* Usage Page: Consumer (0x0C)
** App      - Application
** Btn      - Button
** Ctrl     - Control
** Incr/Decr - Increase/Decrease
** Prog     - Programmable
** Sel      - Select
** Sys      - System
** ILL      - Illumination
*/

#define UC_Consumer_Ctrl					0x1U
#define UC_Numeric_Keypad					0x2U
#define UC_Prog_Btns					      0x3U
#define UC_Mic					            0x4U
#define UC_Headphone					      0x5U
#define UC_Graphic_Equalizer				      0x6U
                                             /* Reserved */
#define UC_Add_10					            0x20U
#define UC_Add_100					      0x21U
#define UC_AM_PM					            0x22U
                                             /* Reserved */
#define UC_Power					            0x30U
#define UC_Reset					            0x31U
#define UC_Sleep				      	      0x32U
#define UC_Sleep_After					      0x33U
#define UC_Sleep_Mode					      0x34U
#define UC_ILL					            0x35U
#define UC_Function_Buttons				      0x36U
                                             /* Reserved */
#define UC_Menu					            0x40U
#define UC_Menu_Pick					      0x41U
#define UC_Menu_Up					      0x42U
#define UC_Menu_Down					      0x43U
#define UC_Menu_Left					      0x44U
#define UC_Menu_Right					      0x45U
#define UC_Menu_Escape					      0x46U
#define UC_Menu_Value_Incr					0x47U
#define UC_Menu_Value_Decr					0x48U
                                             /* Reserved */
#define UC_Data_On_Screen					0x60U
#define UC_Closed_Caption					0x61U
#define UC_Closed_Caption_Sel				      0x62U
#define UC_VCR_TV					            0x63U
#define UC_Broadcast_Mode					0x64U
#define UC_Snapshot					      0x65U
#define UC_Still					            0x66U
                                             /* Reserved */
#define UC_Selion					            0x80U
#define UC_Assign_Selion					0x81U
#define UC_Mode_Step					      0x82U
#define UC_Recall_Last					      0x83U
#define UC_Enter_Channel					0x84U
#define UC_Order_Movie					      0x85U
#define UC_Channel					      0x86U
#define UC_Media_Selion					      0x87U
#define UC_Media_Sel_Computer				      0x88U
#define UC_Media_Sel_TV					      0x89U
#define UC_Media_Sel_WWW					0x8AU
#define UC_Media_Sel_DVD					0x8BU
#define UC_Media_Sel_Telephone				0x8CU
#define UC_Media_Sel_Program_Guide			      0x8DU
#define UC_Media_Sel_Video_Phone				0x8EU
#define UC_Media_Sel_Games					0x8FU
#define UC_Media_Sel_Messages				      0x90U
#define UC_Media_Sel_CD					      0x91U
#define UC_Media_Sel_VCR					0x92U
#define UC_Media_Sel_Tuner					0x93U
#define UC_Quit					            0x94U
#define UC_Help					            0x95U
#define UC_Media_Sel_Tape					0x96U
#define UC_Media_Sel_Cable					0x97U
#define UC_Media_Sel_Satellite				0x98U
#define UC_Media_Sel_Security				      0x99U
#define UC_Media_Sel_Home					0x9AU
#define UC_Media_Sel_Call					0x9BU
#define UC_Channel_Incr					      0x9CU
#define UC_Channel_Decr					      0x9DU
#define UC_Media_Sel_SAP					0x9EU
                                             /* Reserved */
#define UC_VCR_Plus					      0xA0U
#define UC_Once					            0xA1U
#define UC_Daily					            0xA2U
#define UC_Weekly					            0xA3U
#define UC_Monthly					      0xA4U
                                             /* Reserved */
#define UC_Play					            0xB0U
#define UC_Pause					            0xB1U
#define UC_Record					            0xB2U
#define UC_Fast_Forward					      0xB3U
#define UC_Rewind					            0xB4U
#define UC_Scan_Next_Track					0xB5U
#define UC_Scan_Previous_Track				0xB6U
#define UC_Stop					            0xB7U
#define UC_Eject				      	      0xB8U
#define UC_Random_Play					      0xB9U
#define UC_Sel_Disc					      0xBAU
#define UC_Enter_Disc					      0xBBU
#define UC_Repeat					            0xBCU
#define UC_Tracking					      0xBDU
#define UC_Track_Normal					      0xBEU
#define UC_Slow_Tracking					0xBFU
#define UC_Frame_Forward					0xC0U
#define UC_Frame_Back					      0xC1U
#define UC_Mark					            0xC2U
#define UC_Clear_Mark					      0xC3U
#define UC_Repeat_From_Mark                           0xC4U
#define UC_Return_To_Mark					0xC5U
#define UC_Search_Mark_Forward                        0xC6U
#define UC_Search_Mark_Backward                       0xC7U
#define UC_Counter_Reset					0xC8U
#define UC_Show_Counter					      0xC9U
#define UC_Tracking_Incr					0xCAU
#define UC_Tracking_Decr					0xCBU
#define UC_Stop_Eject					      0xCCU
#define UC_Play_Pause					      0xCDU
#define UC_Play_Skip					      0xCEU
                                             /* Reserved */
#define UC_Volume					            0xE0U
#define UC_Balance				      	0xE1U
#define UC_Mute				      	      0xE2U
#define UC_Bass					            0xE3U
#define UC_Treble					            0xE4U
#define UC_Bass_Boost					      0xE5U
#define UC_Surround_Mode					0xE6U
#define UC_Loudness					      0xE7U
#define UC_MPX					            0xE8U
#define UC_Volume_Incr					      0xE9U
#define UC_Volume_Decr					      0xEAU
                                             /* Reserved */
#define UC_Speed_Sel				      	0xF0U
#define UC_Playback_Speed					0xF1U
#define UC_Standard_Play					0xF2U
#define UC_Long_Play					      0xF3U
#define UC_Extended_Play					0xF4U
#define UC_Slow					            0xF5U
                                             /* Reserved */
#define UC_Fan_Enable					      0x100U
#define UC_Fan_Speed					      0x101U
#define UC_Light_Enable					      0x102U
#define UC_Light_ILL_Level					0x103U
#define UC_Climate_Ctrl_Enable                        0x104U
#define UC_Room_Temperature                           0x105U
#define UC_Security_Enable					0x106U
#define UC_Fire_Alarm					      0x107U
#define UC_Police_Alarm					      0x108U
#define UC_Proximity					      0x109U
#define UC_Motion					            0x10AU
#define UC_Duress_Alarm					      0x10BU
#define UC_Holdup_Alarm					      0x10CU
#define UC_Medical_Alarm					0x10DU
                                             /* Reserved */
#define UC_Balance_Right					0x150U
#define UC_Balance_Left					      0x151U
#define UC_Bass_Incr					      0x152U
#define UC_Bass_Decr					      0x153U
#define UC_Treble_Incr					      0x154U
#define UC_Treble_Decr					      0x155U
                                             /* Reserved */
#define UC_Speaker_Sys					      0x160U
#define UC_Channel_Left					      0x161U
#define UC_Channel_Right					0x162U
#define UC_Channel_Center					0x163U
#define UC_Channel_Front					0x164U
#define UC_Channel_Center_Front                       0x165U
#define UC_Channel_Side					      0x166U
#define UC_Channel_Surround                           0x167U
#define UC_Channel_Low_Frequency_Enhancement          0x168U
#define UC_Channel_Top					      0x169U
#define UC_Channel_Unknown					0x16AU
                                             /* Reserved */
#define UC_Subchannel					      0x170U
#define UC_Subchannel_Incr					0x171U
#define UC_Subchannel_Decr					0x172U
#define UC_Alternate_Audio_Incr                       0x173U
#define UC_Alternate_Audio_Decr                       0x174U
                                             /* Reserved */
#define UC_App_Launch_Btns					0x180U
#define UC_AL_Launch_Btn_Config_Tool                  0x181U
#define UC_AL_Prog_Btn_Config                         0x182U
#define UC_AL_Consumer_Ctrl_Config                    0x183U
#define UC_AL_Word_Processor                          0x184U
#define UC_AL_Text_Editor					0x185U
#define UC_AL_Spreadsheet					0x186U
#define UC_AL_Graphics_Editor				      0x187U
#define UC_AL_Presentation_App				0x188U
#define UC_AL_Database_App					0x189U
#define UC_AL_Email_Reader					0x18AU
#define UC_AL_Newsreader					0x18BU
#define UC_AL_Voicemail					      0x18CU
#define UC_AL_Contacts_Address_Book			      0x18DU
#define UC_AL_Calendar_Schedule				0x18EU
#define UC_AL_Task_Project_Manager			      0x18FU
#define UC_AL_Log_Journal_Timecard			      0x190U
#define UC_AL_Checkbook_Finance				0x191U
#define UC_AL_Calculator					0x192U
#define UC_AL_AV_Capture_Playback			      0x193U
#define UC_AL_Local_Machine_Browser			      0x194U
#define UC_AL_LAN_WAN_Browser				      0x195U
#define UC_AL_Internet_Browser				0x196U
#define UC_AL_RemoteNet_ISP_Connect			      0x197U
#define UC_AL_Net_Conference				      0x198U
#define UC_AL_Net_Chat					      0x199U
#define UC_AL_Telephony_Dialer				0x19AU
#define UC_AL_Logon					      0x19BU
#define UC_AL_Logoff					      0x19CU
#define UC_AL_Logon_Logoff					0x19DU
#define UC_AL_Terminal_Lock_Screensaver		      0x19EU
#define UC_AL_Ctrl_Panel					0x19FU
#define UC_AL_Command_Line_Processor_Run		      0x1A0U
#define UC_AL_Process_Task_Manager			      0x1A1U
#define UC_AL_Sel_Task_App					0x1A2U
#define UC_AL_Next_Task_App				      0x1A3U
#define UC_AL_Previous_Task_App				0x1A4U
#define UC_AL_Preemptive_Halt_Task_App			0x1A5U
#define UC_AL_Integrated_Help_Center			0x1A6U
#define UC_AL_Documents					      0x1A7U
#define UC_AL_Thesaurus					      0x1A8U
#define UC_AL_Dictionary					0x1A9U
#define UC_AL_Desktop					      0x1AAU
#define UC_AL_Spell_Check					0x1ABU
#define UC_AL_Grammar_Check				      0x1ACU
#define UC_AL_Wireless_Status				      0x1ADU
#define UC_AL_Keyboard_Layout				      0x1AEU
#define UC_AL_Virus_Protection				0x1AFU
#define UC_AL_Encryption					0x1B0U
#define UC_AL_Screen_Saver					0x1B1U
#define UC_AL_Alarms					      0x1B2U
#define UC_AL_Clock					      0x1B3U
#define UC_AL_File_Browser					0x1B4U
#define UC_AL_Power_Status					0x1B5U
#define UC_AL_Image_Browser				      0x1B6U
#define UC_AL_Audio_Browser				      0x1B7U
#define UC_AL_Movie_Browser				      0x1B8U
#define UC_AL_Digital_Rights_Manager			0x1B9U
#define UC_AL_Digital_Wallet				      0x1BAU
#define UC_AL_Instant_Messaging				0x1BCU
#define UC_AL_OEM_Features_Tips_Tutorial_Browser	0x1BDU
#define UC_AL_OEM_Help					      0x1BEU
#define UC_AL_Online_Community				0x1BFU
#define UC_AL_Entertainment_Content_Browser		0x1C0U
#define UC_AL_Online_Shopping_Browser			0x1C1U
#define UC_AL_SmartCard_Information_Help		      0x1C2U
#define UC_AL_Market_Monitoror_Finance_Browser	      0x1C3U
#define UC_AL_Customized_Corporate_News_Browser	      0x1C4U
#define UC_AL_Online_Activity_Browser			0x1C5U
#define UC_AL_Research_Search_Browser			0x1C6U
#define UC_AL_Audio_Player					0x1C7U
                                             /* Reserved */
#define UC_Generic_GUI_App_Ctrls				0x200U
#define UC_AC_New					            0x201U
#define UC_AC_Open					      0x202U
#define UC_AC_Close					      0x203U
#define UC_AC_Exit					      0x204U
#define UC_AC_Maximize					      0x205U
#define UC_AC_Minimize					      0x206U
#define UC_AC_Save					      0x207U
#define UC_AC_Print					      0x208U
#define UC_AC_Properties					0x209U
                                             /* Reserved */
#define UC_AC_Undo					      0x21AU
#define UC_AC_Copy					      0x21BU
#define UC_AC_Cut					            0x21CU
#define UC_AC_Paste					      0x21DU
#define UC_AC_Sel_All					      0x21EU
#define UC_AC_Find					      0x21FU
                                             /* Reserved */
#define UC_AC_Find_and_Replace				0x220U
#define UC_AC_Search					      0x221U
#define UC_AC_Go_To					      0x222U
#define UC_AC_Home					      0x223U
#define UC_AC_Back					      0x224U
#define UC_AC_Forward					      0x225U
#define UC_AC_Stop					      0x226U
#define UC_AC_Refresh					      0x227U
#define UC_AC_Previous_Link				      0x228U
#define UC_AC_Next_Link					      0x229U
#define UC_AC_Bookmarks					      0x22AU
#define UC_AC_History					      0x22BU
#define UC_AC_Subscriptions				      0x22CU
#define UC_AC_Zoom_In					      0x22DU
#define UC_AC_Zoom_Out					      0x22EU
#define UC_AC_Zoom					      0x22FU
#define UC_AC_Full_Screen_View				0x230U
#define UC_AC_Normal_View					0x231U
#define UC_AC_View_Toggle					0x232U
#define UC_AC_Scroll_Up					      0x233U
#define UC_AC_Scroll_Down					0x234U
#define UC_AC_Scroll					      0x235U
#define UC_AC_Pan_Left					      0x236U
#define UC_AC_Pan_Right					      0x237U
#define UC_AC_Pan					            0x238U
#define UC_AC_New_Window					0x239U
#define UC_AC_Tile_Horizontally				0x23AU
#define UC_AC_Tile_Vertically				      0x23BU
#define UC_AC_Format					      0x23CU
#define UC_AC_Edit					      0x23DU
#define UC_AC_Bold					      0x23EU
#define UC_AC_Italics					      0x23FU
#define UC_AC_Underline					      0x240U
#define UC_AC_Strikethrough				      0x241U
#define UC_AC_Subscript					      0x242U
#define UC_AC_Superscript					0x243U
#define UC_AC_All_Caps					      0x244U
#define UC_AC_Rotate					      0x245U
#define UC_AC_Resize					      0x246U
#define UC_AC_Flip_Horiz					0x247U
#define UC_AC_Flip_Verti					0x248U
#define UC_AC_Mirror_Horizontal				0x249U
#define UC_AC_Mirror_Vertical				      0x24AU
#define UC_AC_Font_Sel					      0x24BU
#define UC_AC_Font_Color					0x24CU
#define UC_AC_Font_Size					      0x24DU
#define UC_AC_Justify_Left					0x24EU
#define UC_AC_Justify_Center_H				0x24FU
#define UC_AC_Justify_Right				      0x250U
#define UC_AC_Justify_Block_H				      0x251U
#define UC_AC_Justify_Top					0x252U
#define UC_AC_Justify_Center_V				0x253U
#define UC_AC_Justify_Bottom				      0x254U
#define UC_AC_Justify_Block_V				      0x255U
#define UC_AC_Indent_Decr					0x256U
#define UC_AC_Indent_Incr					0x257U
#define UC_AC_Numbered_List				      0x258U
#define UC_AC_Restart_Numbering				0x259U
#define UC_AC_Bulleted_List				      0x25AU
#define UC_AC_Promote					      0x25BU
#define UC_AC_Demote					      0x25CU
#define UC_AC_Yes					            0x25DU
#define UC_AC_No					            0x25EU
#define UC_AC_Cancel					      0x25FU
#define UC_AC_Catalog					      0x260U
#define UC_AC_BuyorCheckout				      0x261U
#define UC_AC_Add_to_Cart					0x262U
#define UC_AC_Expand					      0x263U
#define UC_AC_Expand_All					0x264U
#define UC_AC_Collapse					      0x265U
#define UC_AC_Collapse_All					0x266U
#define UC_AC_Print_Preview				      0x267U
#define UC_AC_Paste_Special				      0x268U
#define UC_AC_Insert_Mode					0x269U
#define UC_AC_Delete					      0x26AU
#define UC_AC_Lock					      0x26BU
#define UC_AC_Unlock					      0x26CU
#define UC_AC_Protect					      0x26DU
#define UC_AC_Unprotect					      0x26EU
#define UC_AC_Attach_Comment				      0x26FU
#define UC_AC_Delete_Comment				      0x270U
#define UC_AC_View_Comment					0x271U
#define UC_AC_Sel_Word					      0x272U
#define UC_AC_Sel_Sentence					0x273U
#define UC_AC_Sel_Paragraph				      0x274U
#define UC_AC_Sel_Column					0x275U
#define UC_AC_Sel_Row					      0x276U
#define UC_AC_Sel_Table					      0x277U
#define UC_AC_Sel_Object					0x278U
#define UC_AC_Redo_Repeat					0x279U
#define UC_AC_Sort					      0x27AU
#define UC_AC_Sort_Ascending				      0x27BU
#define UC_AC_Sort_Descending				      0x27CU
#define UC_AC_Filter					      0x27DU
#define UC_AC_Set_Clock					      0x27EU
#define UC_AC_View_Clock					0x27FU
#define UC_AC_Sel_Time_Zone				      0x280U
#define UC_AC_Edit_Time_Zones				      0x281U
#define UC_AC_Set_Alarm					      0x282U
#define UC_AC_Clear_Alarm					0x283U
#define UC_AC_Snooze_Alarm					0x284U
#define UC_AC_Reset_Alarm					0x285U
#define UC_AC_Synchronize					0x286U
#define UC_AC_Send_or_Recv					0x287U
#define UC_AC_Send_To					      0x288U
#define UC_AC_Reply					      0x289U
#define UC_AC_Reply_All					      0x28AU
#define UC_AC_Forward_Msg					0x28BU
#define UC_AC_Send					      0x28CU
#define UC_AC_Attach_File					0x28DU
#define UC_AC_Upload					      0x28EU
#define UC_AC_Download_Save_As				0x28FU
#define UC_AC_Set_Borders					0x290U
#define UC_AC_Insert_Row					0x291U
#define UC_AC_Insert_Column				      0x292U
#define UC_AC_Insert_File					0x293U
#define UC_AC_Insert_Picture				      0x294U
#define UC_AC_Insert_Object				      0x295U
#define UC_AC_Insert_Symbol				      0x296U
#define UC_AC_Save_and_Close				      0x297U
#define UC_AC_Rename					      0x298U
#define UC_AC_Merge					      0x299U
#define UC_AC_Split					      0x29AU
#define UC_AC_Distribute_Horiz				0x29BU
#define UC_AC_Distribute_Verti				0x29CU
                                             /* Reserved */


/* Usage Page: Digitizer (0x0D)
*/
#define D_Digitizer	                              0x1U
#define D_Pen	                                    0x2U
#define D_Light_Pen	                              0x3U
#define D_Touch_Screen	                              0x4U
#define D_Touch_Pad	                              0x5U
#define D_White_Board	                              0x6U
#define D_Coordinate_Measuring_Machine	            0x7U
#define D_4D_Digitizer	                              0x8U
#define D_Stereo_Plotter	                        0x9U
#define D_Articulated_Arm	                        0xAU
#define D_Armature	                              0xBU
#define D_Multiple_Point_Digitizer	                  0xCU
#define D_Free_Space_Wand                    	      0xDU
                                             /* Reserved */
#define D_Stylus	                                    0x20U
#define D_Puck	                                    0x21U
#define D_Finger	                                    0x22U
                                             /* Reserved */
#define D_Tip_Pressure	                              0x30U
#define D_Barrel_Pressure	                        0x31U
#define D_In_Range	                              0x32U
#define D_Touch	                                    0x33U
#define D_Untouch	                                    0x34U
#define D_Tap	                                    0x35U
#define D_Quality	                                    0x36U
#define D_Data_Valid	                              0x37U
#define D_Transducer_Index	                        0x38U
#define D_Tablet_Function_Keys	                  0x39U
#define D_Program_Change_Keys	                        0x3AU
#define D_Battery_Strength	                        0x3BU
#define D_Invert	                                    0x3CU
#define D_X_Tilt	                                    0x3DU
#define D_Y_Tilt	                                    0x3EU
#define D_Azimuth	                                    0x3FU
#define D_Altitude	                              0x40U
#define D_Twist	                                    0x41U
#define D_Tip_Switch	                              0x42U
#define D_Secondary_Tip_Switch	                  0x43U
#define D_Barrel_Switch	                              0x44U
#define D_Eraser	                                    0x45U
#define D_Tablet_Pick	                              0x46U
                                             /* Reserved */


/* Usage Page: Alphanumeric Display (0x14)
*/
#define AD_Alphanumeric_Display				0x1U
#define AD_Bitmapped_Display				      0x2U
                                             /* Reserved */
#define AD_Display_Attributes_Report			0x20U
#define AD_ASCII_Character_Set				0x21U
#define AD_Data_Read_Back				      0x22U
#define AD_Font_Read_Back				      0x23U
#define AD_Display_Control_Report			      0x24U
#define AD_Clear_Display				      0x25U
#define AD_Display_Enable				      0x26U
#define AD_Screen_Saver_Delay				      0x27U
#define AD_Screen_Saver_Enable				0x28U
#define AD_Vertical_Scroll				      0x29U
#define AD_Horizontal_Scroll				      0x2AU
#define AD_Character_Report				      0x2BU
#define AD_Display_Data				            0x2CU
#define AD_Display_Status				      0x2DU
#define AD_Stat_Not_Ready				      0x2EU
#define AD_Stat_Ready				            0x2FU
#define AD_Err_Not_a_loadable_character		      0x30U
#define AD_Err_Font_data_cannot_be_read		      0x31U
#define AD_Cursor_Position_Report			      0x32U
#define AD_Row				                  0x33U
#define AD_Column				                  0x34U
#define AD_Rows				                  0x35U
#define AD_Columns				            0x36U
#define AD_Cursor_Pixel_Positioning			      0x37U
#define AD_Cursor_Mode				            0x38U
#define AD_Cursor_Enable				      0x39U
#define AD_Cursor_Blink				            0x3AU
#define AD_Font_Report				            0x3BU
#define AD_Font_Data				            0x3CU
#define AD_Character_Width				      0x3DU
#define AD_Character_Height				      0x3EU
#define AD_Character_Spacing_Horizontal		      0x3FU
#define AD_Character_Spacing_Vertical			0x40U
#define AD_Unicode_Character_Set				0x41U
#define AD_Font_7_Segment				      0x42U
#define AD_7_Segment_Direct_Map				0x43U
#define AD_Font_14_Segment				      0x44U
#define AD_14_Segment_Direct_Map				0x45U
#define AD_Display_Brightness				      0x46U
#define AD_Display_Contrast				      0x47U
#define AD_Character_Attribute				0x48U
#define AD_Attribute_Readback				      0x49U
#define AD_Attribute_Data				      0x4AU
#define AD_Char_Attr_Enhance				      0x4BU
#define AD_Char_Attr_Underline				0x4CU
#define AD_Char_Attr_Blink				      0x4DU
                                             /* Reserved */
#define AD_Bitmap_Size_X				      0x80U
#define AD_Bitmap_Size_Y				      0x81U
#define AD_Bit_Depth_Format				      0x83U
#define AD_Display_Orientation				0x84U
#define AD_Palette_Report				      0x85U
#define AD_Palette_Data_Size				      0x86U
#define AD_Palette_Data_Offset				0x87U
#define AD_Palette_Data				            0x88U
#define AD_Blit_Report				            0x8AU
#define AD_Blit_Rectangle_X1				      0x8BU
#define AD_Blit_Rectangle_Y1				      0x8CU
#define AD_Blit_Rectangle_X2				      0x8DU
#define AD_Blit_Rectangle_Y2				      0x8EU
#define AD_Blit_Data			            	0x8FU
#define AD_Soft_Button			      	      0x90U
#define AD_Soft_Button_ID		      		0x91U
#define AD_Soft_Button_Side				      0x92U
#define AD_Soft_Button_Offset_1				0x93U
#define AD_Soft_Button_Offset_2				0x94U
#define AD_Soft_Button_Report				      0x95U
                                             /* Reserved */


/* Usage Page: Medical Instrument (0x40)
*/
#define MI_Medical_Ultrasound				      0x1U
                                             /* Reserved */
#define MI_VCR_Acquisition				      0x20U
#define MI_Freeze_Thaw				            0x21U
#define MI_Clip_Store				            0x22U
#define MI_Update				                  0x23U
#define MI_Next				                  0x24U
#define MI_Save				                  0x25U
#define MI_Print				                  0x26U
#define MI_Microphone_Enable				      0x27U
                                             /* Reserved */
#define MI_Cine				                  0x40U
#define MI_Transmit_Power				      0x41U
#define MI_Volume				                  0x42U
#define MI_Focus				                  0x43U
#define MI_Depth              				0x44U
                                             /* Reserved */
#define MI_Soft_Step_Primary	      			0x60U
#define MI_Soft_Step_Secondary				0x61U
                                             /* Reserved */
#define MI_Depth_Gain_Compensation				0x70U
                                             /* Reserved */
#define MI_Zoom_Select			            	0x80U
#define MI_Zoom_Adjust        				0x81U
#define MI_Spectral_Doppler_Mode_Select			0x82U
#define MI_Spectral_Doppler_Adjust				0x83U
#define MI_Color_Doppler_Mode_Select			0x84U
#define MI_Color_Doppler_Adjust				0x85U
#define MI_Motion_Mode_Select		      		0x86U
#define MI_Motion_Mode_Adjust			      	0x87U
#define MI_2D_Mode_Select				      0x88U
#define MI_2D_Mode_Adjust				      0x89U
                                             /* Reserved */
#define MI_Soft_Control_Select				0xA0U
#define MI_Soft_Control_Adjust				0xA1U
                                             /* Reserved */

#include "stdint.h"
#include "bt_type.h"

typedef enum {
    MOUSE_PADDING_EN,
    MOUSE_BUTTON_EN,
    MOUSE_X_EN,
    MOUSE_Y_EN,
    MOUSE_WHEEL_EN,
    MOUSE_AC_PAN_EN,
} Mouse_Item_en;

typedef enum {
    KEYBOARD_PADDING_EN,
    KEYBOARD_BUTTON_EN,
} Keyboard_Item_en;

#define MAX_SUB_DEV_NUM (2)

#pragma pack(1)
typedef struct {
    uint8_t id_;
    uint16_t bit_count_;
    uint16_t start_bit_;
} Bit_Field_s;
#pragma pack()

#pragma pack(1)
typedef struct {
    uint8_t device_id_;
    uint8_t report_id_;
    Bit_Field_s* field_;
    uint8_t field_unit_count_;
} Sub_Dev_Info_s;
#pragma pack()

#pragma pack(1)
typedef struct {
    bt_addr_t addr_;
    Sub_Dev_Info_s dev_[MAX_SUB_DEV_NUM];
} Device_Info_s;
#pragma pack()


void app_bolt_poc_ri_usage(int32_t usagePage, int32_t usage, uint16_t idx);
void app_bolt_poc_process_dev_info(bt_handle_t handle);
void app_bolt_poc_field_buf_init();
bool app_bolt_poc_add_input_field(uint16_t idx);
bool app_bolt_poc_add_output_field(uint16_t idx);
bool app_bolt_poc_add_collection_field(uint16_t idx);
bool app_bolt_poc_add_end_collection_field(uint16_t idx);
bool app_bolt_poc_add_name_id_field(uint8_t name_id, uint16_t idx);
bool app_bolt_poc_add_count_field(int32_t count, uint16_t idx);
bool app_bolt_poc_add_size_field(int32_t size, uint16_t idx);
bool app_bolt_poc_add_report_id_field(int32_t report_id, uint16_t idx);

#endif