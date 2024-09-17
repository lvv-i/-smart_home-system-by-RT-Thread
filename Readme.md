#        项目：基于RT-Thread实时操作系统的智能家居系统（第一版上）

***简介说明：目前为第一版，该版由我参加24年RT夏令营学习到提交RT-Thread全球嵌入式电子设计大赛期间完成，后续会继续添加功能。本项目基于星火一号RTT官方教程板，是一款应用了RT-Thread嵌入式实时操作系统的智能家居系统，具体细节如下介绍***  



星火一号：

![image-20240910171116098](C:\Users\22192\AppData\Roaming\Typora\typora-user-images\image-20240910171116098.png)





## 一、本系统功能

  ### 1.云端实时获取室内温度，湿度，光照强度

> 用户可以上网从阿里云物联网服务平台实时获取室内的温度，湿度，光照强度，同时数据会在LCD上显示

#### 功能解析：

RW007接入网络后，星火一号通过MQTT协议连接上阿里云平台，发布由AHT10获取的温度，湿度话题，由AP3216C获取的光照强度话题被订阅到平台上，数据将实时更新并显示在lcd上，如图：

<img src="C:\Users\22192\Desktop\MyFirst_project\lcd效果图.jpg" alt="lcd效果图" style="zoom: 50%;" />



#### 依赖模块：

| 模块名称           |                         功能 |
| ------------------ | ---------------------------: |
| RW007              |           wifi模块，连接wifi |
| AP3216C            | 光照强度检测，同时可提供距离 |
| AHT10              |               湿度温度传感器 |
| ail MQTT（软件包） |     实现PC端查看实时数据功能 |



### 实现步骤：

1.分别获取温湿度和光照强度的模块aht10和ap3216c都用到了I2C，并且创建了两条线程，线程入口函数如下：

**AHT10.c:**

```c
static void AHT10_Get(void *parameter)

{

  // AHT设备指针

  aht10_device_t Dev = RT_NULL;



  // Humi:湿度值,Temp:温度值



  // 初始化设备

  Dev = aht10_init(AHT10_I2C_BUS);

  if (Dev == RT_NULL)

  {

​    rt_kprintf("AHT10_init Fail");

​    return;

  }



  while (1)

  {

​    // 读取温湿度值

​    Humi = aht10_read_humidity(Dev);

​    Temp = aht10_read_temperature(Dev);



​    // 没有下载rt_vsprintf_full软件包的时候

​    //rt_kprintf("Humi: %d.%d\n", (int)Humi, (int)(Humi * 10) % 10);

​    //rt_kprintf("Temp: %d.%d\n", (int)Temp, (int)(Temp * 10) % 10);



​    // 配合rt_vsnprintf_full软件包使用

​    //rt_kprintf("Humi: %f, Temp: %f\n", Humi, Temp);

​    rt_thread_mdelay(100);

​    lcd_show_string(16,10,16,"temp:%.2f",Temp);

​    lcd_show_string(16,30,16,"humidity:%.2f",Humi);



​    rt_thread_mdelay(1000);

​    if(Temp > 30.0) Temp_fan = 1;else Temp_fan = 0;

​    if(Temp_fan == 1 && Irq_fan == 0)

​    {

​      pwm_fan_set(100000);

​      lcd_show_string(16,110,16,"High temp! fan open");

​    }

​    else

​    {

​      lcd_show_string(16,110,16,"          ");

​    } 

  }

}
```



AP32C.c:

```c
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
```



2.Menuconfig里勾选了ail MQTT软件包和RW007软件包后，在main函数里实现wifi自启动连接，mqtt-example里实现包含温湿度光照强度的数据话题发送到阿里云平台上，由平台订阅，关键代码如下：

**main函数里：**

```c
    char *name = "iPhone";
    char *password = "12345678lwj";
    rt_wlan_scan();
    rt_wlan_connect(name,password);
```



**mqtt-example.c:**

```c
static int example_publish(void *handle)
{
    int             res = 0;
    const char     *fmt = "/sys/%s/%s/thing/event/property/post";
    char           *topic = NULL;
    int             topic_len = 0;
    char           payload[300] = {0};
    
    //k1mm5LUFErB/RT_Spark/user/get
    rt_snprintf(payload,sizeof(payload),"{\"params\":{\"Temp\":%.2f,\"RoomHumidity\":%.2f,\"LightLux\":%.2f}}",Temp, Humi,LightLux);
    topic_len = strlen(fmt) + strlen(DEMO_PRODUCT_KEY) + strlen(DEMO_DEVICE_NAME) + 1;
    topic = HAL_Malloc(topic_len);
    if (topic == NULL) {
        EXAMPLE_TRACE("memory not enough");
        return -1;
    }
    memset(topic, 0, topic_len);
    HAL_Snprintf(topic, topic_len, fmt, DEMO_PRODUCT_KEY, DEMO_DEVICE_NAME);

    res = IOT_MQTT_Publish_Simple(0, topic, IOTX_MQTT_QOS0, payload, strlen(payload));
    if (res < 0) {
        EXAMPLE_TRACE("publish failed, res = %d", res);
        HAL_Free(topic);
        return -1;
    }

    HAL_Free(topic);
    return 0;
}
```

