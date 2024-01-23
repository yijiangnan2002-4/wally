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

/*
 * A sample implementation of pvPortMalloc() and vPortFree() that combines
 * (coalescences) adjacent memory blocks as they are freed, and in so doing
 * limits memory fragmentation.
 *
 * See heap_1.c, heap_2.c and heap_3.c for alternative implementations, and the
 * memory management pages of https://www.FreeRTOS.org for more information.
 */
#include <stdlib.h>
#include <string.h>

/* Defining MPU_WRAPPERS_INCLUDED_FROM_API_FILE prevents task.h from redefining
 * all the API functions to use the MPU wrappers.  That should only be done when
 * task.h is included from an application file. */
#define MPU_WRAPPERS_INCLUDED_FROM_API_FILE

#include "FreeRTOS.h"
#include "task.h"

#include "hal.h"
#include "hal_nvic_internal.h"

#undef MPU_WRAPPERS_INCLUDED_FROM_API_FILE

#if ( configSUPPORT_DYNAMIC_ALLOCATION == 0 )
    #error This file must not be used if configSUPPORT_DYNAMIC_ALLOCATION is 0
#endif

/* Block sizes must not get too small. */
#define heapMINIMUM_BLOCK_SIZE    ( ( size_t ) ( xHeapStructSize << 1 ) )

/* Assumes 8bit bytes! */
#define heapBITS_PER_BYTE         ( ( size_t ) 8 )

/* Allocate the memory for the heap. */
#if ( configAPPLICATION_ALLOCATED_HEAP == 1 )

/* The application writer has already defined the array used for the RTOS
* heap - probably so it can be placed in a special segment or address. */
    extern uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];
#else
    PRIVILEGED_DATA static uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];
#endif /* configAPPLICATION_ALLOCATED_HEAP */

/* Define the linked list structure.  This is used to link free blocks in order
 * of their memory address. */
typedef struct A_BLOCK_LINK
{
    struct A_BLOCK_LINK * pxNextFreeBlock; /*<< The next free block in the list. */
    size_t xBlockSize;                     /*<< The size of the free block. */
#ifdef MTK_SUPPORT_HEAP_DEBUG
	uint32_t xMallocLinkRegAddr;
	uint32_t xFreeLinkRegAddr;
#elif defined(MTK_SUPPORT_HEAP_DEBUG_ADVANCED)
	uint32_t xTick;
	uint32_t xTrace[5];
#endif /* MTK_SUPPORT_HEAP_DEBUG */
} BlockLink_t;

/*-----------------------------------------------------------*/

/*
 * Inserts a block of memory that is being freed into the correct position in
 * the list of free memory blocks.  The block being freed will be merged with
 * the block in front it and/or the block behind it if the memory blocks are
 * adjacent to each other.
 */
static void prvInsertBlockIntoFreeList( BlockLink_t * pxBlockToInsert ) PRIVILEGED_FUNCTION;

/*
 * Called automatically to setup the required heap structures the first time
 * pvPortMalloc() is called.
 */
static void prvHeapInit( void ) PRIVILEGED_FUNCTION;

/*-----------------------------------------------------------*/

/* The size of the structure placed at the beginning of each allocated memory
 * block must by correctly byte aligned. */
static const size_t xHeapStructSize = ( sizeof( BlockLink_t ) + ( ( size_t ) ( portBYTE_ALIGNMENT - 1 ) ) ) & ~( ( size_t ) portBYTE_ALIGNMENT_MASK );

/* Create a couple of list links to mark the start and end of the list. */
PRIVILEGED_DATA static BlockLink_t xStart, * pxEnd = NULL;

/* Keeps track of the number of calls to allocate and free memory as well as the
 * number of free bytes remaining, but says nothing about fragmentation. */
PRIVILEGED_DATA static size_t xFreeBytesRemaining = 0U;
PRIVILEGED_DATA static size_t xMinimumEverFreeBytesRemaining = 0U;
PRIVILEGED_DATA static size_t xNumberOfSuccessfulAllocations = 0;
PRIVILEGED_DATA static size_t xNumberOfSuccessfulFrees = 0;

/* Gets set to the top bit of an size_t type.  When this bit in the xBlockSize
 * member of an BlockLink_t structure is set then the block belongs to the
 * application.  When the bit is free the block is still part of the free heap
 * space. */
PRIVILEGED_DATA static size_t xBlockAllocatedBit = 0;

/*-----------------------------------------------------------*/
#ifdef MTK_SUPPORT_HEAP_DEBUG_ADVANCED
#include <unwind.h>

typedef struct {
	void **array;
	uint32_t nest_counts;
	uint32_t max_nest_counts;
}trace_arg_t;

static _Unwind_Reason_Code trace_func(void* context, void* arg)
{
	trace_arg_t *state = (trace_arg_t *)arg;
	if(state->max_nest_counts > 0)
	{
		void *ip = (void *)_Unwind_GetIP(context);
		if(ip == 0)
		{
			return _URC_END_OF_STACK;

		} else {
			/* Ignore duplicate addresses */
			if (state->nest_counts > 0 && ip == state->array[state->nest_counts - 1])
				return _URC_END_OF_STACK;

			state->array[state->nest_counts++] = ip;
			if(state->nest_counts == state->max_nest_counts)
				return _URC_END_OF_STACK;
		}
	}
	return _URC_NO_REASON;
}
#endif /* MTK_SUPPORT_HEAP_DEBUG_ADVANCED */

#if defined(MTK_SUPPORT_HEAP_DEBUG) || defined(MTK_HEAP_SIZE_GUARD_ENABLE) || defined(MTK_SUPPORT_HEAP_DEBUG_ADVANCED)
/* record first block of heap for heap walk */
BlockLink_t *pxFirstBlock;
#endif

