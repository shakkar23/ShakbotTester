#include "search.hpp"
#include <memory>
#include <cassert>
// a search algorithm that finds all piece placements that are valid

// the movement tree we will use to keep track of the inputs done
class inputNode
{
public:
    constexpr inputNode() {}
    constexpr inputNode(std::vector<inputs> inputs, uint8_t depth) : history(std::move(inputs)), depth(depth), inputs({ nullptr ,nullptr ,nullptr ,nullptr ,nullptr }) {
		
    }
    constexpr ~inputNode()
    {
        for (auto& i : inputs)
        {
            if (i != nullptr)
                delete i;
        }
    }
    constexpr inputNode* getInputNode(const inputs input)const
    {
        return inputs.at((int)input);
    }
    constexpr void setInputNode(const inputs input, inputNode* node)
    {
        if (inputs.at((int)input) != nullptr)
            delete inputs.at((int)input);
        inputs.at((int)input) = node;
    }

     std::vector<inputs> history{};
private:
    std::array<inputNode*, int(inputs::numberOfInputs)> inputs{};
    uint8_t depth{};
};



std::unique_ptr<Cell> historyToCell(std::vector<inputs>& hist, int time)
{
    return std::make_unique<Cell>(Cell(time, hist));
}

//void movementBoard::addCell(std::shared_ptr<Cell> &cell, Coord location, RotationDirection spin)

const void movementBoard::DijkstraFaithful(const Board& board, const Piece& piece) {


}

const void movementBoard::find_moves(const Board & board, const Piece & piece)noexcept
{
    
    std::unique_ptr < boardtype> intermediateBoard;
    intermediateBoard = std::make_unique<boardtype>();
	
    clearBoard();

    inputNode root;
    std::vector<inputNode*> nodes = { &root };
    std::vector<inputNode*> nextNodes;

    // initial check to see if the peice is colliding with the board
    if (board.isCollide(piece))
    {
        return;
    }

    // limiting the depth of the search to 15 but can be increased later on if needed
    for (uint_fast16_t depth = 0;; depth++)
    {
        size_t sizeOfNodes = nodes.size();
        for (auto& node : nodes)
        {
            {
                std::vector<inputs> e = { inputs::Left, inputs::Left, inputs::Left };
                if (e== node->history)
                    depth = depth;
            }
            for (size_t i = 0; i < int(inputs::numberOfInputs); i++)
            {
                spin tSpinned = spin::None;
                //Piece tempPiece = piece;
                const inputs input = (inputs)i;
                // TODO allow inputs to be done simultaneously
                Piece tempPiece = playPiece(piece, node->history, board, tSpinned);
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

                auto addcell = [](inputNode*& node, std::vector<inputNode*> &nextNodes, const inputs &input, 
                    const uint_fast16_t &depth, const Piece &tempPiece, movementBoard * self, spin spinned, boardtype &board){
                    
                        std::vector<inputs> hist;
                        hist.reserve(node->history.size() + 1);
                        // populate the history 
                        hist = node->history;
                        hist.push_back(input);

                        inputNode* currentNode = new inputNode(hist, depth);
                        self->addCell(board,historyToCell(hist, depth), { int_fast8_t(tempPiece.x + int_fast8_t(1)), int_fast8_t(tempPiece.y + int_fast8_t(1)) }, tempPiece.spin, spinned);

                        //add populated node to the next nodes to evaluate
                        nextNodes.push_back(currentNode);

                        // make sure to set the pointer to a node for later on, it will ensure the data gets deleted 
                        node->setInputNode(input, currentNode); 
                };
				

                // if the piece is in a new position add to the movementBoard, if its floating add it to the cells we have visited
                if (intermediateBoard->at(tSpinned).at(tempPiece.spin).at(tempPiece.x + 1).at(tempPiece.y + 1) == nullptr)
                {
                    addcell(node, nextNodes, input, depth, tempPiece, this, tSpinned, *intermediateBoard.get());
                }
                else
                {
                    //if the cell is already populated 
                    // and has sonic drop as its last input, discard it
                    if (intermediateBoard->at(tSpinned).at(tempPiece.spin).at(tempPiece.x + 1).at(tempPiece.y + 1)->inputs.back() == inputs::SonicDrop)
                        continue;

                    // if the cell is already populated
                    // and the the two histories are the same replace the cell with itself
                    if (node->history == intermediateBoard->at(tSpinned).at(tempPiece.spin).at(tempPiece.x + 1).at(tempPiece.y + 1)->inputs)
                    {
                        addcell(node, nextNodes, input, depth, tempPiece, this, tSpinned, *intermediateBoard.get());
                        
                    }
                }
			}
		}
		nodes = nextNodes;
		nextNodes.clear();
		if (nodes.size() == 0)
			break;
    }
    pieces.reserve(this->numberOfPopulatedCells);
    for (size_t spn = 0; spn < intermediateBoard->size(); spn++)
    {
        for (size_t rot = 0; rot < intermediateBoard->at(spn).size(); rot++)
        {
            for (size_t x = 0; x < intermediateBoard->at(spn).at(rot).size(); x++)
            {
                for (size_t y = 0; y < intermediateBoard->at(spn).at(rot).at(x).size(); y++)
                {
                    if (intermediateBoard->at(spn).at(rot).at(x).at(y) != nullptr)
                    {
                        spin temp;
                        Piece p = playPiece(piece, intermediateBoard->at(spn).at(rot).at(x).at(y)->inputs, board, temp);
                        if (!board.trySoftDrop(p))
                            this->pieces.push_back(FullPiece(intermediateBoard->at(spn).at(rot).at(x).at(y)->inputs, p , temp));
                    }
                }
            }
        }
    }
    return;
}