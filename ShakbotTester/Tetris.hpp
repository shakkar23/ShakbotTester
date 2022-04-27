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
    Piece piece = Piece(PieceType::L);
    std::atomic_int32_t combo;
    bool startBot = false;
    bool playsProcessed = false;
    std::atomic_bool needPlays = false;
    bool endBot = true;
	

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
    const int eval(const   Board& board, const bool ClearedLines, const int nDamageSent) const;
    const int eval(const BitBoard& board, const bool ClearedLines, const int nDamageSent) const;


    void waitForStart();
    void EventStart();
    void waitForEnd();
    void EventEnd();
    void backGroundThread();
    std::jthread bot;
	
    // signal this for the bot to wake back up
    std::condition_variable condVar;
	std::mutex botMux;
};