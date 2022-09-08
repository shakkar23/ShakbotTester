#include "BitBoard.hpp"
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


consteval bool boardToBitBoardChecks4() {
	Board b;

	// place random pieces
	for (int i = 0; i < 10; i++) {
		Piece p = Piece(PieceType::T, i, i * 6 % 10, RotationDirection(i % 4));
		b.setPiece(p);
	}
	BitBoard bb = BitBoard::fromBoard(b);
	auto linesCleared = 2;
	auto damage = 1;
	return Tetris::eval(bb,linesCleared, damage) == Tetris::eval(b, linesCleared, damage);
}




static_assert(boardToBitBoardChecks());
static_assert(boardToBitBoardChecks2()); 
static_assert(boardToBitBoardChecks3());
static_assert(boardToBitBoardChecks4());