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

/*

example:
    ring bufer size is 8

    1. fifo is empty
    |<--------------size----------->|
  start                            end
    |                               |
    v                               v
    *---*---*---*---*---*---*---*---*
    |   |   |   |   |   |   |   |   |
    *---*---*---*---*---*---*---*---*
    |
    v
rptr' = wptr'

    logic position
    *---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*
    |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   | ...
    *---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*
    |
    v
rptr = wptr

    2. write 6 data to buffer, data length == wptr - rptr == 6
    *---*---*---*---*---*---*---*---*
    | + | + | + | + | + | + |   |   |
    *---*---*---*---*---*---*---*---*
    |                       |
    v                       v
   rptr'                   wptr'
    logic position
    *---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*
    | + | + | + | + | + | + |   |   |   |   |   |   |   |   |   |   | ...
    *---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*
    |                       |
    v                       v
   rptr                    wptr


    3. then, write 2 data to buffer, buffer is full, data length == wptr - rptr
== 8
    *---*---*---*---*---*---*---*---*
    | + | + | + | + | + | + | + | + |
    *---*---*---*---*---*---*---*---*
    |
    v
wptr' = rptr'

    logic position
    *---*---*---*---*---*---*---*---*---*---*---*---*---*---*
    | + | + | + | + | + | + | + | + |   |   |   |   |   |   | ...
    *---*---*---*---*---*---*---*---*---*---*---*---*---*---*
    |                               |
    v                               v
   rptr                            wptr

    4. read 5 data, data length == wptr - rptr == 3
    *---*---*---*---*---*---*---*---*
    |   |   |   |   |   | + | + | + |
    *---*---*---*---*---*---*---*---*
    |                   |
    v                   v
   wptr'                rptr'
    logic position
    *---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*
    |   |   |   |   |   | + | + | + |   |   |   |   |   |   |   |   | ...
    *---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*
                        |           |
                        v           v
                       rptr        wptr


    5. write 3 data, data length == wptr - rptr == 6
    *---*---*---*---*---*---*---*---*
    | + | + | + |   |   | + | + | + |
    *---*---*---*---*---*---*---*---*
                |       |
                v       v
               wptr'   rptr'
    logic position
    *---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*
    |   |   |   |   |   | + | + | + | + | + | + |   |   |   |   |   | ...
    *---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*
                        |                       |
                        v                       v
                       rptr                    wptr

    6. read 4 data
    *---*---*---*---*---*---*---*---*
    |   | + | + |   |   |   |   |   |
    *---*---*---*---*---*---*---*---*
        |       |
        v       v
       rptr'   wptr'
    logic position
    *---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*
    |   |   |   |   |   |   |   |   |   | + | + |   |   |   |   |   | ...
    *---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*
                                        |       |
                                        v       v
                                       rptr    wptr

The value of size must round up to power of two.
rptr' = rptr & (size - 1)
wptr' = wptr & (size - 1)

1. when ring buffer is empty ?
    rptr == wptr
2. when ring buffer is full?
    size == wptr - rptr
3. how many data in ring buffer?
    wptr - rptr
3. how many free space in ring buffer?
    size - (wptr - rptr)
4. what the behavior when wptr and rptr almost reach the maximum?
    rptr = 0xFFFFFFF - 4 = 0xFFFFFFFB = 4294967291
    wptr = 0xFFFFFFF - 2 = 0xFFFFFFFD = 4294967293
    rptr' = rptr & (size - 1) = 0xFFFFFFFB & 7 = 3
    wptr' = wptr & (size - 1) = 0xFFFFFFFD & 7 = 5
    *---*---*---*---*---*---*---*---*
    |   |   |   | + | + |   |   |   |
    *---*---*---*---*---*---*---*---*
                |       |
                v       v
               rptr'   wptr'
    logic position
    *---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*
    |   |   |   |   |   |   |   |   |   |   |   | + | + |   |   |   | ...
    *---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*
                                                |       |
                                                v       v
                                               rptr    wptr
    free space == size - (wptr - rptr) == 8 - (0xFFFFFFFD - 0xFFFFFFFB) == 8 - 2
== 6

    write 2 data to buffer
    *---*---*---*---*---*---*---*---*
    |   |   |   | + | + | + | + |   |
    *---*---*---*---*---*---*---*---*
                |               |
                v               v
               rptr'           wptr'
    logic position
    *---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*
    |   |   |   |   |   |   |   |   |   |   |   | + | + | + | + |   | ...
    *---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*
                                                |               |
                                                v               v
                                               rptr            wptr
    wptr + 2 = 0xFFFFFFFD + 2 = 0xFFFFFFFF

    write 1 data to buffer
    *---*---*---*---*---*---*---*---*
    |   |   |   | + | + | + | + | + |
    *---*---*---*---*---*---*---*---*
    |           |
    v           v
   wptr'       rptr'

    logic position
    *---*---*---*---*---*---*-...--*---*---*---*---*---*---*---*---*---*
    |   |   |   |   |   |   | ...  |   |   |   |   | + | + | + | + | + |
    *---*---*---*---*---*---*-...--*---*---*---*---*---*---*---*---*---*
    |                                           |
    v                                           v
   wptr                                        rptr

    32 bit uint32_terger overflow
        wptr + 1 = 0xFFFFFFFF + 1 = 0
    wptr and rptr are uint32_teger
        data size = wptr - rptr = 0 - 0xFFFFFFFB = 0 - (-5) = 5

5. How to calculate the data size and the free size at a contingous space of the
ring buffer? case 1: data size = wptr - rptr tail free size = size - wptr' =
size - (wptr & (size - 1))

    1. get minimums between all free size and write_size, min_len_1
    all free size = size - (wptr - rptr)
    2. get minimums between min_len_1 and tail space(The tail space could be in case2)
    0   1   2   3   4   5   6   7
    *---*---*---*---*---*---*---*---*
    |   |   |   | + | + |   |   |   |
    *---*---*---*---*---*---*---*---*
                |       |
                v       v
               rptr'   wptr'
    logic position
    0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15
    *---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*
    |   |   |   |   |   |   |   |   |   |   |   | + | + |   |   |   | ...
    *---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*
                                                |       |
                                                v       v
                                               rptr    wptr

    write 2 bytes:
    --get the minimal value between buffer free size and data length, in case of memory curroption
    len = min(len, rb->size - rb->wptr + rb->rptr); --> len = min(2, 8 - 13 + 11) = 2
    --get the minimal value between the length from wptr to buffer end and data length
    l = min(len, rb->size - (rb->wptr & (rb->size - 1))); -->l = min(2, 8 - (13&7)) = 2
    memcpy(rb->buffer + (rb->wptr & (rb->size - 1)), buffer, l); --> copy 2 ,
    memcpy(rb->buffer, buffer + l, len - l); --> copy 0 ,  rb->wptr += len;

    write 4 bytes:
    --get the minimal value of buffer free size and data length, in case of memory curroption
    len = min(len, rb->size - rb->wptr + rb->rptr); --> len = 4
    --get the minimal value between the length from wptr to buffer end and data length
    l = min(len, rb->size - (rb->wptr & (rb->size - 1))); -->l = 3
    memcpy(rb->buffer + (rb->wptr & (rb->size - 1)), buffer, l); --> copy 3 ,
    memcpy(rb->buffer, buffer + l, len - l); --> copy 1 ,
    rb->wptr += len;


case 2:
    data size = size - rptr' = size - (rptr & (size - 1))
    free size = rptr' - wptr' = (rptr & (size - 1) - (wptr & (size - 1)
    0   1   2   3   4   5   6   7
    *---*---*---*---*---*---*---*---*
    | + |   |   |   |   |   | + | + |
    *---*---*---*---*---*---*---*---*
        |                   |
        v                   v
       wptr'               rptr'
    logic position
    0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15
    *---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*
    |   |   |   |   |   |   | + | + | + |   |   |   |   |   |   |   | ...
    *---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*
                            |           |
                            v           v
                           rptr        wptr

    write 4 bytes:
    --get the minimal value of buffer free size and data length, in case of memory curroption
    len = min(len, rb->size - rb->wptr + rb->rptr); --> len = min(4, 8 - 9 + 6) = 4
    --get the minimal value between the length from wptr to buffer end and data length(tail_length),
   when case2, this must be the value of len, len must be less than tail_length
   l = min(len, rb->size - (rb->wptr & (rb->size - 1))); --> l = min(4, 8 - (9 & 7)) = 4
   memcpy(rb->buffer + (rb->wptr & (rb->size - 1)), buffer, l); --> copy 4 bytes
   memcpy(rb->buffer, buffer + l, len - l); --> copy 0 ,  rb->wptr += len;


## support multi-user write and read simutaneously

    replace rptr with r_head and r_tail, when r_head == r_tail, read done
    replace wptr with w_head and w_tail, when w_head == w_tail, write done
    r_head and r_tail , w_head and w_tail are global value, and can be accessed by all users
    so:
        The data length in buffer = w_head - r_tail
        The total amount of free space in the buffer = size - (w_head - r_tail)

    0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15
    *---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*
    |   |   |   |   |   |   | + | + | + |   |   |   |   |   |   |   | ...
    *---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*
                            |           |
                            v           v
                         r_head        w_head
                         r_tail        w_tail


Write n bytes data:
    1. Pre-allocate the space you need
        First update the position of n bytesdata to w_head, occupy this data first, other users to write data, will start from w_head

        old_w_head = w_head;//The start location of the write is stored in a local variable.TODO:Is there a problem with multiple user reads here?
        rmb()
        //The read memory barrier ensures that w_head is read to old_w_head before the following code is executed

        free_size = size - (w_head - r_tail);//Gets the remaining space size

        if(n > free_size) {//The length of written data cannot exceed the free buffer space
            n = free_size;
        }

        new_w_head = old_w_head + n;

        w_head = new_w_head;//How can I guarantee that multiple user writes will not be a problem? CAS, disable interrupt?

        After this command is executed, the space from old_w_head to new_w_head has been occupied, and data needs to be written into the space in the next step
    2. Write data to space from old_w_head to new_w_head
    3. After writing, update new_w_head to w_tail
        wmb()//Write memory barrier to ensure that data has been written to buffer
        We also need to check whether the current w_tail is equal to old_w_head. If they are equal, it means the previous user has written data.
        At this point, the last user may not have finished writing, that is, w_tail does not equal old_w_head，If new_w_head is directly updated to w_tail,
        w_tail will be overwritten after the last user is written

### For example：
    write 3 bytes data

    #### single user case

    1. The user is not in the write or read state: r_head==r_tail, w_head==w_tail, Update the value of w_head to old_w_head
    0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15
    *---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*
    |   |   |   |   |   |   | + | + | + |   |   |   |   |   |   |   | ...
    *---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*
                            |           |
                            v           v
                         r_head        w_head
                         r_tail        w_tail
                                     old_w_head
    2. Let's see if w_head is equal to old_w_head, If equal, update w_head to the new new_w_head location to take up space
    0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15
    *---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*
    |   |   |   |   |   |   | + | + | + |   |   |   |   |   |   |   | ...
    *---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*
                            |           |           |
                            v           v           v
                         r_head       w_tail      w_head
                         r_tail     old_w_head    new_w_head
    3. write data
    0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15
    *---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*
    |   |   |   |   |   |   | + | + | + | + | + | + |   |   |   |   | ...
    *---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*
                            |           |           |
                            v           v           v
                         r_head       w_tail      w_head
                         r_tail     old_w_head    new_w_head
    4. update w_tail
    0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15
    *---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*
    |   |   |   |   |   |   | + | + | + | + | + | + |   |   |   |   | ...
    *---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*
                            |                       |
                            v                       v
                         r_head                  w_head
                         r_tail                  w_tail

    #### 2 user case
    1. The user is not in the write or read state: r_head==r_tail, w_head==w_tail, Update the value of w_head to old_w_head
    0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15
    *---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*
    |   |   |   |   |   |   | + | + | + |   |   |   |   |   |   |   | ...
    *---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*
                            |           |
                            v           v
                         r_head        w_head
                         r_tail        w_tail
                                     old_w_head
    2. user 1
    Let's see if w_head is equal to old_w_head. If so, update w_head to new_w_head to take up space
    0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15
    *---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*
    |   |   |   |   |   |   | + | + | + |   |   |   |   |   |   |   | ...
    *---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*
                            |           |           |
                            v           v           v
                         r_head       w_tail     w_head
                         r_tail     old_w_head new_w_head
    3. So user 2 comes in, wants to write two pieces of data, sees w_head at position 12, wants to update to position 14
    User 2 checks whether w_head is equal to old_w_head'. Old_w_head'is the starting write position of user2, and if equal, w_head will be updated
    0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15
    *---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*
    |   |   |   |   |   |   | + | + | + |   |   |   |   |   |   |   | ...
    *---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*
                            |           |           |       |
                            v           v           v       v
                         r_head       w_tail   new_w_head  w_head
                         r_tail     old_w_head old_w_head'
    2. user 2 write data
    0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15
    *---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*
    |   |   |   |   |   |   | + | + | + |   |   |   | + | + |   |   | ...
    *---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*
                            |           |           |       |
                            v           v           v       v
                         r_head       w_tail   new_w_head  w_head
                         r_tail     old_w_head old_w_head'

    3. User 2 wants to update the w_tail value, but sees that the w_tail value is not equal to old_w_head'.
    User2 has to wait until they are equal before updating w_tail
    0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15
    *---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*
    |   |   |   |   |   |   | + | + | + |   |   |   | + | + |   |   | ...
    *---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*
                            |           |           |       |
                            v           v           v       v
                         r_head       w_tail   new_w_head  w_head
                         r_tail     old_w_head old_w_head'

    4. Go back to User 1 and write the data
    0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15
    *---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*
    |   |   |   |   |   |   | + | + | + | + | + | + | + | + |   |   | ...
    *---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*
                            |           |           |       |
                            v           v           v       v
                         r_head       w_tail   new_w_head  w_head
                         r_tail     old_w_head old_w_head'
    5. User 1 finds that w_tail == old_w_head and updates the value of w_tail to new_w_head
    0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15
    *---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*
    |   |   |   |   |   |   | + | + | + | + | + | + | + | + |   |   | ...
    *---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*
                            |           |           |       |
                            v           v           v       v
                         r_head                new_w_head  w_head
                         r_tail     old_w_head old_w_head'
                                                  w_tail
    6. you go back to user2 and find that w_tail is equal to old_w_head', update w_tail to w_head
    0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15
    *---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*
    |   |   |   |   |   |   | + | + | + | + | + | + | + | + |   |   | ...
    *---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*
                            |           |           |       |
                            v           v           v       v
                         r_head                new_w_head  w_head
                         r_tail     old_w_head old_w_head' w_tail

Notes:
    The second user waits until the first user writes done

*/
#include <string.h>
#include "mux_ringbuffer.h"
#ifndef PC_SIMU
#include "mux_ll_uart_config.h"
#include "hal_resource_assignment.h"
#endif
const uint16_t crc16_table[256] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
    0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
    0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
    0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
    0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
    0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
    0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
    0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
    0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
    0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
    0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
    0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
    0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
    0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
    0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
    0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
    0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
    0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
    0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
    0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
    0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
    0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
    0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0};
