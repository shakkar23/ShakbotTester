#pragma once

#include <bit>
#include <bits.hpp>
#include <cassert>
#include <numeric>

#include "Board.hpp"

enum minotype {
    isEmpty = 0,
    isFull = 1
};

class BitBoard {
   public:
    // set the column to be the type of integer that fits the board height

    using column = uint32_t;

    constexpr inline void addGarbage(uint_fast8_t holePlacement, uint_fast8_t linesSent) {
        // bring up entire board by linesSent line(s)
        for (int i = 0; i < BOARDWIDTH; ++i) {
            m_cols[i] <<= linesSent;
            m_cols[i] |= (1 << linesSent) - 1;
        }

        // add garbage to the bottom line
        m_cols[holePlacement] &= ~((1 << linesSent) - 1);
    }

    // compare two bitboards from the bottom row to the top
    constexpr bool operator==(const BitBoard& rhs) const {
        bool ret = true;
        for (uint8_t i = 0; i < BOARDWIDTH; ++i) {
            if (m_cols[i] != rhs.m_cols[i]) {
                ret = false;
            }
        }
        return ret;
    }

    constexpr inline minotype getBit(int x, int y) const {
        return minotype((m_cols[x] >> y) & 1);
    }

    constexpr inline void setBit(int row, int col, minotype val) {
        if (val == isFull) {
            m_cols[row] |= 1 << col;
        } else {
            m_cols[row] &= ~(1 << col);
        }
    }

    // conversion from BitBoard to Board
    // the board loses all color, and is now all garbage
    constexpr inline Board toBoard() const {
        Board board;
        for (int i = 0; i < LOGICALBOARDHEIGHT; i++) {
            for (int j = 0; j < BOARDWIDTH; j++) {
                if (getBit(i, j) == isFull) {
                    board.board[i][j] = ColorType::garbage;
                }
            }
        }
        return board;
    }

    // conversion from Board to BitBoard
    // the board loses all color
    constexpr inline static BitBoard fromBoard(const Board& board) {
        BitBoard bitBoard;
        for (int i = 0; i < LOGICALBOARDHEIGHT; i++) {
            for (int j = 0; j < BOARDWIDTH; j++) {
                if (board.board[j][i] != ColorType::empty) {
                    bitBoard.setBit(j, i, isFull);
                }
            }
        }
        return bitBoard;
    }

