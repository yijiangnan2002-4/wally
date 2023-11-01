/*
 * FreeRTOS Kernel V10.4.5
 * Copyright (C) 2021 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

/* Standard includes. */
#include <stdint.h>

/* Defining MPU_WRAPPERS_INCLUDED_FROM_API_FILE ensures that PRIVILEGED_FUNCTION
 * is defined correctly and privileged functions are placed in correct sections. */
#define MPU_WRAPPERS_INCLUDED_FROM_API_FILE

/* Portasm includes. */
#include "portasm.h"
#include "memory_attribute.h"


/* MPU_WRAPPERS_INCLUDED_FROM_API_FILE is needed to be defined only for the
 * header files. */
#undef MPU_WRAPPERS_INCLUDED_FROM_API_FILE

void vRestoreContextOfFirstTask( void ) /* __attribute__ (( naked )) PRIVILEGED_FUNCTION */
{
    __asm volatile
    (
        "	.syntax unified									\n"
        "													\n"
        "	ldr  r2, pxCurrentTCBConst2						\n"/* Read the location of pxCurrentTCB i.e. &( pxCurrentTCB ). */
        "	ldr  r1, [r2]									\n"/* Read pxCurrentTCB. */
        "	ldr  r0, [r1]									\n"/* Read top of stack from TCB - The first item in pxCurrentTCB is the task top of stack. */
        "													\n"
        #if ( configENABLE_MPU == 1 )
            "	dmb												\n"/* Complete outstanding transfers before disabling MPU. */
            "	ldr r2, xMPUCTRLConst2							\n"/* r2 = 0xe000ed94 [Location of MPU_CTRL]. */
            "	ldr r4, [r2]									\n"/* Read the value of MPU_CTRL. */
            "	bic r4, #1										\n"/* r4 = r4 & ~1 i.e. Clear the bit 0 in r4. */
            "	str r4, [r2]									\n"/* Disable MPU. */
            "													\n"
            "	adds r1, #4										\n"/* r1 = r1 + 4. r1 now points to MAIR0 in TCB. */
            "	ldr  r3, [r1]									\n"/* r3 = *r1 i.e. r3 = MAIR0. */
            "	ldr  r2, xMAIR0Const2							\n"/* r2 = 0xe000edc0 [Location of MAIR0]. */
            "	str  r3, [r2]									\n"/* Program MAIR0. */
            "	ldr  r2, xRNRConst2								\n"/* r2 = 0xe000ed98 [Location of RNR]. */
            "	movs r3, #4										\n"/* r3 = 4. */
            "	str  r3, [r2]									\n"/* Program RNR = 4. */
            "	adds r1, #4										\n"/* r1 = r1 + 4. r1 now points to first RBAR in TCB. */
            "	ldr  r2, xRBARConst2							\n"/* r2 = 0xe000ed9c [Location of RBAR]. */
            "	ldmia r1!, {r4-r11}								\n"/* Read 4 set of RBAR/RLAR registers from TCB. */
            "	stmia r2!, {r4-r11}								\n"/* Write 4 set of RBAR/RLAR registers using alias registers. */
            "													\n"
            "	ldr r2, xMPUCTRLConst2							\n"/* r2 = 0xe000ed94 [Location of MPU_CTRL]. */
            "	ldr r4, [r2]									\n"/* Read the value of MPU_CTRL. */
            "	orr r4, #1										\n"/* r4 = r4 | 1 i.e. Set the bit 0 in r4. */
            "	str r4, [r2]									\n"/* Enable MPU. */
            "	dsb												\n"/* Force memory writes before continuing. */
        #endif /* configENABLE_MPU */
        "													\n"
        #if ( configENABLE_MPU == 1 )
            "	ldm  r0!, {r1-r3}								\n"/* Read from stack - r1 = PSPLIM, r2 = CONTROL and r3 = EXC_RETURN. */
            "	msr  psplim, r1									\n"/* Set this task's PSPLIM value. */
            "	msr  control, r2								\n"/* Set this task's CONTROL value. */
            "	adds r0, #32									\n"/* Discard everything up to r0. */
            "	msr  psp, r0									\n"/* This is now the new top of stack to use in the task. */
            "	isb												\n"
            "	mov  r0, #0										\n"
            "	msr  basepri, r0								\n"/* Ensure that interrupts are enabled when the first task starts. */
            "	bx   r3											\n"/* Finally, branch to EXC_RETURN. */
        #else /* configENABLE_MPU */
            "	ldm  r0!, {r1-r2}								\n"/* Read from stack - r1 = PSPLIM and r2 = EXC_RETURN. */
            "	msr  psplim, r1									\n"/* Set this task's PSPLIM value. */
            "	movs r1, #2										\n"/* r1 = 2. */
            "	msr  CONTROL, r1								\n"/* Switch to use PSP in the thread mode. */
            "	adds r0, #32									\n"/* Discard everything up to r0. */
            "	msr  psp, r0									\n"/* This is now the new top of stack to use in the task. */
            "	isb												\n"
            "	mov  r0, #0										\n"
            "	msr  basepri, r0								\n"/* Ensure that interrupts are enabled when the first task starts. */
            "	bx   r2											\n"/* Finally, branch to EXC_RETURN. */
        #endif /* configENABLE_MPU */
        "													\n"
        "	.align 4										\n"
        "pxCurrentTCBConst2: .word pxCurrentTCB				\n"
        #if ( configENABLE_MPU == 1 )
            "xMPUCTRLConst2: .word 0xe000ed94					\n"
            "xMAIR0Const2: .word 0xe000edc0						\n"
            "xRNRConst2: .word 0xe000ed98						\n"
            "xRBARConst2: .word 0xe000ed9c						\n"
        #endif /* configENABLE_MPU */
    );
}
/*-----------------------------------------------------------*/