ATTR_TEXT_IN_TCM void *pvPortMalloc( size_t xWantedSize )
{
BlockLink_t *pxBlock, *pxPreviousBlock, *pxNewBlockLink;
void *pvReturn = NULL;

#ifdef MTK_SUPPORT_HEAP_DEBUG
/* Obtain the return address of caller from link register */
#if defined(__GNUC__)
uint32_t xLinkRegAddr = (uint32_t)__builtin_return_address(0);
#elif defined(__CC_ARM)
uint32_t xLinkRegAddr = __return_address();
#elif defined(__ICCARM__)
uint32_t xLinkRegAddr = __get_LR();
#endif /* __GNUC__ */
#endif /* MTK_SUPPORT_HEAP_DEBUG */
uint32_t mask;

#ifdef AIR_CHECK_BLOCK_SIZE_ENABLE
    configASSERT(xWantedSize > xHeapStructSize);
#endif

#ifdef MTK_SUPPORT_HEAP_DEBUG_ADVANCED
	#define MAX_DEPTH 5
	const void* stack[MAX_DEPTH] = {NULL,NULL,NULL,NULL,NULL};
	trace_arg_t state = {(void*)&stack, 0, MAX_DEPTH};
	_Unwind_Backtrace(trace_func,&state);
#endif /* MTK_SUPPORT_HEAP_DEBUG_ADVANCED */

	//vTaskSuspendAll();
	hal_nvic_save_and_set_interrupt_mask_special(&mask);
	{
		/* If this is the first call to malloc then the heap will require
		initialisation to setup the list of free blocks. */
		if( pxEnd == NULL )
		{
			prvHeapInit();
			/* Register heap dump callback */
			#if defined(MTK_SUPPORT_HEAP_DEBUG) || defined(MTK_SUPPORT_HEAP_DEBUG_ADVANCED)
			extern void prvPortDumpHeapInfo(void);
			prvPortDumpHeapInfo();
			#endif /* MTK_SUPPORT_HEAP_DEBUG */
		}
		else
		{
			mtCOVERAGE_TEST_MARKER();
		}

		/* Check the requested block size is not so large that the top bit is
		set.  The top bit of the block size member of the BlockLink_t structure
		is used to determine who owns the block - the application or the
		kernel, so it must be free. */
		if( ( xWantedSize & xBlockAllocatedBit ) == 0 )
		{
            /* The wanted size must be increased so it can contain a BlockLink_t
             * structure in addition to the requested amount of bytes. */
            if( ( xWantedSize > 0 ) &&
                ( ( xWantedSize + xHeapStructSize ) >  xWantedSize ) ) /* Overflow check */
            {
                xWantedSize += xHeapStructSize;

                /* Ensure that blocks are always aligned. */
                if( ( xWantedSize & portBYTE_ALIGNMENT_MASK ) != 0x00 )
                {
                    /* Byte alignment required. Check for overflow. */
                    if( ( xWantedSize + ( portBYTE_ALIGNMENT - ( xWantedSize & portBYTE_ALIGNMENT_MASK ) ) )
                            > xWantedSize )
                    {
                        xWantedSize += ( portBYTE_ALIGNMENT - ( xWantedSize & portBYTE_ALIGNMENT_MASK ) );
                        configASSERT( ( xWantedSize & portBYTE_ALIGNMENT_MASK ) == 0 );
                    }
                    else
                    {
                        xWantedSize = 0;
                    }
                }
                else
                {
                    mtCOVERAGE_TEST_MARKER();
                }
            }
            else
            {
                xWantedSize = 0;
            }

            if( ( xWantedSize > 0 ) && ( xWantedSize <= xFreeBytesRemaining ) )
            {
                /* Traverse the list from the start (lowest address) block until
                 * one of adequate size is found. */
                pxPreviousBlock = &xStart;
                pxBlock = xStart.pxNextFreeBlock;

                while( ( pxBlock->xBlockSize < xWantedSize ) && ( pxBlock->pxNextFreeBlock != NULL ) )
                {
                    pxPreviousBlock = pxBlock;
                    pxBlock = pxBlock->pxNextFreeBlock;
                }

                /* If the end marker was reached then a block of adequate size
                 * was not found. */
                if( pxBlock != pxEnd )
                {
                    /* Return the memory space pointed to - jumping over the
                     * BlockLink_t structure at its start. */
                    pvReturn = ( void * ) ( ( ( uint8_t * ) pxPreviousBlock->pxNextFreeBlock ) + xHeapStructSize );
					#ifdef MTK_SUPPORT_HEAP_DEBUG
					pxPreviousBlock->pxNextFreeBlock->xMallocLinkRegAddr = xLinkRegAddr;
					#elif defined(MTK_SUPPORT_HEAP_DEBUG_ADVANCED)
					pxPreviousBlock->pxNextFreeBlock->xTick = xTaskGetTickCount();
					memcpy(pxPreviousBlock->pxNextFreeBlock->xTrace, stack, sizeof(stack));
					#endif /* MTK_SUPPORT_HEAP_DEBUG_ADVANCED */
                    /* This block is being returned for use so must be taken out
                     * of the list of free blocks. */
                    pxPreviousBlock->pxNextFreeBlock = pxBlock->pxNextFreeBlock;

                    /* If the block is larger than required it can be split into
                     * two. */
                    if( ( pxBlock->xBlockSize - xWantedSize ) > heapMINIMUM_BLOCK_SIZE )
                    {
                        /* This block is to be split into two.  Create a new
                         * block following the number of bytes requested. The void
                         * cast is used to prevent byte alignment warnings from the
                         * compiler. */
                        pxNewBlockLink = ( void * ) ( ( ( uint8_t * ) pxBlock ) + xWantedSize );
                        configASSERT( ( ( ( size_t ) pxNewBlockLink ) & portBYTE_ALIGNMENT_MASK ) == 0 );

                        /* Calculate the sizes of two blocks split from the
                         * single block. */
                        pxNewBlockLink->xBlockSize = pxBlock->xBlockSize - xWantedSize;
                        pxBlock->xBlockSize = xWantedSize;

                        /* Insert the new block into the list of free blocks. */
                        prvInsertBlockIntoFreeList( pxNewBlockLink );
                    }
                    else
                    {
                        mtCOVERAGE_TEST_MARKER();
                    }

                    xFreeBytesRemaining -= pxBlock->xBlockSize;

                    if( xFreeBytesRemaining < xMinimumEverFreeBytesRemaining )
                    {
                        xMinimumEverFreeBytesRemaining = xFreeBytesRemaining;
                    }
                    else
                    {
                        mtCOVERAGE_TEST_MARKER();
                    }

                    /* The block is being returned - it is allocated and owned
                     * by the application and has no "next" block. */
                    pxBlock->xBlockSize |= xBlockAllocatedBit;
                    pxBlock->pxNextFreeBlock = NULL;
                    xNumberOfSuccessfulAllocations++;
                }
                else
                {
                    mtCOVERAGE_TEST_MARKER();
                }
            }
            else
            {
                mtCOVERAGE_TEST_MARKER();
            }
        }
        else
        {
            mtCOVERAGE_TEST_MARKER();
        }

        traceMALLOC( pvReturn, xWantedSize );
    }

	//( void ) xTaskResumeAll();
	hal_nvic_restore_interrupt_mask_special(mask);

    #if ( configUSE_MALLOC_FAILED_HOOK == 1 )
        {
		if( (pvReturn == NULL) && (xWantedSize > 0) )
            {
                extern void vApplicationMallocFailedHook( void );
                vApplicationMallocFailedHook();
            }
            else
            {
                mtCOVERAGE_TEST_MARKER();
            }
        }
    #endif /* if ( configUSE_MALLOC_FAILED_HOOK == 1 ) */

    configASSERT( ( ( ( size_t ) pvReturn ) & ( size_t ) portBYTE_ALIGNMENT_MASK ) == 0 );
    return pvReturn;
}
/*-----------------------------------------------------------*/

