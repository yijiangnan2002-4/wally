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
 
#ifndef __HAL_SLEEP_MANAGER_H__
#define __HAL_SLEEP_MANAGER_H__
#include "hal_platform.h"

#ifdef HAL_SLEEP_MANAGER_ENABLED
#ifdef HAL_SLEEP_MANAGER_SUPPORT_WAKEUP_SOURCE_CONFIG
/**
 * @addtogroup HAL
 * @{
 * @addtogroup SLEEP_MANAGER
 * @{
 *
 * This section introduces the Sleep Manager driver APIs including terms and acronyms, supported features, software architecture, details on how to use this driver, Sleep Manager function groups, enums, structures and functions.
 *
 * @section HAL_Sleep_Manager_Terms_Chapter Terms and acronyms
 * |Terms                         |Details                                                                 |
 * |------------------------------|------------------------------------------------------------------------|
 * |\b Sleep                      | Set the CPU and related modules to Sleep mode to save power consumption and extend the battery life. |
 * |\b Wakeup \b Source           | An event that wakes the device up, such as a hardware interrupt triggered by a timer or a keypad. |
 * |\b Sleep \b Lock              | The Sleep Lock mechanism prevents the device to enter Sleep mode when required. |
 *
 * @section HAL_Sleep_Manager_Features_Chapter Supported features
 *
 * - \b Enter \b power \b saving \b mode \n
 */
#ifndef HAL_SLEEP_MANAGER_SUPPROT_DEEPSLEEP_LOCK
/*
 *    Check the Sleep Lock status by calling #hal_sleep_manager_is_sleep_locked(), then call hal_sleep_manager_enter_sleep_mode() to enter sleep mode, defined in #hal_sleep_mode_t, to save power.
 */
#else
/*
 *    Check the Sleep Lock status by calling #hal_sleep_manager_is_sleep_lock_locked(), then call hal_sleep_manager_enter_sleep_mode() to enter sleep mode, defined in #hal_sleep_mode_t, to save power.
 */
#endif
/*
 *
 * - \b Sleep \b lock \b control \n
 *    Call #hal_sleep_manager_lock_sleep() to apply Sleep Lock or call #hal_sleep_manager_unlock_sleep() to unlock the applied Sleep Lock.
 *
 * - \b Set \b wake \b up \b time \n
 *    Call #hal_sleep_manager_set_sleep_time() to wake up the system after a specific time in milliseconds.
 *
 * - \b Config \b the \b Wakeup \b Source \n
 *    It is possible to apply multiple Wakeup Sources in the hardware design to wake up the system from the Sleep Mode.
 *    Call #hal_sleep_manager_enable_wakeup_pin() to set a specific Wakeup Source. Call #hal_sleep_manager_disable_wakeup_pin() to disable the specific Wakeup Source.
 *
 */
#else
/**
 * @addtogroup HAL
 * @{
 * @addtogroup SLEEP_MANAGER
 * @{
 *
 * This section introduces the Sleep Manager driver APIs including terms and acronyms, supported features, software architecture, details on how to use this driver, Sleep Manager function groups, enums, structures and functions.
 *
 * @section HAL_Sleep_Manager_Terms_Chapter Terms and acronyms
 * |Terms                         |Details                                                                 |
 * |------------------------------|------------------------------------------------------------------------|
 * |\b Sleep                      | Set the CPU and related modules to Sleep mode to save power consumption and extend the battery life. |
 * |\b Sleep \b Lock              | The Sleep Lock mechanism prevents the device to enter Sleep mode when required. |
 *
 * @section HAL_Sleep_Manager_Features_Chapter Supported features
 *
 * - \b Enter \b power \b saving \b mode \n
 */
#ifndef HAL_SLEEP_MANAGER_SUPPROT_DEEPSLEEP_LOCK
/*
 *    Check the Sleep Lock status by calling #hal_sleep_manager_is_sleep_locked(), then call hal_sleep_manager_enter_sleep_mode() to enter sleep mode, defined in #hal_sleep_mode_t, to save power.
 */
#else
/*
 *    Check the Sleep Lock status by calling #hal_sleep_manager_is_sleep_lock_locked(), then call hal_sleep_manager_enter_sleep_mode() to enter sleep mode, defined in #hal_sleep_mode_t, to save power.
 */
#endif
/*
 *
 * - \b Sleep \b lock \b control \n
 *    Call #hal_sleep_manager_lock_sleep() to apply Sleep Lock or call #hal_sleep_manager_unlock_sleep() to unlock the applied Sleep Lock.
 *
 * - \b Set \b wake \b up \b time \n
 *    Call #hal_sleep_manager_set_sleep_time() to wake up the system after a specific time in milliseconds.
 *
 */