BaseType_t xIsPrivileged( void ) /* __attribute__ (( naked )) */
{
    __asm volatile
    (
        "	mrs r0, control									\n"/* r0 = CONTROL. */
        "	tst r0, #1										\n"/* Perform r0 & 1 (bitwise AND) and update the conditions flag. */
        "	ite ne											\n"
        "	movne r0, #0									\n"/* CONTROL[0]!=0. Return false to indicate that the processor is not privileged. */
        "	moveq r0, #1									\n"/* CONTROL[0]==0. Return true to indicate that the processor is privileged. */
        "	bx lr											\n"/* Return. */
        "													\n"
        "	.align 4										\n"
        ::: "r0", "memory"
    );
}
/*-----------------------------------------------------------*/

void vRaisePrivilege( void ) /* __attribute__ (( naked )) PRIVILEGED_FUNCTION */
{
    __asm volatile
    (
        "	mrs  r0, control								\n"/* Read the CONTROL register. */
        "	bic r0, #1										\n"/* Clear the bit 0. */
        "	msr  control, r0								\n"/* Write back the new CONTROL value. */
        "	bx lr											\n"/* Return to the caller. */
        ::: "r0", "memory"
    );
}
/*-----------------------------------------------------------*/

void vResetPrivilege( void ) /* __attribute__ (( naked )) */
{
    __asm volatile
    (
        "	mrs r0, control									\n"/* r0 = CONTROL. */
        "	orr r0, #1										\n"/* r0 = r0 | 1. */
        "	msr control, r0									\n"/* CONTROL = r0. */
        "	bx lr											\n"/* Return to the caller. */
        ::: "r0", "memory"
    );
}
/*-----------------------------------------------------------*/

void vStartFirstTask( void ) /* __attribute__ (( naked )) PRIVILEGED_FUNCTION */
{
    __asm volatile
    (
        "	ldr r0, xVTORConst								\n"/* Use the NVIC offset register to locate the stack. */
        "	ldr r0, [r0]									\n"/* Read the VTOR register which gives the address of vector table. */
        "	ldr r0, [r0]									\n"/* The first entry in vector table is stack pointer. */
        "	msr msp, r0										\n"/* Set the MSP back to the start of the stack. */
        "	cpsie i											\n"/* Globally enable interrupts. */
        "	cpsie f											\n"
        "	dsb												\n"
        "	isb												\n"
        "	svc %0											\n"/* System call to start the first task. */
        "	nop												\n"
        "													\n"
        "   .align 4										\n"
        "xVTORConst: .word 0xe000ed08						\n"
        ::"i" ( portSVC_START_SCHEDULER ) : "memory"
    );
}
/*-----------------------------------------------------------*/