#ifdef MULTI_CORE_ENABLE
#include "hal_hw_semaphore.h"
#endif

#ifdef MUX_RB_ADDRESS_POWER_OF_2_ENABLE
static bool is_power_of_2(uint32_t x)
{
  return (x != 0) && ((x & (x - 1)) == 0);
}

static uint32_t roundup_pow_of_2(uint32_t x)
{
  uint32_t a = x;
  uint32_t i;

  for (i = 0; a; i++) {
    a >>= 1;
  }

  return 1 << i;
}
#endif
bool mux_ringbuffer_init(mux_ringbuffer_t *rb, uint8_t *buffer, uint32_t len, uint32_t flags)
{
#ifdef MUX_RB_ADDRESS_POWER_OF_2_ENABLE
  MUX_RB_ASSERT(is_power_of_2(len));
#endif

  // RB_LOG_I("mux_ringbuffer_init start rb=0x%x buffer=0x%x", 2, (uint32_t)rb, (uint32_t)buffer);
  if (!rb || !buffer)
    return false;

  rb->size = 0;
#ifndef MUX_RB_ADDRESS_POWER_OF_2_ENABLE
  rb->capacity = len + 1;
#else
  rb->capacity = len;
#endif

#ifdef MUX_RB_ADDRESS_POWER_OF_2_ENABLE
  rb->mask = len - 1;
#else
  rb->mask = 0xFFFFFFFF;
#endif
  rb->flags = flags;
  rb->wptr.head = 0;
  rb->wptr.tail = 0;
  rb->rptr.head = 0;
  rb->rptr.tail = 0;
#if defined(MTK_CPU_NUMBER_0)
  if (flags & MUX_RB_FLAG_SHARED_BUFFER) {
    rb->dsp_view_buffer = (uint8_t*)hal_memview_mcu_to_dsp0((uint32_t)buffer);
    rb->dsp_view_buffer_end = rb->dsp_view_buffer + len;
  } else {
    rb->dsp_view_buffer = 0;
    rb->dsp_view_buffer_end = 0;
  }

  rb->mcu_view_buffer = buffer;
  rb->mcu_view_buffer_end = rb->mcu_view_buffer + len;
#elif defined(MTK_CPU_NUMBER_1)
  if (flags & MUX_RB_FLAG_SHARED_BUFFER) {
    rb->mcu_view_buffer = (uint8_t*)hal_memview_dsp0_to_mcu((uint32_t)buffer);
    rb->mcu_view_buffer_end = rb->mcu_view_buffer + len;
  } else {
    rb->mcu_view_buffer = 0;
    rb->mcu_view_buffer_end = 0;
  }

  rb->dsp_view_buffer = buffer;
  rb->dsp_view_buffer_end = rb->dsp_view_buffer + len;
#else
#error "CORE ERROR!!"
#endif

#ifdef MUX_RB_WATER_MARK_ENABLE
  rb->water_mark = 0;
#endif
  rb->ops = NULL;
  rb->nest_count = 0;
  // RB_LOG_I("uid=%d rb->capacity=%d buffer_addr=0x%x len=%d", 4, rb->uid, rb->capacity, (uint32_t)buffer, len);
  /* RB_LOG_I("dsp_view_buffer=0x%x dsp_view_buffer_end=0x%x mcu_view_buffer=0x%x mcu_view_buffer_end=0x%x", 4, \
    (uint32_t)rb->dsp_view_buffer, (uint32_t)rb->dsp_view_buffer_end, (uint32_t)rb->mcu_view_buffer, (uint32_t)rb->mcu_view_buffer_end); */
  return true;
}

