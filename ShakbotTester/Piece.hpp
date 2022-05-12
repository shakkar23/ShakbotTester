#pragma once
#include "Color.hpp"
#include "Coord.hpp"
#include "BoardAttr.hpp"
#include <tuple>
#include <array>
#include <stdint.h>

constexpr inline auto MINOSINAPIECE = 4;

enum spin : uint_fast8_t
{
    None,
    Mini,
    Full,
	numberOfSpins
};

enum class PieceType : uint_fast8_t
{
    // actual pieces
    S = ColorType::S,
    Z = ColorType::Z,
    J = ColorType::J,
    L = ColorType::L,
    T = ColorType::T,
    O = ColorType::O,
    I = ColorType::I,
    empty = ColorType::empty,
    number_of_PieceTypes
};


enum RotationDirection : uint_fast8_t
{
    North,
    East,
    South,
    West,
    number_of_RotationDirections
};
enum TurnDirection : uint_fast8_t
{
    Left,
    Right,
    number_of_TurnDirections,
    oneEighty,
};
enum class MoveDirection : uint_fast8_t
{
    Left,
    Right,
    number_of_MoveDirections,
};

constexpr inline std::array<std::array<Coord, MINOSINAPIECE>, (int)PieceType::number_of_PieceTypes> PieceDefintions= {

    {
        {{{-1, 0}, {0, 0}, {0, 1}, {1, 1}}},  // S
        {{{-1, 1}, {0, 1}, {0, 0}, {1, 0}}},  // Z
        {{{-1, 0}, {0, 0}, {1, 0}, {-1, 1}}}, // J
        {{{-1, 0}, {0, 0}, {1, 0}, {1, 1}}},  // L
        {{{-1, 0}, {0, 0}, {1, 0}, {0, 1}}},  // T
        {{{0, 0}, {1, 0}, {0, 1}, {1, 1}}},   // O
        {{{-1, 0}, {0, 0}, {1, 0}, {2, 0}}},  // I
        {{{0, 0}, {0, 0}, {0, 0}, {0, 0}}}    // NULL
    } };

constexpr inline ColorType PieceTypeToColorType(PieceType color) noexcept

{
    switch (color)
    {
    case PieceType::S:
        return S;
    case PieceType::Z:
        return Z;
    case PieceType::J:
        return J;
    case PieceType::L:
        return L;
    case PieceType::T:
        return T;
    case PieceType::O:
        return O;
    case PieceType::I:
        return I;
    case PieceType::empty:
    case PieceType::number_of_PieceTypes:
    default:
        return empty;
    }
    return empty;
}


class Piece
{
public:
    constexpr bool operator ==(const Piece& other) const noexcept {
        /* Can't use default because then piecedef would be included. */
        return std::tie(this->x, this->y, this->kind, this->spin) == std::tie(other.x, other.y, other.kind, other.spin);
    }
    constexpr inline Piece(PieceType kind, int_fast8_t x = 4, int_fast8_t y = VISUALBOARDHEIGHT - 2,
        RotationDirection spin = RotationDirection::North)noexcept
    {
        // force the RNG to throw away its previous bag, and make a new one

        this->kind = kind;
        setX(x);
        setY(y);
        this->spin = spin;

        // populate local piece definition
        for (size_t i = 0; i < 4; i++)
        {
            piecedef[i] = PieceDefintions[PieceTypeToColorType(kind)][i];
        }
        // match up the spin direction of the local piece definition with what is given by the params
        switch (spin)
        {
        case North:
            break;
        case East:
            rotateCW();
            break;
        case South:
            rotate180();
            break;
        case West:
            rotateCCW();
            break;
        default:
            break;
        }
    }

    constexpr inline Piece() = delete;


    // rotates the piece 90 degrees counter clock wise
    // Thanks MinusKelvin for the idea of using coordinates and not a definition
    constexpr inline void rotateCW()noexcept
    {
        for (auto& coords : piecedef)
        {
            std::swap(coords.y, coords.x);
            coords.y = -coords.y;
        }
    }

    constexpr inline void rotateCCW()noexcept
    {
        for (auto& coords : piecedef)
        {
            std::swap(coords.y, coords.x);
            coords.x = -coords.x;
        }
    }


    constexpr inline void rotate180()noexcept
    {
        for (auto& coords : piecedef)
        {
            //std::swap(coords.y, coords.x);
            coords.x = -coords.x;
            coords.y = -coords.y;
        }
    }
    constexpr inline void setX(int_fast8_t setter)noexcept
    {
        x = setter;
    }
    constexpr inline void setY(int_fast8_t setter)noexcept
    {
        y = setter;
    }
    std::array<Coord, MINOSINAPIECE> piecedef{};
    PieceType kind{};
    RotationDirection spin{};
    int_fast8_t x{};
    int_fast8_t y{};
};