void vPortFree( void * pv )
{
uint8_t *puc = ( uint8_t * ) pv;
BlockLink_t *pxLink, *pxNextLink;
uint32_t mask;

#ifdef MTK_SUPPORT_HEAP_DEBUG
/* Obtain the return address of caller from link register */
#if defined(__GNUC__)
uint32_t xLinkRegAddr = (uint32_t)__builtin_return_address(0);
#elif defined(__CC_ARM)
uint32_t xLinkRegAddr = __return_address();
#elif defined(__ICCARM__)
uint32_t xLinkRegAddr = __get_LR();
#else
	#error "plase define (__GNUC__) or (__CC_ARM) or (__ICCARM__)"
#endif /* __GNUC__ */
#endif /* MTK_SUPPORT_HEAP_DEBUG */

	if( pv != NULL )
	{
		/* The memory being freed will have an BlockLink_t structure immediately
		before it. */
		puc -= xHeapStructSize;

		/* This casting is to keep the compiler from issuing warnings. */
		pxLink = ( void * ) puc;

		/* Check the block is actually allocated. */
		configASSERT( ( pxLink->xBlockSize & xBlockAllocatedBit ) != 0 );
		configASSERT( pxLink->pxNextFreeBlock == NULL );

		if( ( pxLink->xBlockSize & xBlockAllocatedBit ) != 0 )
		{
			if( pxLink->pxNextFreeBlock == NULL )
			{
				#ifdef MTK_SUPPORT_HEAP_DEBUG
				pxLink->xFreeLinkRegAddr = xLinkRegAddr;
				#elif defined(MTK_SUPPORT_HEAP_DEBUG_ADVANCED)
				pxLink->xTick = xTaskGetTickCount();
				#endif /* MTK_SUPPORT_HEAP_DEBUG */

				//vTaskSuspendAll();
				hal_nvic_save_and_set_interrupt_mask(&mask);
				{
				/* The block is being returned to the heap - it is no longer
				allocated. */
					#ifdef MTK_HEAP_SIZE_GUARD_ENABLE
				#define GET_ACTUAL_BLOCK_SIZE(xBlockSize)  ((xBlockSize) & (uint32_t)0x00FFFFFF)    /* mask off top byte*/
				pxLink->xBlockSize = GET_ACTUAL_BLOCK_SIZE(pxLink->xBlockSize);
					#else
				pxLink->xBlockSize &= ~xBlockAllocatedBit;
					#endif /* MTK_HEAP_SIZE_GUARD_ENABLE */

					/* To check is there any memory corruption on the next block.
					** MTK_HEAP_SIZE_GUARD_ENABLE feature is a internal debug feature, and will use some bits of xBlockSize.
					** Ignore the check when MTK_HEAP_SIZE_GUARD_ENABLE feature is enabled.
					*/
					#ifndef MTK_HEAP_SIZE_GUARD_ENABLE
				{
						/* Get the next block address */
						pxNextLink = ( BlockLink_t * ) ((uint8_t *)pxLink + (pxLink->xBlockSize & ~xBlockAllocatedBit));
						if((pxNextLink != NULL) && (pxNextLink != pxEnd))
						{
							/* the next block size must be bigger than the heap header size + mini-mum user size(portBYTE_ALIGNMENT) */
							configASSERT((pxNextLink->xBlockSize & ~xBlockAllocatedBit) >= (xHeapStructSize + portBYTE_ALIGNMENT))
							/* the next block must be in the valid heap space */
							configASSERT( ((uint32_t)(pxNextLink) >= (uint32_t)ucHeap) && ((uint32_t)(pxNextLink) <= (uint32_t)ucHeap + configTOTAL_HEAP_SIZE));

							if(pxNextLink->pxNextFreeBlock == NULL){
								/* the block is a in-use block, the allocated-flag must be set */
								configASSERT( ( pxNextLink->xBlockSize & xBlockAllocatedBit ) != 0 );
							}else{
								/* the block is a free block, the next-pointer must be in the valid heap space */
								configASSERT( ((uint32_t)(pxNextLink->pxNextFreeBlock) >= (uint32_t)ucHeap) && ((uint32_t)(pxNextLink->pxNextFreeBlock) <= (uint32_t)ucHeap + configTOTAL_HEAP_SIZE));
							}
						}
					}
					#endif /* MTK_HEAP_SIZE_GUARD_ENABLE */
					/* Add this block to the list of free blocks. */
					xFreeBytesRemaining += pxLink->xBlockSize;
					traceFREE( pv, pxLink->xBlockSize );
					prvInsertBlockIntoFreeList( ( ( BlockLink_t * ) pxLink ) );
                    xNumberOfSuccessfulFrees++;
				}
				//( void ) xTaskResumeAll();
				hal_nvic_restore_interrupt_mask(mask);
			}
			else
			{
				mtCOVERAGE_TEST_MARKER();
			}
		}
		else
		{
			mtCOVERAGE_TEST_MARKER();
		}
	}
}
/*-----------------------------------------------------------*/

size_t xPortGetFreeHeapSize( void )
{
	return xFreeBytesRemaining;
}
/*-----------------------------------------------------------*/

size_t xPortGetMinimumEverFreeHeapSize( void )
{
	return xMinimumEverFreeBytesRemaining;
}
/*-----------------------------------------------------------*/

void vPortInitialiseBlocks( void )
{
	/* This just exists to keep the linker quiet. */
}
/*-----------------------------------------------------------*/

static void prvHeapInit( void )
{
BlockLink_t *pxFirstFreeBlock;
uint8_t *pucAlignedHeap;
size_t uxAddress;
size_t xTotalHeapSize = configTOTAL_HEAP_SIZE;

	/* Ensure the heap starts on a correctly aligned boundary. */
	uxAddress = ( size_t ) ucHeap;

	if( ( uxAddress & portBYTE_ALIGNMENT_MASK ) != 0 )
	{
		uxAddress += ( portBYTE_ALIGNMENT - 1 );
		uxAddress &= ~( ( size_t ) portBYTE_ALIGNMENT_MASK );
		xTotalHeapSize -= uxAddress - ( size_t ) ucHeap;
	}

	pucAlignedHeap = ( uint8_t * ) uxAddress;

	/* xStart is used to hold a pointer to the first item in the list of free
	blocks.  The void cast is used to prevent compiler warnings. */
	xStart.pxNextFreeBlock = ( void * ) pucAlignedHeap;
	xStart.xBlockSize = ( size_t ) 0;

	/* pxEnd is used to mark the end of the list of free blocks and is inserted
	at the end of the heap space. */
	uxAddress = ( ( size_t ) pucAlignedHeap ) + xTotalHeapSize;
	uxAddress -= xHeapStructSize;
	uxAddress &= ~( ( size_t ) portBYTE_ALIGNMENT_MASK );
	pxEnd = ( void * ) uxAddress;
	pxEnd->xBlockSize = 0;
	pxEnd->pxNextFreeBlock = NULL;

	/* To start with there is a single free block that is sized to take up the
	entire heap space, minus the space taken by pxEnd. */
	pxFirstFreeBlock = ( void * ) pucAlignedHeap;
	pxFirstFreeBlock->xBlockSize = uxAddress - ( size_t ) pxFirstFreeBlock;
	pxFirstFreeBlock->pxNextFreeBlock = pxEnd;
	#if defined(MTK_SUPPORT_HEAP_DEBUG) || defined(MTK_HEAP_SIZE_GUARD_ENABLE) || defined(MTK_SUPPORT_HEAP_DEBUG_ADVANCED)
	pxFirstBlock = pxFirstFreeBlock;
	#endif

	/* Only one block exists - and it covers the entire usable heap space. */
	xMinimumEverFreeBytesRemaining = pxFirstFreeBlock->xBlockSize;
	xFreeBytesRemaining = pxFirstFreeBlock->xBlockSize;

	/* Work out the position of the top bit in a size_t variable. */
	xBlockAllocatedBit = ( ( size_t ) 1 ) << ( ( sizeof( size_t ) * heapBITS_PER_BYTE ) - 1 );
}
/*-----------------------------------------------------------*/