bool mux_ringbuffer_alloc(mux_ringbuffer_t *rb, uint32_t len, uint32_t flags)
{
#ifdef MUX_RB_ADDRESS_POWER_OF_2_ENABLE
  if (!is_power_of_2(len)) {
    RB_ASSERT(len > 0x80000000);
    len = roundup_pow_of_2(len);
  }
#endif

  return mux_ringbuffer_init(rb, MUX_RB_PORTABLE_MALLOC(len), len, flags);
}

void mux_ringbuffer_free(mux_ringbuffer_t *rb)
{
  MUX_RB_PORTABLE_FREE(MUX_RB_START_ADDR(rb));
  MUX_RB_START_ADDR(rb) = NULL;
  rb->size = 0;
  mux_ringbuffer_reset(rb);
}

#if 0
//get data from ringbuffer, buffer must be mux_buffer_t buffer[2]; data may be two part in fifo
ATTR_TEXT_IN_FAST_MEM bool mux_ringbuffer_get_data_buffer(mux_ringbuffer_t *rb, mux_buffer_t *buffer, uint32_t *buffer_count)
{
    mux_buffer_t *pb = buffer;
    if (!pb || !buffer_count || *buffer_count < 2) {
        return false;
    }
    uint32_t len;
    uint32_t total_data;

    pb->p_buf = RB_CONTIGUOUS_DATA_START_ADDR(rb);
    len = RB_CONTIGUOUS_DATA_SIZE(rb);
    pb->buf_size = len;
    pb++;
    total_data = mux_ringbuffer_data_length(rb);

    if (total_data > len) {
        pb->p_buf = MUX_RB_START_ADDR(rb);
        pb->buf_size = total_data - len;
        *buffer_count = 2;
    } else {
        pb->p_buf = NULL;
        pb->buf_size = 0;
        *buffer_count = 1;
    }
    RB_LOG_D("[mux_ringbuffer_get_data_buffer] 1 buf_size=%d total_data=%d buffer_count=%d", 3, len, total_data, *buffer_count);

    return true;
}
#endif
ATTR_TEXT_IN_FAST_MEM void mux_ringbuffer_dump(mux_ringbuffer_t *rb, const char *file_name, bool is_dump_data)
{

  uint8_t *start_addr = MUX_RB_START_ADDR(rb) + (rb->rptr.head & rb->mask);
  uint32_t readable_len = RB_TOTAL_DATA_SIZE(rb);//mux_ringbuffer_data_length(rb);

  uint32_t readable_len_head;
  uint32_t readable_len_tail;

  rb->size = readable_len;
  LOG_MSGID_I(common, "uid[%02X] rb         = 0x%x", 2, (uint32_t)rb->uid, (uint32_t)rb);
  LOG_MSGID_I(common, "uid[%02X] wptr.head  = %d(%d):0x%x", 4, (uint32_t)rb->uid, rb->wptr.head, rb->wptr.head & rb->mask, MUX_RB_START_ADDR(rb) + (rb->wptr.head & rb->mask));
  LOG_MSGID_I(common, "uid[%02X] wptr.tail  = %d(%d):0x%x", 4, (uint32_t)rb->uid, rb->wptr.tail, rb->wptr.tail & rb->mask, MUX_RB_START_ADDR(rb) + (rb->wptr.tail & rb->mask));
  LOG_MSGID_I(common, "uid[%02X] rptr.head  = %d(%d):0x%x", 4, (uint32_t)rb->uid, rb->rptr.head, rb->rptr.head & rb->mask, MUX_RB_START_ADDR(rb) + (rb->rptr.head & rb->mask));
  LOG_MSGID_I(common, "uid[%02X] rptr.tail  = %d(%d):0x%x", 4, (uint32_t)rb->uid, rb->rptr.tail, rb->rptr.tail & rb->mask, MUX_RB_START_ADDR(rb) + (rb->rptr.tail & rb->mask));
  LOG_MSGID_I(common, "uid[%02X] size       = %d", 2, (uint32_t)rb->uid, rb->size);
#ifdef MUX_RB_WATER_MARK_ENABLE
  LOG_MSGID_I(common, "uid[%02X] water_mark = %d", 2, (uint32_t)rb->uid, rb->water_mark);
#endif
#if 0
LOG_MSGID_I(common, "uid[%02X] flags      = 0x%x", 2, (uint32_t)rb->uid, rb->flags);
LOG_MSGID_I(common, "uid[%02X] mask       = 0x%x", 2, (uint32_t)rb->uid, rb->mask);
LOG_MSGID_I(common, "uid[%02X] capacity   = %d"  , 2, (uint32_t)rb->uid, rb->capacity);
#endif
#if 0
 LOG_MSGID_I(common, "uid[%02X] mcu_buffer     = 0x%x", 2, (uint32_t)rb->uid, (uint32_t)(rb->mcu_view_buffer));
 LOG_MSGID_I(common, "uid[%02X] mcu_buffer_end = 0x%x", 2, (uint32_t)rb->uid, (uint32_t)(rb->mcu_view_buffer_end));
 LOG_MSGID_I(common, "uid[%02X] dsp_buffer     = 0x%x", 2, (uint32_t)rb->uid, (uint32_t)(rb->dsp_view_buffer));
 LOG_MSGID_I(common, "uid[%02X] dsp_buffer_end = 0x%x", 2, (uint32_t)rb->uid, (uint32_t)(rb->dsp_view_buffer_end));
#endif

  if (is_dump_data) {
    if ((uint32_t)(start_addr + readable_len) > (uint32_t)MUX_RB_END_ADDR(rb)) {
      readable_len_head = (uint32_t)rb->capacity - (rb->rptr.tail & rb->mask);
      readable_len_tail = readable_len - readable_len_head;
    } else {
      readable_len_head = readable_len;
      readable_len_tail = 0;
    }
    // LOG_MSGID_I(common, "readable_len_head=%d readable_len_tail=%d", 2, readable_len_head, readable_len_tail);

    _hexdump(file_name, start_addr, readable_len_head, MUX_RB_START_ADDR(rb), readable_len_tail);
  }
}

