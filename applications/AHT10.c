#include <board.h>
#include <rtthread.h>
#include <drv_gpio.h>
#include <drv_lcd.h>
#include <string.h>
#include "aht10.h"

// AHT挂载的总线名字
#define AHT10_I2C_BUS "i2c3"

// 创建AHT线程时待用
#define THREAD_PRIORITY 25
#define THREAD_STACK_SIZE 2048
#define THREAD_TIMESLICE 5

// AHT线程指针
rt_thread_t AHT10 = RT_NULL;
float Humi, Temp;
int Temp_fan = 0;
extern int Irq_fan;

extern void pwm_fan_set(uint32_t pulse);

// AHT测试样例
static void AHT10_Get(void *parameter)
{
    // AHT设备指针
    aht10_device_t Dev = RT_NULL;

    // Humi:湿度值,Temp:温度值

    // 初始化设备
    Dev = aht10_init(AHT10_I2C_BUS);
    if (Dev == RT_NULL)
    {
        rt_kprintf("AHT10_init Fail");
        return;
    }

    while (1)
    {
        // 读取温湿度值
        Humi = aht10_read_humidity(Dev);
        Temp = aht10_read_temperature(Dev);

        // 没有下载rt_vsprintf_full软件包的时候
        //rt_kprintf("Humi: %d.%d\n", (int)Humi, (int)(Humi * 10) % 10);
        //rt_kprintf("Temp: %d.%d\n", (int)Temp, (int)(Temp * 10) % 10);

        // 配合rt_vsnprintf_full软件包使用
        //rt_kprintf("Humi: %f, Temp: %f\n", Humi, Temp);
        rt_thread_mdelay(100);
        lcd_show_string(16,10,16,"temp:%.2f",Temp);
        lcd_show_string(16,30,16,"humidity:%.2f",Humi);

        rt_thread_mdelay(1000);
        if(Temp > 30.0) Temp_fan = 1;else Temp_fan = 0;
        if(Temp_fan == 1 && Irq_fan == 0)
        {
            pwm_fan_set(100000);
            lcd_show_string(16,110,16,"High temp! fan open");
        }
        else
        {
            lcd_show_string(16,110,16,"                   ");
        } 
    }
}

void AHT10_Creat(void)
{
    // 创建线程
    AHT10 = rt_thread_create("aht10", AHT10_Get, RT_NULL, THREAD_STACK_SIZE, THREAD_PRIORITY, THREAD_TIMESLICE);
    // 创建成功就启动
    if (AHT10 != RT_NULL)
    {
        rt_thread_startup(AHT10);
    }
    else
    {
        rt_kprintf("AHT10_Thread Create Fail");
    }
}
// 导出Shell命令
MSH_CMD_EXPORT(AHT10_Creat, AHT10_Creat);