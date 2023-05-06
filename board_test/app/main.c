#include "board.h"
#include "delay.h"
#include "tests.h"

#include <debug.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

static bool perform_test(void);

int main(void){
    debug_config_t debug_config;
    debug_config.get_elapsed_ms = get_elapsed_time;

    DEBUG_INIT(&debug_config);
    SysTick_Config(SystemCoreClock / 1000);
    board_init();

    DEBUG_INFO("Device initialized.");

    printf("Running board test.\r\n");
    printf("FCPU: %"PRIu32" MHz\r\n", SystemCoreClock / 1000000);

    if(!perform_test()){
        printf("Test failed!\r\n");
    }
    else{
        printf("Test succeeded!\r\n");
    }

    while(1) __NOP();
    return 0;
}

void SysTick_Handler(void){
    delay_periodic_call();
    board_periodic_call();
}

static bool perform_test(void){
    bool test_ret_eeprom = false;
    bool test_ret_flash = false;
    bool test_ret_sdram = false;
    bool test_ret_led_0 = false;
    bool test_ret_led_1 = false;

    printf("Press user button to start test...\r\n");
    while(button_driver_read(button_user_button_driver) != true) __NOP();
    while(button_driver_read(button_user_button_driver) == true) __NOP();

    test_ret_led_0 = test_led(led_user_led0_driver);
    test_ret_led_1 = test_led(led_user_led1_driver);
    test_ret_eeprom = test_eeprom(eeprom_eeprom_driver);
    test_ret_flash = test_flash(flash_flash_driver);
    test_ret_sdram = test_sdram(sdram_sdram_driver);

    printf("All tests done. Results: \r\n");
    printf(" LED.0  - %s\r\n", test_ret_led_0 ? "pass" : "fail");
    printf(" LED.1  - %s\r\n", test_ret_led_1 ? "pass" : "fail");
    printf(" EEPROM - %s\r\n", test_ret_eeprom ? "pass" : "fail");
    printf(" FLASH  - %s\r\n", test_ret_flash ? "pass" : "fail");
    printf(" SDRAM  - %s\r\n", test_ret_sdram ? "pass" : "fail");

    return test_ret_led_0 && test_ret_led_1 && test_ret_eeprom && test_ret_flash && test_ret_sdram;
}