ATTR_TEXT_IN_FAST_MEM uint32_t mux_ringbuffer_peek(mux_ringbuffer_t *rb, uint8_t *buffer, uint32_t len)
{
  uint32_t read_len;
  uint32_t size = len;

  read_len = RB_MIN(size, RB_TOTAL_DATA_SIZE(rb));
  mux_ringbuffer_read_move_data(rb, buffer, read_len, rb->rptr.head);

  return read_len;
}

ATTR_TEXT_IN_FAST_MEM uint32_t mux_ringbuffer_read(mux_ringbuffer_t *rb, uint8_t *buffer, uint32_t len) {
  uint32_t old_head;
  uint32_t new_head;
  uint32_t read_len;
  uint32_t size = len;

  if (mux_ringbuffer_empty(rb)) {
    return 0;
  }

  read_len = mux_ringbuffer_read_move_head(rb, &old_head, &new_head, size);
  mux_ringbuffer_read_move_data(rb, buffer, read_len, old_head);
  mux_ringbuffer_read_move_tail(rb, old_head, new_head);

  return read_len;
}

ATTR_TEXT_IN_FAST_MEM static __FORCE_INLINE__ void mux_ringbuffer_move_core(mux_ringbuffer_t * src_rb, uint32_t src_r_head, mux_ringbuffer_t * dst_rb, uint32_t dst_w_head, uint32_t length,  uint16_t *crc)
{
    uint32_t src_read_addr;
    uint32_t dst_write_addr;
    uint32_t remain_data_length = length;
    uint32_t move_flag = (src_rb->flags & MUX_RB_FLAG_DATA_4TO3) | (dst_rb->flags & MUX_RB_FLAG_DATA_3TO4);
    uint32_t src_len_2_boundary;
    uint32_t dst_len_2_boundary;
    uint32_t min_len_2_boundary;
    uint32_t min_block_len;
    uint32_t block_n;
    uint32_t remain_n;
    uint32_t i;
    uint32_t j;
    uint32_t src_step = src_rb->step;
    uint32_t dst_step = dst_rb->step;
    uint32_t src_offset = src_rb->start_offset;
    uint32_t dst_offset = dst_rb->start_offset;
#ifdef MUX_LL_UART_CRC_CHECK_ENABLE
    uint16_t crc_init = 0;
#endif

    #define B2B_NO_WRAP_ADDR(rb, start_addr, idx) ((start_addr) + (idx))
    #define B2B_WRAP_ADDR(rb, start_addr, idx)     \
    ({\
        uint32_t addr = (uint32_t)B2B_NO_WRAP_ADDR(rb, start_addr, idx);\
        if (addr >= (uint32_t)MUX_RB_END_ADDR(rb)) {\
            addr -= rb->capacity;\
        }\
        addr;\
    })

    #define B2B_SRC_ADDR(wrap, s_idx)    B2B_##wrap##_ADDR(src_rb, src_read_addr, s_idx)
    #define B2B_DST_ADDR(wrap, d_idx)    B2B_##wrap##_ADDR(dst_rb, dst_write_addr, d_idx)

    #define B2B_SRC_ADDR_MOVE_WRAP(s_idx)    (src_read_addr = (uint32_t)B2B_WRAP_ADDR(src_rb, src_read_addr, s_idx))
    #define B2B_DST_ADDR_MOVE_WRAP(d_idx)    (dst_write_addr = (uint32_t)B2B_WRAP_ADDR(dst_rb, dst_write_addr, d_idx))

    #define B2B_SRC_TYPE_VAL(wrap, type, s_idx)   *(volatile type *)(B2B_SRC_ADDR(wrap, s_idx))
    #define B2B_DST_TYPE_VAL(wrap, type, d_idx)   *(volatile type *)(B2B_DST_ADDR(wrap, d_idx))

    #ifdef MUX_LL_UART_CRC_CHECK_ENABLE
    #define CRC_INIT_VAR  crc_init
    #define B2B_BYTE(idx)       BYTE_CRC16(CRC_INIT_VAR, B2B_SRC_TYPE_VAL(WRAP, uint8_t, idx))
    #define B2B_HALF_WORD(idx)  HALF_WORD_CRC16(CRC_INIT_VAR, B2B_SRC_TYPE_VAL(WRAP, uint16_t, idx))
    #define B2B_WORD(idx)       WORD_CRC16(CRC_INIT_VAR, B2B_SRC_TYPE_VAL(NO_WRAP, uint32_t, idx))
    #else
    #define CRC_INIT_VAR  0
    #define B2B_BYTE(idx) B2B_SRC_TYPE_VAL(WRAP, uint8_t, idx)
    #define B2B_HALF_WORD(idx) B2B_SRC_TYPE_VAL(WRAP, uint16_t, idx)
    #define B2B_WORD(idx) B2B_SRC_TYPE_VAL(NO_WRAP, uint32_t, idx)
    #endif

    #define B2B_COPY_BYTE(d_idx, s_idx)          \
    do { \
        B2B_DST_TYPE_VAL(WRAP, uint8_t, d_idx) = B2B_BYTE(s_idx);\
    } while (0)

    #define B2B_COPY_HALF_WORD(d_idx, s_idx)          \
    do { \
        B2B_DST_TYPE_VAL(WRAP, uint16_t, d_idx) = B2B_HALF_WORD(s_idx);\
    } while (0)

    #define B2B_COPY_WORD(d_idx, s_idx)          \
    do { \
        B2B_DST_TYPE_VAL(NO_WRAP, uint32_t,d_idx) = B2B_WORD(s_idx);\
    } while (0)

    src_read_addr = (uint32_t)(MUX_RB_START_ADDR(src_rb)) + src_r_head;
    dst_write_addr = (uint32_t)(MUX_RB_START_ADDR(dst_rb)) + dst_w_head;
    for (;remain_data_length != 0; ) {
        src_len_2_boundary = (uint32_t)MUX_RB_END_ADDR(src_rb) - src_read_addr;
        dst_len_2_boundary = (uint32_t)MUX_RB_END_ADDR(dst_rb) - dst_write_addr;
        min_len_2_boundary = RB_MIN(src_len_2_boundary, dst_len_2_boundary);
        min_block_len = RB_MIN(min_len_2_boundary, remain_data_length);
        if (src_step == 3) {
            remain_n = min_block_len % 0x3;
            block_n = min_block_len - remain_n;
        } else {
            block_n = min_block_len & 0xFFFFFFFC;
            remain_n = min_block_len & 0x3;
        }
        if (block_n != 0) {
            #if 0
            LOG_MSGID_W(common,"[%02x] copy block=%3u s2b=%3u d2b=%3u sra=0x%08x dwa=0x%08x remain_data_len=%3u remain_n=%u min_blk_len=%3u src_addr=(0x%08x:0x%08x) dst_addr=(0x%08x:0x%08x)",13, \
                dst_rb->uid, block_n, src_len_2_boundary, dst_len_2_boundary, src_read_addr, dst_write_addr, remain_data_length, remain_n, min_block_len, \
                (uint32_t)(MUX_RB_START_ADDR(src_rb)), (uint32_t)(MUX_RB_END_ADDR(src_rb)), (uint32_t)(MUX_RB_START_ADDR(dst_rb)), (uint32_t)(MUX_RB_END_ADDR(dst_rb)));
            #endif
#if 1
            uint32_t block_n_tmp;
            switch(move_flag) {
            case 0://4->4
                #if 1
                for(i = src_offset, j = dst_offset; i < block_n; i += src_step, j += dst_step) {
                    B2B_COPY_WORD(j, i);
                }
                #else
                memcpy((uint8_t*)dst_write_addr, (uint8_t*)src_read_addr, block_n);
                i = j = block_n;
                #endif
            break;
            case MUX_RB_FLAG_DATA_4TOH3:
            case MUX_RB_FLAG_DATA_4TOL3:
                block_n_tmp = block_n + src_offset;
                for(i = src_offset, j = dst_offset; i < block_n_tmp; i += src_step, j += dst_step) {
                    B2B_COPY_WORD(j, i);
                }
                i -= src_offset;
            break;
            case MUX_RB_FLAG_DATA_3TOH4:
            case MUX_RB_FLAG_DATA_3TOL4:
                block_n_tmp = block_n + dst_offset;
                for(i = src_offset, j = dst_offset; j < block_n_tmp; i += src_step, j += dst_step) {
                    B2B_COPY_WORD(j, i);
                }
                j -= dst_offset;
            break;
            default:
                assert(0 && "Error move flag!!");
                return;
            }

#else
            block_n += src_offset;
            for(i = src_offset, j = dst_offset; i < block_n; i += src_step, j += dst_step) {
                B2B_COPY_WORD(j, i);
            }
            i -= src_offset;
            j -= dst_offset;
#endif
            remain_data_length -= i;
            B2B_SRC_ADDR_MOVE_WRAP(i);
            B2B_DST_ADDR_MOVE_WRAP(j);
            #if 0
            LOG_MSGID_W(common,"[%02x] copy block end s2b=%3u d2b=%3u sra=0x%08x dwa=0x%08x remain_data_len=%3u i=%3u j=%3u",8, \
                dst_rb->uid, (uint32_t)MUX_RB_END_ADDR(src_rb) - src_read_addr, (uint32_t)MUX_RB_END_ADDR(dst_rb) - dst_write_addr, \
                src_read_addr, dst_write_addr, remain_data_length,i, j);
            #endif
        }
        if (remain_n != 0) {
            if (move_flag == 0) {
                switch(remain_n) {
                case 1:
                    B2B_COPY_BYTE(0, 0);
                    break;
                case 2:
                    B2B_COPY_HALF_WORD(0, 0);
                    break;
                case 3:
                    B2B_COPY_HALF_WORD(0, 0);
                    B2B_COPY_BYTE(2, 2);
                    break;
                default:
                    break;
                }
                B2B_SRC_ADDR_MOVE_WRAP(remain_n);
                B2B_DST_ADDR_MOVE_WRAP(remain_n);
                remain_data_length -= remain_n;
            } else {
                B2B_COPY_BYTE(dst_offset    , src_offset);
                B2B_COPY_BYTE(dst_offset + 1, src_offset + 1);
                B2B_COPY_BYTE(dst_offset + 2, src_offset + 2);

                B2B_SRC_ADDR_MOVE_WRAP(src_step);
                B2B_DST_ADDR_MOVE_WRAP(dst_step);
                remain_data_length -= src_step;
            }
        }
    #if 0
        LOG_MSGID_W(common,"[%02x] copy remain= %u s2b=%3u d2b=%3u sra=0x%08x dwa=0x%08x remain_data_len=%3u src_offset=%u dst_offset=%u",9, \
            dst_rb->uid, remain_n, (uint32_t)MUX_RB_END_ADDR(src_rb) - src_read_addr, (uint32_t)MUX_RB_END_ADDR(dst_rb) - dst_write_addr, \
            src_read_addr, dst_write_addr, remain_data_length, src_offset, dst_offset);
        if ((int32_t)remain_data_length < 0) {
            LOG_MSGID_E(common,"remain_data_length=%u",1 ,remain_data_length);
            assert(0 && "remain_data_length < 0");
        }
    #endif
    }

    *crc = CRC_INIT_VAR;
}


