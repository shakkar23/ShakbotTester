#pragma once
#include "Piece.hpp"
#include "Coord.hpp"
#include <array>

// number of kicks srs has, including for initial
inline constexpr auto kicks = 5;

// number of kicks misamino 180 has not counting for initial
inline constexpr auto kicks180 = 12;

// left and right, or clockwise and counter clockwise
// these are the only directions you can turn towards not including 180 due to it being a special case
inline constexpr auto turnDirections = 2;

// the number of kicks asc uses
inline constexpr auto ascKicks = 20;


inline constexpr int asc[ascKicks][2] = {
    {-1, 00}, {00, -1}, {-1, -1}, {00, -2}, {-1, -2}, {-2, 00}, {-2, -1}, {-2, -2}, {01, 00}, {01, -1}, {00, 01}, {-1, 01}, {-2, 01}, {01, -2}, {02, 00}, {00, 02}, {-1, 02}, {-2, 02}, {02, -1}, {02, -2} };

inline constexpr std::array<std::array<Coord, kicks180>, number_of_RotationDirections> Iwallkick180data = { {
    {{{0, 0}, {0, -1}, {0, -2}, {0, 1}, {0, 2}, {1, 0}}},
    {{{0, 0}, {1, 0}, {2, 0}, {-1, 0}, {-2, 0}, {0, -1}}},
    {{{0, 0}, {0, 1}, {0, 2}, {0, -1}, {0, -2}, {-1, 0}}},
    {{{0, 0}, {1, 0}, {2, 0}, {-1, 0}, {-2, 0}, {0, 1}}},

} };
inline constexpr std::array<Coord, number_of_RotationDirections> IPrecalculatedwallkick180offsets = {
    {
        {1, -1},
        {-1, 1},
        {-1, 1},
        {1, -1},
    } };
inline constexpr std::array<std::array<Coord, kicks180>, number_of_RotationDirections> wallkick180data = { {{{// North -> South
                                                                                                  {0, 0},
                                                                                                  {1, 0},
                                                                                                  {2, 0},
                                                                                                  {1, 1},
                                                                                                  {2, 1},
                                                                                                  {-1, 0},
                                                                                                  {-2, 0},
                                                                                                  {-1, 1},
                                                                                                  {-2, 1},
                                                                                                  {0, -1},
                                                                                                  {3, 0},
                                                                                                  {-3, 0}}},
                                                                                                {{// East -> West
                                                                                                  {0, 0},
                                                                                                  {0, 1},
                                                                                                  {0, 2},
                                                                                                  {-1, 1},
                                                                                                  {-1, 2},
                                                                                                  {0, -1},
                                                                                                  {0, -2},
                                                                                                  {-1, -1},
                                                                                                  {-1, -2},
                                                                                                  {1, 0},
                                                                                                  {0, 3},
                                                                                                  {0, -3}}},
                                                                                                {{// South -> North
                                                                                                  {0, 0},
                                                                                                  {-1, 0},
                                                                                                  {-2, 0},
                                                                                                  {-1, -1},
                                                                                                  {-2, -1},
                                                                                                  {1, 0},
                                                                                                  {2, 0},
                                                                                                  {1, -1},
                                                                                                  {2, -1},
                                                                                                  {0, 1},
                                                                                                  {-3, 0},
                                                                                                  {3, 0}}},
                                                                                                {{// West -> East
                                                                                                  {0, 0},
                                                                                                  {0, 1},
                                                                                                  {0, 2},
                                                                                                  {1, 1},
                                                                                                  {1, 2},
                                                                                                  {0, -1},
                                                                                                  {0, -2},
                                                                                                  {1, -1},
                                                                                                  {1, -2},
                                                                                                  {-1, 0},
                                                                                                  {0, 3},
                                                                                                  {0, -3}}}} };

inline constexpr std::array<std::array<Coord, kicks>, number_of_RotationDirections> JLSTZPieceOffsetData = { {
    {{{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}}},
    {{{0, 0}, {1, 0}, {1, -1}, {0, 2}, {1, 2}}},
    {{{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}}},
    {{{0, 0}, {-1, 0}, {-1, -1}, {0, 2}, {-1, 2}}},
} };

inline constexpr std::array<std::array<Coord, kicks>, number_of_RotationDirections> IPieceOffsetData = { {

    {{{0, 0}, {-1, 0}, {2, 0}, {-1, 0}, {2, 0}}},
    {{{-1, 0}, {0, 0}, {0, 0}, {0, 1}, {0, -2}}},
    {{{-1, 1}, {1, 1}, {-2, 1}, {1, 0}, {-2, 0}}},
    {{{0, 1}, {0, 1}, {0, 1}, {0, -1}, {0, 2}}},

} };

inline constexpr std::array<std::array<Coord, kicks>, number_of_RotationDirections> OPieceOffsetData = { {
    {{{0, 0}}},
    {{{0, -1}}},
    {{{-1, -1}}},
    {{{-1, 0}}},
} };
inline constexpr std::array<Coord, number_of_RotationDirections> IPieceOffsetData180 = { {
    {0, 0},
    {0, -1},
    {-1, 0},
    {0, 0},
} };
