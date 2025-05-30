/*
* FreeRTOS Kernel V10.4.5
* Copyright (C) 2015-2019 Cadence Design Systems, Inc.
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

/*
 * Xtensa-specific API for RTOS ports.
 */

#ifndef __XTENSA_API_H__
#define __XTENSA_API_H__

#include <xtensa/hal.h>

#include "xtensa_context.h"


/* Typedef for C-callable interrupt handler function */
typedef void (*xt_handler)(void *);

/* Typedef for C-callable exception handler function */
typedef void (*xt_exc_handler)(XtExcFrame *);


/*
-------------------------------------------------------------------------------
  Call this function to set a handler for the specified exception.

    n        - Exception number (type)
    f        - Handler function address, NULL to uninstall handler.

  The handler will be passed a pointer to the exception frame, which is created
  on the stack of the thread that caused the exception.

  If the handler returns, the thread context will be restored and the faulting
  instruction will be retried. Any values in the exception frame that are
  modified by the handler will be restored as part of the context. For details
  of the exception frame structure see xtensa_context.h.
-------------------------------------------------------------------------------
*/
extern xt_exc_handler xt_set_exception_handler(int n, xt_exc_handler f);


/*
-------------------------------------------------------------------------------
  Call this function to set a handler for the specified interrupt.

    n        - Interrupt number.
    f        - Handler function address, NULL to uninstall handler.
    arg      - Argument to be passed to handler.
-------------------------------------------------------------------------------
*/
extern xt_handler xt_set_interrupt_handler(int n, xt_handler f, void *arg);


/*
-------------------------------------------------------------------------------
  Call this function to enable the specified interrupts.

    mask     - Bit mask of interrupts to be enabled.
-------------------------------------------------------------------------------
*/
extern void xt_ints_on(unsigned int mask);


/*
-------------------------------------------------------------------------------
  Call this function to disable the specified interrupts.

    mask     - Bit mask of interrupts to be disabled.
-------------------------------------------------------------------------------
*/
extern void xt_ints_off(unsigned int mask);


/*
-------------------------------------------------------------------------------
  Call this function to set the specified (s/w) interrupt.
-------------------------------------------------------------------------------
*/
static inline void xt_set_intset(unsigned int arg)
{
    xthal_set_intset(arg);
}


/*
-------------------------------------------------------------------------------
  Call this function to clear the specified (s/w or edge-triggered)
  interrupt.
-------------------------------------------------------------------------------
*/
static inline void xt_set_intclear(unsigned int arg)
{
    xthal_set_intclear(arg);
}


#endif /* __XTENSA_API_H__ */