ATTR_TEXT_IN_RAM uint32_t ulSetInterruptMask( void ) /* __attribute__(( naked )) PRIVILEGED_FUNCTION */
{
	__asm volatile
	(
	"	mrs r0, basepri									\n" /* r0 = basepri. Return original basepri value. */
	"	mov r1, %0										\n" /* r1 = configMAX_SYSCALL_INTERRUPT_PRIORITY. */
	"	msr basepri, r1									\n" /* Disable interrupts upto configMAX_SYSCALL_INTERRUPT_PRIORITY. */
	"	dsb												\n"
	"	isb												\n"
	"	bx lr											\n" /* Return. */
	:: "i" ( configMAX_SYSCALL_INTERRUPT_PRIORITY ) : "memory"
	);
}
/*-----------------------------------------------------------*/

ATTR_TEXT_IN_RAM void vClearInterruptMask( __attribute__( ( unused ) ) uint32_t ulMask ) /* __attribute__(( naked )) PRIVILEGED_FUNCTION */
{
	__asm volatile
	(
	"	msr basepri, r0									\n" /* basepri = ulMask. */
	"	dsb												\n"
	"	isb												\n"
	"	bx lr											\n" /* Return. */
	::: "memory"
	);
}
/*-----------------------------------------------------------*/


#ifdef AIR_CONFIGGDB_HELPER
/* prototypes to avoid compiler warnings */
__attribute__ ((naked, used)) void vPortPendSVHandler_native(void);
__attribute__ ((naked, used)) void PendSV_Handler_jumper(void);

