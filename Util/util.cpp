#include <cstdint>

#include "bits.hpp"

static_assert(pext<uint32_t>(0b1111, 0b0101) == 0b0011);