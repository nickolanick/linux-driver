#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kobject.h>

#include "led7_module.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Andrii Koval <andykoval@ucu.edu.ua>");
MODULE_DESCRIPTION("A simple Linux driver for 7-segment LED display.");
MODULE_VERSION("0.1");

//        4 
//     #=====#
//  17 |  25 | 13
//     #=====#
//  6  |     | 22
//     #=====#
//        26

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
    .name = "7-segment led display",
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