关于mqtt线程入口函数和RW007连接wifi的实现，由于软件包自带的无改动，所以不做赘述



### 2.智能家居辅助功能

> 第一版辅助功能有：1.检测到人体，自动打开灯光
>
> 2.室温高于30摄氏度，自动打开一档风扇，也可以按键控制风扇，有三档和关闭风扇方式，检测按键按下矩阵灯会有不同灯光表现
>
> 以上功能在lcd屏幕会有显示

*外接模块：*

*TB6612电机驱动和一个直流电机风扇以及面包板基础器件*



#### 关键程序解释：

1.风扇直流电机的驱动，需要星火一号pmod引脚里可用的TIM引脚来实现pwm输出，这里选用PB10（TIM2_CH3）,以及PE3和PE4来作为电机输入，这里要事先配置工程board里的stm32cubemx来配置电机输入引脚，输出模式即可，代码如下：

```c
#define GPIO_Wind_AIN1 GET_PIN(E,3)
#define GPIO_Wind_AIN2 GET_PIN(E,4)

#define PWM_DEV_NAME        "pwm2"  /* PWM设备名称 */
#define PWM_DEV_CHANNEL     3      /* PWM通道 */

struct rt_device_pwm *pwm_dev;      /* PWM设备句柄 */

void motor_Init(void)
{
    rt_pin_mode(GPIO_Wind_AIN1,PIN_MODE_OUTPUT);
    rt_pin_mode(GPIO_Wind_AIN2,PIN_MODE_OUTPUT);

    rt_pin_write(GPIO_Wind_AIN1, PIN_HIGH);
    rt_pin_write(GPIO_Wind_AIN2, PIN_LOW);

}

int pwm_fan_init()
{
    motor_Init();
    rt_uint32_t period,dir;

    period = 500000;    /* 周期为0.5ms，单位为纳秒ns */
    dir = 1;            /* PWM脉冲宽度值的增减方向 */
   // pulse = 0;          /* PWM脉冲宽度值，单位为纳秒ns */

    /* 查找设备 */
    pwm_dev = (struct rt_device_pwm *)rt_device_find(PWM_DEV_NAME);
    if (pwm_dev == RT_NULL)
    {
        rt_kprintf("pwm sample run failed! can't find %s device!\n", PWM_DEV_NAME);
        return RT_ERROR;
    }

    /* 设置PWM周期和脉冲宽度默认值 */
    rt_pwm_set(pwm_dev, PWM_DEV_CHANNEL, period, 0);
    /* 使能设备 */
    rt_pwm_enable(pwm_dev, PWM_DEV_CHANNEL);
}

void pwm_fan_set(uint32_t pulse)
{
    rt_pwm_set(pwm_dev, PWM_DEV_CHANNEL, 500000, pulse);
}

```



2.检测到人体打开灯光功能依赖ap3216c模块，根据光照强度判断即可，不做简述；

3.风扇模式，即根据按键中断实现，在中断函数里写即可，这里为了协同温度超过30°打开风扇的功能添加了两个int参数作为标识符，本来想着用一下线程间同步的知识，可是按键是中断实现，在中断函数里释放和接受信号量是不允许的所以放弃了，采用了简单的方法，代码如下：

```c
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
```





### 二、总结

   **由于完成这一作品时知识有限，花在上面的时间不多，功能就这些，我也将其定为第一版半成品，第一版待实现功能还有如下：**



0.用一下lvgl试一下能不能实现下面功能

1.增加lcd显示世界时钟功能，以及天气预报api的获取，将实时当前地区的时间和天气显示在lcd屏幕上（最好在弄一个麦克风播报）

2.写一个手机软件或者微信小程序，能获取星火一号获取的信息并且控制灯光和风扇

3.加入离线语音模块，加入人机对话功能，类似小爱同学



**总之，在第一版完成期间我会继续补习RT-Thread的知识，代码我会不断改进优化，用到更多RTT内核的知识和代码实现才是我的目的。**



### 三、预告

​    ***在第一版完成之后，我将加入一块板子作为上位机，也是我手上闲置的有算力的板子RDKx3，加入ROS操作系统和计算机视觉来组成第二版，至于功能等我完成第一版再去考虑。***



[^1]:视频链接：https://www.bilibili.com/video/BV18ft4exEUc/?buvid=Y24C1F29669AC4514053B6345EC68E7BF127&is_story_h5=false&mid=XM5u4uiogouCEINUrZgPkA%3D%3D&plat_id=240&share_from=ugc&share_medium=iphone&share_plat=ios&share_source=WEIXIN&share_tag=s_i&timestamp=1726499636&unique_k=eEOodxQ&up_id=677059901
[^2]:GitHub仓库：https://github.com/lvv-i/smart_home-system-by-RT-Thread
