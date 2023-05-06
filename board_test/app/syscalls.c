#include <sys/stat.h>
#include <sys/types.h>
#include <sys/unistd.h>

#include <debug.h>
#include <stdlib.h>
#include <stdio.h>

#include "board.h"

extern uint32_t _heap_start;
extern uint32_t _heap_end;

static void * heap_end = 0;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

int _close(int file){
    return -1;
}

int _fstat(int file, struct stat* st){
    st->st_mode = S_IFCHR;
    return 0;
}

int _isatty(int file){
    if(file == STDOUT_FILENO || file == STDIN_FILENO || file == STDERR_FILENO)
        return 1;
    else
        return 0;
}

int _lseek(int file, int ptr, int dir){
    return 0;
}

int _open(const char* name, int flags, int mode){
    return -1;
}

int _read(int file, char* ptr, int len){
    if(file != STDIN_FILENO)
        return -1;

    while(uart_driver_rx_buffer_is_empty(uart_uart2_driver) == true) __NOP();

    *ptr = uart_driver_rx_buffer_read(uart_uart2_driver);

    if(*ptr == '\r')
        *ptr = '\n';

    if(*ptr == '\n'){
        uart_driver_puts(uart_uart2_driver, "\r\n");
    }
    else{
        uart_driver_putc(uart_uart2_driver, *ptr);
    }

    return 1;
}

int _write(int file, char *ptr, int len){
    if(file != STDOUT_FILENO && file != STDERR_FILENO)
        return -1;

    for(int i = 0; i < len; i++){
        uart_driver_putc(uart_uart2_driver, ptr[i]);
    }

    return len;
}

void _exit(int status){
    __asm("BKPT #0");
    while(1);
}

void _kill(int pid, int sig){
    return;
}

int _getpid(void){
    return -1;
}

void * _sbrk(int incr){
    char* prev_heap_end;

    if(heap_end == 0){
        heap_end = &_heap_start;
    }

    prev_heap_end = heap_end;

    if(heap_end + incr > (void *)&_heap_end){
        return (void *)-1; //no more memory left
    }

    heap_end += incr;
    return (void *) prev_heap_end;
}


#pragma GCC diagnostic pop
