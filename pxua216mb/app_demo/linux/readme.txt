���뷽ʽ��

gcc hidwrite.c waster_led.c hidtest.c libhidapi-hidraw.a -ludev

��ע�⣺linux��Ҫ��װlibdev-dev��

�����Ƶķ�ʽ�£�

unsigned int led_value;
led_value = 0xFFFFFFFF;
led_value &= ~(LED1|LED32|LED24);
hid_write_led_value(led_value);

�����ǵ���1,32,24���ŵơ����Ϩ�������ŵƣ����£�

unsigned int led_value;
led_value = 0xFFFFFFFF;
led_value |= (LED1|LED32|LED24);
hid_write_led_value(led_value);
