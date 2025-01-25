#ifndef __FSMOD_SOURCE_UTIL_H_
#define __FSMOD_SOURCE_UTIL_H_

#include <linux/printk.h>

#define MODULE_NAME "snfs"
#define LOG(fmt, ...) pr_info("[" MODULE_NAME "]: " fmt, ##__VA_ARGS__)

#define LOGM(method, fmt, ...) pr_info("[" MODULE_NAME "|" method "]: " fmt, ##__VA_ARGS__)

#endif  // __FSMOD_SOURCE_UTIL_H_
