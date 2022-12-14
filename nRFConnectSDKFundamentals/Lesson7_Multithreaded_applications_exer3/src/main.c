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
    (Preemptive) C?? th??? ng???t ngang nh???ng process kh??c ??ang ch???y
    Khi c?? thread kh??c xu???t hi???n th?? th???c hi???n Scheduling l???i (Rescheduling point)
    ????? ??u ti???n -> th??? t??? xu???t hi???n trong queue
    
    
    Gi???i th??ch k???t qu???:
    Thread1 t???n kho???ng 30ms ????? ho??n th??nh -> nhi???u h??n th???i gian sleep l?? 20ms

    0    Ban ?????u Thread0 (T0) ch???y v?? t???o 1 work cho v??o w_queue
                (W_Queue n??y b???n ch???t gi???ng nh?? 1 thread kh??c c?? prio l?? 4)
    0    T0 ho??n th??nh v?? in ra console -> 0ms
    0    T0 sleep 20ms
    20    T1 b???t ?????u v?? ch???y ???????c 20ms (T0 "t???nh" l???i th?? ng???ng)
    20    T0 ch???y v?? t???o 1 work cho v??o w_queue
    20    T0 ho??n th??nh v?? in ra console -> 0ms
    20    T0 sleep 20ms
    30    T1 ch???y ti???p v?? ch???y m???t 10ms th?? ho??n th??nh -> 30ms
    30    T1 sleep 20ms
    40    W_Queue ???????c ch???y 10ms (T0 "t???nh" l???i th?? ng???ng)
    40    T0 ch???y v?? t???o 1 work cho v??o w_queue
    40    T0 ho??n th??nh v?? in ra console -> 0ms
    40    T0 sleep 20ms
    50    W_Queue ???????c ch???y 10ms (T1 "t???nh" l???i th?? ng???ng)
    60    T1 b???t ?????u v?? ch???y ???????c 10ms (T0 "t???nh" l???i th?? ng???ng)
    60    T0 ch???y v?? t???o 1 work cho v??o w_queue
    60    T0 ho??n th??nh v?? in ra console -> 0ms
    60    T0 sleep 20ms
    80    T1 b???t ?????u v?? ch???y ???????c 20ms v?? ho??n th??nh -> 30ms (T0 "t???nh" l???i)
    80    T1 sleep 20ms
    80    T0 ch???y v?? t???o 1 work cho v??o w_queue
    80    T0 ho??n th??nh v?? in ra console -> 0ms
    80    T0 sleep 20ms
    90    W_Queue ???????c ch???y 10ms th?? ho??n th??nh -> In ra finish
    100   W_Queue ???????c ch???y 10ms (T0 v?? T1 "t???nh" l???i th?? ng???ng)
    100   T0 .........
    
*/

K_THREAD_DEFINE(thread0_id, THREAD0_STACKSIZE, thread0, NULL, NULL, NULL,
		THREAD0_PRIORITY, 0, 0);
K_THREAD_DEFINE(thread1_id, THREAD1_STACKSIZE, thread1, NULL, NULL, NULL,
		THREAD1_PRIORITY, 0, 0);