    constexpr inline bool isBoardEmpty() noexcept {
        bool ret = true;
        for (uint8_t i = 0; i < LOGICALBOARDHEIGHT; ++i) {
            if (m_cols[i] != 0) {
                ret = false;
            }
        }
        return ret;
    }
    constexpr inline void reset() noexcept {
        for (auto& col : m_cols) {
            col = 0;
        }
    }
    constexpr inline int clearLines() noexcept {
        column mask = std::numeric_limits<column>::max();
        for (const auto& col : m_cols) {
            mask &= col;
        }

        mask = ~mask;

        for (auto& col : m_cols) {
            col = pext(col, mask);
        }

        return std::popcount(mask);
    };
    constexpr inline int highest_point() const noexcept {
        column mask = 0;
        for (const auto& col : m_cols) {
            mask |= col;
        }
        return LOGICALBOARDHEIGHT - std::countl_zero(mask);
    }
    constexpr inline void sonicDrop(Piece& piece) const noexcept {
        while (trySoftDrop(piece))
            ;
    }
    constexpr inline bool trySoftDrop(Piece& piece) const noexcept {
        piece.setY(piece.y - 1);

        if (isCollide(piece)) {
            piece.setY(piece.y + 1);  // if it collided, go back up where it should be safe
            return false;
        }
        return true;
    }
    constexpr inline void setPiece(const Piece& piece) noexcept {
        if (piece.kind == PieceType::empty)
            return;
        for (auto& coord : piece.piecedef) {
            if ((
                    ((0 <= (coord.y + piece.y)) && ((coord.y + piece.y) < LOGICALBOARDHEIGHT))) &&
                (((0 <= (coord.x + piece.x)) && ((coord.x + piece.x) < BOARDWIDTH))))
            // if inbounds of board
            {
                setBit((coord.x + piece.x), ((coord.y + piece.y)), isFull);
            }
        }
    }
    constexpr inline bool isCollide(const Piece piece) const noexcept {
        for (const auto& coord : piece.piecedef) {
            auto x = coord.x + piece.x;
            auto y = coord.y + piece.y;

            if (x < 0 || x >= BOARDWIDTH || y < 0 || y >= LOGICALBOARDHEIGHT || getBit(x, y))
                return true;
        }
        return false;

        /*
        bool ret[4] = {};
        for (int i = 0; i < piece.piecedef.size(); i++) {
            auto x = piece.piecedef[i].x + piece.x;
            auto y = piece.piecedef[i].y + piece.y;
            ret[i] = (bool(x < 0) | bool(x >= BOARDWIDTH) | bool(y < 0) | bool(y >= LOGICALBOARDHEIGHT) || getBit(x, y));

        }
        return ret[0] + ret[1] + ret[2] + ret[3]*/
        ;
    }
    constexpr inline column getColumn(int x) const noexcept {
        return m_cols[x];
    }
    constexpr inline bool tryRotate(Piece& piece, TurnDirection direction, Spin& Tspinned) const noexcept {
        constexpr auto incrRotClockWise = [&](RotationDirection& spin) noexcept {
            switch (spin) {
                case North:
                    spin = East;
                    break;
                case East:
                    spin = South;
                    break;
                case South:
                    spin = West;
                    break;
                case West:
                    spin = North;
                    break;
                default:
                    break;
            }
        };
        constexpr auto TspinDetection = [](const Piece& piece, const BitBoard* board, Spin& Tspinned, RotationDirection dir, int kick) noexcept {
            constexpr auto isntEmpty = [](int x, int y, const BitBoard* board) noexcept {
                if ((((0 <= y) && (y < LOGICALBOARDHEIGHT))) && (((0 <= x) && (x < BOARDWIDTH))))  // if inbounds of board
                {
                    // if (piece.piecedef[y][x] != empty) // is the cell empty in the piece matrix is empty

                    return (board->getBit(x, y) == isFull);  // is the cell in the board matrix empty
                }

                if ((x < 0) || (x >= BOARDWIDTH))  // cant be out of bounds on either direction
                {
                    return true;
                }

                if (y < 0)  // can be above, but not below the board
                {
                    return true;
                }
                return false;
            };

            [[unlikely]] if (piece.kind == PieceType::T) {
                constexpr std::array<std::array<Coord, 4>, 4> directionToCords = {{
                    // a       b       c        d
                    {{{-1, 1}, {1, 1}, {1, -1}, {-1, -1}}},  // North
                    {{{1, 1}, {1, -1}, {-1, -1}, {-1, 1}}},  // East
                    {{{1, -1}, {-1, -1}, {-1, 1}, {1, 1}}},  // South
                    {{{-1, -1}, {-1, 1}, {1, 1}, {1, -1}}},  // West
                }};

                bool a = false, b = false, c = false, d = false;
                switch (dir) {
                    case North:
                        a = isntEmpty(directionToCords[0][0].x + piece.x, directionToCords[0][0].y + piece.y, board);
                        b = isntEmpty(directionToCords[0][1].x + piece.x, directionToCords[0][1].y + piece.y, board);
                        c = isntEmpty(directionToCords[0][2].x + piece.x, directionToCords[0][2].y + piece.y, board);
                        d = isntEmpty(directionToCords[0][3].x + piece.x, directionToCords[0][3].y + piece.y, board);
                        break;
                    case East:
                        a = isntEmpty(directionToCords[1][0].x + piece.x, directionToCords[1][0].y + piece.y, board);
                        b = isntEmpty(directionToCords[1][1].x + piece.x, directionToCords[1][1].y + piece.y, board);
                        c = isntEmpty(directionToCords[1][2].x + piece.x, directionToCords[1][2].y + piece.y, board);
                        d = isntEmpty(directionToCords[1][3].x + piece.x, directionToCords[1][3].y + piece.y, board);
                        break;
                    case South:
                        a = isntEmpty(directionToCords[2][0].x + piece.x, directionToCords[2][0].y + piece.y, board);
                        b = isntEmpty(directionToCords[2][1].x + piece.x, directionToCords[2][1].y + piece.y, board);
                        c = isntEmpty(directionToCords[2][2].x + piece.x, directionToCords[2][2].y + piece.y, board);
                        d = isntEmpty(directionToCords[2][3].x + piece.x, directionToCords[2][3].y + piece.y, board);
                        break;
                    case West:
                        a = isntEmpty(directionToCords[3][0].x + piece.x, directionToCords[3][0].y + piece.y, board);
                        b = isntEmpty(directionToCords[3][1].x + piece.x, directionToCords[3][1].y + piece.y, board);
                        c = isntEmpty(directionToCords[3][2].x + piece.x, directionToCords[3][2].y + piece.y, board);
                        d = isntEmpty(directionToCords[3][3].x + piece.x, directionToCords[3][3].y + piece.y, board);
                        break;
                    default:
                        break;
                }
                if (a && b && (c || d))
                    Tspinned = Spin::Full;
                else if ((a || b) && c && d) {
                    if (kick >= (kicks - 1)) {
                        Tspinned = Spin::Full;
                    } else
                        Tspinned = Spin::Mini;
                } else
                    Tspinned = Spin::None;
            }
        };
        // temporary x and y to know their initial location
        const int_fast8_t x = piece.x;
        const int_fast8_t y = piece.y;
        if (direction == Right) {
            piece.rotateCW();
        } else if (direction == Left) {
            piece.rotateCCW();
        } else if (direction == oneEighty) {
            piece.rotate180();
        }

        // spinclockwise should be a bool, but it can also be 2 as in rotating twice
        // in one frame aka 180 spin
        if (direction != TurnDirection::oneEighty) {
            RotationDirection nextDir = piece.spin;
            if (direction == Right)
                incrRotClockWise(nextDir);
            else if (direction == Left) {
                incrRotClockWise(nextDir);
                incrRotClockWise(nextDir);
                incrRotClockWise(nextDir);
            }

            auto* offsetData = &JLSTZPieceOffsetData[piece.spin];
            auto* nextOffset = &JLSTZPieceOffsetData[nextDir];
            [[unlikely]] if (piece.kind == PieceType::I) {
                offsetData = &IPieceOffsetData[piece.spin];
                nextOffset = &IPieceOffsetData[nextDir];
            } else [[unlikely]] if (piece.kind == PieceType::O) {
                offsetData = &OPieceOffsetData[piece.spin];
                nextOffset = &OPieceOffsetData[nextDir];
            }

            for (int i = 0; i < kicks; ++i) {
                piece.setX(x + (*offsetData)[i].x - (*nextOffset)[i].x);
                piece.setY(y + (*offsetData)[i].y - (*nextOffset)[i].y);

                if (!isCollide(piece)) {
                    piece.spin = nextDir;
                    TspinDetection(piece, this, Tspinned, nextDir, i);
                    return true;
                }
            }
            piece.setX(x);
            piece.setY(y);
        } else {
            RotationDirection nextDir = piece.spin;
            incrRotClockWise(nextDir);
            incrRotClockWise(nextDir);

            auto* kickdata = &wallkick180data[piece.spin];
            bool isI = false;
            if (piece.kind == PieceType::I) {
                isI = true;
                kickdata = &Iwallkick180data[piece.spin];
            }

            for (int i = 0; i < 12; ++i) {
                piece.setX(x + ((*kickdata)[i].x + (isI * IPrecalculatedwallkick180offsets[piece.spin].x)));
                piece.setY(y - ((*kickdata)[i].y + (isI * IPrecalculatedwallkick180offsets[piece.spin].y)));

                if (!this->isCollide(piece)) {
                    piece.spin = nextDir;
                    TspinDetection(piece, this, Tspinned, nextDir, i);
                    return true;
                }
            }
            piece.setX(x);
            piece.setY(y);
        }

        // rotate the matrix back if nothing worked
        if (direction == Right) {
            piece.rotateCCW();
        } else if (direction == Left) {
            piece.rotateCW();
        } else [[unlikely]] if (direction == oneEighty) {
            piece.rotate180();
        }
        sizeof(BitBoard);
        return false;
    }

    std::array<column, BOARDWIDTH> m_cols{};
};