static void prvInsertBlockIntoFreeList( BlockLink_t *pxBlockToInsert )
{
BlockLink_t *pxIterator;
uint8_t *puc;

	/* Iterate through the list until a block is found that has a higher address
	than the block being inserted. */
	for( pxIterator = &xStart; pxIterator->pxNextFreeBlock < pxBlockToInsert; pxIterator = pxIterator->pxNextFreeBlock )
	{
		/* Nothing to do here, just iterate to the right position. */
	}

	/* Do the block being inserted, and the block it is being inserted after
	make a contiguous block of memory? */
	puc = ( uint8_t * ) pxIterator;
	if( ( puc + pxIterator->xBlockSize ) == ( uint8_t * ) pxBlockToInsert )
	{
		pxIterator->xBlockSize += pxBlockToInsert->xBlockSize;
		pxBlockToInsert = pxIterator;
	}
	else
	{
		mtCOVERAGE_TEST_MARKER();
	}

	/* Do the block being inserted, and the block it is being inserted before
	make a contiguous block of memory? */
	puc = ( uint8_t * ) pxBlockToInsert;

	configASSERT(((uint32_t)pxIterator->pxNextFreeBlock > (uint32_t)ucHeap) && ((uint32_t)pxIterator->pxNextFreeBlock) <= (uint32_t)ucHeap + configTOTAL_HEAP_SIZE);
	if( ( puc + pxBlockToInsert->xBlockSize ) == ( uint8_t * ) pxIterator->pxNextFreeBlock )
	{
		if( pxIterator->pxNextFreeBlock != pxEnd )
		{
			configASSERT(((uint32_t)pxIterator->pxNextFreeBlock->pxNextFreeBlock > (uint32_t)(pxIterator->pxNextFreeBlock)) && ((uint32_t)pxIterator->pxNextFreeBlock->pxNextFreeBlock <= (uint32_t)ucHeap + configTOTAL_HEAP_SIZE ));
			/* Form one big block from the two blocks. */
			pxBlockToInsert->xBlockSize += pxIterator->pxNextFreeBlock->xBlockSize;
			pxBlockToInsert->pxNextFreeBlock = pxIterator->pxNextFreeBlock->pxNextFreeBlock;
		}
		else
		{
			pxBlockToInsert->pxNextFreeBlock = pxEnd;
		}
	}
	else
	{
		pxBlockToInsert->pxNextFreeBlock = pxIterator->pxNextFreeBlock;
	}

	/* If the block being inserted plugged a gab, so was merged with the block
	before and the block after, then it's pxNextFreeBlock pointer will have
	already been set, and should not be set here as that would make it point
	to itself. */
	if( pxIterator != pxBlockToInsert )
	{
		pxIterator->pxNextFreeBlock = pxBlockToInsert;
	}
	else
	{
		mtCOVERAGE_TEST_MARKER();
	}
}


/*-----------------------------------------------------------*/

void *pvPortCalloc( size_t nmemb, size_t size )
{
    void *pvReturn;

    /* unsigned integer wrap around protection */
    #define __LIM ( 1 << ( sizeof( size_t ) * 8 / 2 ) )
    if( ( nmemb | size ) >= __LIM && size && ( nmemb > SIZE_MAX / size ) )
    {
        size = 0;
    }

#ifdef MTK_SUPPORT_HEAP_DEBUG
    BlockLink_t *pxBlock;
    /* Obtain the return address of caller from link register */
#if defined(__GNUC__)
    uint32_t xLinkRegAddr = (uint32_t)__builtin_return_address(0);
#elif defined(__CC_ARM)
    uint32_t xLinkRegAddr = __return_address();
#elif defined(__ICCARM__)
    uint32_t xLinkRegAddr = __get_LR();
#endif /* __GNUC__ */
#endif /* MTK_SUPPORT_HEAP_DEBUG */

#ifdef MTK_HEAP_SIZE_GUARD_ENABLE
    #if defined(__GNUC__)
    extern void *__wrap_pvPortMalloc(size_t);
    pvReturn = (void *)__wrap_pvPortMalloc(nmemb*size);
    #elif defined(__CC_ARM)
    pvReturn = pvPortMalloc( nmemb*size );
    #endif /* __GNUC__ */
#else
    pvReturn = pvPortMalloc( nmemb*size );
#endif /* MTK_HEAP_SIZE_GUARD_ENABLE */
    if (pvReturn)
    {
        #ifdef MTK_SUPPORT_HEAP_DEBUG
        pxBlock = (BlockLink_t *) ((uint32_t)pvReturn - xHeapStructSize);
        pxBlock->xMallocLinkRegAddr = xLinkRegAddr;
        #endif /* MTK_SUPPORT_HEAP_DEBUG */

        memset(pvReturn, 0, nmemb*size);
    }

    return pvReturn;
}
/*-----------------------------------------------------------*/


void *pvPortRealloc( void *pv, size_t size )
{
    void        *pvReturn   = NULL;
    size_t       xBlockSize = 0;
    uint8_t     *puc        = ( uint8_t * ) pv;
    BlockLink_t *pxLink     = NULL;

#ifdef MTK_SUPPORT_HEAP_DEBUG
    BlockLink_t *pxBlock;
    /* Obtain the return address of caller from link register */
#if defined(__GNUC__)
    uint32_t xLinkRegAddr = (uint32_t)__builtin_return_address(0);
#elif defined(__CC_ARM)
    uint32_t xLinkRegAddr = __return_address();
#elif defined(__ICCARM__)
    uint32_t xLinkRegAddr = __get_LR();
#endif /* __GNUC__ */
#endif /* MTK_SUPPORT_HEAP_DEBUG */

    if((pv != NULL) && (size == 0))
    {
        vPortFree(pv);
        return NULL;
    }

    pvReturn = pvPortCalloc( size, 1 );

#ifdef MTK_SUPPORT_HEAP_DEBUG
    if(pvReturn != NULL)
    {
        pxBlock = (BlockLink_t *) ((uint32_t)pvReturn - xHeapStructSize);
        pxBlock->xMallocLinkRegAddr = xLinkRegAddr;
    }
#endif /* MTK_SUPPORT_HEAP_DEBUG */

    if( (pv != NULL) && (pvReturn != NULL) )
    {
        // The memory being freed will have an BlockLink_t structure immediately before it.
        puc -= xHeapStructSize;

        // This casting is to keep the compiler from issuing warnings.
        pxLink = ( void * ) puc;

        // Check the block is actually allocated
        configASSERT( ( pxLink->xBlockSize & xBlockAllocatedBit ) != 0 );
        configASSERT( pxLink->pxNextFreeBlock == NULL );

        // Get Original Block Size
        xBlockSize = (pxLink->xBlockSize & ~xBlockAllocatedBit);

        // Get Original data length
        xBlockSize = (xBlockSize - xHeapStructSize);

        if(xBlockSize < size)
            memcpy(pvReturn, pv, xBlockSize);
        else
            memcpy(pvReturn, pv, size);

        // Free Original Ptr
        vPortFree(pv);
    }

    return pvReturn;
}

