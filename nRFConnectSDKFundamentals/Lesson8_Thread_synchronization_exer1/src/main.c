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

#define PRODUCER_STACKSIZE       512
#define CONSUMER_STACKSIZE       512

/* STEP 2 - Set the priority of the producer and consumper thread */
#define PRODUCER_PRIORITY 5
#define CONSUMER_PRIORITY 4
/* STEP 9 - Khai báo semaphore và số lượng resource */
K_SEM_DEFINE(instance_monitor_sem, 10, 10);
/* STEP 3 - Initialize the available instances of this resource */
volatile uint32_t available_instance_count = 10;
// Function for getting access of resource
void get_access(void)
{
	/* STEP 10.1 - Get truy cập vào resource */
	k_sem_take(&instance_monitor_sem, K_FOREVER);
	/* STEP 6.1 - Decrement available resource */
	// available_instance_count--;
	// printk("Resource taken and available_instance_count = %d\n", available_instance_count);
	printk("Resource taken and available_instance_count = %d\n", k_sem_count_get(&instance_monitor_sem));
}

// Function for releasing access of resource
void release_access(void)
{
	/* STEP 6.2 - Increment available resource */
	// available_instance_count++;
	// printk("Resource given and available_instance_count = %d\n", available_instance_count);
	/* STEP 10.2 - Give trả resource */
	k_sem_give(&instance_monitor_sem);
	printk("Resource given and available_instance_count = %d\n", k_sem_count_get(&instance_monitor_sem));
}

/* STEP 4 - Producer thread relinquishing access to instance */
void producer(void)
{
	printk("Producer thread started\n");
	while (1)
	{
		release_access();
		// Assume the resource instance access is released at this point
		k_msleep(sys_rand32_get() % 10);
	}
}

/* STEP 5 - Consumer thread obtaining access to instance */
void consumer(void)
{
	printk("Consumer thread started\n");
	while (1)
	{
		get_access();
		// Assume the resource instance access is released at this point
		k_msleep(sys_rand32_get() % 10);
	}
}
/*
	Semaphore sử dụng 1 biến để lưu số lượng resource còn có sẵn để sử dụng
	Số lượng resource được khai báo ngay từ đầu gồm số lượng tối đa và số lượng hiện có
	Take  -- Lấy resource -> giảm 1
	Give  -- Trả resource -> tăng 1
	Khi = 0 thì phải chờ những thằng khác trả resource mới làm tiếp

	Semaphore sử dụng để không request quá số lượng resource có sẵn để sử dụng
*/

// Define and initialize threads
K_THREAD_DEFINE(producer_id, PRODUCER_STACKSIZE, producer, NULL, NULL, NULL,
		PRODUCER_PRIORITY, 0, 0);

K_THREAD_DEFINE(consumer_id, CONSUMER_STACKSIZE, consumer, NULL, NULL, NULL,
		CONSUMER_PRIORITY, 0, 0);