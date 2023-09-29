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
#include "fasthash.h"


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
    constexpr FullPiece(std::vector<inputs> &inputs, Piece piece, spin &spin) noexcept 
    : piece(piece), spin(spin) {
        std::swap(inputs, this->inputs);
    }

    constexpr FullPiece(std::vector<inputs> &&inputs,const Piece& piece,const spin &spin) noexcept 
        : piece(piece), spin(spin), inputs(std::move(inputs)) {}

    constexpr bool operator==(const FullPiece& other) {
        return piece == other.piece && spin == other.spin;
    }
    constexpr bool operator==(FullPiece& other) {
        return piece == other.piece && spin == other.spin;
    }
    constexpr bool operator<(const FullPiece& other) {
        return piece < other.piece;
    }

    std::vector<inputs> inputs;
    Piece piece;
    spin spin;
};
const inline uint32_t hash(const FullPiece& piece) {
        return fasthash32(&piece.piece, sizeof(piece.piece), 123456789);
}


struct inputNode
{
public:
    constexpr inline inputNode(FullPiece fullPiece) : pieceData(fullPiece), children({ nullptr ,nullptr ,nullptr ,nullptr ,nullptr }) {

    }
    constexpr inline ~inputNode()
    {
        for (auto& i : children)
        {
            if (i != nullptr)
                delete i;
        }
    }
    constexpr inline inputNode* getInputNode(const inputs input)const
    {
        return children.at((int)input);
    }
    constexpr inline void setInputNode(const inputs input, inputNode* node)
    {
        if (children.at((int)input) != nullptr)
            delete children.at((int)input);
        children.at((int)input) = node;
    }

    FullPiece pieceData;
    std::array<inputNode*, int(inputs::numberOfInputs)> children{};
};

// this class will store a board where each cell points to a piece and its movements to get to that cell
class movementBoard
{
public:
    // needed padding as the X coordinate of a placed piece can be negative 1
    // padding the board on all 4 sides by one
	constexpr void DijkstraFaithful(const Board& board, const Piece& piece);
    private:
        
public:	
    typedef uint32_t piece_hash;
    const inline std::vector<FullPiece> find_moves(const BitBoard& board, const Piece& piece)noexcept
    {
        // initial check to see if the peice is colliding with the board
        if (board.isCollide(piece))
        {
            return{};
        }

        std::vector<FullPiece> moves;

        inputNode root = inputNode(FullPiece({}, piece, spin::None));
        std::vector<inputNode*> nodes = { &root };
        nodes.reserve(150);

        std::vector<inputNode*> nextNodes;
        nextNodes.reserve(150);

        // all the previous nodes that ive hit so far
        std::unordered_map<piece_hash, int> packedPieces;


        // limiting the depth of the search to inf but can be increased later on if needed
        while (true)
        {
            for (auto& node : nodes)
            {
                auto& curNode = *node;
                for (size_t i = 0; i < int(inputs::numberOfInputs); i++)
                {
                    std::vector<inputs> newHist(curNode.pieceData.inputs.size() + 1);
                    newHist = curNode.pieceData.inputs;
                    newHist.push_back((inputs)i);

                    spin tSpinned = spin::None;

                    // TODO allow inputs to be done simultaneously
                    const FullPiece newFullPiece = FullPiece(newHist, playPiece(piece, newHist, board, tSpinned), tSpinned);

                    const auto tempPieceHash = hash(newFullPiece);
                    const auto iter = packedPieces.find(tempPieceHash);

                    // havent found the hash, new state!
                    if (iter == packedPieces.end()) {
                        // add the new hash to the state
                        packedPieces[tempPieceHash] = 0;

                        moves.push_back(newFullPiece);

                        curNode.children[i] = new inputNode(newFullPiece);

                        nextNodes.push_back(curNode.children[i]);
                    }
                }
            }
            std::swap(nextNodes, nodes);
            nextNodes.clear();
            if (nodes.size() == 0)
                break;
        }

        for (auto& move : moves) {
            if (board.trySoftDrop(move.piece))
            {
                move.spin = spin::None;
                board.sonicDrop(move.piece);
            }
        }

        std::sort(moves.begin(), moves.end());
        moves.erase(
            std::unique(moves.begin(), moves.end()), moves.end()
        );

        return moves;
    }
};
