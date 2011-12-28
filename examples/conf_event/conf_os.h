#ifndef __CONF_OS_H__
#define __CONF_OS_H__

#define CONFIG_OS_SCHEDULER_TYPE CONFIG_OS_SCHEDULER_USE_COMPARE
#define CONFIG_OS_USE_PRIORITY false
#define CONFIG_OS_DEBUG true
#define CONFIG_OS_USE_TICK_COUNTER true
#define CONFIG_OS_USE_16BIT_TICKS false
#define CONFIG_OS_USE_SW_INTERRUPTS true
#define CONFIG_OS_USE_EVENTS true
#define CONFIG_OS_TASK_DEFAULT_PRIORITY OS_PRIORITY_1

#define CONFIG_OS_PROCESS_ENABLE_FIFO false

#endif // __CONF_OS_H__
