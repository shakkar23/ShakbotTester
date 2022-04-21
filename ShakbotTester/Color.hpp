#pragma once
#include <stdint.h>

enum ColorType : uint_fast8_t
{

    // Color for pieces
    S,
    Z,
    J,
    L,
    T,
    O,
    I,
    // special types
    empty,
    line_clear,
    garbage,
    number_of_ColorTypes
};

struct Colors
{
    ColorType a : 4;
    ColorType b : 4;
};