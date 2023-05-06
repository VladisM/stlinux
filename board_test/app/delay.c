#include "delay.h"

#include <stdint.h>

volatile uint32_t timer = 0;
volatile uint32_t elapsed = 0;

uint32_t get_elapsed_time(void){
    return elapsed;
}

void delay(uint16_t ms){
    timer = 0;
    while(timer < ms);
}

void delay_periodic_call(void){
    timer++;
    elapsed++;
    return;
}
