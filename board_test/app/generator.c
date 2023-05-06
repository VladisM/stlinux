#include "generator.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <debug.h>

bool generator_init(test_data_generator_t **generator, uint8_t seed){
    test_data_generator_t *tmp = (test_data_generator_t *)malloc(sizeof(test_data_generator_t));

    if(tmp == NULL){
        DEBUG_ERROR("Malloc failed!");
        return false;
    }

    memset((void *)tmp, 0, sizeof(test_data_generator_t));

    tmp->seed = seed;
    tmp->generator_last_output = 0;

    generator_reset(tmp);

    *generator = tmp;
    return true;
}

void generator_reset(test_data_generator_t *generator){
    generator->generator_last_output = 0;
}

uint8_t generator_gen(test_data_generator_t *generator, uint32_t address){
    uint8_t address_part = (uint8_t)address;
    return (address_part ^ generator->seed) + generator->generator_last_output;
}

void generator_destroy(test_data_generator_t *generator){
    free(generator);
}
