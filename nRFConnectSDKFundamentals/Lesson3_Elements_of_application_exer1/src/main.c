#include <zephyr.h>
#include <sys/printk.h>
#include "myfunction.h"

void main(void)
{
    int a = 3, b = 4;
    while (1)
    {
        if (!(IS_ENABLED(CONFIG_MYFUNCTION)))
        {
            printk("MYFUNCTION not enabled\r\n");
            return;
        }
        printk("The sum of %d and %d is %d\n\r", a, b, sum(a, b));
        //LOG_INF("nRF Connect SDK Fundamentals");
        k_msleep(1000);
    }
}