#endif

/**
 *
 * @section HAL_Sleep_Manager_Architecture_Chapter Software architecture of the Sleep Manager
 *
 * Call #hal_sleep_manager_init() to initialize the hardware before using the Sleep Manager APIs to control the sleep behavior of the device.
 *
 * For more information about Sleep Mode, please refer #hal_sleep_mode_t.
 *
 * @image html hal_sleep_manager_arch.png
 *
 * @section Hal_Sleep_Manager How to use this driver
 *
 * - Enter sleep mode: \n
 *  The steps are shown below:
 *  - Step1: Call #hal_sleep_manager_init() to initialize the Sleep Manager.
 */
#ifndef HAL_SLEEP_MANAGER_SUPPROT_DEEPSLEEP_LOCK
/**
 *  - Step2: Call #hal_sleep_manager_is_sleep_locked() to check the status of the Sleep Lock.
 */
#else
/**
 *  - Step2: Call #hal_sleep_manager_is_sleep_lock_locked() to check the status of the Sleep Lock.
 */
#endif
/**
 *  - Step3: Call #hal_sleep_manager_set_sleep_time() to set the sleep time in milliseconds (ms).
 *  - Step4: Call #hal_sleep_manager_enter_sleep_mode() to enter the sleep mode.
 *  - sample code:
 *    @code
 *    {
 *        if(hal_sleep_manager_init() == HAL_SLEEP_MANAGER_ERROR)
 *        {
 *            // Check the log for error handling.
 *            return;
 *        }
 */
#ifndef HAL_SLEEP_MANAGER_SUPPROT_DEEPSLEEP_LOCK
/**
 *        if(hal_sleep_manager_is_sleep_locked())
 */
#else
/**
 *        if(hal_sleep_manager_is_sleep_lock_locked(HAL_SLEEP_LOCK_ALL))
 */
#endif
/*
 *        {
 *            printf("Cannot enter the sleep mode, as the sleep is locked\n");
 *        }else
 *        {
 *            if (hal_sleep_manager_set_sleep_time(15000) == HAL_SLEEP_MANAGER_OK)
 *            {
 *                hal_sleep_manager_enter_sleep_mode(mode);
 *            }
 *        }
 *    }
 *
 *    @endcode
 *
 */
#ifdef HAL_SLEEP_MANAGER_SUPPORT_POWER_OFF
/**
 *
 * - Power off \n
 *  The steps are shown below:
 *  - Step1: Call #hal_sleep_manager_enter_power_off_mode() to power off the system.
 *  - sample code:
 *    @code
 *    {
 *        if(power_off)
 *        {
 *            hal_sleep_manager_enter_power_off_mode();
 *        }
 *    }
 *
 *    @endcode
 *
 */
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
* Enum
*****************************************************************************/
/**
 * @defgroup hal_sleep_manager_enum Enum
 * @{
 */

/** @brief This enum defines the Sleep Manager API return status*/
typedef enum {
    HAL_SLEEP_MANAGER_ERROR                           = -1,    /**< An undefined error occurred. */
    HAL_SLEEP_MANAGER_OK                              = 0,     /**< The operation completed successfully. */
} hal_sleep_manager_status_t;
/**
  * @}
  */

#ifdef HAL_SLEEP_MANAGER_SUPPROT_DEEPSLEEP_LOCK
/**
 * @defgroup hal_sleep_manager_typedef Typedef
 * @{
 */

/** @brief This typedef defines user's callback function prototype.
 *             User should call hal_sleep_manager_register_suspend_callback() to register callback before entering sleep mode,\n
 *             or call hal_sleep_manager_register_resume_callback() to register callback after leaving sleep mode.\n
 *             Note, that the callback function is not appropriate for time-consuming operations.\n
 *             parameter "data" : a user defined data used in the callback function.\n
 *             parameter "mode" : a combination value of #hal_sleep_mode_t to indicate in which sleep mode this callback should be called.
 */
typedef void (*hal_sleep_manager_callback_t)(void *data, uint32_t mode);
/**
  * @}
  */
#endif

/*****************************************************************************
 * Functions
 *****************************************************************************/

/**
 * @brief       This function initializes the Sleep Manager.
 * @return      #HAL_SLEEP_MANAGER_OK, if this function completed successfully. */
hal_sleep_manager_status_t hal_sleep_manager_init(void);