#ifdef HAL_CACHE_WITH_REMAP_FEATURE
#define portCacheline_ALIGNMENT HAL_CACHE_LINE_SIZE
#endif

void *pvPortMallocNC( size_t xWantedSize )
{
#ifdef MTK_SUPPORT_HEAP_DEBUG
    BlockLink_t *pxBlock;
    /* Obtain the return address of caller from link register */
#if defined(__GNUC__)
    uint32_t xLinkRegAddr = (uint32_t)__builtin_return_address(0);
#elif defined(__CC_ARM)
    uint32_t xLinkRegAddr = __return_address();
#elif defined(__ICCARM__)
    uint32_t xLinkRegAddr = __get_LR();
#endif /* __GNUC__ */
#endif /* MTK_SUPPORT_HEAP_DEBUG */

#ifdef HAL_CACHE_WITH_REMAP_FEATURE

/*
      head        res            xBlockAlignWantedSize         res
    |_____|________|______________________|________|
    p1     p2     p3     p4

    res is a const value: portCacheline_ALIGNMENT - portBYTE_ALIGNMENT,
    the first res is to confirm this non-cacheable block is located at the different cache line compared with the front heap block
    the second res is to confirm this non-cacheable block is located at the differet cache line compared with the next heap block

    p1: block begin address
    p2: return address of pvPortMalloc
    p3: cache line align address, which is the begin of the cache line invalidate operation
    p4: user address,which is equal to p2 + res(portCacheline_ALIGNMENT - portBYTE_ALIGNMENT)
*/
    const size_t xResSize =  portCacheline_ALIGNMENT - portBYTE_ALIGNMENT; /* res */
    size_t xBlockAlignWantedSize = 0;
    void *pvReturn = NULL;          /* p2*/
    uint32_t xCacheAlignAddr;       /* p3 */
    uint32_t xUserAddr;             /* p4 */
    uint32_t xInvalidLength;
    if( ( xWantedSize & xBlockAllocatedBit ) == 0 )
    {
        /* The wanted size is increased so it can contain a BlockLink_t
        structure in addition to the requested amount of bytes. */
        if( xWantedSize > 0 )
        {
            xBlockAlignWantedSize = xWantedSize;
            /* Ensure that blocks are always aligned to the required number of bytes. */
            if( ( xBlockAlignWantedSize & portBYTE_ALIGNMENT_MASK ) != 0x00 )
            {
                /* Byte alignment required. */
                xBlockAlignWantedSize += ( portBYTE_ALIGNMENT - ( xBlockAlignWantedSize & portBYTE_ALIGNMENT_MASK ) );
                configASSERT( ( xBlockAlignWantedSize & portBYTE_ALIGNMENT_MASK ) == 0 );
            }
            else
            {
                mtCOVERAGE_TEST_MARKER();
            }
            #ifdef MTK_HEAP_SIZE_GUARD_ENABLE
            #if defined(__GNUC__)
            extern void *__wrap_pvPortMalloc(size_t);
            pvReturn = (void *)__wrap_pvPortMalloc(xBlockAlignWantedSize + xResSize * 2);
            #endif /* __GNUC__ */
            #else
            /* Allocate a block from heap memory */
            pvReturn = pvPortMalloc(xBlockAlignWantedSize + xResSize * 2);
            #endif /* MTK_HEAP_SIZE_GUARD_ENABLE */
        }
        else
        {
            mtCOVERAGE_TEST_MARKER();
        }
    }
    else
    {
        mtCOVERAGE_TEST_MARKER();
    }

    /* directly return if allocate fail */
    if(pvReturn == NULL)
    {
        return pvReturn;
    }
    #ifdef MTK_SUPPORT_HEAP_DEBUG
    pxBlock = (BlockLink_t *) ((uint32_t)pvReturn - xHeapStructSize);
    pxBlock->xMallocLinkRegAddr = xLinkRegAddr;
    #endif /* MTK_SUPPORT_HEAP_DEBUG */

    /* round up to cache line align size for invalidation */
    xCacheAlignAddr = ((uint32_t)pvReturn + portCacheline_ALIGNMENT - 1) & ~(portCacheline_ALIGNMENT - 1); /* p3 */
    xUserAddr = (uint32_t)pvReturn + xResSize;      /* p4 = p2 + res */
    configASSERT(xCacheAlignAddr <= xUserAddr);     /* p3 <= p4 */

    xInvalidLength = (xUserAddr - xCacheAlignAddr + xBlockAlignWantedSize + portCacheline_ALIGNMENT - 1) & ~(portCacheline_ALIGNMENT - 1); /* (p4 - p3 + xBlockAlignWantedSize) round up to cache line aligne size */
    configASSERT((xCacheAlignAddr + xInvalidLength) <= (xUserAddr + xBlockAlignWantedSize + xResSize)); /* (p3 + xInvalidLength) <= (p4 + xBlockAlignWantedSize + res) */

    /* do invalidation*/
    if(HAL_CACHE_STATUS_OK != hal_cache_invalidate_multiple_cache_lines(xCacheAlignAddr, xInvalidLength))
    {
        configASSERT(0);
    }

    /* change to non-cacheable address */
    //xUserAddr = HAL_CACHE_VIRTUAL_TO_PHYSICAL(xUserAddr);
    xUserAddr = hal_cache_cacheable_to_noncacheable(xUserAddr);

    return (void*)xUserAddr;
#else
    void *pvReturn = NULL;
    pvReturn = pvPortMalloc(xWantedSize);

    #ifdef MTK_SUPPORT_HEAP_DEBUG
    if(pvReturn != NULL)
    {
        pxBlock = (BlockLink_t *) ((uint32_t)pvReturn - xHeapStructSize);
        pxBlock->xMallocLinkRegAddr = xLinkRegAddr;
    }
    #endif /* MTK_SUPPORT_HEAP_DEBUG */

	/* TODO: workaround for cache feature not define, but cache code in NSC */
#ifdef HAL_CACHE_MODULE_ENABLED
    pvReturn = hal_cache_cacheable_to_noncacheable(pvReturn);
#endif

    return pvReturn;
#endif /* HAL_CACHE_WITH_REMAP_FEATURE */
}


void vPortFreeNC( void *pv )
{
#ifdef HAL_CACHE_WITH_REMAP_FEATURE
/*
      head        res        xBlockAlignWantedSize         res
    |_____|________|______________________|________|
    p1     p2     p3     p4

    p2 = p4 - res
*/
    const uint32_t xResSize =  portCacheline_ALIGNMENT - portBYTE_ALIGNMENT; /* res */
    uint32_t xAddr;

    if(pv != NULL)
    {
        xAddr = (uint32_t)pv - xResSize; /* p2 */

        /* check address is cacheable or not, if yes, then assert */
        configASSERT(pdFALSE == hal_cache_is_cacheable(xAddr));

        /* change to virtual address */
        // xAddr = HAL_CACHE_PHYSICAL_TO_VIRTUAL(xAddr);
		xAddr = hal_cache_noncacheable_to_cacheable(xAddr);

        /* free */
        vPortFree((void*)xAddr);
    }

#else
    vPortFree(pv);
#endif /* HAL_CACHE_WITH_REMAP_FEATURE*/
}

