#pragma once

#include "Coord.hpp"
#include "Board.hpp"
#include "BitBoard.hpp"
#include "BoardAttr.hpp"
#include "Piece.hpp"
#include <vector>
#include <memory>
#include <unordered_map>
#include <array>
#include <queue>
enum class inputs : uint_fast8_t
{
	// normal inputs
    CW = 0,
    CCW = 1,
    Left = 2,
    Right = 3,
    SonicDrop = 4,
    numberOfInputs = 5,
    // special inputs that are not part of movegen
	Hold = 6,
	
};
// this class stores the time taken to get to this cell, and a vector of inputs that resulted getting to this cell

constexpr inline Piece playPiece(const Piece& piece, const std::vector<inputs>& history, const  Board& board, spin& tSpinned) noexcept {

    Piece tempPiece = piece;
    for (const auto &input : history) {
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
constexpr inline Piece playPiece(const Piece piece, const std::vector<inputs>& history, const BitBoard &board, spin& tSpinned) noexcept {

    Piece tempPiece = piece;
    for (const auto &input : history) {
        // TODO allow inputs to be done simultaneously
        switch (input)
        {
        case inputs::CW:
            board.tryRotate(tempPiece, Right, tSpinned);
            break;
        case inputs::CCW:
            board.tryRotate(tempPiece, Left, tSpinned);
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


class FullPiece {
public:
    constexpr FullPiece(std::vector<inputs>& inputs, Piece& piece, spin spin) noexcept : piece(piece), spin(spin) {
        std::swap(inputs, this->inputs);
    }
    constexpr FullPiece(std::vector<inputs> &&inputs,const Piece& piece,const spin &spin) noexcept : piece(piece), spin(spin), inputs(std::move(inputs)) {}
    std::vector<inputs> inputs;
    Piece piece;
    spin spin;
};

typedef std::array<std::array<std::array<std::array<FullPiece*, LOGICALBOARDHEIGHT + 2>, BOARDWIDTH + 2>, number_of_RotationDirections>, int(spin::numberOfSpins)> boardtype;

//#pragma pack(push, 1)
struct inputNode
{
public:
    constexpr inline inputNode() = default;
    constexpr inline inputNode(const std::vector<inputs> &inputs, uint8_t depth) : history(inputs), inputs({ nullptr ,nullptr ,nullptr ,nullptr ,nullptr }) {

    }
    constexpr inline ~inputNode()
    {
        for (auto& i : inputs)
        {
            if (i != nullptr)
                delete i;
        }
    }
    constexpr inline inputNode* getInputNode(const inputs input)const
    {
        return inputs.at((int)input);
    }
    constexpr inline void setInputNode(const inputs input, inputNode* node)
    {
        if (inputs.at((int)input) != nullptr)
            delete inputs.at((int)input);
        inputs.at((int)input) = node;
    }

    std::vector<inputs> history{};
private:
    std::array<inputNode*, int(inputs::numberOfInputs)> inputs{};
};

// this class will store a board where each cell points to a piece and its movements to get to that cell
class movementBoard
{
public:
    uint_fast16_t numberOfPopulatedCells = 0;
    constexpr inline void addCell(boardtype *&board, std::vector<inputs> &&hist, const int &depth,const Coord &location,const RotationDirection &rot,const spin &spn, const Piece &piece) noexcept {
        if ((*board)[(spn)][(rot)][(location.x+1)][(location.y+1)] != nullptr) {
            numberOfPopulatedCells++;
        }
        (*board)[(spn)][(rot)][(location.x)][(location.y)] = new FullPiece(std::move(hist), piece, spn);
    }
	constexpr inline void clearBoard() {
        numberOfPopulatedCells = 0;
        pieces.clear();
	}
    // needed padding as the X coordinate of a placed piece can be negative 1
    // padding the board on all 4 sides by one
    std::vector<FullPiece> pieces{};
	constexpr void DijkstraFaithful(const Board& board, const Piece& piece);
    private:
        
public:
    constexpr inline std::vector<FullPiece> find_moves(const Board& board, const Piece& piece)noexcept
    {

        int nPiecePlacements = 0;
        //clearBoard();
        // initial check to see if the peice is colliding with the board
        if (board.isCollide(piece))
        {
            return{};
        }

        boardtype* intermediateBoard;
        intermediateBoard = new boardtype();


        inputNode root;
        std::vector<inputNode*> nodes = { &root };
        nodes.reserve(128);
        std::vector<inputNode*> nextNodes;
        nextNodes.reserve(128);


        // limiting the depth of the search to 15 but can be increased later on if needed
        for (uint_fast16_t depth = 0;; depth++)
        {
            for (auto& node : nodes)
            {
                auto& realNode = *node;
                for (size_t i = 0; i < int(inputs::numberOfInputs); i++)
                {
                    spin tSpinned = spin::None;
                    //Piece tempPiece = piece;
                    const inputs& input = (inputs)i;
                    // TODO allow inputs to be done simultaneously
                    Piece tempPiece = playPiece(piece, realNode.history, board, tSpinned);
                    switch (input)
                    {
                    case inputs::CW:
                        if (!board.tryRotate(tempPiece, Right, tSpinned))
                            continue;
                        break;
                    case inputs::CCW:
                        if (!board.tryRotate(tempPiece, Left, tSpinned))
                            continue;
                        break;
                    case inputs::Left:
                        tempPiece.setX(tempPiece.x - 1);
                        if (board.isCollide(tempPiece))
                        {
                            tempPiece.setX(tempPiece.x + 1); // failed, go back
                            continue;
                        }
                        break;
                    case inputs::Right:
                        tempPiece.setX(tempPiece.x + 1);
                        if (board.isCollide(tempPiece))
                        {
                            tempPiece.setX(tempPiece.x - 1); // failed, go back
                            continue;
                        }
                        break;
                    case inputs::SonicDrop:
                    default:
                        board.sonicDrop(tempPiece);
                        break;
                    }

                    auto addcell = [](inputNode& realNode, std::vector<inputNode*>& nextNodes, const inputs& input,
                        const uint_fast16_t& depth, const Piece& tempPiece, movementBoard* self, const spin& spinned, boardtype* board) {

                            std::vector<inputs> hist;
                            hist.reserve(realNode.history.size() + 1);
                            // populate the history 
                            hist = realNode.history;
                            hist.push_back(input);

                            inputNode* currentNode = new inputNode(hist, depth);
                            self->addCell(board, std::move(hist), depth, { int_fast8_t(tempPiece.x + int_fast8_t(1)), int_fast8_t(tempPiece.y + int_fast8_t(1)) }, tempPiece.spin, spinned, tempPiece);

                            //add populated node to the next nodes to evaluate
                            nextNodes.push_back(currentNode);

                            // make sure to set the pointer to a node for later on, it will ensure the data gets deleted 
                            realNode.setInputNode(input, currentNode);
                    };


                    // if the piece is in a new position add to the movementBoard, if its floating add it to the cells we have visited
                    auto& cellFromLocation = (*intermediateBoard)[(tSpinned)][(tempPiece.spin)][(tempPiece.x + 1)][(tempPiece.y + 1)];
                    if (cellFromLocation == nullptr)
                    {
                        nPiecePlacements++;
                        addcell(realNode, nextNodes, input, depth, tempPiece, this, tSpinned, intermediateBoard);
                    }
                    else
                    {
                        //if the cell is already populated 
                        // and has sonic drop as its last input, discard it
                        if (cellFromLocation->inputs.back() == inputs::SonicDrop)
                            continue;

                        // if the cell is already populated
                        // and the the two histories are the same replace the cell with itself
                        if (realNode.history == cellFromLocation->inputs)
                        {
                            nPiecePlacements++;
                            addcell(realNode, nextNodes, input, depth, tempPiece, this, tSpinned, intermediateBoard);

                        }
                    }
                }
            }
            std::swap(nextNodes, nodes);
            nextNodes.clear();
            if (nodes.size() == 0)
                break;
        }
		std::vector<FullPiece> moves;
        moves.reserve(nPiecePlacements);
        for (size_t spn = 0; spn < intermediateBoard->size(); spn++)
        {
            auto& temspn = (*intermediateBoard)[spn];
            for (size_t rot = 0; rot < temspn.size(); rot++)
            {
                auto& tempspnrot = temspn[(rot)];
                for (size_t x = 0; x < tempspnrot.size(); x++)
                {
                    auto& tempspnrotx = tempspnrot[(x)];
                    for (size_t y = 0; y < tempspnrotx.size(); y++)
                    {
                        auto& tempspnrotxy = tempspnrotx[(y)];
                        if (tempspnrotx[(y)] != nullptr)
                        {
                            spin temp;
                            Piece p = playPiece(piece, tempspnrotxy->inputs, board, temp);
                            if (!board.trySoftDrop(p))
                                moves.push_back(FullPiece(tempspnrotxy->inputs, p, temp));
                            delete tempspnrotxy;
                        }
                    }
                }
            }
        }
        delete intermediateBoard;
        return moves;
    }

	
    constexpr inline std::vector<FullPiece> find_moves(const BitBoard& board, const Piece& piece)noexcept
    {
        int nPiecePlacements = 0;
        //clearBoard();
        // initial check to see if the peice is colliding with the board
        if (board.isCollide(piece))
        {
            return {};
        }

        boardtype* intermediateBoard;
        intermediateBoard = new boardtype();

        // here to destruct the entire tree at the end of the function
        inputNode root = {};

        std::vector<inputNode*> nodes = { &root };  nodes.reserve(128);
        std::vector<inputNode*> nextNodes;          nextNodes.reserve(128);

		
        // limiting the depth of the search to 15 but can be increased later on if needed
        for (uint_fast16_t depth = 0;; depth++)
        {
            for (auto& node : nodes)
            {
                auto& realNode = *node;
                for (size_t i = 0; i < int(inputs::numberOfInputs); i++)
                {
                    spin tSpinned = spin::None;
                    //Piece tempPiece = piece;
                    const inputs input = (inputs)i;
                    // TODO allow inputs to be done simultaneously
                    Piece tempPiece = playPiece(piece, realNode.history, board, tSpinned);
                    switch (input)
                    {
                    case inputs::CW:
                        if (!board.tryRotate(tempPiece, Right, tSpinned))
                            continue;
                        break;
                    case inputs::CCW:
                        if (!board.tryRotate(tempPiece, Left, tSpinned))
                            continue;
                        break;
                    case inputs::Left:
                        tempPiece.setX(tempPiece.x - 1);
                        if (board.isCollide(tempPiece))
                        {
                            tempPiece.setX(tempPiece.x + 1); // failed, go back
                            continue;
                        }
                        break;
                    case inputs::Right:
                        tempPiece.setX(tempPiece.x + 1);
                        if (board.isCollide(tempPiece))
                        {
                            tempPiece.setX(tempPiece.x - 1); // failed, go back
                            continue;
                        }
                        break;
                    case inputs::SonicDrop:
                    default:
                        board.sonicDrop(tempPiece);
                        break;
                    }

                    auto addcell = [](inputNode& realNode, std::vector<inputNode*>& nextNodes, const inputs& input,
                        const uint_fast16_t& depth, const Piece& tempPiece, movementBoard* self, const spin& spinned, boardtype* board) {

                            std::vector<inputs> hist;
                            hist.reserve(realNode.history.size() + 1);
                            // populate the history 
                            hist = realNode.history;
                            hist.push_back(input);

                            inputNode* currentNode = new inputNode(hist, depth);
                            self->addCell(board, std::move(hist), depth, {int_fast8_t(tempPiece.x + int_fast8_t(1)), int_fast8_t(tempPiece.y + int_fast8_t(1))}, tempPiece.spin, spinned, tempPiece);

                            //add populated node to the next nodes to evaluate
                            nextNodes.push_back(currentNode);

                            // make sure to set the pointer to a node for later on, it will ensure the data gets deleted 
                            realNode.setInputNode(input, currentNode);
                    };


                    // if the piece is in a new position add to the movementBoard, if its floating add it to the cells we have visited
                    auto& cellFromLocation = (*intermediateBoard)[(tSpinned)][(tempPiece.spin)][(tempPiece.x + 1)][(tempPiece.y + 1)];
                    if (cellFromLocation == nullptr)
                    {
                        nPiecePlacements++;
                        addcell(realNode, nextNodes, input, depth, tempPiece, this, tSpinned, intermediateBoard);
                    }
                    else
                    {
                        //if the cell is already populated 
                        // and has sonic drop as its last input, discard it
                        if (cellFromLocation->inputs.back() == inputs::SonicDrop)
                            continue;

                        // if the cell is already populated
                        // and the the two histories are the same replace the cell with itself
                        if (realNode.history == cellFromLocation->inputs)
                        {
                            nPiecePlacements++;
                            addcell(realNode, nextNodes, input, depth, tempPiece, this, tSpinned, intermediateBoard);

                        }
                    }
                }
            }
            std::swap(nextNodes, nodes);
            nextNodes.clear();
            if (nodes.size() == 0)
                break;
        }
        std::vector<FullPiece> PossiblePiecePlacements;
        PossiblePiecePlacements.reserve(nPiecePlacements);
		
        for (size_t spn = 0; spn < intermediateBoard->size(); spn++)
        {
            auto& temspn = (*intermediateBoard).data()[spn];
			
            for (size_t rot = 0; rot < temspn.size(); rot++)
            {
                auto& tempspnrot = temspn.data()[(rot)];
				
                for (size_t x = 0; x < tempspnrot.size(); x++)
                {
                    auto& tempspnrotx = tempspnrot.data()[(x)];
					
                    for (size_t y = 0; y < tempspnrotx.size(); y++)
                    {
                        auto& tempspnrotxy = tempspnrotx.data()[(y)];

                        if (tempspnrotx[(y)] != nullptr)
                        {
                            if (!board.trySoftDrop(tempspnrotxy->piece))
                                PossiblePiecePlacements.push_back(std::move(*tempspnrotxy));
                            delete tempspnrotxy;
                        }
                    }
                }
            }
        }
        delete intermediateBoard;
        return PossiblePiecePlacements;
    }
};
consteval inline bool searchConstantEvalCheck() {
    {
        BitBoard board;
        Piece piece = PieceType::T;
        movementBoard MB;
        auto input = MB.find_moves(board, piece);
        return input.at(0).inputs.at(0) == inputs::Left;
    }
}