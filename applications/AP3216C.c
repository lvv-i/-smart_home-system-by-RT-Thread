#include <rtthread.h>
#include <rtdevice.h>
#include <ap3216c.h>
#include <drv_lcd.h>
#include <drv_matrix_led.h>

rt_uint16_t ps_data;
float LightLux;

#define AP3216C_I2C_BUS "i2c2"

// 创建AHT线程时待用
#define THREAD_PRIORITY 25
#define THREAD_STACK_SIZE 2048
#define THREAD_TIMESLICE 5

static void ap3216c_get(void *param)
{
    const char * i2c_bus_name = AP3216C_I2C_BUS;
    //初始化设备
    ap3216c_device_t dev = ap3216c_init(i2c_bus_name);
    if(dev == RT_NULL)
    {
        rt_kprintf("dev init failed");
        return ;
    }

    while(1)
    {
        ps_data = ap3216c_read_ps_data(dev);
        LightLux = ap3216c_read_ambient_light(dev);

        rt_thread_mdelay(100);
        lcd_show_string(16,50,16,"lightLux:%.2f",LightLux);
        if(LightLux <= 5.0)//检测到人 
        {
            lcd_show_string(16,70,16,"People detected, Light open       ");
            led_matrix_fill_test(3);
        }
        else if(LightLux >= 1500.0)
        {
            lcd_show_string(16,70,16,"Light abnormality !               ");
            lcd_show_string(16,90,16,"Pay attention to protect the eyes ");
            led_matrix_fill_test(0);
        }
        else
        {
            lcd_show_string(16,70,16,"Light close                       ");
            led_matrix_fill(BLACK);
        }
    }
}

int rt_ap3216(void)
{
    rt_thread_t ap3216c = rt_thread_create("rt_ap3216",ap3216c_get,RT_NULL,THREAD_STACK_SIZE,THREAD_PRIORITY,THREAD_TIMESLICE);
    if(ap3216c != RT_NULL)
    {
        rt_thread_startup(ap3216c);
    }
    else 
        rt_kprintf(" rt_ap3216 thread create failed\n");
    return 1;
}
MSH_CMD_EXPORT(rt_ap3216,rt_ap3216);