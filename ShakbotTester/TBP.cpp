#include "Tetris.hpp"
#include <cstdlib>
#include <queue>

// frontEnd to bot

// stop
// The stop message tells the bot to stop calculating.
void Tetris::stop() { /*this->isStop = true; */ }

// suggest
// The suggest message tells the bot to suggest some next moves in order of preference.
// It is only valid to send this message if the bot is calculating.
// The bot must reply as quickly as possible with a suggestion message containing suggested moves.
suggestion Tetris::suggest() {
    //isStop = true;

    return { };
}

// play
// The play message tells the bot to advance its game state as if the specified move was played and begin calculating from that position.
// It is only valid to send this message if the bot is calculating.
// Whether a hold was performed is inferred from the type of piece placed.
move Tetris::play() { return {}; }

// new_piece
// The new_piece message informs the bot that a new piece has been added to the queue.
// This message is generally paired with a play message.
PieceType Tetris::new_piece() { return {}; }

// quit
// The quit message tells the bot to exit.
void Tetris::quit() {}

// bot to frontEnd

// error
// The error message informs the frontend that the bot cannot play with the specified rules.
void Tetris::error() {}

// ready
// The ready message tells the frontend that the bot understands the specified game rules and is ready to receive a position to calculate from.
void Tetris::ready() {}

// info
// The info message must be sent by the bot to advertise to the frontend what TBP features it supports.
// Once the frontend receives this message, it can inform the bot of the game rules using the rules message.
void Tetris::info(information info) {}

// suggestion
// The suggestion message is sent in response to a suggest message.
// It informs the frontend of what moves the bot wishes to make in order of preference.
// The frontend should play the most preferred valid move.
// If no moves are valid, the bot forfeits and the frontend should tell the bot to stop calculation.
// Whether a hold should be performed is inferred from the type of piece to be placed.
void Tetris::sugestion(suggestion moves) {}

// evaluates the board and returns the score
// the evaluation is going to attempt to minimize bumpiness as well as the number of holes

/// Evaluates the bumpiness of the playfield.
///
/// The first returned value is the total amount of height change outside of an apparent well. The
/// second returned value is the sum of the squares of the height changes outside of an apparent
/// well.
std::pair<int32_t, int32_t> Tetris::bumpiness(Board& board, size_t well)
{
    int32_t bumpiness = -1;
    int32_t bumpiness_sq = -1;

    int32_t prev = well == 0 ? 1 : 0;

    std::array<int, BOARDWIDTH>columnHeights{};
    int last = columnHeights[1];
    for (int x = 0; x < BOARDWIDTH; last = columnHeights[x], ++x) {
        int v = columnHeights[x] - last;
    }

    for (int i = 1; i <= BOARDWIDTH; i++)
    {
        if (i == well)
        {
            continue;
        }
        int32_t dh = columnHeights[prev] - abs(columnHeights[i]);
        bumpiness += dh;
        bumpiness_sq += dh * dh;
        prev = i;
    }

    return { abs(bumpiness), abs(bumpiness_sq) };
}

// bot thread
void Tetris::waitForStart()
{
    std::unique_lock<std::mutex> ulk(this->botMux);
    condVar.wait(ulk, [&] { return startBot.load(); });
    startBot = false;
}
//bot thread
void Tetris::waitForEnd() {
    playsProcessed = true;
    condVar.notify_one();
    endBot = false;
}
// tbp thread
void Tetris::EventStart() {
    needPlays = false;
    startBot = true;
    endBot = false;
    playsProcessed = false;
    condVar.notify_one();
}
// tbp thread
void Tetris::EventEnd() {
    std::unique_lock<std::mutex> ulk(this->botMux);
    needPlays = true;
    condVar.wait(ulk, [&] { return playsProcessed.load(); });
    // get plays
    playsProcessed = false;
}

