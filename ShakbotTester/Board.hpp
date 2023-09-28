#pragma once

#include "BoardAttr.hpp"
#include "Piece.hpp"
#include "Rotation.hpp"
#include "Color.hpp"
#include <array>

class Piece;
class Board
{
public:
    constexpr Board() noexcept
    {
        clear();
    }
    constexpr inline void makeBoardGarbage() noexcept
    {
        for (auto& lines : board)
            for (auto& mino : lines)
            {
                if (mino != ColorType::empty)
                    mino = ColorType::garbage;
            }
    }
	
	// clear the entire board back to empty
    constexpr inline void clear() noexcept
    {
        for (auto& width : board)
        {
            for (auto& cell : width)
            {
                cell = empty;
            }
        }
    }
    constexpr inline int clearLines() noexcept
    {
        uint_fast16_t linesCleared{};
        for (uint_fast16_t h = 0; h < LOGICALBOARDHEIGHT; h++)
        {
            for (uint_fast16_t w = 0; w < BOARDWIDTH; w++)
            {

                if (board.at(w).at(h) == empty)
                    break;
                if (w == BOARDWIDTH - 1)
                {
                    clearLine(h);
                    linesCleared++;
                    h--;
                }
            }
        }
        return linesCleared;
    };
    constexpr inline void clearLine(uint_fast8_t whichLine) noexcept
    {

        if (whichLine >= LOGICALBOARDHEIGHT)
            whichLine = LOGICALBOARDHEIGHT - 1;
        // clear the line in question
        for (size_t i = 0; i < BOARDWIDTH; i++)
        {
            board.at(i).at(whichLine) = empty;
        }

        // pull down the rest of the lines above the cleared line down
        for (size_t w = 0; w < BOARDWIDTH; w++)
        {
            for (size_t h = whichLine; h < LOGICALBOARDHEIGHT; h++)
            {
                if (h == (LOGICALBOARDHEIGHT - 1))
                {

                    board.at(w).at(h) = empty;
                    break;
                }
                else
                {
                    board.at(w).at(h) = board.at(w).at((h + 1));
                }
            }
        }
    }
    constexpr inline void addGarbage(uint_fast8_t holePlacement, uint_fast8_t linesSent) {
        // bring up entire board by linesSent line(s)
        for (int i = 0; i < linesSent; i++)
            for (auto &column : board) {
                for(long long i = board.at(0).size() - 2; i >= 0; --i){
		    		column[i+1] = column[i];
		    	}
            }
		
		// add garbage to the bottom line
        for (int i = 0; i < linesSent; i++)
        {
			for (int o = 0; o < board.size(); o++){
                auto& column = board.at(o);
				if(o != holePlacement)
			        column[i] = ColorType::garbage;
                else
				    column[i] = ColorType::empty;
            }
        }
    }
    constexpr inline void sonicDrop(Piece& piece) const noexcept
    {
        while (trySoftDrop(piece))
            ;
    }
    constexpr inline bool trySoftDrop(Piece& piece) const noexcept {
        piece.setY(piece.y - 1);
        if (isCollide(piece))
        {
            piece.setY(piece.y + 1); // if it collided, go back up where it should be safe
            return false;
        }
        return true;
    }
    constexpr inline void setPiece(const Piece& piece) noexcept
    {
        for (auto& coord : piece.piecedef)
        {
            if ((((0 <= (coord.y + piece.y)) && ((coord.y + piece.y) < LOGICALBOARDHEIGHT))) && (((0 <= (coord.x + piece.x)) && ((coord.x + piece.x) < BOARDWIDTH)))) // if inbounds of board
            {

                board.at(coord.x + piece.x).at((coord.y + piece.y)) = PieceTypeToColorType(piece.kind);
            }
        }
    }
    constexpr inline bool isCollide(const Piece& piece) const noexcept
    {

        for (const auto& coord : piece.piecedef)
        {

            if ((((0 <= (coord.y + piece.y)) && ((coord.y + piece.y) < LOGICALBOARDHEIGHT))) && (((0 <= (coord.x + piece.x)) && ((coord.x + piece.x) < BOARDWIDTH)))) // if inbounds of board
            {
                if (board[coord.x + piece.x][coord.y + piece.y] != empty) // is the cell in the board matrix empty
                    return true;
            }

            if (((coord.x + piece.x) < 0) || ((coord.x + piece.x) >= BOARDWIDTH)) // cant be out of bounds on either direction
            {
                return true;
            }

            if ((coord.y + piece.y) < 0) // can be above, but not below the board
            {
                return true;
            }
        }
        return false;
    }
    constexpr inline bool tryRotate(Piece& piece, TurnDirection direction, spin& Tspinned) const noexcept
    {
        constexpr auto incrRotClockWise = [&](RotationDirection& spin) noexcept
        {
            switch (spin)
            {
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
        constexpr auto TspinDetection = [](const Piece& piece, const Board& board, spin& Tspinned, RotationDirection dir, int kick)noexcept
        {
            constexpr auto isntEmpty = [](int x, int y, const Board& board)noexcept
            {
                if ((((0 <= y) && (y < LOGICALBOARDHEIGHT))) && (((0 <= x) && (x < BOARDWIDTH)))) // if inbounds of board
                {
                    // if (piece.piecedef[y][x] != empty) // is the cell empty in the piece matrix is empty
                    
                    return (board.board.at(x).at(y) != empty); // is the cell in the board matrix empty
                }

                if ((x < 0) || (x >= BOARDWIDTH)) // cant be out of bounds on either direction
                {
                    return true;
                }

                if (y < 0) // can be above, but not below the board
                {
                    return true;
                }
                return false;
            };
            if (piece.kind == PieceType::T)
            {
                constexpr std::array < std::array<Coord, 4>, 4 > directionToCords = { {
                        //a       b       c        d
                    { { {-1, 1}, { 1, 1}, { 1,-1}, {-1,-1}} }, // North
                    { { { 1, 1}, { 1,-1}, {-1,-1}, {-1, 1}} }, // East
                    { { { 1,-1}, {-1,-1}, {-1, 1}, { 1, 1}} }, // South
                    { { {-1,-1}, {-1, 1}, { 1, 1}, { 1,-1} } }, // West
                    } };

                bool a = false, b = false, c = false, d = false;
                switch (dir)
                {
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
                    Tspinned = spin::Full;
                else if ((a || b) && c && d)
                {   
                    if (kick >= (kicks - 1)) {
                        Tspinned = spin::Full;
                    } else
                        Tspinned = spin::Mini;
                }
                else 
                    Tspinned = spin::None;
            }
        };
        // temporary x and y to know their initial location
        const int_fast8_t x = piece.x;
        const int_fast8_t y = piece.y;
        if (direction == Right)
        {
            piece.rotateCW();
        }
        else if (direction == Left)
        {
            piece.rotateCCW();
        }
        else if (direction == oneEighty)
        {
            piece.rotate180();
        }

        // spinclockwise should be a bool, but it can also be 2 as in rotating twice
        // in one frame aka 180 spin
        if (direction != TurnDirection::oneEighty)
        {
            RotationDirection nextDir = piece.spin;
            if (direction == Right)
                incrRotClockWise(nextDir);
            else if (direction == Left)
            {
                incrRotClockWise(nextDir);
                incrRotClockWise(nextDir);
                incrRotClockWise(nextDir);
            }

            auto* offsetData = &JLSTZPieceOffsetData[piece.spin];
            auto* nextOffset = &JLSTZPieceOffsetData[nextDir];
            if (piece.kind == PieceType::I)
            {
                offsetData = &IPieceOffsetData[piece.spin];
                nextOffset = &IPieceOffsetData[nextDir];
            }
            else if (piece.kind == PieceType::O)
            {
                offsetData = &OPieceOffsetData[piece.spin];
                nextOffset = &OPieceOffsetData[nextDir];
            }

            for (int i = 0; i < kicks; ++i)
            {

                piece.setX(x + (*offsetData)[i].x - (*nextOffset)[i].x);
                piece.setY(y + (*offsetData)[i].y - (*nextOffset)[i].y);

                if (!this->isCollide(piece))
                {
                    piece.spin = nextDir;
                    TspinDetection(piece, (*this), Tspinned, nextDir, i);
                    return true;
                }
            }
            piece.setX(x);
            piece.setY(y);
        }
        else
        {
            RotationDirection nextDir = piece.spin;
            incrRotClockWise(nextDir);
            incrRotClockWise(nextDir);

            auto* kickdata = &wallkick180data[piece.spin];
            bool isI = false;
            if (piece.kind == PieceType::I)
            {
                isI = true;
                kickdata = &Iwallkick180data[piece.spin];
            }

            for (int i = 0; i < 12; ++i)
            {

                piece.setX(x + ((*kickdata)[i].x + (isI * IPrecalculatedwallkick180offsets[piece.spin].x)));
                piece.setY(y - ((*kickdata)[i].y + (isI * IPrecalculatedwallkick180offsets[piece.spin].y)));

                if (!this->isCollide(piece))
                {
                    piece.spin = nextDir;
                    TspinDetection(piece, (*this), Tspinned, nextDir, i);
                    return true;
                }
            }
            piece.setX(x);
            piece.setY(y);
        }

        // rotate the matrix back if nothing worked
        if (direction == Right)
        {
            piece.rotateCCW();
        }
        else if (direction == Left)
        {
            piece.rotateCW();
        }
        else if (direction == oneEighty)
        {
            piece.rotate180();
        }
        return false;
    }

    constexpr inline char colorTypeToString(const ColorType color) noexcept
    {
        switch (color)
        {
        case S:
            return 'S';
            break;
        case Z:
            return 'Z';
            break;
        case J:
            return 'J';
            break;
        case L:
            return 'L';
            break;
        case T:
            return 'T';
            break;
        case O:
            return 'O';
            break;
        case I:
            return 'I';
            break;
        case empty:
        case line_clear:
        case garbage:
        case number_of_ColorTypes:
        default:
            return '#';
        }
    }
    constexpr inline bool rowIsEmpty(const int row) const {
        for (const auto& column : board)
        {
            if (column[row] != empty)
                return false;
        }
        return true;

    }
    constexpr inline bool rowIsEmpty(const int row) const {
        for(const auto & column: board)
		{
			if(column[row] != empty)
				return false;
		}
		return true;
			
    }
    std::array<std::array<ColorType, LOGICALBOARDHEIGHT>, BOARDWIDTH> board{};
};