#ifdef HAL_SLEEP_MANAGER_SUPPORT_WAKEUP_SOURCE_CONFIG
/**
 * @brief       This function enables a specific Wakeup Source.
 * @param[in]   pin is the specified Wakeup Source defined in #hal_sleep_manager_wakeup_source_t.
 * @return      #HAL_SLEEP_MANAGER_OK, if this function completed successfully.
 */
hal_sleep_manager_status_t hal_sleep_manager_enable_wakeup_pin(hal_sleep_manager_wakeup_source_t pin);

/**
 * @brief       This function disables the specific Wakeup Source.
 * @param[in]   pin is the specific Wakeup Source to be disabled.
 * @return      #HAL_SLEEP_MANAGER_OK, if this function completed successfully.
 */
hal_sleep_manager_status_t hal_sleep_manager_disable_wakeup_pin(hal_sleep_manager_wakeup_source_t pin);
#endif

/**
 * @brief       This function checks if there are any Sleep Locks in the system. Apply it before entering the sleep mode.
 * @return      True if any locks are on hold.
 */
bool hal_sleep_manager_is_sleep_locked(void);

/**
 * @brief       This function sets up a sleep handle to control the sleep state of the system. \n
 *              This handle enables the Sleep Manager to identify the sleep states based on the sleep handles assigned.
 * @param[in]   handle_name is the name of the sleep handle of type string.
 * @return      The sleep handle index, if the operation is successful. If (0xFF) is returned, the operation failed.
 */
uint8_t hal_sleep_manager_set_sleep_handle(const char *handle_name);

/**
 * @brief       This function releases the sleep handle if it is no longer in use. \n
 * @param[in]   handle_index is the sleep handle index returned from #hal_sleep_manager_set_sleep_handle(). \n
 * @return      #HAL_SLEEP_MANAGER_OK, if this function completed successfully.
 * @note        Call #hal_sleep_manager_set_sleep_handle() to set up the sleep handle, before calling this function.
 */
hal_sleep_manager_status_t hal_sleep_manager_release_sleep_handle(uint8_t handle_index);

#ifndef HAL_SLEEP_MANAGER_SUPPROT_DEEPSLEEP_LOCK
/**
 * @brief       This function prevents the MCU from getting into sleep mode.
 * @param[in]   handle_index is the sleep handle index, returned from #hal_sleep_manager_set_sleep_handle().
 * @return      #HAL_SLEEP_MANAGER_OK, Sleep Lock is locked.
 * @note        Call #hal_sleep_manager_set_sleep_handle() to set up the sleep handle, before calling this function.
 */
hal_sleep_manager_status_t hal_sleep_manager_lock_sleep(uint8_t handle_index);

/**
 * @brief       This function unlocks the specific Sleep Lock and permits the MCU going into sleep mode when needed if all Sleep Locks are unlocked.
 * @param[in]   handle_index is the sleep handle index returned from #hal_sleep_manager_set_sleep_handle().
 * @return      #HAL_SLEEP_MANAGER_OK, sleep lock is unlocked.
 * @note        Call #hal_sleep_manager_set_sleep_handle() to set up the sleep handle, before calling this function.
 */
hal_sleep_manager_status_t hal_sleep_manager_unlock_sleep(uint8_t handle_index);

/**
 * @brief       The sleep lock status is represented by an unsigned integer (uint32_t). Each sleep handle corresponds to a bit position in this unsigned integer. If the sleep handle index is 5, for example, the value at the 5th bit position of this unsigned integer represents the status of the sleep handle of index 5. If the value of the 5th bit is 1, it indicates that the sleep handle with index 5 is currently holding the sleep lock.
 * @return      If handle_index's corresponding bit is 1, indicates this handle_index is holding a Sleep Lock.
 * @note        Call #hal_sleep_manager_set_sleep_handle() to set up the sleep handle, before calling this function.
 */
uint32_t hal_sleep_manager_get_lock_status(void);

/**
 * @brief       This function checks if there are any Sleep Locks in the system. Apply it before entering the sleep mode.
 * @return      True if any locks are on hold.
 */
bool hal_sleep_manager_is_sleep_locked(void);

#else

/**
 * @brief       This function prevents the MCU from getting into sleep or deep sleep mode.
 * @param[in]   handle_index is the sleep handle index returned from #hal_sleep_manager_set_sleep_handle().
 * @param[in]   level means which sleep mode to be controled.
 * @return      #HAL_SLEEP_MANAGER_OK, Sleep Lock is locked.
 * @note        Call #hal_sleep_manager_set_sleep_handle() to set up the sleep handle, before calling this function.
 */
