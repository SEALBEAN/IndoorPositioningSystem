/*
 * Copyright (c) 2017 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <sys/printk.h>

/* STEP 2 - Define stack size and scheduling priority used by each thread */
#define STACKSIZE 1024
#define THREAD0_PRIORITY 7
#define THREAD1_PRIORITY 7
#define THREAD2_PRIORITY 7

void thread0(void)
{
	while (1) {
		  /* STEP 3 - Call printk() to display a simple string "Hello, I am thread0" */
		  printk("Hello, I am thread0\n");
		  /* STEP 6 - Make the thread yield */
		  k_yield();
		  /* STEP 10 - Put the thread to sleep */
		//   k_msleep(500);
		  /* Remember to comment out the line from STEP 6 */
	}
}

void thread1(void)
{
	while (1) {
		  /* STEP 3 - Call printk() to display a simple string "Hello, I am thread1" */
		  printk("Hello, I am thread1\n");
		  /* STEP 8 - Make the thread yield */
		  k_yield();
		  /* STEP 10 - Put the thread to sleep */
		//   k_msleep(500);
		  /* Remember to comment out the line from STEP 8 */
	}
}

void thread2(void)
{
	while (1)
	{
		/* STEP 3 - Call printk() to display a simple string "Hello, I am thread1" */
		printk("Hello, I am thread2\n");
		/* STEP 8 - Make the thread yield */
		  k_yield();
		/* STEP 10 - Put the thread to sleep */
		// k_msleep(500);
		/* Remember to comment out the line from STEP 8 */
	}
}

/*
	Khi dùng k_yeild() thì thread sẽ chuyển sang ready state và nhường cho thread tiếp theo do schedule quyết định
	Độ ưu tiên : Priority -> Thứ tự được vào hàng chờ 
	k_msleep(msec) thì thread sẽ chuyển sang block state hết thời gian thì sẽ quay lại vào hàng chờ
*/


/* STEP 4 - Define and initialize the two threads */
K_THREAD_DEFINE(thread0_id, STACKSIZE, thread0, NULL, NULL, NULL,
				THREAD0_PRIORITY, 0, 0);
K_THREAD_DEFINE(thread1_id, STACKSIZE, thread1, NULL, NULL, NULL,
				THREAD1_PRIORITY, 0, 0);
K_THREAD_DEFINE(thread2_id, STACKSIZE, thread2, NULL, NULL, NULL,
				THREAD2_PRIORITY, 0, 0);