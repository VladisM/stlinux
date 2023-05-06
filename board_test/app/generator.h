#ifndef GENERATOR_H_included
#define GENERATOR_H_included

#include <stdint.h>
#include <stdbool.h>

typedef struct{
    uint8_t generator_last_output;
    uint8_t seed;
}test_data_generator_t;

bool generator_init(test_data_generator_t **generator, uint8_t seed);
void generator_reset(test_data_generator_t *generator);
uint8_t generator_gen(test_data_generator_t *generator, uint32_t address);
void generator_destroy(test_data_generator_t *generator);

#endif