ATTR_TEXT_IN_FAST_MEM static  void mux_ringbuffer_move_st(mux_ringbuffer_t * src_rb, mux_ringbuffer_t * dst_rb, uint32_t length,  uint16_t *crc)
{
#ifdef MUX_RB_ADDRESS_POWER_OF_2_ENABLE
    mux_ringbuffer_move_core(src_rb, src_rb->rptr.head & src_rb->mask, dst_rb, dst_rb->wptr.head & dst_rb->mask, length, crc);
#else
    mux_ringbuffer_move_core(src_rb, src_rb->rptr.head, dst_rb, dst_rb->wptr.head, length, crc);
#endif
}

ATTR_TEXT_IN_FAST_MEM static __FORCE_INLINE__ void mux_ringbuffer_move(mux_ringbuffer_t * src_rb, uint32_t src_r_head, mux_ringbuffer_t * dst_rb, uint32_t dst_w_head, uint32_t length,  uint16_t *crc)
{
#ifdef MUX_RB_ADDRESS_POWER_OF_2_ENABLE
    mux_ringbuffer_move_core(src_rb, src_r_head & src_rb->mask, dst_rb, dst_w_head & dst_rb->mask, length, crc);
#else
    mux_ringbuffer_move_core(src_rb, src_r_head, dst_rb, dst_w_head, length, crc);
#endif
}