__attribute__ ((naked, used)) void vPortPendSVHandler_native(void) {
#else
ATTR_TEXT_IN_RAM void PendSV_Handler( void ) /* __attribute__ (( naked )) PRIVILEGED_FUNCTION */
{
#endif 
	__asm volatile
	(
	"	.syntax unified									\n"
	"													\n"
	"	cpsid i								            \n"
	"	push {lr}							            \n"
	"	blx Flash_ReturnReady				            \n" /* must suspend flash before fetch code from flash. */
#if defined(HAL_ESC_MODULE_ENABLED) && defined(ESC_FLASH_ENABLE)
	"	blx esc_flash_return_ready			            \n"
#endif
#if defined(MTK_SWLA_ENABLE) && defined(MTK_SWLA_ENABLE_O1)
	"   bl pendsv_swla_start				             \n"
#endif
	"	pop  {lr}							            \n"
	"	cpsie i 							            \n"
	"	mrs r0, psp										\n" /* Read PSP in r0. */
	#if( configENABLE_FPU == 1 )
	"	tst lr, #0x10									\n" /* Test Bit[4] in LR. Bit[4] of EXC_RETURN is 0 if the FPU is in use. */
	"	it eq											\n"
	"	vstmdbeq r0!, {s16-s31}							\n" /* Store the FPU registers which are not saved automatically. */
	#endif /* configENABLE_FPU */
	#if( configENABLE_MPU == 1 )
	"	mrs r1, psplim									\n" /* r1 = PSPLIM. */
	"	mrs r2, control									\n" /* r2 = CONTROL. */
	"	mov r3, lr										\n" /* r3 = LR/EXC_RETURN. */
	"	stmdb r0!, {r1-r11}								\n" /* Store on the stack - PSPLIM, CONTROL, LR and registers that are not automatically saved. */
	#else /* configENABLE_MPU */
	"	mrs r2, psplim									\n" /* r2 = PSPLIM. */
	"	mov r3, lr										\n" /* r3 = LR/EXC_RETURN. */
	"	stmdb r0!, {r2-r11}								\n" /* Store on the stack - PSPLIM, LR and registers that are not automatically saved. */
	#endif /* configENABLE_MPU */
	"													\n"
	"	ldr r2, pxCurrentTCBConstG						\n" /* Read the location of pxCurrentTCB i.e. &( pxCurrentTCB ). */
	"	ldr r1, [r2]									\n" /* Read pxCurrentTCB. */
	"	str r0, [r1]									\n" /* Save the new top of stack in TCB. */
	"													\n"
	"	mov r0, %0										\n" /* r0 = configMAX_SYSCALL_INTERRUPT_PRIORITY */
	"	msr basepri, r0									\n" /* Disable interrupts upto configMAX_SYSCALL_INTERRUPT_PRIORITY. */
	"	dsb												\n"
	"	isb												\n"
    #if defined(MTK_SYSTEM_HANG_TRACER_ENABLE_O1) || defined(MTK_SYSTEM_HANG_TRACER_ENABLE_O2)
	"	mrs r0, psp							            \n"
	"	bl systemhang_task_exit_trace	                \n"
    #endif /* defined(MTK_SYSTEM_HANG_TRACER_ENABLE_O1) || defined(MTK_SYSTEM_HANG_TRACER_ENABLE_O2)*/
    #ifdef MTK_MEMORY_MONITOR_ENABLE
	"	bl memorymonitor_task_exit_trace	            \n"
    #endif /* MTK_MEMORY_MONITOR_ENABLE */

	"	bl vTaskSwitchContext							\n"

    #ifdef MTK_MEMORY_MONITOR_ENABLE
	"	bl memorymonitor_task_enter_trace	            \n"
    #endif /* MTK_MEMORY_MONITOR_ENABLE */
    #if defined(MTK_SYSTEM_HANG_TRACER_ENABLE_O1) || defined(MTK_SYSTEM_HANG_TRACER_ENABLE_O2)
	"	bl systemhang_task_enter_trace	                \n"
    #endif /* defined(MTK_SYSTEM_HANG_TRACER_ENABLE_O1) || defined(MTK_SYSTEM_HANG_TRACER_ENABLE_O2)*/
    #if MTK_SYSTEM_HANG_TRACER_ENABLE
	"	bl systemhang_wdt_feed_in_task_switch           \n"
    #endif /* MTK_SYSTEM_HANG_TRACER_ENABLE */
	"													\n"
	
	#if (configCHECK_FOR_STACK_OVERFLOW > 0)
	#ifdef HAL_DWT_MODULE_ENABLED
	"	bl vPortCurrentTaskStackOverflowCheck\n" /* Enable the stack overflow check by DWT. */
	#endif /* HAL_DWT_MODULE_ENABLED */
	#endif /* (configCHECK_FOR_STACK_OVERFLOW > 0) */
	
	"	mov r0, #0										\n" /* r0 = 0. */
	"	msr basepri, r0									\n" /* Enable interrupts. */
	"													\n"
	"	ldr r2, pxCurrentTCBConstG						\n" /* Read the location of pxCurrentTCB i.e. &( pxCurrentTCB ). */
	"	ldr r1, [r2]									\n" /* Read pxCurrentTCB. */
	"	ldr r0, [r1]									\n" /* The first item in pxCurrentTCB is the task top of stack. r0 now points to the top of stack. */
	"													\n"
	#if( configENABLE_MPU == 1 )
	"	dmb												\n" /* Complete outstanding transfers before disabling MPU. */
	"	ldr r2, xMPUCTRLConst							\n" /* r2 = 0xe000ed94 [Location of MPU_CTRL]. */
	"	ldr r4, [r2]									\n" /* Read the value of MPU_CTRL. */
	"	bic r4, #1										\n" /* r4 = r4 & ~1 i.e. Clear the bit 0 in r4. */
	"	str r4, [r2]									\n" /* Disable MPU. */
	"													\n"
	"	adds r1, #4										\n" /* r1 = r1 + 4. r1 now points to MAIR0 in TCB. */
	"	ldr r3, [r1]									\n" /* r3 = *r1 i.e. r3 = MAIR0. */
	"	ldr r2, xMAIR0Const								\n" /* r2 = 0xe000edc0 [Location of MAIR0]. */
	"	str r3, [r2]									\n" /* Program MAIR0. */
	"	ldr r2, xRNRConst								\n" /* r2 = 0xe000ed98 [Location of RNR]. */
	"	movs r3, #4										\n" /* r3 = 4. */
	"	str r3, [r2]									\n" /* Program RNR = 4. */
	"	adds r1, #4										\n" /* r1 = r1 + 4. r1 now points to first RBAR in TCB. */
	"	ldr r2, xRBARConst								\n" /* r2 = 0xe000ed9c [Location of RBAR]. */
	"	ldmia r1!, {r4-r11}								\n" /* Read 4 sets of RBAR/RLAR registers from TCB. */
	"	stmia r2!, {r4-r11}								\n" /* Write 4 set of RBAR/RLAR registers using alias registers. */
	"													\n"
	"	ldr r2, xMPUCTRLConst							\n" /* r2 = 0xe000ed94 [Location of MPU_CTRL]. */
	"	ldr r4, [r2]									\n" /* Read the value of MPU_CTRL. */
	"	orr r4, #1										\n" /* r4 = r4 | 1 i.e. Set the bit 0 in r4. */
	"	str r4, [r2]									\n" /* Enable MPU. */
	"	dsb												\n" /* Force memory writes before continuing. */
	#endif /* configENABLE_MPU */
	"													\n"
	#if( configENABLE_MPU == 1 )
	"	ldmia r0!, {r1-r11}								\n" /* Read from stack - r1 = PSPLIM, r2 = CONTROL, r3 = LR and r4-r11 restored. */
	#else /* configENABLE_MPU */
	"	ldmia r0!, {r2-r11}								\n" /* Read from stack - r2 = PSPLIM, r3 = LR and r4-r11 restored. */
	#endif /* configENABLE_MPU */
	"													\n"
	#if( configENABLE_FPU == 1 )
	"	tst r3, #0x10									\n" /* Test Bit[4] in LR. Bit[4] of EXC_RETURN is 0 if the FPU is in use. */
	"	it eq											\n"
	"	vldmiaeq r0!, {s16-s31}							\n" /* Restore the FPU registers which are not restored automatically. */
	#endif /* configENABLE_FPU */
	"													\n"
	#if( configENABLE_MPU == 1 )
	"	msr psplim, r1									\n" /* Restore the PSPLIM register value for the task. */
	"	msr control, r2									\n" /* Restore the CONTROL register value for the task. */
	#else /* configENABLE_MPU */
	"	msr psplim, r2									\n" /* Restore the PSPLIM register value for the task. */
	#endif /* configENABLE_MPU */
	"	msr psp, r0										\n" /* Remember the new top of stack for the task. */
	"	bx r3											\n"
	"													\n"
	"	.align 4										\n"
	"pxCurrentTCBConstG: .word pxCurrentTCB				\n"
	#if( configENABLE_MPU == 1 )
	"xMPUCTRLConst: .word 0xe000ed94					\n"
	"xMAIR0Const: .word 0xe000edc0						\n"
	"xRNRConst: .word 0xe000ed98						\n"
	"xRBARConst: .word 0xe000ed9c						\n"
	#endif /* configENABLE_MPU */
	:: "i"( configMAX_SYSCALL_INTERRUPT_PRIORITY )
	);
}


#ifdef AIR_CONFIGGDB_HELPER /* if GDB debug helper is enabled */
/* Credits to:
 * - Artem Pisarneko for his initial contribution
 * - Prasana for the PendSVHandler updates
 * - Geoffrey Wossum for the Cortex-M4 contribution
 */

/* Switch control variable:
 * 0 - no hook installed (normal execution),
 * 1 - hook installation performed,
 * 2 - following hooked switches
 */
#ifdef __GNUC__
__attribute__((used))  /* needed for -flto and highest optimizations */
#endif
int volatile dbgPendSVHookState = 0;
/* Requested target task handle variable */
#ifdef __GNUC__
__attribute__((used))  /* needed for -flto and highest optimizations */
#endif
void *volatile dbgPendingTaskHandle;

#ifdef __GNUC__
__attribute__((used))  /* needed for -flto and highest optimizations */
#endif
const int volatile dbgFreeRTOSConfig_suspend_value = INCLUDE_vTaskSuspend;
#ifdef __GNUC__
__attribute__((used))  /* needed for -flto and highest optimizations */
#endif
const int volatile dbgFreeRTOSConfig_delete_value = INCLUDE_vTaskDelete;

__attribute__ ((naked, used)) void PendSV_Handler_jumper(void) {
  __asm volatile("b vPortPendSVHandler_native \n");
}
ATTR_TEXT_IN_RAM void PendSV_Handler( void ) /* __attribute__ (( naked )) PRIVILEGED_FUNCTION */
{ 
	__asm volatile
	(
	"	.syntax unified									\n"
	#ifdef AIR_CONFIGGDB_HELPER
    " ldr r1, _dbgPendSVHookState    \n" /* Check hook installed */
    " ldr r0, [r1]                   \n"
    " cmp r0, #0                     \n"
    " beq PendSV_Handler_jumper      \n" /* if no hook installed then jump to native handler, else proceed... */
    " cmp r0, #1                     \n" /* check whether hook triggered for the first time...  */
    " bne dbg_switch_to_pending_task \n" /* if not so, then jump to switching right now, otherwise current task context must be saved first...  */
    " mov r0, #2                     \n" /* mark hook after triggered for the first time */
    " str r0, [r1]                   \n"
	#endif /* AIR_CONFIGGDB_HELPER */
	"													\n"
	"	cpsid i								            \n"
	"	push {lr}							            \n"
	"	blx Flash_ReturnReady				            \n" /* must suspend flash before fetch code from flash. */
#if defined(HAL_ESC_MODULE_ENABLED) && defined(ESC_FLASH_ENABLE)
	"	blx esc_flash_return_ready			            \n"
#endif
#if defined(MTK_SWLA_ENABLE) && defined(MTK_SWLA_ENABLE_O1)
	"   bl pendsv_swla_start				             \n"
#endif
	"	pop  {lr}							            \n"
	"	cpsie i 							            \n"
	"	mrs r0, psp										\n" /* Read PSP in r0. */
	#if( configENABLE_FPU == 1 )
	"	tst lr, #0x10									\n" /* Test Bit[4] in LR. Bit[4] of EXC_RETURN is 0 if the FPU is in use. */
	"	it eq											\n"
	"	vstmdbeq r0!, {s16-s31}							\n" /* Store the FPU registers which are not saved automatically. */
	#endif /* configENABLE_FPU */
	#if( configENABLE_MPU == 1 )
	"	mrs r1, psplim									\n" /* r1 = PSPLIM. */
	"	mrs r2, control									\n" /* r2 = CONTROL. */
	"	mov r3, lr										\n" /* r3 = LR/EXC_RETURN. */
	"	stmdb r0!, {r1-r11}								\n" /* Store on the stack - PSPLIM, CONTROL, LR and registers that are not automatically saved. */
	#else /* configENABLE_MPU */
	"	mrs r2, psplim									\n" /* r2 = PSPLIM. */
	"	mov r3, lr										\n" /* r3 = LR/EXC_RETURN. */
	"	stmdb r0!, {r2-r11}								\n" /* Store on the stack - PSPLIM, LR and registers that are not automatically saved. */
	#endif /* configENABLE_MPU */
	"													\n"
	"	ldr r2, pxCurrentTCBConst						\n" /* Read the location of pxCurrentTCB i.e. &( pxCurrentTCB ). */
	"	ldr r1, [r2]									\n" /* Read pxCurrentTCB. */
	"	str r0, [r1]									\n" /* Save the new top of stack in TCB. */
	"													\n"
	"	mov r0, %0										\n" /* r0 = configMAX_SYSCALL_INTERRUPT_PRIORITY */
	"	msr basepri, r0									\n" /* Disable interrupts upto configMAX_SYSCALL_INTERRUPT_PRIORITY. */
	"	dsb												\n"
	"	isb												\n"
    #if defined(MTK_SYSTEM_HANG_TRACER_ENABLE_O1) || defined(MTK_SYSTEM_HANG_TRACER_ENABLE_O2)
	"	mrs r0, psp							            \n"
	"	bl systemhang_task_exit_trace	                \n"
    #endif /* defined(MTK_SYSTEM_HANG_TRACER_ENABLE_O1) || defined(MTK_SYSTEM_HANG_TRACER_ENABLE_O2)*/
    #ifdef MTK_MEMORY_MONITOR_ENABLE
	"	bl memorymonitor_task_exit_trace	            \n"
    #endif /* MTK_MEMORY_MONITOR_ENABLE */

	"	bl vTaskSwitchContext							\n"

    #ifdef MTK_MEMORY_MONITOR_ENABLE
	"	bl memorymonitor_task_enter_trace	            \n"
    #endif /* MTK_MEMORY_MONITOR_ENABLE */
    #if defined(MTK_SYSTEM_HANG_TRACER_ENABLE_O1) || defined(MTK_SYSTEM_HANG_TRACER_ENABLE_O2)
	"	bl systemhang_task_enter_trace	                \n"
    #endif /* defined(MTK_SYSTEM_HANG_TRACER_ENABLE_O1) || defined(MTK_SYSTEM_HANG_TRACER_ENABLE_O2)*/
    #if MTK_SYSTEM_HANG_TRACER_ENABLE
	"	bl systemhang_wdt_feed_in_task_switch           \n"
    #endif /* MTK_SYSTEM_HANG_TRACER_ENABLE */
	"													\n"
	
	#if (configCHECK_FOR_STACK_OVERFLOW > 0)
	#ifdef HAL_DWT_MODULE_ENABLED
	"	bl vPortCurrentTaskStackOverflowCheck\n" /* Enable the stack overflow check by DWT. */
	#endif /* HAL_DWT_MODULE_ENABLED */
	#endif /* (configCHECK_FOR_STACK_OVERFLOW > 0) */
	
	"	mov r0, #0										\n" /* r0 = 0. */
	"	msr basepri, r0									\n" /* Enable interrupts. */
	"													\n"
	#ifdef AIR_CONFIGGDB_HELPER
    "dbg_switch_to_pending_task:     \n"
    " ldr r2, _dbgPendingTaskHandle  \n" /* --> Load task handle going to switch to <-- */
	#else
	"	ldr r2, pxCurrentTCBConst						\n" /* Read the location of pxCurrentTCB i.e. &( pxCurrentTCB ). */
	#endif /* AIR_CONFIGGDB_HELPER */
	
	"	ldr r1, [r2]									\n" /* Read pxCurrentTCB. */
	"	ldr r0, [r1]									\n" /* The first item in pxCurrentTCB is the task top of stack. r0 now points to the top of stack. */
	"													\n"
	#if( configENABLE_MPU == 1 )
	"	dmb												\n" /* Complete outstanding transfers before disabling MPU. */
	"	ldr r2, xMPUCTRLConst							\n" /* r2 = 0xe000ed94 [Location of MPU_CTRL]. */
	"	ldr r4, [r2]									\n" /* Read the value of MPU_CTRL. */
	"	bic r4, #1										\n" /* r4 = r4 & ~1 i.e. Clear the bit 0 in r4. */
	"	str r4, [r2]									\n" /* Disable MPU. */
	"													\n"
	"	adds r1, #4										\n" /* r1 = r1 + 4. r1 now points to MAIR0 in TCB. */
	"	ldr r3, [r1]									\n" /* r3 = *r1 i.e. r3 = MAIR0. */
	"	ldr r2, xMAIR0Const								\n" /* r2 = 0xe000edc0 [Location of MAIR0]. */
	"	str r3, [r2]									\n" /* Program MAIR0. */
	"	ldr r2, xRNRConst								\n" /* r2 = 0xe000ed98 [Location of RNR]. */
	"	movs r3, #4										\n" /* r3 = 4. */
	"	str r3, [r2]									\n" /* Program RNR = 4. */
	"	adds r1, #4										\n" /* r1 = r1 + 4. r1 now points to first RBAR in TCB. */
	"	ldr r2, xRBARConst								\n" /* r2 = 0xe000ed9c [Location of RBAR]. */
	"	ldmia r1!, {r4-r11}								\n" /* Read 4 sets of RBAR/RLAR registers from TCB. */
	"	stmia r2!, {r4-r11}								\n" /* Write 4 set of RBAR/RLAR registers using alias registers. */
	"													\n"
	"	ldr r2, xMPUCTRLConst							\n" /* r2 = 0xe000ed94 [Location of MPU_CTRL]. */
	"	ldr r4, [r2]									\n" /* Read the value of MPU_CTRL. */
	"	orr r4, #1										\n" /* r4 = r4 | 1 i.e. Set the bit 0 in r4. */
	"	str r4, [r2]									\n" /* Enable MPU. */
	"	dsb												\n" /* Force memory writes before continuing. */
	#endif /* configENABLE_MPU */
	"													\n"
	#if( configENABLE_MPU == 1 )
	"	ldmia r0!, {r1-r11}								\n" /* Read from stack - r1 = PSPLIM, r2 = CONTROL, r3 = LR and r4-r11 restored. */
	#else /* configENABLE_MPU */
	"	ldmia r0!, {r2-r11}								\n" /* Read from stack - r2 = PSPLIM, r3 = LR and r4-r11 restored. */
	#endif /* configENABLE_MPU */
	"													\n"
	#if( configENABLE_FPU == 1 )
	"	tst r3, #0x10									\n" /* Test Bit[4] in LR. Bit[4] of EXC_RETURN is 0 if the FPU is in use. */
	"	it eq											\n"
	"	vldmiaeq r0!, {s16-s31}							\n" /* Restore the FPU registers which are not restored automatically. */
	#endif /* configENABLE_FPU */
	"													\n"
	#if( configENABLE_MPU == 1 )
	"	msr psplim, r1									\n" /* Restore the PSPLIM register value for the task. */
	"	msr control, r2									\n" /* Restore the CONTROL register value for the task. */
	#else /* configENABLE_MPU */
	"	msr psplim, r2									\n" /* Restore the PSPLIM register value for the task. */
	#endif /* configENABLE_MPU */
	"	msr psp, r0										\n" /* Remember the new top of stack for the task. */
	#ifdef AIR_CONFIGGDB_HELPER
    " bkpt                                              \n" /* <-- here debugger stops and steps out to target task context */
	#endif /* AIR_CONFIGGDB_HELPER */
	"	bx r3											\n"
	"													\n"
	"	.align 4										\n"
	"pxCurrentTCBConst: .word pxCurrentTCB				\n"
	#if( configENABLE_MPU == 1 )
	"xMPUCTRLConst: .word 0xe000ed94					\n"
	"xMAIR0Const: .word 0xe000edc0						\n"
	"xRNRConst: .word 0xe000ed98						\n"
	"xRBARConst: .word 0xe000ed9c						\n"
	#endif /* configENABLE_MPU */
	#ifdef AIR_CONFIGGDB_HELPER
    "_dbgPendSVHookState: .word dbgPendSVHookState     \n"
    "_dbgPendingTaskHandle: .word dbgPendingTaskHandle \n"
    ".word dbgFreeRTOSConfig_suspend_value             \n" /* force keep these symbols from cutting away by linker garbage collector */
    ".word dbgFreeRTOSConfig_delete_value              \n"
	#endif
	:: "i"( configMAX_SYSCALL_INTERRUPT_PRIORITY )
	);
}
#endif

/*-----------------------------------------------------------*/

ATTR_TEXT_IN_RAM void SVC_Handler( void ) /* __attribute__ (( naked )) PRIVILEGED_FUNCTION */
{
	__asm volatile
	(
	"	cpsid i							                \n"
	"	push {lr}							            \n"
	"	blx Flash_ReturnReady			                \n" /* must suspend flash before fetch code from flash */
	"	pop  {lr}							            \n"
	"	cpsie i 						                \n"
	"													\n"
	"	tst lr, #4										\n"
	"	ite eq											\n"
	"	mrseq r0, msp									\n"
	"	mrsne r0, psp									\n"
	"	ldr r1, svchandler_address_const				\n"
	"	bx r1											\n"
	"													\n"
	"	.align 4										\n"
	"svchandler_address_const: .word vPortSVCHandler_C	\n"
	);
}
/*-----------------------------------------------------------*/
