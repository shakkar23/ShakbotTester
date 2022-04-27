#include "Tetris.hpp"
#include <cstdlib>
#include <queue>

// frontEnd to bot

// stop
// The stop message tells the bot to stop calculating.
void Tetris::stop() { /*this->isStop = true; */}

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
const int Tetris::eval(const Board& board, const bool ClearedLines, const int nDamageSent) const
{
    int score = 0;
    constexpr auto field_h = VISUALBOARDHEIGHT;
    constexpr auto field_w = BOARDWIDTH;
    std::array<int, BOARDWIDTH> min_y = {};
    struct factor {
        int hole, h_change, y_factor, h_variance, nDamageSent, noattclear;
    } constexpr ai_factor = {
            -50, -5, -10, -10,  40, -30
    };

    //if ( depth > 2 ) ai_factor = ai_factor_l[2];
    //else ai_factor = ai_factor_l[depth];

    // get all columns height
    {
        auto getMaxColumn = [](const Board& board) -> int {
            int max = VISUALBOARDHEIGHT - 1;
            // while the row is not full checking from top to bottom because misamino backwards upside down board representation
            while (board.rowIsEmpty(max))
                if (max == 0) { break; }
                else --max;
            return max;
        };
        const int MaxColumn = getMaxColumn(board);

        // iterate through the rows
        for (int x = 0; x < field_w; ++x) {
            // iterate through the columns down to the bottom most row
            for (int y = MaxColumn; y >= 0; --y) {
                if (board.board[x][y] != empty) {
                    min_y[x] = y;
                    break;
                }
            }
        }
    }

    //some magic cause why not
    min_y[BOARDWIDTH - 1] = min_y[BOARDWIDTH - 3];

    // find holes
    {
        int hole_score = 0;
        // for every row
        for (int x = 0; x < field_w; ++x) {
            // for every columns max height 
            for (int y = min_y[x] + 1; y >= 0; --y) {
                // if there is an empty cell in the row add the ai hole factor to the hole score
                // which is -50 because holes bad
                if (board.board[x][y] == empty) {
                    hole_score += ai_factor.hole;
                }
            }
        }
        // add the hole score to the score
        // hole score is negative, so we are punishing the bot for having holes
        score += hole_score;
    }
    // height change
    {
        // there is no row to the left of the left most row, or right to the right most
        // so we use the other side of the row to see its height change
        // if we used min_y[0] here we would never have any height change for the first row we evaluate
        int last = min_y[1];
        for (int x = 0; x < field_w; last = min_y[x], ++x) {
            int v = min_y[x] - last;
            int absv = abs(v);
            score += absv * ai_factor.h_change;
        }
    }
    // variance
    {
        int h_variance_score = 0;
        int AllHeights = 0;
        {
            int sum = 0;
            int sample_cnt = 0;
            for (int x = 0; x < field_w; ++x) {
                AllHeights += min_y[x];
            }
            {
                double h = field_h - (double)AllHeights / field_w;

                // field_h -  will make it a higher score when its a lower height
                // /field_w will make it so if the board is super wide its harder to get a good score because its easier to make less height
                // exponentially harder to get a good score the wider the board is which makes sense because its exponentially easier the more 
                // width you have

                score += int(ai_factor.y_factor * h * h / field_h);
            }
            for (int x = 0; x < field_w; ++x) {
                int t = AllHeights - min_y[x] * field_w;
                if (abs(t) > field_h * field_w / 4) {
                    if (t > 0) t = field_h * field_w / 4;
                    else t = -int(field_h * field_w / 4);
                }
                sum += t * t;
                ++sample_cnt;
            }
            if (sample_cnt > 0) {
                h_variance_score = sum * ai_factor.h_variance / (sample_cnt * 100);
            }
            score += h_variance_score;
        }
    }
    // clear and attack
    score += nDamageSent * ai_factor.nDamageSent;
    if (nDamageSent == 0) {
        score += ClearedLines * ai_factor.noattclear;
    }
    return score;
}


