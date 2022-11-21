/*
 * Copyright (c) 2017 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <sys/printk.h>
#include <kernel.h>
#include <kernel_structs.h>
#include <string.h>

// Define stack size used by each thread
#define THREAD0_STACKSIZE       512
#define THREAD1_STACKSIZE       512
#define WORQ_THREAD_STACK_SIZE  512

/* STEP 2 - Set the priorities of the threads */
#define THREAD0_PRIORITY 2
#define THREAD1_PRIORITY 3
#define WORKQ_PRIORITY 4

// Define stack area used by workqueue thread
static K_THREAD_STACK_DEFINE(my_stack_area, WORQ_THREAD_STACK_SIZE);

// Define queue structure
static struct k_work_q offload_work_q = {0};


/* STEP 5 - Define function to emulate non-urgent work */
static inline void emulate_work()
{
    for (volatile int count_out = 0; count_out < 150000; count_out++);
}

/* STEP 7 - Create work_info structure and offload function */
struct work_info
{
    struct k_work work;
    char name[25];
} my_work;
void offload_function(struct k_work *work_tem)
{
    emulate_work();
    printk("Finish\n");
}

void thread0(void)
{
    uint64_t time_stamp;
    int64_t delta_time;
	/* STEP 8 - Start the workqueue, */
    k_work_queue_start(&offload_work_q, my_stack_area,
                       K_THREAD_STACK_SIZEOF(my_stack_area), WORKQ_PRIORITY,
                       NULL);
    strcpy(my_work.name, "Thread0 emulate_work()");
    k_work_init(&my_work.work, offload_function);
    /* initialize the work item and connect it to its handler function */ 
    while (1) {
        time_stamp = k_uptime_get();		
        
        /* STEP 9 - Submit the work item to the workqueue instead of calling emulate_work() directly */
        k_work_submit_to_queue(&offload_work_q, &my_work.work);
        /* Remember to comment out emulate_work(); */
        // emulate_work();
		delta_time = k_uptime_delta(&time_stamp);
        printk("thread0 yielding this round in %lld ms\n", delta_time);
        k_msleep(20);
    }   
}

/* STEP 4 - Define entry function for thread1 */
void thread1(void)
{
    uint64_t time_stamp;
    int64_t delta_time;

    while (1) {
        time_stamp = k_uptime_get();
        emulate_work();
        delta_time = k_uptime_delta(&time_stamp);
        printk("thread1 yielding this round in %lld ms\n", delta_time);
        k_msleep(20);
    }   
}
/*

    ***Priority Scheduling***
    (Preemptive) Có thể ngắt ngang những process khác đang chạy
    Khi có thread khác xuất hiện thì thực hiện Scheduling lại (Rescheduling point)
    Độ ưu tiền -> thứ tự xuất hiện trong queue
    
    
    Giải thích kết quả:
    Thread1 tốn khoảng 30ms để hoàn thành -> nhiều hơn thời gian sleep là 20ms

    0    Ban đầu Thread0 (T0) chạy và tạo 1 work cho vào w_queue
                (W_Queue này bản chất giống như 1 thread khác có prio là 4)
    0    T0 hoàn thành và in ra console -> 0ms
    0    T0 sleep 20ms
    20    T1 bắt đầu và chạy được 20ms (T0 "tỉnh" lại thì ngừng)
    20    T0 chạy và tạo 1 work cho vào w_queue
    20    T0 hoàn thành và in ra console -> 0ms
    20    T0 sleep 20ms
    30    T1 chạy tiếp và chạy mất 10ms thì hoàn thành -> 30ms
    30    T1 sleep 20ms
    40    W_Queue được chạy 10ms (T0 "tỉnh" lại thì ngừng)
    40    T0 chạy và tạo 1 work cho vào w_queue
    40    T0 hoàn thành và in ra console -> 0ms
    40    T0 sleep 20ms
    50    W_Queue được chạy 10ms (T1 "tỉnh" lại thì ngừng)
    60    T1 bắt đầu và chạy được 10ms (T0 "tỉnh" lại thì ngừng)
    60    T0 chạy và tạo 1 work cho vào w_queue
    60    T0 hoàn thành và in ra console -> 0ms
    60    T0 sleep 20ms
    80    T1 bắt đầu và chạy được 20ms và hoàn thành -> 30ms (T0 "tỉnh" lại)
    80    T1 sleep 20ms
    80    T0 chạy và tạo 1 work cho vào w_queue
    80    T0 hoàn thành và in ra console -> 0ms
    80    T0 sleep 20ms
    90    W_Queue được chạy 10ms thì hoàn thành -> In ra finish
    100   W_Queue được chạy 10ms (T0 và T1 "tỉnh" lại thì ngừng)
    100   T0 .........
    
*/

K_THREAD_DEFINE(thread0_id, THREAD0_STACKSIZE, thread0, NULL, NULL, NULL,
		THREAD0_PRIORITY, 0, 0);
K_THREAD_DEFINE(thread1_id, THREAD1_STACKSIZE, thread1, NULL, NULL, NULL,
		THREAD1_PRIORITY, 0, 0);

