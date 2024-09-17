/*
 * Copyright (c) 2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-06     Supperthomas first version
 * 2023-12-03     Meco Man     support nano version
 */

#include <board.h>
#include <rtthread.h>
#include <drv_gpio.h>
#ifndef RT_USING_NANO
#include <rtdevice.h>
#endif /* RT_USING_NANO */

#define GPIO_LED_B    GET_PIN(F, 11)
#define GPIO_LED_R    GET_PIN(F, 12)
#define GPIO_Wind_AIN1 GET_PIN(E,3)
#define GPIO_Wind_AIN2 GET_PIN(E,4)

extern void AHT10_Creat(void);
extern void MQTT_Creat_Thread(void);
extern int rt_irq_init(void);
extern int rt_ap3216(void);
extern int pwm_fan_init();

int main(void)
{   
    char *name = "iPhone";
    char *password = "12345678lwj";
    rt_wlan_scan();
    rt_wlan_connect(name,password);
    pwm_fan_init();
    rt_irq_init();
    AHT10_Creat();
    rt_ap3216();
    if(rt_wlan_connect(name,password) == RT_EOK)
    {
        MQTT_Creat_Thread();
    }
    rt_pin_mode(GPIO_LED_R, PIN_MODE_OUTPUT);
   

    while (1)
    {
        rt_pin_write(GPIO_LED_R, PIN_HIGH);
        rt_thread_mdelay(500);
        rt_pin_write(GPIO_LED_R, PIN_LOW);
        rt_thread_mdelay(500);
      
    }
}