const int Tetris::eval(const BitBoard& board, const bool ClearedLines, const int nDamageSent) const
{
    int score = 0;
    constexpr auto field_h = VISUALBOARDHEIGHT;
    constexpr auto field_w = BOARDWIDTH;
    std::array<int, BOARDWIDTH> min_y = {};
    struct factor {
        int hole, h_change, y_factor, h_variance, nDamageSent, noattclear;
    } constexpr ai_factor = {
            -50, -5, -10, -10,  40, -30
    };

    //if ( depth > 2 ) ai_factor = ai_factor_l[2];
    //else ai_factor = ai_factor_l[depth];

    // get all columns height
    {
        auto getMaxColumn = [](const BitBoard& board) -> int {
            int max = VISUALBOARDHEIGHT - 1;
            // while the row is not full checking from top to bottom because misamino backwards upside down board representation
            while (board.rowIsEmpty(max))
                if (max == 0) { break; }
                else --max;
            return max;
        };
        const int MaxColumn = getMaxColumn(board);

        // iterate through the rows
        for (int x = 0; x < field_w; ++x) {
            // iterate through the columns down to the bottom most row
            for (int y = MaxColumn; y >= 0; --y) {
                if (board.getBit(x,y) != minotype::isEmpty) {
                    min_y[x] = y;
                    break;
                }
            }
        }
    }

    //some magic cause why not
    //min_y[BOARDWIDTH - 1] = min_y[BOARDWIDTH - 3];

    // find holes
    {
        int hole_score = 0;
        // for every row
        for (int x = 0; x < field_w; ++x) {
            // for every columns max height 
            for (int y = min_y[x] + 1; y >= 0; --y) {
                // if there is an empty cell in the row add the ai hole factor to the hole score
                // which is -50 because holes bad
                if (board.getBit(x, y) == minotype::isEmpty) {
                    hole_score += ai_factor.hole;
                }
            }
        }
        // add the hole score to the score
        // hole score is negative, so we are punishing the bot for having holes
        score += hole_score;
    }
    // height change
    {
        // there is no row to the left of the left most row, or right to the right most
        // so we use the other side of the row to see its height change
        // if we used min_y[0] here we would never have any height change for the first row we evaluate
        int last = min_y[1];
        for (int x = 0; x < field_w; last = min_y[x], ++x) {
            int v = min_y[x] - last;
            int absv = abs(v);
            if(absv > 1)
                score += absv * ai_factor.h_change;
        }
    }
    // variance
    /* {
        int h_variance_score = 0;
        int AllHeights = 0;
        {
            int sum = 0;
            int sample_cnt = 0;
            for (int x = 0; x < field_w; ++x) {
                AllHeights += min_y[x];
            }
            {
                double h = field_h - (double)AllHeights / field_w;

                // field_h -  will make it a higher score when its a lower height
                // /field_w will make it so if the board is super wide its harder to get a good score because its easier to make less height
                // exponentially harder to get a good score the wider the board is which makes sense because its exponentially easier the more 
                // width you have

                score += int(ai_factor.y_factor * h * h / field_h);
            }
            for (int x = 0; x < field_w; ++x) {
                int t = AllHeights - min_y[x] * field_w;
                if (abs(t) > field_h * field_w / 4) {
                    if (t > 0) t = field_h * field_w / 4;
                    else t = -int(field_h * field_w / 4);
                }
                sum += t * t;
                ++sample_cnt;
            }
            if (sample_cnt > 0) {
                h_variance_score = sum * ai_factor.h_variance / (sample_cnt * 100);
            }
            score += h_variance_score;
        }
    }*/
    // clear and attack
    if (nDamageSent == 0) {
        score += ClearedLines * ai_factor.noattclear;
    }
    else 
        score += int(nDamageSent * ai_factor.nDamageSent * double((double)nDamageSent / double(ClearedLines) - 0.7));
    return score;
}
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
void Tetris::waitForStart(){
}
// tbp thread
void Tetris::EventStart(){
    std::lock_guard lk(botMux);
    needPlays = false;
    startBot = true;
    endBot = false;
    playsProcessed = false;
    condVar.notify_one();
}
//bot thread
void Tetris::waitForEnd(){
    //std::unique_lock<std::mutex> ul(botMux);
    //condVar.wait(ul, [&]() { return endBot; });
    endBot = false;
}
// tbp thread
void Tetris::EventEnd(){
    needPlays = true;
    std::unique_lock<std::mutex> ul(botMux);
    endBot = true;
    condVar.wait(ul, [&] { return playsProcessed; });
	// get plays
	

	
    playsProcessed = false;
}