void Tetris::backGroundThread() {
    struct TetNode {
        TetNode(const BitBoard& board, const std::vector<std::vector<inputs>> &input, const std::vector<Piece> &queue, const Piece &hold, int score) : board(board), input(input), score(score), queue(queue) {};
        //overload the == operator for the priority queue
        bool operator==(const TetNode& other) const {
            return this->score == other.score;
        }
        ~TetNode() {
            for (auto& i : children) {
                delete i;
            }
        }
        //overload the < operator for the priority queue
        bool operator<(const TetNode& other) const {
            return this->score < other.score;
        }
        bool operator>(const TetNode& other) const {
            return this->score > other.score;
        }

        BitBoard board;
        int16_t score;
        std::vector<std::vector<inputs>> input;
        std::vector<TetNode*> children;
        std::vector<Piece> queue;
        Piece hold = Piece(PieceType::empty);
    };
        auto cmp = [](TetNode*& left, TetNode*& right) {
            return left->score < right->score;
        };

    while (true) {
		
        BitBoard bitboard;
		
        std::priority_queue<TetNode*, std::vector<TetNode*>, decltype(cmp)> finishedNodes(cmp);
        std::priority_queue < TetNode*, std::vector<TetNode*>, decltype(cmp)> nodes(cmp);
        std::vector<TetNode*> nextNodes; nextNodes.reserve(1485);

        //this->needPlays = false;
        // wait for the game to start 
        waitForStart();
		
        // grab the data we are going to work with
        bitboard = BitBoard::fromBoard(board);
        TetNode root = TetNode(bitboard, {}, this->queue, this->hold, INT16_MIN);
        nodes.push(&root);

        while (true) {
            bool abort = false;
            // we need to evaluate nodes, but also need to add new nodes to the queue
            // 50 is random number I picked, feel free to change it
            for (size_t i = 0; i < 50; i++)
            {
                if (nodes.empty()) {
                    nodes.push(&root);;
                }
                auto curNode = nodes.top();
                nodes.pop();
                finishedNodes.push(curNode);
                if (curNode->queue.empty())
                    continue;
                // no hold
                {
                    // Possible Piece Placements
                    auto PPP = moveBoard->find_moves(curNode->board, curNode->queue[0]);
                    // muliplying by two since the amounnt of children will be about twice the amount of possible placements
                    // twice because we need to evaluate both the piece placement and the piece placement with a hold
                    curNode->children.reserve(PPP.size() * 2);

                    // for every possible Piece placement
                    for (auto& p : PPP) {


                        // create a new board
                        BitBoard newBoard = curNode->board;
                        // place the piece on the board
                        newBoard.setPiece(p.piece);
                        // how many lines are cleared by this move
                        int nCleared = newBoard.clearLines();

                        // damage dealt
                        constexpr auto perfectClearDamage = 10;
                        constexpr auto DAMAGETABLESIZE = 5;
                        constexpr std::array<const uint_fast8_t, DAMAGETABLESIZE> damageTable = {
                            0,0,1,2,4
                        };

                        //if the board is left empty, it is a perfect clear and the damage dealt is 10 + damageTable[ the number of lines cleared ]
                        int nDamage;
                        [[unlikely]]
                        if (newBoard.isBoardEmpty())
                            nDamage = perfectClearDamage + damageTable[std::clamp(nCleared, 0, DAMAGETABLESIZE - 1)];
                        else
                            nDamage = (damageTable[std::clamp(nCleared, 0, DAMAGETABLESIZE - 1)] * (2 * (p.spin == Full || p.spin == Mini)));

                        // create and set up the node to go into the tree
                        auto newqueue = curNode->queue;
                        newqueue.erase(newqueue.begin());
                        TetNode* newPN = new TetNode(newBoard, curNode->input, newqueue, curNode->hold, eval(newBoard, nCleared, nDamage));

                        newPN->input.push_back(p.inputs);
                        // add node to the tree
                        curNode->children.push_back(newPN);
                        // add node to the nodes we are going to evaluate
                        nextNodes.push_back(newPN);
                        if (this->needPlays && (!finishedNodes.empty())) {
                            auto top = finishedNodes.top();

                            if (top->input.size() == 0)
                            {
                                if (finishedNodes.top() != &root)
                                    continue;
                                else
                                    finishedNodes.pop();

                                if (!finishedNodes.empty())
                                {
                                    top = finishedNodes.top();
                                }
                                else
                                    continue;
                            }
                            botReturnInput = top->input;
                            abort = true;

                            waitForEnd();
                            this->needPlays = false;
                            break;
                        }
                    }
                        if (abort)break;
                }
                // hold
                /* {
                    auto PPP = moveBoard->find_moves(curNode->board, curNode->hold);

                    // for every possible Piece placement
                    for (auto& p : PPP) {


                        if (this->needPlays) {
                            this->needPlays = false;
							abort = true;
                            auto top = finishedNodes.top();

                            if (top->input.size() == 0)
                            {
                                finishedNodes.pop();
								if(!finishedNodes.empty())
                                    top = finishedNodes.top();
                            }
                            botReturnInput = top->input;
                            playsProcessed = true;
                            condVar.notify_one();

                            waitForEnd();
                            break;
                        }
                        // create a new board
                        BitBoard newBoard = curNode->board;
                        // place the piece on the board
                        newBoard.setPiece(p.piece);
                        // how many lines are cleared by this move
                        int nCleared = newBoard.clearLines();

                        // damage dealt
                        constexpr auto perfectClearDamage = 10;
                        constexpr auto DAMAGETABLESIZE = 5;
                        constexpr std::array<const uint_fast8_t, DAMAGETABLESIZE> damageTable = {
                            0,0,1,2,4
                        };

                        //if the board is left empty, it is a perfect clear and the damage dealt is 10 + damageTable[ the number of lines cleared ]
                        int nDamage;
						
                        if (newBoard.isBoardEmpty())
                            nDamage = perfectClearDamage + damageTable[std::clamp(nCleared, 0, DAMAGETABLESIZE - 1)];
                        else
                            nDamage = (damageTable[std::clamp(nCleared, 0, DAMAGETABLESIZE - 1)] * (2 * (p.spin == Full || p.spin == Mini)));

                        // create and set up the node to go into the tree
						
                        auto newqueue = curNode->queue;
                        auto newhold  = curNode->hold;
                        std::swap(newhold, newqueue[0]);
						
                        if(newqueue[0].kind == PieceType::empty)
                            newqueue.erase(newqueue.begin());
						
                        TetNode* newPN = new TetNode(newBoard, curNode->input, newqueue, newhold, eval(newBoard, nCleared, nDamage));

                        // hold specific 
                        newPN->input.push_back({inputs::Hold});

                        newPN->input.push_back(p.inputs);
                        // add node to the tree
                        curNode->children.push_back(newPN);
                        // add node to the nodes we are going to evaluate
                        nextNodes.push_back(newPN);
                    }
                }*/
            }
            if (abort) break;
			
            for (auto& n : nextNodes) {
                nodes.push(n);
            }
            nextNodes.clear();


            if (this->needPlays && (!finishedNodes.empty())) {
                auto top = finishedNodes.top();

                if (top->input.size() == 0)
                {
                    if (finishedNodes.top() != &root)
                        continue;
                    else
                        finishedNodes.pop();

                    if (!finishedNodes.empty())
                    {
                        top = finishedNodes.top();
                    }
                    else
                        continue;
                }
                botReturnInput = top->input;
                abort = true;

                waitForEnd();
                this->needPlays = false;
                break;
            }
        }

    }
    // somehow thread safely give the other CPU the moves we found

}

