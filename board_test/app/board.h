#ifndef BOARD_H_included
#define BOARD_H_included

#include <stm32.h>

#include <driver_mcu.h>
#include <driver_dev.h>

#define EEPROM_SIZE SIZE_4Kb
#define FLASH_SIZE SIZE_8MB
#define FLASH_SECTOR_SIZE SIZE_4KB
#define SDRAM_SIZE SIZE_32MB

extern led_driver_t *led_user_led0_driver;
extern led_driver_t *led_user_led1_driver;
extern button_driver_t *button_user_button_driver;
extern uart_driver_t *uart_uart2_driver;
extern eeprom_driver_t *eeprom_eeprom_driver;
extern flash_driver_t *flash_flash_driver;
extern sdram_driver_t *sdram_sdram_driver;

void board_init(void);
void board_periodic_call(void);

#endif
