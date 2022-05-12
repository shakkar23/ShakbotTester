#pragma once
#include "Piece.hpp"
#include "Rotation.hpp"
#include "suggestion.hpp"
#include "Color.hpp"
#include "Coord.hpp"
#include "Board.hpp"
#include "info.hpp"
#include "search.hpp"
#include <atomic>
#include <thread>
#include <condition_variable>
#include <functional>
#include <cmath>
#include <cstdlib>

class movementBoard;
class Tetris
{
public:
    Tetris() {
        moveBoard = std::make_unique<movementBoard>();
        bot = std::jthread(std::function<void()>([&]() { backGroundThread(); }));
    }
    std::unique_ptr<movementBoard> moveBoard;
    Board board = Board();
    Piece hold = Piece(PieceType::empty);
    std::vector<Piece> queue;
    std::vector<std::vector<inputs>> botReturnInput;
    uint16_t damage;
    uint16_t lines;
    Piece piece = Piece(PieceType::L);
    std::atomic_int32_t combo;
    std::atomic_bool startBot = false;
    std::atomic_bool playsProcessed = false;
    std::atomic_bool needPlays = false;
    std::atomic_bool endBot = true;
	

    // frontEnd to bot

    // stop
    // The stop message tells the bot to stop calculating.
    void stop();

    // suggest
    // The suggest message tells the bot to suggest some next moves in order of preference.
    // It is only valid to send this message if the bot is calculating.
    // The bot must reply as quickly as possible with a suggestion message containing suggested moves.
    suggestion suggest();

    // play
    // The play message tells the bot to advance its game state as if the specified move was played and begin calculating from that position.
    // It is only valid to send this message if the bot is calculating.
    // Whether a hold was performed is inferred from the type of piece placed.
    move play();

    // new_piece
    // The new_piece message informs the bot that a new piece has been added to the queue.
    // This message is generally paired with a play message.
    PieceType new_piece();

    // quit
    // The quit message tells the bot to exit.
    void quit();

    // bot to frontEnd

    // error
    // The error message informs the frontend that the bot cannot play with the specified rules.
    void error();

    // ready
    // The ready message tells the frontend that the bot understands the specified game rules and is ready to receive a position to calculate from.
    void ready();

    // info
    // The info message must be sent by the bot to advertise to the frontend what TBP features it supports.
    // Once the frontend receives this message, it can inform the bot of the game rules using the rules message.
    void info(information info);

    // suggestion
    // The suggestion message is sent in response to a suggest message.
    // It informs the frontend of what moves the bot wishes to make in order of preference.
    // The frontend should play the most preferred valid move.
    // If no moves are valid, the bot forfeits and the frontend should tell the bot to stop calculation.
    // Whether a hold should be performed is inferred from the type of piece to be placed.
    void sugestion(suggestion moves);

    // private
public:
    std::pair<int32_t, int32_t> bumpiness(Board& board, size_t well);

    // constexpr implementation of absolute value for consteval purposes
    template<class T, std::enable_if_t<
        std::is_arithmetic_v<T>
    >...>
        constexpr auto static abs(T const& x) noexcept
    {
        return x < 0 ? -x : x;
    }
	
    static constexpr inline int eval(const Board& board, const bool ClearedLines, const int nDamageSent)
    {
        int score = 0;
        constexpr auto field_h = VISUALBOARDHEIGHT;
        constexpr auto field_w = BOARDWIDTH;
        std::array<int, BOARDWIDTH> min_y = {};
        struct factor {
            int hole, h_change, y_factor, h_variance, nDamageSent, noattclear;
        } constexpr ai_factor = {
                .hole = -60, 
                .h_change = -5, 
                .y_factor = -10, 
                .h_variance = -10,  
                .nDamageSent = 40, 
                .noattclear = -30
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
        score += int(nDamageSent * ai_factor.nDamageSent * double(nDamageSent) / double(ClearedLines - 0.7));

        if (nDamageSent == 0) {
            score += ClearedLines * ai_factor.noattclear;
        }
        return score;
    }


    static constexpr inline int eval(const BitBoard& board, const bool ClearedLines, const int nDamageSent)
    {
        int score = 0;
        constexpr auto field_h = VISUALBOARDHEIGHT;
        constexpr auto field_w = BOARDWIDTH;
        std::array<int, BOARDWIDTH> min_y = {};
        struct factor {
            int hole, h_change, y_factor, h_variance, nDamageSent, noattclear;
        } constexpr ai_factor = {
                .hole = -60,
                .h_change = -5,
                .y_factor = -10,
                .h_variance = -10,
                .nDamageSent = 40,
                .noattclear = -30
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
                    if (board.getBit(x, y) != minotype::isEmpty) {
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
        score += int(nDamageSent * ai_factor.nDamageSent * double(nDamageSent) / double(ClearedLines - 0.7));
        if (nDamageSent == 0) {
            score += ClearedLines * ai_factor.noattclear;
        }
        return score;
    }


    void waitForStart();
    void EventStart();
    void waitForEnd();
    void EventEnd();
    void backGroundThread();
    void concurrentThread();
    std::jthread bot;
	
    // signal this for the bot to wake back up
    std::condition_variable condVar;
	std::mutex botMux;
    
};