void Tetris::concurrentThread() {
    struct TetNode {
        constexpr TetNode(
            const BitBoard& board, 
            int16_t score,
            int16_t linesCleared,
            int16_t damageSent,
            const std::vector<std::vector<inputs>>& input, 
            const std::vector<Piece>& queue, 
            const Piece& hold
        )
            : 
            board(board), score(score),linesCleared(linesCleared), damageSent(damageSent), input(input), queue(queue) {};
		
        //overload the == operator for the priority queue
        constexpr bool operator==(const TetNode& other) const {
            return this->score == other.score;
        }
        constexpr ~TetNode() {
            for (auto& i : children) {
                delete i;
            }
        }
        //overload the < operator for the priority queue
        constexpr bool operator<(const TetNode& other) const {
            return this->score < other.score;
        }
        constexpr bool operator>(const TetNode& other) const {
            return this->score > other.score;
        }

        BitBoard board;
        int16_t score;
        int16_t linesCleared;
        int16_t damageSent;
        std::vector<std::vector<inputs>> input;
        std::vector<Piece> queue;
        std::vector<TetNode*> children;
        Piece hold = Piece(PieceType::empty);
    };
    constexpr auto cmp = [](TetNode*& left, TetNode*& right) {
        return left->score < right->score;
    };

    BitBoard bitboard;

    // nodes we have evaluated, even if we havent evaluated the children
    std::priority_queue<TetNode*, std::vector<TetNode*>, decltype(cmp)> finishedNodes(cmp);

    // nodes we are going to evaluate
    std::priority_queue < TetNode*, std::vector<TetNode*>, decltype(cmp)> processingNodes(cmp);
    

    // grab the data we are going to work with
    bitboard = BitBoard::fromBoard(board);
    TetNode root = TetNode(bitboard, INT16_MIN, 0,0, {}, this->queue, this->hold);
    processingNodes.push(&root);

    if(root.board.isCollide(root.queue[0])) {
        botReturnInput = root.input;
        return;
    }


    while (true) {
        // we need to evaluate nodes, but also need to add new nodes to the queue
        // 50 is random number I picked, feel free to change it
        for (size_t i = 0; i < 50; i++)
        {
            if (processingNodes.empty()) {
                processingNodes.push(&root);;
            }
            auto curNode = processingNodes.top();
            processingNodes.pop();
            finishedNodes.push(curNode);
            if (curNode->queue.empty())
                continue;
            // no hold
            {
                // Possible Piece Placements
                auto PPP = moveBoard->find_moves(curNode->board, curNode->queue[0]);
                // muliplying by two since the amounnt of children will be about twice the amount of possible placements
                // twice because we need to evaluate both the piece placement and the piece placement with a hold
                curNode->children.reserve(PPP.size() * 2);

                // for every possible Piece placement
                for (auto& p : PPP) {

                    // create a new board
                    BitBoard newBoard = curNode->board;
                    // place the piece on the board
                    newBoard.setPiece(p.piece);
                    // how many lines are cleared by this move
                    int nCleared = newBoard.clearLines();
                    if (nCleared > 2) {
                        int i = 0;
                    }
                    // damage dealt
                    constexpr auto perfectClearDamage = 10;
                    constexpr auto DAMAGETABLESIZE = 5;
                    constexpr std::array<const uint_fast8_t, DAMAGETABLESIZE> damageTable = {
                        0,0,1,2,4
                    };

                    //if the board is left empty, it is a perfect clear and the damage dealt is 10 + damageTable[ the number of lines cleared ]
                    int nDamage;
                    [[unlikely]]
                    if (newBoard.isBoardEmpty())
                        nDamage = perfectClearDamage + damageTable[std::clamp(nCleared, 0, DAMAGETABLESIZE - 1)];
                    else
                        nDamage = (damageTable[std::clamp(nCleared, 0, DAMAGETABLESIZE - 1)] * (1 << (p.spin == Full || p.spin == Mini)));

                    // create and set up the node to go into the tree
                    auto newqueue = curNode->queue;
                    newqueue.erase(newqueue.begin());
					
                    TetNode* newPN = new TetNode(newBoard, eval(newBoard, nCleared, curNode->damageSent + nDamage), curNode->linesCleared + nCleared, curNode->damageSent + nDamage, curNode->input, newqueue, curNode->hold);

                    if (newPN->linesCleared > 2) {
                    int i = 0;
                    }
					
                    newPN->input.push_back(p.inputs);
                    // add node to the tree
                    curNode->children.push_back(newPN);
                    // add node to the nodes we are going to evaluate
                    processingNodes.push(newPN);
                }
            }
            if (this->needPlays) {
                        auto tippyTop = [&]() {
                            if(finishedNodes.top() < processingNodes.top())
								return processingNodes.top();
                                else
								return finishedNodes.top();
                        };
                        auto top = tippyTop();
                        if (top->input.size() == 0)
                        {
                            if (top == &root) 
                                continue;

                            finishedNodes.pop();
                            top = tippyTop();
                        }
                        this->botReturnInput = top->input;
                        this->damage = top->damageSent;
						this->lines = top->linesCleared;

                        this->needPlays = false;
                        return;
                    }
        }
    }
}