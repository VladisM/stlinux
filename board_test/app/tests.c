#include "tests.h"

#include "generator.h"

#include <debug.h>

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include <utillib/cli.h>

#define TEST_SEED           42
#define EEPROM_TEST_SEED    TEST_SEED
#define FLASH_TEST_SEED     TEST_SEED
#define SDRAM_TEST_SEED     TEST_SEED

#define FLASH_PROGRESS_BAR_UPDATE_INTERVAL      32
#define SDRAM_PROGRESS_BAR_UPDATE_INTERVAL      (SIZE_128KB)

bool test_led(led_driver_t *led){
    printf("Testing LED device '%s'.\r\n", led->led_name);

    led_driver_set(led, LED_ON);

    bool ret = question_YES_no("Is LED device '%s' on?", led->led_name);

    led_driver_set(led, LED_OFF);

    return ret;
}

bool test_eeprom(eeprom_driver_t *eeprom){
    bool testResult = true;

    if(eeprom == NULL){
        DEBUG_ERROR("NULL argument to test_eeprom!");
        return false;
    }

    uint32_t memory_size = eeprom_driver_get_size(eeprom);

    printf("Testing EEPROM memory device '%s' of size %"PRIu32" bytes.\r\n", eeprom->eeprom_name, memory_size);

    uint8_t *tmp = (uint8_t *)malloc(sizeof(uint8_t) * memory_size);

    if(tmp == NULL){
        DEBUG_ERROR("Malloc failed!");
        return false;
    }

    memset((void *)tmp, 0, sizeof(uint8_t) * memory_size);

    test_data_generator_t *generator = NULL;

    if(!generator_init(&generator, EEPROM_TEST_SEED)){
        DEBUG_ERROR("Generator initialization failed!");
        free(tmp);
        return false;
    }

    for(uint32_t i = 0; i < memory_size; i++){
        tmp[i] = generator_gen(generator, i);
    }

    eeprom_driver_write(eeprom, tmp);

    free(tmp);
    tmp = NULL;

    generator_reset(generator);

    uint32_t misses = 0;

    for(uint32_t i = 0; i < memory_size; i++){
        uint8_t generated_data = generator_gen(generator, i);
        uint8_t eeprom_data = eeprom_driver_read_byte(eeprom, i);

        if(generated_data != eeprom_data){
            printf("data in eeprom mismatch; eeprom[0x%"PRIX32"] = 0x%X (should be: 0x%X)\r\n", i, eeprom_data, generated_data);
            misses++;
            testResult = false;
        }
    }

    generator_destroy(generator);

    if(testResult == true){
        printf("EEPROM result is OK.\r\n");
    }
    else{
        printf("EEPROM result is NOK (%"PRIu32" misses).\r\n", misses);
    }

    return testResult;
}

