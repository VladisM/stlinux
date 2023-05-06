#ifndef TESTS_H_included
#define TESTS_H_included

#include <driver_dev.h>

#include <stdbool.h>

bool test_led(led_driver_t *led);
bool test_eeprom(eeprom_driver_t *eeprom);
bool test_flash(flash_driver_t *flash);
bool test_sdram(sdram_driver_t *sdram);

#endif
