编译方式：

gcc hidwrite.c waster_led.c hidtest.c libhidapi-hidraw.a -ludev

请注意：linux需要安装libdev-dev库

点亮灯的方式下：

unsigned int led_value;
led_value = 0xFFFFFFFF;
led_value &= ~(LED1|LED32|LED24);
hid_write_led_value(led_value);

以上是点亮1,32,24三颗灯。如果熄灭这三颗灯，如下：

unsigned int led_value;
led_value = 0xFFFFFFFF;
led_value |= (LED1|LED32|LED24);
hid_write_led_value(led_value);