bool test_flash(flash_driver_t *flash){
    bool testResult = true;

    if(flash == NULL){
        DEBUG_ERROR("NULL argument to test_flash!");
        return false;
    }

    uint32_t memory_size = flash_driver_get_size(flash);
    uint32_t sector_size = flash_driver_get_sector_size(flash);
    uint32_t sector_count = flash_driver_get_sector_count(flash);

    printf("Testing FLASH memory device '%s' of size %"PRIu32" bytes.\r\n", flash->flash_name, memory_size);

    uint8_t *tmp = (uint8_t *)malloc(sizeof(uint8_t) * sector_size);

    if(tmp == NULL){
        DEBUG_ERROR("Malloc failed!");
        return false;
    }

    memset((void *)tmp, 0, sizeof(uint8_t) * sector_size);

    test_data_generator_t *generator = NULL;

    if(!generator_init(&generator, FLASH_TEST_SEED)){
        DEBUG_ERROR("Generator initialization failed!");
        free(tmp);
        return false;
    }

    printf("Mass erase operation ongoing...\r\n");
    flash_driver_mass_erase(flash);

    progress_bar_t *write_progress_bar = NULL;
    progress_bar_init(&write_progress_bar, "writing", sector_count, stdout, PROGRESS_BAR_BAR);

    for(uint32_t sector = 0; sector < sector_count; sector++){
        for(uint32_t i = 0; i < sector_size; i++){
            uint32_t address = i + (sector * sector_size);
            tmp[i] = generator_gen(generator, address);
        }

        if((sector + 1) % FLASH_PROGRESS_BAR_UPDATE_INTERVAL == 0){    //less frequent updating of progress bar
            progress_bar_update(write_progress_bar, sector + 1);
        }

        flash_driver_write_sector(flash, sector, tmp);
        memset((void *)tmp, 0, sizeof(uint8_t) * sector_size);
    }

    progress_bar_clean(write_progress_bar);
    progress_bar_destroy(write_progress_bar);

    generator_reset(generator);

    uint32_t misses = 0;

    progress_bar_t *reading_progress_bar = NULL;
    progress_bar_init(&reading_progress_bar, "reading", sector_count, stdout, PROGRESS_BAR_BAR);

    for(uint32_t sector = 0; sector < sector_count; sector++){
        memset((void *)tmp, 0, sizeof(uint8_t) * sector_size);

        if((sector + 1) % FLASH_PROGRESS_BAR_UPDATE_INTERVAL == 0){    //less frequent updating of progress bar
            progress_bar_update(write_progress_bar, sector + 1);
        }

        flash_driver_read_sector(flash, sector, tmp);

        for(uint32_t i = 0; i < sector_size; i++){
            uint32_t address = i + (sector * sector_size);
            uint8_t generated_data = generator_gen(generator, address);
            uint8_t flash_data = tmp[i];

            if(generated_data != flash_data){
                progress_bar_clean(reading_progress_bar);
                printf("data in flash mismatch; flash[0x%"PRIX32"] = 0x%"PRIX8" (should be: 0x%"PRIX8")\r\n", address, flash_data, generated_data);
                misses++;
                testResult = false;
            }
        }
    }

    progress_bar_clean(reading_progress_bar);
    progress_bar_destroy(reading_progress_bar);
    generator_destroy(generator);

    free(tmp);
    tmp = NULL;

    if(testResult == true){
        printf("FLASH result is OK.\r\n");
    }
    else{
        printf("FLASH result is NOK (%"PRIu32" misses).\r\n", misses);
    }

    return testResult;
}

bool test_sdram(sdram_driver_t *sdram){
    bool testResult = true;

    if(sdram == NULL){
        DEBUG_ERROR("NULL argument to test_sdram!");
        return false;
    }

    uint32_t memory_size = sdram_driver_get_size(sdram);

    printf("Testing SDRAM memory device '%s' of size %"PRIu32" bytes.\r\n", sdram->sdram_name, memory_size);

    test_data_generator_t *generator = NULL;

    if(!generator_init(&generator, SDRAM_TEST_SEED)){
        DEBUG_ERROR("Generator initialization failed!");
        return false;
    }

    progress_bar_t *write_progress_bar = NULL;
    progress_bar_init(&write_progress_bar, "writing", memory_size, stdout, PROGRESS_BAR_BAR);

    for(uint32_t i = 0; i < memory_size; i++){
        uint8_t generated_data = generator_gen(generator, i);

        if(i % SDRAM_PROGRESS_BAR_UPDATE_INTERVAL == 0){
            progress_bar_update(write_progress_bar, i);
        }

        sdram_driver_write_byte(sdram, i, generated_data);
    }

    progress_bar_clean(write_progress_bar);
    progress_bar_destroy(write_progress_bar);

    generator_reset(generator);

    uint32_t misses = 0;

    progress_bar_t *reading_progress_bar = NULL;
    progress_bar_init(&reading_progress_bar, "reading", memory_size, stdout, PROGRESS_BAR_BAR);

    for(uint32_t i = 0; i < memory_size; i++){
        uint8_t generated_data = generator_gen(generator, i);
        uint8_t sdram_data = sdram_driver_read_byte(sdram, i);

        if(i % SDRAM_PROGRESS_BAR_UPDATE_INTERVAL == 0){
            progress_bar_update(reading_progress_bar, i);
        }

        if(generated_data != sdram_data){
            progress_bar_clean(reading_progress_bar);
            printf("data in sdram mismatch; sdram[0x%"PRIX32"] = 0x%"PRIX8" (should be: 0x%"PRIX8")\r\n", i, sdram_data, generated_data);
            misses++;
            testResult = false;
        }
    }

    progress_bar_clean(reading_progress_bar);
    progress_bar_destroy(reading_progress_bar);
    generator_destroy(generator);

    if(testResult == true){
        printf("SDRAM result is OK.\r\n");
    }
    else{
        printf("SDRAM result is NOK (%"PRIu32" misses).\r\n", misses);
    }

    return testResult;
}
