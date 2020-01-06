#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kobject.h>

#include "led7_module.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Andrii Koval <andykoval@ucu.edu.ua>");
MODULE_DESCRIPTION("A simple Linux driver for 7-segment LED display.");
MODULE_VERSION("0.1");

static unsigned int gpio_pins[] = {4, 13, 22, 26, 6, 17, 25};

static unsigned int current_number = 0;

int display_number(unsigned char num) {

    //        4 
    //     #=====#
    //  17 |  25 | 13
    //     #=====#
    //  6  |     | 22
    //     #=====#
    //        26

    const unsigned int numbers[] = {
        0b0111111,
        0b0000110,
        0b1011011,
        0b1001111,
        0b1100110,
        0b1101101,
        0b1111101,
        0b0000111,
        0b1111111,
        0b1101111,
    };

    if (num > 9) {
        printk(KERN_ERR "Invalid input number. Try in range [0-9]\n");
        return 1;
    }

    int i = 0;
    for (; i < 7; ++i) {
        toggle_pin(gpio_pins[i], (numbers[num] >> i) & 1);
    }

    current_number = num;
    return 0;
}

static ssize_t number_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
    return sprintf(buf, "Current number: %u\n", (unsigned char) current_number);
}

static ssize_t number_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) {
    int i;
    for (i = 0; i < count; ++i) {
        display_number(buf[0] - '0');
    }
    return count;
}

static struct kobj_attribute number_attr = __ATTR(current_number, 0660, number_show, number_store);

static struct attribute *led_display_attributes[] = {
        &number_attr.attr,
        NULL,
};

static struct attribute_group attr_group = {
    .name = "led_display",
    .attrs = led_display_attributes
};

static struct kobject *led_display_kobj;

static int __init led_display_init(void) {
    int result = 0;

    printk(KERN_INFO "Initializing 7-segment display\n");

    led_display_kobj = kobject_create_and_add("led_display", kernel_kobj->parent);
    if (!led_display_kobj){
        printk(KERN_ALERT "EBB LED: failed to create kobject\n");
        return -ENOMEM;
    }

    result = sysfs_create_group(led_display_kobj, &attr_group);
    if (result) {
        printk(KERN_ALERT "EBB LED: failed to create sysfs group\n");
        kobject_put(led_display_kobj);
        return result;
    }

    return result;

}

static void __exit led_display_exit(void) {
    kobject_put(led_display_kobj);
    printk(KERN_INFO "7-segment LED display shutdown...\n");
}

module_init(led_display_init);
module_exit(led_display_exit);