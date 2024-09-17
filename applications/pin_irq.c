#include <rtthread.h>
#include <rtdevice.h>
#include <drv_gpio.h>
#include <drv_matrix_led.h>
//定义按键
#define KEY_UP GET_PIN(C,5)
#define KEY_DOWN GET_PIN(C,1)

#define KEY_LEFT GET_PIN(C,0)
#define KEY_RIGHT GET_PIN(C,4)

void led_matrix_stepfill_Blue(uint8_t index);
extern void pwm_fan_set(uint32_t pulse);
extern int Temp_fan;
int Irq_fan = 0;

//按键回调函数
void key_up_callback( void * args)
{
    Temp_fan = 0;
    Irq_fan = 1;
    led_matrix_stepfill_Blue(14);
    rt_kprintf("fan turn two \n");
    pwm_fan_set(250000);

}

void key_down_callback( void * args)
{
    Temp_fan = 0;
    Irq_fan = 0;
    led_matrix_stepfill_Blue(0);
    rt_kprintf("key down! fan close \n");
    pwm_fan_set(0);
}

void key_left_callback( void * args)
{
    Temp_fan = 0;
    Irq_fan = 1;
    led_matrix_stepfill_Blue(7);
    rt_kprintf("fan turn one \n");
    pwm_fan_set(100000);
}

void key_right_callback( void * args)
{
    Temp_fan = 0;
    Irq_fan = 1;
    led_matrix_stepfill_Blue(20);
    rt_kprintf("fan turn three \n");
    pwm_fan_set(400000);
}

int rt_irq_init(void)
{
    //初始化引脚模式
    rt_pin_mode(KEY_UP, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(KEY_DOWN, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(KEY_LEFT, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(KEY_RIGHT, PIN_MODE_INPUT_PULLUP);
    //将其与中断回调函数绑定
    rt_pin_attach_irq(KEY_UP, PIN_IRQ_MODE_FALLING,key_up_callback,RT_NULL);
    rt_pin_attach_irq(KEY_DOWN, PIN_IRQ_MODE_FALLING,key_down_callback,RT_NULL);
    rt_pin_attach_irq(KEY_LEFT, PIN_IRQ_MODE_FALLING,key_left_callback,RT_NULL);
    rt_pin_attach_irq(KEY_RIGHT, PIN_IRQ_MODE_FALLING,key_right_callback,RT_NULL);
    //使能中断
    rt_pin_irq_enable(KEY_UP, PIN_IRQ_ENABLE);
    rt_pin_irq_enable(KEY_DOWN, PIN_IRQ_ENABLE);
    rt_pin_irq_enable(KEY_LEFT, PIN_IRQ_ENABLE);
    rt_pin_irq_enable(KEY_RIGHT, PIN_IRQ_ENABLE);
    return RT_EOK;
}
MSH_CMD_EXPORT(rt_irq_init,rt_irq_init);