/* Wrap c stand library malloc family, include malloc/calloc/realloc/free to FreeRTOS heap service */
#if defined(__GNUC__)
#include <reent.h>
void *_malloc_r(struct _reent *rptr,size_t size)
{
    (void)rptr;
    #ifdef MTK_HEAP_SIZE_GUARD_ENABLE
    extern void *__wrap_pvPortMalloc(size_t);
    return __wrap_pvPortMalloc(size);
    #else
    return pvPortMalloc(size);
    #endif /* MTK_HEAP_SIZE_GUARD_ENABLE */
}
void _free_r(struct _reent *rptr,void *pv)
{
    (void)rptr;
    #ifdef MTK_HEAP_SIZE_GUARD_ENABLE
    extern void __wrap_vPortFree(void *pv);
    __wrap_vPortFree(pv);
    #else
     vPortFree(pv);
    #endif /* MTK_HEAP_SIZE_GUARD_ENABLE */
}
void *_calloc_r(struct _reent *rptr,size_t nmemb, size_t size )
{
    (void)rptr;
    return pvPortCalloc(nmemb,size);
}
void *_realloc_r(struct _reent *rptr, void *pv, size_t size )
{
    (void)rptr;
    return pvPortRealloc(pv,size);
}

void *__wrap_malloc(size_t size)
{
    #ifdef MTK_HEAP_SIZE_GUARD_ENABLE
    extern void *__wrap_pvPortMalloc(size_t);
    return __wrap_pvPortMalloc(size);
    #else
    return pvPortMalloc(size);
    #endif /* MTK_HEAP_SIZE_GUARD_ENABLE */
}
void *__wrap_calloc(size_t nmemb, size_t size )
{
    return pvPortCalloc(nmemb,size);
}
void *__wrap_realloc(void *pv, size_t size )
{
    return pvPortRealloc(pv,size);
}
void __wrap_free(void *pv)
{
    #ifdef MTK_HEAP_SIZE_GUARD_ENABLE
    extern void __wrap_vPortFree(void *pv);
    __wrap_vPortFree(pv);
    #else
     vPortFree(pv);
    #endif /* MTK_HEAP_SIZE_GUARD_ENABLE */
}
#elif defined(__CC_ARM)
void *$Sub$$malloc(size_t size)
{
    return pvPortMalloc(size);
}
void *$Sub$$calloc(size_t nmemb, size_t size )
{
    return pvPortCalloc(nmemb,size);
}
void *$Sub$$realloc(void *pv, size_t size )
{
    return pvPortRealloc(pv,size);
}
void $Sub$$free(void *pv)
{
     vPortFree(pv);
}
#endif /* __GNUC__ */

#ifdef MTK_SUPPORT_HEAP_DEBUG
void vPortUpdateBlockHeaderLR(void* addr, uint32_t lr)
{
    BlockLink_t *pxBlock;
    pxBlock = (BlockLink_t *) ((uint32_t)addr - xHeapStructSize);
    pxBlock->xMallocLinkRegAddr = lr;
}
#endif /* MTK_SUPPORT_HEAP_DEBUG */
#if defined(MTK_SUPPORT_HEAP_DEBUG) || defined(MTK_HEAP_SIZE_GUARD_ENABLE) || defined(MTK_SUPPORT_HEAP_DEBUG_ADVANCED)
#include "exception_handler.h"
#if !defined(MTK_HEAP_SIZE_GUARD_ENABLE) &&((PRODUCT_VERSION == 2523)||(PRODUCT_VERSION == 7687)||(PRODUCT_VERSION == 7697)||(PRODUCT_VERSION == 7686)||(PRODUCT_VERSION == 7682)||(PRODUCT_VERSION == 5932)||(PRODUCT_VERSION == 7698))
#define PRINTF printf
#else
#define PRINTF exception_printf
#endif

void vCheckAccessRegion(void* addr, size_t size)
{
	BlockLink_t *blk_iter = pxFirstBlock;
	uint32_t blk_size = 0;
	uint32_t xAddr = (uint32_t)addr;

	taskENTER_CRITICAL();
	while (blk_iter != pxEnd)
	{
		blk_size = (blk_iter->xBlockSize & ~xBlockAllocatedBit);
		if (xAddr >= (uint32_t)blk_iter + sizeof(BlockLink_t)
			  && xAddr < ((uint32_t)blk_iter + blk_size))
		{
			if(xAddr + size > ((uint32_t)blk_iter + blk_size))
			{
				configASSERT(0);
			}
		}
		blk_iter = (BlockLink_t*)((uint32_t)blk_iter + blk_size);
	}
	taskEXIT_CRITICAL();
}