ATTR_TEXT_IN_FAST_MEM static __FORCE_INLINE__ void mux_ringbuffer_3_to_4_copy(mux_ringbuffer_t * src_rb, uint32_t src_r_head, mux_ringbuffer_t * dst_rb, uint32_t dst_w_head, uint32_t length, uint16_t *crc)
{
    mux_ringbuffer_move(src_rb, src_r_head, dst_rb, dst_w_head, length, crc);
}

ATTR_TEXT_IN_FAST_MEM static __FORCE_INLINE__ void mux_ringbuffer_4_to_3_copy(mux_ringbuffer_t * src_rb, uint32_t src_r_head, mux_ringbuffer_t * dst_rb, uint32_t dst_w_head, uint32_t length, uint16_t *crc)
{
    mux_ringbuffer_move(src_rb, src_r_head, dst_rb, dst_w_head, length, crc);
}

ATTR_TEXT_IN_FAST_MEM static __FORCE_INLINE__ void mux_ringbuffer_4_to_4_copy(mux_ringbuffer_t * src_rb, uint32_t src_r_head, mux_ringbuffer_t * dst_rb, uint32_t dst_w_head, uint32_t length, uint16_t *crc)
{
    mux_ringbuffer_move(src_rb, src_r_head, dst_rb, dst_w_head, length, crc);
}

ATTR_TEXT_IN_FAST_MEM void mux_ringbuffer_buffer_2_buffer(mux_ringbuffer_t * src_rb, mux_ringbuffer_t * dst_rb, uint32_t r_old_head, uint32_t w_old_head, uint32_t writable_len, uint16_t *crc)
{
    uint32_t move_flag = (src_rb->flags & MUX_RB_FLAG_DATA_4TO3) | (dst_rb->flags & MUX_RB_FLAG_DATA_3TO4);

    switch(move_flag) {
    case 0://4->4
        mux_ringbuffer_4_to_4_copy(src_rb, r_old_head, dst_rb, w_old_head, writable_len, crc);
        break;
    case MUX_RB_FLAG_DATA_4TOH3:
        mux_ringbuffer_4_to_3_copy(src_rb, r_old_head, dst_rb, w_old_head, writable_len, crc);
        break;
    case MUX_RB_FLAG_DATA_4TOL3:
        mux_ringbuffer_4_to_3_copy(src_rb, r_old_head, dst_rb, w_old_head, writable_len, crc);
    break;
    case MUX_RB_FLAG_DATA_3TOH4:
        mux_ringbuffer_3_to_4_copy(src_rb, r_old_head, dst_rb, w_old_head, writable_len, crc);
        break;
    case MUX_RB_FLAG_DATA_3TOL4:
        mux_ringbuffer_3_to_4_copy(src_rb, r_old_head, dst_rb, w_old_head, writable_len, crc);
    break;
    default:
        assert(0 && "Error move flag!!");
    }
}

