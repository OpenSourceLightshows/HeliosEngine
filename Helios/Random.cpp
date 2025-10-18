#include "Random.h"

void random_init(random_t *rng)
{
  rng->m_seed = 0;
}

void random_init_seed(random_t *rng, uint32_t newseed)
{
  random_init(rng);
  random_seed(rng, newseed);
}

void random_seed(random_t *rng, uint32_t newseed)
{
  if (!newseed) {
    rng->m_seed = 42;
  } else {
    rng->m_seed = newseed;
  }
}

uint16_t random_next16(random_t *rng, uint16_t minValue, uint16_t maxValue)
{
  // walk the LCG forward to the next step
  rng->m_seed = (rng->m_seed * 1103515245 + 12345) & 0x7FFFFFFF;
  uint32_t range = maxValue - minValue;
  if (range != 0xFFFFFFFF) {
    /* shift the seed 16 bits to the right because the lower 16 bits
     * of this LCG are apparently not uniform whatsoever, where as the
     * upper 16 bits appear to be quite uniform as per tests. We don't
     * really need 32bit random values so we offer max 16bits of entropy */
    return ((rng->m_seed >> 16) % (range + 1)) + minValue;
  }
  return (rng->m_seed >> 16);
}

uint8_t random_next8(random_t *rng, uint8_t minValue, uint8_t maxValue)
{
  uint32_t result = random_next16(rng, minValue, maxValue);
  return (uint8_t)result;
}

