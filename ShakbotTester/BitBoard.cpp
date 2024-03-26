#include "BitBoard.hpp"

#include "Board.hpp"
#include "Tetris.hpp"
// test if placing an empty piece puts down a mino that isnt empty
consteval bool boardToBitBoardChecks() {
    Piece piece = PieceType::empty;

    Board board;
    BitBoard bb;

    board.setPiece(piece);
    bb.setPiece(piece);

    BitBoard bbtest = BitBoard::fromBoard(board);
    return bb == bbtest;
}

// test if placing down a piece is the same for both boards
consteval bool boardToBitBoardChecks2() {
    Piece piece = Piece(PieceType::T, 4, 6, RotationDirection::East);

    Board board;
    BitBoard bb;

    board.setPiece(piece);
    bb.setPiece(piece);

    BitBoard bbtest = BitBoard::fromBoard(board);
    return bb == bbtest;
}

// test if placing down three pieces and then clearing the neccesary lines is the same for both boards

consteval bool boardToBitBoardChecks3() {
    Piece piece1 = Piece(PieceType::T, 1, 0, RotationDirection::North);
    Piece piece2 = Piece(PieceType::T, 1 + 3, 0, RotationDirection::North);
    Piece piece3 = Piece(PieceType::T, 1 + 3 + 4, 0, RotationDirection::North);
    Piece piece4 = Piece(PieceType::T, 1 + 3 + 4 - 1 - 1, 1, RotationDirection::South);

    Board board;
    BitBoard bb;

    board.setPiece(piece1);
    bb.setPiece(piece1);

    board.setPiece(piece2);
    bb.setPiece(piece2);

    board.setPiece(piece3);
    bb.setPiece(piece3);

    board.setPiece(piece4);
    bb.setPiece(piece4);

    board.clearLines();
    bb.clearLines();

    BitBoard bbtest = BitBoard::fromBoard(board);
    return bb == bbtest;
}

// test if getting the highest point in a board is the same
consteval bool boardToBitBoardChecks4() {
    auto getMaxColumn = [](const Board& board) -> int {
        int max = LOGICALBOARDHEIGHT - 1;
        // while the row is not full checking from top to bottom because misamino backwards upside down board representation
        while (board.rowIsEmpty(max))
            if (max == 0) {
                break;
            } else
                --max;
        return max;
    };
    Board board;

    // place random pieces
    for (int i = 0; i < 10; i++) {
        Piece p = Piece(PieceType::T, i, i * 6 % 10, RotationDirection(i % 4));
        board.setPiece(p);
    }
    BitBoard bb = BitBoard::fromBoard(board);
    const int MaxColumn = getMaxColumn(board);
    return bb.highest_point()-1 == MaxColumn;
}

// test if doing eval on both boards is the same
consteval bool boardToBitBoardChecks5() {
    Board b;

    // place random pieces
    for (int i = 0; i < 10; i++) {
        Piece p = Piece(PieceType::T, i, i * 6 % 10, RotationDirection(i % 4));
        b.setPiece(p);
    }
    BitBoard bb = BitBoard::fromBoard(b);
    auto linesCleared = 2;
    auto damage = 1;
    return Tetris::eval(bb, linesCleared, damage) == Tetris::eval(b, linesCleared, damage);
}

static_assert(boardToBitBoardChecks(), "placing empty pieces is not the same between boards and bitboards");
static_assert(boardToBitBoardChecks2(), "placing pieces is not the same between boards and bitboards");
static_assert(boardToBitBoardChecks3(), "clearing lines is not the same between boards and bitboards");
static_assert(boardToBitBoardChecks4(), "getting the highest point is not the same between boards and bitboards");
static_assert(boardToBitBoardChecks5(), "eval is not the same between boards and bitboards");