ATTR_TEXT_IN_FAST_MEM uint32_t mux_ringbuffer_write_buffer_st(mux_ringbuffer_t *src_rb, mux_ringbuffer_t *dst_rb, uint32_t data_len, uint16_t *crc)
{
    uint32_t real_writable_len;
    uint32_t move_flag = (src_rb->flags & MUX_RB_FLAG_DATA_4TO3) | (dst_rb->flags & MUX_RB_FLAG_DATA_3TO4);

    switch(move_flag) {
    case 0://4->4
        real_writable_len = data_len;
        src_rb->start_offset = 0;
        dst_rb->start_offset = 0;
        src_rb->step = 4;
        dst_rb->step = 4;
        break;
    case MUX_RB_FLAG_DATA_4TOH3:
    case MUX_RB_FLAG_DATA_4TOL3:
        if (data_len & 0x3) {
            RB_LOG_E("[%02x]:data_len=%d is not multiple of 4", 2, src_rb->uid, data_len);
            assert(0);
            return 0;
        }
        real_writable_len = data_len >> 2;
        real_writable_len = real_writable_len + (real_writable_len << 1);
        RB_LOG_D("[write_buffer_st][%02x] 4->3 real_writable_len=%d data_len=%d", 3, src_rb->uid , real_writable_len, data_len);

        if (move_flag == MUX_RB_FLAG_DATA_4TOH3) {
            src_rb->start_offset = 1;
        } else {
            src_rb->start_offset = 0;
        }
        dst_rb->start_offset = 0;
        src_rb->step = 4;
        dst_rb->step = 3;
        break;
    case MUX_RB_FLAG_DATA_3TOH4:
    case MUX_RB_FLAG_DATA_3TOL4:
        /* data_len must be multiple of 3 */
        if (data_len % 3) {
            RB_LOG_E("[%02x]:data_len=%d is not multiple of 3", 2, dst_rb->uid, data_len);
            assert(0);
            return 0;
        }
        real_writable_len = (data_len/3) << 2;//TODO:optimize divide
        // RB_LOG_D("[write_buffer_st][%02x] 3->4 real_writable_len=%d data_len=%d", 3, dst_rb->uid, real_writable_len, data_len);
        //only check dst buffer for 3->4 case here
        if (RB_TOTAL_FREE_SPACE_SIZE(dst_rb) < real_writable_len) {
            RB_LOG_E("[write_buffer_st] error !! uid[%02X]->uid[%02X]:data_len=%d r_len=%d real_w_len=%d src_free_len=%d dest_free_len=%d", 7,\
            src_rb->uid, dst_rb->uid, data_len, data_len, real_writable_len, RB_TOTAL_FREE_SPACE_SIZE(src_rb), RB_TOTAL_FREE_SPACE_SIZE(dst_rb));
            assert(0 && "dest fifo have not enough free space");
            return 0;
        }
        if (move_flag == MUX_RB_FLAG_DATA_3TOH4) {
            dst_rb->start_offset = 1;
        } else {
            dst_rb->start_offset = 0;
        }
        src_rb->start_offset = 0;
        src_rb->step = 3;
        dst_rb->step = 4;
        break;
    default:
        assert(0 && "Error move flag!!");
        return 0;
    }

    mux_ringbuffer_move_st(src_rb, dst_rb, data_len, crc);

    RB_MOVE_RPTR(src_rb, data_len);
    RB_MOVE_WPTR(dst_rb, real_writable_len);

#ifdef MUX_RB_WATER_MARK_ENABLE
  if (!dst_rb->ops) { // only SW fifo water mark
    uint32_t total_data_len = RB_TOTAL_DATA_SIZE(dst_rb);
    if (total_data_len > dst_rb->water_mark) {
      dst_rb->water_mark = total_data_len;
    }
  }
#endif
  return real_writable_len;
}

/**
 * @brief move 'len' bytes data from 'src_rb' to 'dst_rb'
 *
 * @param src_rb
 * @param dst_rb
 * @param len
 * @return real length that have been moved done.
 */
ATTR_TEXT_IN_FAST_MEM uint32_t mux_ringbuffer_write_buffer(mux_ringbuffer_t *src_rb, mux_ringbuffer_t *dst_rb, uint32_t len, uint16_t *crc)
{
  uint32_t r_old_head;
  uint32_t w_old_head;
  uint32_t r_new_head;
  uint32_t w_new_head;
  uint32_t size = len;
  uint32_t readable_len;
  uint32_t writable_len;

  readable_len = mux_ringbuffer_read_move_head(src_rb, &r_old_head, &r_new_head, size);
  writable_len = mux_ringbuffer_write_move_head(dst_rb, &w_old_head, &w_new_head, readable_len);

  if ((src_rb->flags & MUX_RB_FLAG_DATA_4TO3) || (dst_rb->flags & MUX_RB_FLAG_DATA_3TO4)) {
    if ((writable_len < readable_len) || (writable_len & 0x3)) {
      RB_LOG_E("[mux_ringbuffer_write_buffer] destination buffer have no enough free space!! or writable_len is not multiple of 4", 0);
      assert(0);
      return 0;
    }
  }

  mux_ringbuffer_buffer_2_buffer(src_rb, dst_rb, r_old_head, w_old_head, writable_len, crc);

  mux_ringbuffer_read_move_tail(src_rb, r_old_head, r_new_head);
  mux_ringbuffer_write_move_tail(dst_rb, w_old_head, w_new_head);
#ifdef MUX_RB_WATER_MARK_ENABLE
    uint32_t total_data_len = RB_TOTAL_DATA_SIZE(dst_rb);
    if (total_data_len > dst_rb->water_mark) {
      dst_rb->water_mark = total_data_len;
    }
#endif
  return writable_len;
}

ATTR_TEXT_IN_FAST_MEM uint32_t mux_ringbuffer_write(mux_ringbuffer_t *rb, uint8_t *buffer, uint32_t len)
{
    uint32_t old_head;
    uint32_t new_head;
    uint32_t write_len;
    uint32_t size = len;

    write_len = mux_ringbuffer_write_move_head(rb, &old_head, &new_head, size);

    mux_ringbuffer_write_move_data(rb, buffer, write_len, old_head);

    mux_ringbuffer_write_move_tail(rb, old_head, new_head);

    return write_len;
}

ATTR_TEXT_IN_FAST_MEM uint32_t mux_ringbuffer_write_try(mux_ringbuffer_t *rb, uint8_t *buffer, uint32_t len)
{
  if (mux_ringbuffer_free_space(rb) < len) {
    return 0;
  }
  return mux_ringbuffer_write(rb, buffer, len);
}

void mux_ringbuffer_reset(mux_ringbuffer_t *rb) {
  MUX_RB_ASSERT(rb);
  rb->wptr.head = 0;
  rb->wptr.tail = 0;
  rb->rptr.head = 0;
  rb->rptr.tail = 0;
}

#define RB_LOG_I_ARG16(cnt, i, pdata) LOG_MSGID_I(common, "|%08X: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X |", cnt + 1, (uint32_t)(pdata + i), \
                pdata[i + 0], pdata[i + 1], pdata[i + 2], pdata[i + 3], pdata[i + 4], pdata[i + 5], pdata[i + 6], pdata[i + 7],\
                pdata[i + 8], pdata[i + 9], pdata[i + 10], pdata[i + 11], pdata[i + 12], pdata[i + 13], pdata[i + 14], pdata[i + 15]);
#define RB_LOG_I_ARG15(cnt, i, pdata) LOG_MSGID_I(common, "|%08X: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X   |", cnt + 1, (uint32_t)(pdata + i), \
                pdata[i + 0], pdata[i + 1], pdata[i + 2], pdata[i + 3], pdata[i + 4], pdata[i + 5], pdata[i + 6], pdata[i + 7],\
                pdata[i + 8], pdata[i + 9], pdata[i + 10], pdata[i + 11], pdata[i + 12], pdata[i + 13], pdata[i + 14]);
#define RB_LOG_I_ARG14(cnt, i, pdata) LOG_MSGID_I(common, "|%08X: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X      |", cnt + 1, (uint32_t)(pdata + i), \
                pdata[i + 0], pdata[i + 1], pdata[i + 2], pdata[i + 3], pdata[i + 4], pdata[i + 5], pdata[i + 6], pdata[i + 7],\
                pdata[i + 8], pdata[i + 9], pdata[i + 10], pdata[i + 11], pdata[i + 12], pdata[i + 13]);