void Tetris::backGroundThread() {
    struct PieceNode {
        PieceNode(BitBoard &board, std::vector<std::vector<inputs>> input, int score): board(board), input(input), score(score) {};
        //overload the == operator for the priority queue
		bool operator==(const PieceNode& other) const {
			return this->score == other.score;
		}
        ~PieceNode() {
            for(auto & i : children) {
					delete i;
			}
        }
		//overload the < operator for the priority queue
        bool operator<(const PieceNode& other) const {
            return this->score < other.score;
        }
        bool operator>(const PieceNode& other) const {
            return this->score > other.score;
        }
		
        BitBoard board;
        std::vector<std::vector<inputs>> input;
        int score;
        std::vector<PieceNode*> children;
    };

    while (true) {
        // wait for the game to start
        std::unique_lock<std::mutex> ul(botMux);
        condVar.wait(ul, [&] { return startBot; });
        startBot = false;

        // start the bot
        std::priority_queue<PieceNode> pq;
        //copy the board to a local variable
        Board board = this->board;
        BitBoard bitboard = BitBoard::fromBoard(board);

        // copy the queue to a local variable
        std::vector<Piece> queue = this->queue;

        // copy the hold to a local variable
        Piece hold = this->hold;

        PieceNode root = PieceNode( bitboard, {}, 0 );
		
		
        auto cmp = [](PieceNode* left, PieceNode* right) { 
            bool outcome = left->score < right->score;
            return outcome; 
        };
        std::priority_queue<PieceNode*, std::vector<PieceNode*>, decltype(cmp)> finishedNodes(cmp);
        std::priority_queue < PieceNode*, std::vector<PieceNode*>, decltype(cmp)> nodes(cmp);
        nodes.push(&root);
        std::vector<PieceNode*> nextNodes; nextNodes.reserve(128);
        // forever
        while (true) {
            // for all nodes
            for (int i = 0; i < 50; ++i) {
                if (nodes.empty()) {
                    break;
                }
                auto curNode = nodes.top();
                nodes.pop();
                finishedNodes.push(curNode);
                // create a new piece node

                // Possible Piece Placements
                auto PPP = moveBoard->find_moves(curNode->board, piece);
                curNode->children.reserve(PPP.size());

                // for every possible Piece placement
                for (auto& p : PPP) {
                    {
                        if (this->needPlays) {
                            // implement priority queue, and finished_nodes for less of an overhead for sorting
                            auto top = finishedNodes.top();
                            [[likely]]if(top->input.size() == 0)
                            {
								finishedNodes.pop();
								top = finishedNodes.top();
						    }
                            botReturnInput = top->input;
                            playsProcessed = true;
                            this->needPlays = false;
                            ul.unlock();
                            condVar.notify_one();
							
                            waitForEnd(); 
                            break;
                        }
                    }
                    // create a new board
                    BitBoard newBoard = curNode->board;
                    // place the piece on the board
                    newBoard.setPiece(p.piece);
                    // how many lines are cleared by this move
                    int nCleared = newBoard.clearLines();

                    // damage dealt
                    constexpr auto perfectClearDamage = 10;
                    constexpr int DAMAGETABLESIZE = 5;
                    const std::array<const uint8_t, DAMAGETABLESIZE> damageTable = {
                        0,0,1,2,4
                    };

                    //if the board is left empty, it is a perfect clear and the damage dealt is 10 + damageTable[ the number of lines cleared ]
                    int nDamage;
                    if (newBoard.isBoardEmpty())
                        nDamage = perfectClearDamage + damageTable[std::clamp(nCleared, 0, DAMAGETABLESIZE - 1)];
                    else
                        nDamage = (damageTable[std::clamp(nCleared, 0, DAMAGETABLESIZE - 1)] * (2 * (p.spin == Full || p.spin == Mini)));
					
                    // create and set up the node to go into the tree
                    PieceNode *newPN = new PieceNode(newBoard, curNode->input,eval(newBoard, nCleared, nDamage));
                    newPN->input.push_back(p.inputs);
					// add node to the tree
                    curNode->children.push_back(newPN);
                    // add node to the nodes we are going to evaluate
					nextNodes.push_back(newPN);
				}
                if (this->needPlays) break;
			}
            if (this->needPlays) break;
            for (auto& n : nextNodes) {
                nodes.push(n);
            }
			nextNodes.clear();
		}

    }
		// somehow thread safely give the other CPU the moves we found
		
}