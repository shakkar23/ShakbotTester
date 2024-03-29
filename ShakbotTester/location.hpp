#pragma once
#include "Piece.hpp"
#include "Coord.hpp"
#include <stdint.h>

// a type for the piece location on the board
class Location {
   public:
    PieceType type;
    RotationDirection rotation;
    Coord coord;
};