#define RB_LOG_I_ARG13(cnt, i, pdata) LOG_MSGID_I(common, "|%08X: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X         |", cnt + 1, (uint32_t)(pdata + i), \
                pdata[i + 0], pdata[i + 1], pdata[i + 2], pdata[i + 3], pdata[i + 4], pdata[i + 5], pdata[i + 6], pdata[i + 7],\
                pdata[i + 8], pdata[i + 9], pdata[i + 10], pdata[i + 11], pdata[i + 12]);
#define RB_LOG_I_ARG12(cnt, i, pdata) LOG_MSGID_I(common, "|%08X: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X            |", cnt + 1, (uint32_t)(pdata + i), \
                pdata[i + 0], pdata[i + 1], pdata[i + 2], pdata[i + 3], pdata[i + 4], pdata[i + 5], pdata[i + 6], pdata[i + 7],\
                pdata[i + 8], pdata[i + 9], pdata[i + 10], pdata[i + 11]);
#define RB_LOG_I_ARG11(cnt, i, pdata) LOG_MSGID_I(common, "|%08X: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X               |", cnt + 1, (uint32_t)(pdata + i), \
                pdata[i + 0], pdata[i + 1], pdata[i + 2], pdata[i + 3], pdata[i + 4], pdata[i + 5], pdata[i + 6], pdata[i + 7],\
                pdata[i + 8], pdata[i + 9], pdata[i + 10]);
#define RB_LOG_I_ARG10(cnt, i, pdata) LOG_MSGID_I(common, "|%08X: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X                  |", cnt + 1, (uint32_t)(pdata + i), \
                pdata[i + 0], pdata[i + 1], pdata[i + 2], pdata[i + 3], pdata[i + 4], pdata[i + 5], pdata[i + 6], pdata[i + 7],\
                pdata[i + 8], pdata[i + 9]);
#define RB_LOG_I_ARG9(cnt, i, pdata) LOG_MSGID_I(common, "|%08X: %02X %02X %02X %02X %02X %02X %02X %02X %02X                      |", cnt + 1, (uint32_t)(pdata + i), \
                pdata[i + 0], pdata[i + 1], pdata[i + 2], pdata[i + 3], pdata[i + 4], pdata[i + 5], pdata[i + 6], pdata[i + 7],\
                pdata[i + 8]);
#define RB_LOG_I_ARG8(cnt, i, pdata) LOG_MSGID_I(common, "|%08X: %02X %02X %02X %02X %02X %02X %02X %02X                         |", cnt + 1, (uint32_t)(pdata + i), \
                pdata[i + 0], pdata[i + 1], pdata[i + 2], pdata[i + 3], pdata[i + 4], pdata[i + 5], pdata[i + 6], pdata[i + 7]);
#define RB_LOG_I_ARG7(cnt, i, pdata) LOG_MSGID_I(common, "|%08X: %02X %02X %02X %02X %02X %02X %02X                            |", cnt + 1, (uint32_t)(pdata + i), \
                pdata[i + 0], pdata[i + 1], pdata[i + 2], pdata[i + 3], pdata[i + 4], pdata[i + 5], pdata[i + 6]);
#define RB_LOG_I_ARG6(cnt, i, pdata) LOG_MSGID_I(common, "|%08X: %02X %02X %02X %02X %02X %02X                               |", cnt + 1, (uint32_t)(pdata + i), \
                pdata[i + 0], pdata[i + 1], pdata[i + 2], pdata[i + 3], pdata[i + 4], pdata[i + 5]);
#define RB_LOG_I_ARG5(cnt, i, pdata) LOG_MSGID_I(common, "|%08X: %02X %02X %02X %02X %02X                                  |", cnt + 1, (uint32_t)(pdata + i), \
                pdata[i + 0], pdata[i + 1], pdata[i + 2], pdata[i + 3], pdata[i + 4]);
#define RB_LOG_I_ARG4(cnt, i, pdata) LOG_MSGID_I(common, "|%08X: %02X %02X %02X %02X                                     |", cnt + 1, (uint32_t)(pdata + i), \
                pdata[i + 0], pdata[i + 1], pdata[i + 2], pdata[i + 3]);
#define RB_LOG_I_ARG3(cnt, i, pdata) LOG_MSGID_I(common, "|%08X: %02X %02X %02X                                        |", cnt + 1, (uint32_t)(pdata + i), \
                pdata[i + 0], pdata[i + 1], pdata[i + 2]);
#define RB_LOG_I_ARG2(cnt, i, pdata) LOG_MSGID_I(common, "|%08X: %02X %02X                                           |", cnt + 1, (uint32_t)(pdata + i), \
                pdata[i + 0], pdata[i + 1]);
#define RB_LOG_I_ARG1(cnt, i, pdata) LOG_MSGID_I(common, "|%08X: %02X                                              |", cnt + 1, (uint32_t)(pdata + i), \
                pdata[i + 0]);


#define dp_log(arg_num, i, pdata) RB_LOG_I_ARG##arg_num (arg_num, i, pdata)

static void _hexdump_internal(const void *data, unsigned int len)
{
    uint32_t i = 0;
    uint32_t n_remain = len & 0xF;
    uint32_t n_block = len >> 4;
    const uint8_t *pdata= data;
    uint32_t block_len = len & (~0xF);
    // printf("block_len=%d n_remain=%d n_block=%d len=%d\r\n", block_len, n_remain, n_block, len);
    if (n_block > 0) {
        for (i = 0; i < block_len; i += 16) {
            dp_log(16, i, pdata);
        }
    }
    switch (n_remain) {
        case 1 : dp_log(1 , i, pdata);break;
        case 2 : dp_log(2 , i, pdata);break;
        case 3 : dp_log(3 , i, pdata);break;
        case 4 : dp_log(4 , i, pdata);break;
        case 5 : dp_log(5 , i, pdata);break;
        case 6 : dp_log(6 , i, pdata);break;
        case 7 : dp_log(7 , i, pdata);break;
        case 8 : dp_log(8 , i, pdata);break;
        case 9 : dp_log(9 , i, pdata);break;
        case 10: dp_log(10, i, pdata);break;
        case 11: dp_log(11, i, pdata);break;
        case 12: dp_log(12, i, pdata);break;
        case 13: dp_log(13, i, pdata);break;
        case 14: dp_log(14, i, pdata);break;
        case 15: dp_log(15, i, pdata);break;
    }
}
void _hexdump(const char *funcname, const void *data, unsigned int len, const void *data1, unsigned int len1)
{
    (void)funcname;

    if (len > 0) {
        LOG_MSGID_I(common, "+------------------------len = %3d-------------------------+", 1, len);
        // LOG_MSGID_I(common, "| Offset : 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F|", 0);
        // LOG_MSGID_I(common, "+---------------------------------------------------------+", 0);
        _hexdump_internal(data, len);

    }
    if (data1 && (len1 > 0)) {
        LOG_MSGID_I(common, "+------------------------len = %3d------------------------+", 1, len1);
        _hexdump_internal(data1, len1);
    }
    if ((len > 0) || (len1 > 0)) {
      LOG_MSGID_I(common, "+----------------------------------------------------------+", 0);
    }
}