void vDumpHeapStatusInternal(){
    
    BlockLink_t *blk_iter = pxFirstBlock;
	int32_t blk_size = 0;

	PRINTF("cm4 heap block dump begin \r\n");

	while (blk_iter != pxEnd)
	{
		#ifdef MTK_HEAP_SIZE_GUARD_ENABLE
		#define GET_ACTUAL_BLOCK_SIZE(xBlockSize)  ((xBlockSize) & (uint32_t)0x00FFFFFF)    /* mask off top byte*/
		blk_size = GET_ACTUAL_BLOCK_SIZE(blk_iter->xBlockSize);
	    PRINTF("block start = 0x%x,    size = 0x%-8x\n", (unsigned int)blk_iter, (unsigned int)blk_iter->xBlockSize);

		#elif defined(MTK_SUPPORT_HEAP_DEBUG_ADVANCED)
		PRINTF("block start = 0x%x, size = 0x%-8x, tick = 0x%-8x, trace = 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n",
		(unsigned int)blk_iter,
		(unsigned int)blk_iter->xBlockSize,
		(unsigned int)blk_iter->xTick,
		(unsigned int)(blk_iter->xTrace[0]),
		(unsigned int)(blk_iter->xTrace[1]),
		(unsigned int)(blk_iter->xTrace[2]),
		(unsigned int)(blk_iter->xTrace[3]),
		(unsigned int)(blk_iter->xTrace[4]));

		#elif defined(MTK_SUPPORT_HEAP_DEBUG)
		if(HAL_CORE_EXCEPTION == hal_core_status_read(HAL_CORE_MCU)){
            PRINTF("block start = 0x%x,    pxNext=0x%-8x,    size = 0x%-8x,    lr = 0x%x\n",
    			   (unsigned int)blk_iter,
    			   (unsigned int)blk_iter->pxNextFreeBlock,
    			   (unsigned int)blk_iter->xBlockSize,
    			   (blk_iter->xBlockSize & xBlockAllocatedBit)? (unsigned int)(blk_iter->xMallocLinkRegAddr) : (unsigned int)(blk_iter->xFreeLinkRegAddr));
        }else{
            printf("block start = 0x%x,    pxNext=0x%-8x,    size = 0x%-8x,    lr = 0x%x\n",
    			   (unsigned int)blk_iter,
    			   (unsigned int)blk_iter->pxNextFreeBlock,
    			   (unsigned int)blk_iter->xBlockSize,
    			   (blk_iter->xBlockSize & xBlockAllocatedBit)? (unsigned int)(blk_iter->xMallocLinkRegAddr) : (unsigned int)(blk_iter->xFreeLinkRegAddr));
        }

		#endif /* MTK_HEAP_SIZE_GUARD_ENABLE */

		blk_size = blk_iter->xBlockSize & ~xBlockAllocatedBit;
		blk_iter = (BlockLink_t*)((uint32_t)blk_iter + blk_size);

		if((blk_size <= 0)
			|| (blk_size & portBYTE_ALIGNMENT_MASK)
			|| ((uint32_t)blk_iter < (uint32_t)ucHeap)
			|| ((uint32_t)blk_iter >= (uint32_t)ucHeap + configTOTAL_HEAP_SIZE)
			|| ((uint32_t)blk_iter > (uint32_t)pxEnd))
		{
			PRINTF("heap crash!!!!! blk_size=%d, blk_iter=0x%x \n", blk_size, (unsigned int )blk_iter);
			return;
		}
	}

	#ifdef MTK_HEAP_SIZE_GUARD_ENABLE
    PRINTF("block start = 0x%x,    size = 0x%-8x\n", (unsigned int)blk_iter, (unsigned int)blk_iter->xBlockSize);

	#elif defined(MTK_SUPPORT_HEAP_DEBUG_ADVANCED)
	PRINTF("block start = 0x%x, size = 0x%-8x, tick = 0x%-8x, trace = 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n",
	(unsigned int)blk_iter,
	(unsigned int)blk_iter->xBlockSize,
	(unsigned int)blk_iter->xTick,
	(unsigned int)(blk_iter->xTrace[0]),
	(unsigned int)(blk_iter->xTrace[1]),
	(unsigned int)(blk_iter->xTrace[2]),
	(unsigned int)(blk_iter->xTrace[3]),
	(unsigned int)(blk_iter->xTrace[4]));

	#elif defined(MTK_SUPPORT_HEAP_DEBUG)
	if(HAL_CORE_EXCEPTION == hal_core_status_read(HAL_CORE_MCU)){
        PRINTF("block start = 0x%x,    pxNext=0x%-8x,    size = 0x%-8x,    lr = 0x%x \n",
		   (unsigned int)blk_iter,
		   (unsigned int)blk_iter->pxNextFreeBlock,
		   (unsigned int)blk_iter->xBlockSize,
		   (blk_iter->xBlockSize & xBlockAllocatedBit)? (unsigned int)(blk_iter->xMallocLinkRegAddr) : (unsigned int)(blk_iter->xFreeLinkRegAddr));
    }else{
        printf("block start = 0x%x,    pxNext=0x%-8x,    size = 0x%-8x,    lr = 0x%x \n",
		   (unsigned int)blk_iter,
		   (unsigned int)blk_iter->pxNextFreeBlock,
		   (unsigned int)blk_iter->xBlockSize,
		   (blk_iter->xBlockSize & xBlockAllocatedBit)? (unsigned int)(blk_iter->xMallocLinkRegAddr) : (unsigned int)(blk_iter->xFreeLinkRegAddr));
    }
    #endif /* MTK_HEAP_SIZE_GUARD_ENABLE */

	PRINTF("cm4 reach blk_end \n");
}

void vDumpHeapStatus()
{
	if(HAL_CORE_EXCEPTION == hal_core_status_read(HAL_CORE_MCU)){
        vDumpHeapStatusInternal();
    }
}


void prvPortDumpHeapInfo(void)
{
	uint32_t ret;
	exception_config_type callback_config;

	/* register callback to exception flow */
	callback_config.init_cb = vDumpHeapStatus;
	callback_config.dump_cb = NULL;

	ret = exception_register_callbacks(&callback_config);
	if (!ret) {
		configASSERT(0);
	}
}
#endif /* defined(MTK_SUPPORT_HEAP_DEBUG) || defined(MTK_HEAP_SIZE_GUARD_ENABLE) || defined(MTK_SUPPORT_HEAP_DEBUG_ADVANCED)*/

void vPortGetHeapStats( HeapStats_t * pxHeapStats )
{
    BlockLink_t * pxBlock;
    size_t xBlocks = 0, xMaxSize = 0, xMinSize = portMAX_DELAY; /* portMAX_DELAY used as a portable way of getting the maximum value. */

    vTaskSuspendAll();
    {
        pxBlock = xStart.pxNextFreeBlock;

        /* pxBlock will be NULL if the heap has not been initialised.  The heap
         * is initialised automatically when the first allocation is made. */
        if( pxBlock != NULL )
        {
            do
            {
                /* Increment the number of blocks and record the largest block seen
                 * so far. */
                xBlocks++;

                if( pxBlock->xBlockSize > xMaxSize )
                {
                    xMaxSize = pxBlock->xBlockSize;
                }

                if( pxBlock->xBlockSize < xMinSize )
                {
                    xMinSize = pxBlock->xBlockSize;
                }

                /* Move to the next block in the chain until the last block is
                 * reached. */
                pxBlock = pxBlock->pxNextFreeBlock;
            } while( pxBlock != pxEnd );
        }
    }
    ( void ) xTaskResumeAll();

    pxHeapStats->xSizeOfLargestFreeBlockInBytes = xMaxSize;
    pxHeapStats->xSizeOfSmallestFreeBlockInBytes = xMinSize;
    pxHeapStats->xNumberOfFreeBlocks = xBlocks;

    taskENTER_CRITICAL();
    {
        pxHeapStats->xAvailableHeapSpaceInBytes = xFreeBytesRemaining;
        pxHeapStats->xNumberOfSuccessfulAllocations = xNumberOfSuccessfulAllocations;
        pxHeapStats->xNumberOfSuccessfulFrees = xNumberOfSuccessfulFrees;
        pxHeapStats->xMinimumEverFreeBytesRemaining = xMinimumEverFreeBytesRemaining;
    }
    taskEXIT_CRITICAL();
}


#include "FreeRTOS.h"
#include "portmacro.h"
#include "assert.h"


//#define HEAP_LEAK_SELF_CHECK_DEBUG
#ifdef HEAP_LEAK_SELF_CHECK_DEBUG
    #define heap_leak_dbg_printf(format,...)        printf(format, ##__VA_ARGS__)
#else
    #define heap_leak_dbg_printf(format,...)
#endif
//#define HEAP_LEAK_MIPS_MEASURE

#define HEAP_LEAK_RESERVE_SIZE 1024*25
#define ARM_FPU_REG_SIZE 136
#define TASK_IDLE_GAP_VALUE  100

typedef struct{
    uint32_t pxTopOfStack;
    uint32_t exec_return;
}stack_info_t;

static stack_info_t ori_stack_info[32];
static uint32_t oriHeapRemainSize = 0;
static uint32_t oriRecordFlag = TRUE;
extern uint32_t * air_record_task_array[32] ;
extern uint32_t air_record_task_number ;

