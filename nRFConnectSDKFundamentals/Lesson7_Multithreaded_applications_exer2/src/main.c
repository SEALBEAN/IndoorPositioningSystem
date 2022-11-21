/*
 * Copyright (c) 2017 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <sys/printk.h>

#define STACKSIZE 1024

/* STEP 5 - Change the priority of thread0 to 6 */
#define THREAD0_PRIORITY 7
#define THREAD1_PRIORITY 7
#define THREAD2_PRIORITY 7

void thread0(void)
{
	while (1) {
          printk("Hello, I am thread0\n");
		  k_msleep(10);
	}
}

void thread1(void)
{
	while (1) {
          printk("Hello, I am thread1\n");
		  k_msleep(1);
	}
}

void thread2(void)
{
	while (1)
	{
		printk("Hello, I am thread2\n");
	}
}

/*
	***Priority Scheduling***

	CONFIG_TIMESLICE_PRIORITY=0 Những thread có độ ưu tiên >= thì mới áp dụng timeslice
	Hết timeslice thì schedule sẽ lựa thread tiếp theo dựa trên độ ưu tiên như trước
	Là Độ ưu tiền -> thứ tự khi vào queue
*/

K_THREAD_DEFINE(thread0_id, STACKSIZE, thread0, NULL, NULL, NULL,
		THREAD0_PRIORITY, 0, 0);
K_THREAD_DEFINE(thread1_id, STACKSIZE, thread1, NULL, NULL, NULL,
		THREAD1_PRIORITY, 0, 0);
K_THREAD_DEFINE(thread2_id, STACKSIZE, thread2, NULL, NULL, NULL,
				THREAD2_PRIORITY, 0, 0);