hal_sleep_manager_status_t hal_sleep_manager_acquire_sleeplock(uint8_t handle_index, hal_sleep_lock_t level);

/**
 * @brief       This function unlocks the specific Sleep Lock and permits the MCU going into sleep mode when needed if all Sleep Locks are unlocked.
 * @param[in]   handle_index is the sleep handle index returned from #hal_sleep_manager_set_sleep_handle().
 * @param[in]   level means which sleep mode to be controled.
 * @return      #HAL_SLEEP_MANAGER_OK, sleep lock is unlocked.
 * @note        Call #hal_sleep_manager_set_sleep_handle() to set up the sleep handle, before calling this function.
 */
hal_sleep_manager_status_t hal_sleep_manager_release_sleeplock(uint8_t handle_index, hal_sleep_lock_t level);

/**
 * @brief       The sleep lock status is represented by an unsigned integer (uint32_t). Each sleep handle corresponds to a bit position in this unsigned integer. If the sleep handle index is 5, for example, the value at the 5th bit position of this unsigned integer represents the status of the sleep handle of index 5. If the value of the 5th bit is 1, it indicates that the sleep handle with index 5 is currently holding the sleep lock.
 * @param[in]   level means which sleep mode to be checked.
 * @param[in]   info_high contains the upper 32 handle_index's status.
 * @param[in]   info_low contains the lower 32 handle_index's status.
 * @return      ture means success.
 * @note        Call #hal_sleep_manager_set_sleep_handle() to set up the sleep handle, before calling this function.
 */
bool hal_sleep_manager_get_sleep_lock_status(hal_sleep_lock_t level, uint32_t* info_high, uint32_t* info_low);

/**
 * @brief       This function checks if there are any Sleep Locks with specific level in the system. Apply it before entering the sleep mode.
 * @param[in]   level means which sleep mode to be checked.
 * @return      True if any locks with specific level are on hold.
 */
bool hal_sleep_manager_is_sleep_lock_locked(hal_sleep_lock_t level);

/**
 * @brief       Register a callback function to be called before entering sleep mode.
 * @param[in]   callback is the user defined function to be registered.
 * @param[in]   data is a user defined data to be used in the callback function.
 * @param[in]   mode a combination value of #hal_sleep_mode_t to indicate in which sleep mode this callback should be called.
 */
void hal_sleep_manager_register_suspend_callback(hal_sleep_manager_callback_t callback, void *data, uint32_t mode);

/**
 * @brief       Register a callback function to be called after leaving sleep mode.
 * @param[in]   callback is the user defined function to be registered.
 * @param[in]   data is a user defined data to be used in the callback function.
 * @param[in]   mode a combination value of #hal_sleep_mode_t to indicate in which sleep mode this callback should be called.
 */
void hal_sleep_manager_register_resume_callback(hal_sleep_manager_callback_t callback, void *data, uint32_t mode);
#endif

/**
 * @brief       This function checks whether the sleep handle is still alive. If it is alive, it means that the system still can be controled by this sleep handle.
 * @param[in]   handle_index is the sleep handle index returned from #hal_sleep_manager_set_sleep_handle().
 * @return      True, if the the sleep handle is alive. False, if the sleep handle is invalid.
 */
bool hal_sleep_manager_is_sleep_handle_alive(uint8_t handle_index);

#ifdef HAL_SLEEP_MANAGER_SUPPORT_POWER_OFF
/**
 * @brief       This function sets the system to power off mode.
 * @note        If you are using this function, the system will be shut down immediately.
 * @par         Example
 */
void hal_sleep_manager_enter_power_off_mode(void);
#endif

/**
 * @brief       Set sleep(suspend) time length. If no other wake up event occurs, the system will sleep for a specific time.
 * @param[in]   sleep_time_ms is time duration in milliseconds.
 * @return      return HAL_SLEEP_MANAGER_OK, if supported.
 * @note        This api only use in non os environment
 */
hal_sleep_manager_status_t hal_sleep_manager_set_sleep_time(uint32_t sleep_time_ms);

/**
 * @brief       This function sets the system to any of the modes defined in #hal_sleep_mode_t.
 * @param[in]   mode is the desired sleep mode.
 */
void hal_sleep_manager_enter_sleep_mode(hal_sleep_mode_t mode);

#ifdef __cplusplus
}
#endif

/**
* @}
* @}
*/

#endif /*HAL_SLEEP_MANAGER_ENABLED*/
#endif /* __HAL_SLEEP_MANAGER_H__ */