void vHeapLeakSelfCheck(void){
    uint32_t uxArraySize = 0;
    uint32_t idx = 0;
    uint32_t curPxTopOfStack = 0;
    uint32_t stackDiff = 0;
    uint32_t bitmap = 0xffffffff;
    uint32_t idle_task_number = 0;
#ifdef HEAP_LEAK_MIPS_MEASURE
    uint32_t t0,t1,t2,t3;
#endif

#ifdef HEAP_LEAK_MIPS_MEASURE
    hal_gpt_get_free_run_count(0, &t0);
#endif
    /* Get system task total number */
    uxArraySize = uxTaskGetNumberOfTasks();

    /* total task number should equal, we should not delete task*/
    if (uxArraySize != air_record_task_number) {
        heap_leak_dbg_printf("uxArraySize = %d,air_record_task_number = %d\r\n",
                             (unsigned int)uxArraySize,(unsigned int)air_record_task_number);
    }

    /* just record task information when power on */
    if( oriRecordFlag == TRUE){
        for (idx = 0; idx < air_record_task_number; idx++){
            ori_stack_info[idx].pxTopOfStack= uxTaskGetPxTopOfStack((TaskHandle_t)air_record_task_array[idx]);
            ori_stack_info[idx].exec_return = *((uint32_t *)(ori_stack_info[idx].pxTopOfStack +0x20));
        }
        oriHeapRemainSize =  xPortGetFreeHeapSize();
        vDumpHeapStatusInternal();
        oriRecordFlag = FALSE;
    }

#ifdef HEAP_LEAK_MIPS_MEASURE
    hal_gpt_get_free_run_count(0, &t1);
#endif

    /* check stack diff to judge system idle */
    heap_leak_dbg_printf("[heap_leak]*************************<Task Stack Information>*********************\r\n");
    for (idx = 0; idx < air_record_task_number; idx++){
        if(0 == strncmp("IDLE",pcTaskGetName((TaskHandle_t)air_record_task_array[idx]),4)){
            bitmap &= ~(1 << (31 - idx) );
            continue;
        }
        curPxTopOfStack = uxTaskGetPxTopOfStack((TaskHandle_t)air_record_task_array[idx]);

        /* power on stack <= current stack */
        if (curPxTopOfStack <= ori_stack_info[idx].pxTopOfStack){
            stackDiff = ori_stack_info[idx].pxTopOfStack - curPxTopOfStack;

            /* current context fpu active */
            if (((ori_stack_info[idx].exec_return & 0x10) != 0)
                && ((*((uint32_t *)(curPxTopOfStack+0x20)) & 0x10) == 0) ){
                /* Fpu reg number is 136 byte,30-s31 + fpscr+reserve 4bytes */
                stackDiff -= ARM_FPU_REG_SIZE;
                heap_leak_dbg_printf("[heap_leak]fpu active,task name:%s,stack diff:0x%x\r\n",pcTaskGetName((TaskHandle_t)air_record_task_array[idx]),(unsigned int)stackDiff);
            }
            heap_leak_dbg_printf("[heap_leak]task name:%s,current pxTopOfStack = 0x%x,ori pxTopOfStack = 0x%x,stack diff:0x%x\r\n"
                                , pcTaskGetName((TaskHandle_t)air_record_task_array[idx])
                                ,(unsigned int)curPxTopOfStack
                                ,(unsigned int)ori_stack_info[idx].pxTopOfStack
                                ,(unsigned int)stackDiff);
        }else{
            /* it means cur stack used less space,stack diff is 0 */
            stackDiff = 0;
            heap_leak_dbg_printf("[heap_leak]less stack,task name:%s,current pxTopOfStack = 0x%x,ori pxTopOfStack = 0x,%xstack diff:0x%x\r\n"
                                , pcTaskGetName((TaskHandle_t)air_record_task_array[idx])
                                ,(unsigned int)curPxTopOfStack
                                ,(unsigned int)ori_stack_info[idx].pxTopOfStack
                                ,(unsigned int)stackDiff);
        }

        /* this task idle,if stack diff <= TASK_IDLE_GAP_VALUE */
        if (stackDiff <= TASK_IDLE_GAP_VALUE){
            bitmap &= ~(1 << (31 - idx) );
        }
    }

#ifdef HEAP_LEAK_MIPS_MEASURE
    hal_gpt_get_free_run_count(0, &t2);
#endif
	__asm volatile ( "clz %0, %1" : "=r" ( idle_task_number ) : "r" ( bitmap) : "memory" );

    heap_leak_dbg_printf("[heap_leak]air_record_task_number = %d,idle_task_number = %d\r\n",(unsigned int)air_record_task_number,(unsigned int)idle_task_number);

    heap_leak_dbg_printf("[heap_leak]*************************<Heap Information>*********************");

    heap_leak_dbg_printf("[heap_leak]oriHeapRemainSize = %d,curHeapRemainSize = %d",(unsigned int)oriHeapRemainSize,(unsigned int)xPortGetFreeHeapSize());

    /* we consider it as system idle mode */
    if (air_record_task_number == idle_task_number){
        heap_leak_dbg_printf("[heap_leak]*************************<system enter in idle mode>*********************\r\n");
        uint32_t curHeapRemainSize =xPortGetFreeHeapSize();
        /* we need check heap status */
        if(oriHeapRemainSize > curHeapRemainSize ){
            /* Heap diff >= 1k */
            if ((oriHeapRemainSize - curHeapRemainSize) >= HEAP_LEAK_RESERVE_SIZE){
                assert("<Heap Leak occurs>" && 0);
            }
        }
    }else{
        heap_leak_dbg_printf("[heap_leak]*************************<system is busy>*********************\r\n");
    }
        //hal_gpt_delay_ms(10);
#ifdef HEAP_LEAK_MIPS_MEASURE
    hal_gpt_get_free_run_count(0, &t3);
    printf("[heap_leak]first time:%dus,calc time:%dus,judge time:%dus",(unsigned int)(t1-t0),(unsigned int)(t2-t1),(unsigned int)(t3-t2));
#endif


}

uint32_t g_tmp_enable_heak_leak = 0;

void vHeapLeakSelfCheckHook(void){
    extern exception_config_mode_t exception_config_mode ;
    //uint32_t irq_mask = 0;

    /*at cmd set this value as 1 */
    if (1 == g_tmp_enable_heak_leak){
        /* we need get task stack information,so we need disable irq */
        //hal_nvic_save_and_set_interrupt_mask(&irq_mask);
        vTaskSuspendAll();

#ifdef HEAP_LEAK_SELF_CHECK_DEBUG
        /* disable mask irq check assert to show log */
        //exception_config_mode.exception_mode_t.mask_irq_check_assert = 0;
#endif

        if (!exception_config_mode.exception_mode_t.wdt_reset_mode) {
            vHeapLeakSelfCheck();
        }

        /* enable irq */
        //hal_nvic_restore_interrupt_mask(irq_mask);
        xTaskResumeAll();
    }
}
/*-----------------------------------------------------------*/

