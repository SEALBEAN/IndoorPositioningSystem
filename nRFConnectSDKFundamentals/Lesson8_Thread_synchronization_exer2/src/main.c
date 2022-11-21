/*
 * Copyright (c) 2017 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <device.h>
#include <drivers/gpio.h>
#include <sys/printk.h>
#include <sys/__assert.h>
#include <kernel.h>
#include <kernel_structs.h>
#include <string.h>
#include <random/rand32.h>

#define THREAD0_STACKSIZE       512
#define THREAD1_STACKSIZE       512
// Bật CONFIG_MULTITHREADING=y bên prj.conf
/* STEP 3 - Set the priority of the two threads to have equal priority*/
#define THREAD0_PRIORITY 4
#define THREAD1_PRIORITY 4
/* STEP 5 - Khai báo vùng nhớ dùng chung giữa 2 thread */
#define COMBINED_TOTAL 40
int32_t increment_partner = 0;
int32_t decrement_partner = COMBINED_TOTAL;
/* STEP 11 - Khai báo mutex để bảo vệ CR */
K_MUTEX_DEFINE(test_mutex);
// Shared code run by both threads
void shared_code_section(void)
{
	/* STEP 12.1 - Khóa mutex */
	k_mutex_lock(&test_mutex, K_FOREVER);
	
	/* STEP 6 - Increment partner and decrement partner changed */
	increment_partner += 1;
	increment_partner = increment_partner % COMBINED_TOTAL;
	decrement_partner -= 1;
	if (decrement_partner == 0)
	{
		decrement_partner = COMBINED_TOTAL;
	}
	/* according to logic defined in exercise text */

	/* STEP 12.2 - Mở khóa mutex */
	k_mutex_unlock(&test_mutex);
	/* STEP 7 - Nếu sự chính xác của dữ liệu không còn bảo đảm thì in ra */
	if (increment_partner + decrement_partner != COMBINED_TOTAL)
	{
		printk("Increment_partner (%d) + Decrement_partner (%d) = %d \n",
			   increment_partner, decrement_partner, (increment_partner + decrement_partner));
	}
}

/* STEP 4 - Functions for thread0 and thread1 with a shared code section */
void thread0(void)
{
	printk("Thread 0 started\n");
	while (1)
	{
		shared_code_section();
	}
}
void thread1(void)
{
	printk("Thread 1 started\n");
	while (1)
	{
		shared_code_section();
	}
}
/*
	Mutexes sử dụng 1 biến để xác định thread nào đã truy cập vào critical region (CR)
	Và chỉ cho thread đấy có khả năng unlock CR
	Mỗi lần thread đó lock thì sẽ tăng giá trị của biến lên 1
	Khi biến = 0 thì sẽ unlock và thread khác có thể truy cập vào

	Dùng Mutexes khi có vùng nhớ cần bảo vệ (CR) hoặc cần sự đồng bộ giữa các thread (sync)
*/


// Define and initialize threads
K_THREAD_DEFINE(thread0_id, THREAD0_STACKSIZE, thread0, NULL, NULL, NULL,
		THREAD0_PRIORITY, 0, 0);

K_THREAD_DEFINE(thread1_id, THREAD1_STACKSIZE, thread1, NULL, NULL, NULL,
		THREAD1_PRIORITY, 0, 0);

