#ifndef DELAY_H_included
#define DELAY_H_included

#include <stdint.h>

uint32_t get_elapsed_time(void);
void delay(uint16_t ms);
void delay_periodic_call(void);

#endif
