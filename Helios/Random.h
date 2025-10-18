#ifndef RANDOM_H
#define RANDOM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

// Forward declaration (with include guard to prevent SDCC conflicts)
#ifndef RANDOM_T_FORWARD_DECLARED
#define RANDOM_T_FORWARD_DECLARED
typedef struct random_t random_t;
#endif

struct random_t
{
  uint32_t m_seed;
};

// Initialize a random struct with default seed
void random_init(random_t *rng);

// Initialize a random struct with a specific seed
void random_init_seed(random_t *rng, uint32_t newseed);

// Set the seed for the random number generator
void random_seed(random_t *rng, uint32_t newseed);

// Generate next random 8-bit value within range [minValue, maxValue]
uint8_t random_next8(random_t *rng, uint8_t minValue, uint8_t maxValue);

// Generate next random 16-bit value within range [minValue, maxValue]
uint16_t random_next16(random_t *rng, uint16_t minValue, uint16_t maxValue);

#ifdef __cplusplus
}
#endif

#endif
