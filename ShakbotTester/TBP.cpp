#include "Tetris.hpp"
#include <cstdlib>

// frontEnd to bot

// stop
// The stop message tells the bot to stop calculating.
void Tetris::stop() { this->isStop = true; }

// suggest
// The suggest message tells the bot to suggest some next moves in order of preference.
// It is only valid to send this message if the bot is calculating.
// The bot must reply as quickly as possible with a suggestion message containing suggested moves.
suggestion Tetris::suggest() { return {}; }

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
int Tetris::eval(Board board)
{
    return 0;
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
    for (int i = 1; i <= BOARDWIDTH; i++)
    {
        if (i == well)
        {
            continue;
        }
        int32_t dh = board.columnHeights[prev] - abs(board.columnHeights[i]);
        bumpiness += dh;
        bumpiness_sq += dh * dh;
        prev = i;
    }

    return { abs(bumpiness), abs(bumpiness_sq) };
}