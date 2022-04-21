#pragma once

#include "Coord.hpp"
#include "Board.hpp"
#include "BoardAttr.hpp"
#include "Piece.hpp"
#include <vector>
#include <memory>
enum class inputs : uint_fast8_t
{
    CW = 0,
    CCW = 1,
    Left = 2,
    Right = 3,
    SonicDrop = 4,
    numberOfInputs = 5
};
// this class stores the time taken to get to this cell, and a vector of inputs that resulted getting to this cell
class Cell
{
public:
    constexpr Cell(uint8_t time, std::vector<inputs>& inputs) noexcept : time(time) {
        std::swap(inputs, this->inputs);
    }

    uint8_t time{};
    std::vector<inputs> inputs;
};

constexpr inline Piece playPiece(const Piece& piece, const std::vector<inputs> &history, const  Board& board, spin &tSpinned) noexcept {
	
	Piece tempPiece = piece;
	for (const auto& input : history) {
		// TODO allow inputs to be done simultaneously
        switch (input)
        {
        case inputs::CW:
            if (board.tryRotate(tempPiece, Right, tSpinned))
                
            break;
        case inputs::CCW:
            if (board.tryRotate(tempPiece, Left, tSpinned))
                
            break;
        case inputs::Left:
            tempPiece.setX(tempPiece.x - 1);
            if (board.isCollide(tempPiece))
            {
                tempPiece.setX(tempPiece.x + 1); // failed, go back
            }
            break;
        case inputs::Right:
            tempPiece.setX(tempPiece.x + 1);
            if (board.isCollide(tempPiece))
            {
                tempPiece.setX(tempPiece.x - 1); // failed, go back
            }
            break;
        case inputs::SonicDrop:
            board.sonicDrop(tempPiece);
        default:
            break;
        }

	}


	return tempPiece;
}


typedef std::array<std::array<std::array<std::array<std::unique_ptr<Cell>, LOGICALBOARDHEIGHT + 2>, BOARDWIDTH + 2>, number_of_RotationDirections>, int(spin::numberOfSpins)> boardtype;

// this class will store a board where each cell points to a piece and its movements to get to that cell
class movementBoard
{
public:
    uint16_t numberOfPopulatedCells = 0;
    inline void addCell(boardtype &board, std::unique_ptr<Cell> cell, Coord location, RotationDirection rot, spin spn = spin::None) noexcept {
        if (board.at(spn).at(rot).at(location.x).at(location.y) != nullptr) {
            numberOfPopulatedCells++;
        }
        board.at(spn).at(rot).at(location.x).at(location.y) = std::move(cell);
    }
	inline void clearBoard() {
        numberOfPopulatedCells = 0;
        pieces.clear();
	}
    // needed padding as the X coordinate of a placed piece can be negative 1
    // padding the board on all 4 sides by one
    std::vector<Piece> pieces{};
	const void DijkstraFaithful(const Board& board, const Piece& piece);
	const void find_moves(const Board& board, const Piece& piece) noexcept;
};
