#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"


#include "Tetris.hpp"
#include <cassert>
#include <algorithm>
constexpr auto SCREENHEIGHT = VISUALBOARDHEIGHT * 10;
constexpr auto SCREENWIDTH = BOARDWIDTH * 10;

class Visualizer : public olc::PixelGameEngine {
	Tetris tet;
	int go(const Board &board, int curDepth) {
		if (curDepth == 7) return 1;
		int count = 0; 
		tet.moveBoard->find_moves(board, PieceType::T);
		for (const auto &mv : tet.moveBoard->pieces) {
			Board new_board = board;
			new_board.setPiece(mv.piece);
			count += go(new_board, curDepth +1);
		}
		tet.moveBoard->find_moves(board, PieceType::O);
		for (const auto &mv : tet.moveBoard->pieces) {
			Board new_board = board;
			new_board.setPiece(mv.piece);
			count += go(new_board,curDepth + 1);
		}


		return count;
	}
	void renderQueuePiece(const Piece& piece, uint_fast8_t queueNumber) {
		int sw = ScreenWidth() * (0.90);
		int swRemainder = ScreenWidth() - sw;
		int sh = ScreenHeight() * (0.90);
		int cellSize = sw / BOARDWIDTH;
		int offset = (queueNumber * cellSize);
		for (const auto &mino : piece.piecedef) {
			int xPos = sw + ((mino.x + (2)) * (swRemainder / 5));
			int yPos = ((-mino.y + (2)) * (swRemainder / 5)) + offset;

			auto clr = PieceTypeToColorType(piece.kind);
			olc::Pixel color = olc::WHITE;
			switch (clr)
			{
			case S:
				color = olc::GREEN;
				break;
			case Z:
				color = olc::RED;
				break;
			case J:
				color = olc::BLUE;
				break;
			case L:
				color = { 255,165,0 };
				break;
			case T:
				color = olc::MAGENTA;
				break;
			case O:
				color = olc::YELLOW;
				break;
			case I:
				color = olc::CYAN;
				break;
			case empty:
				color = olc::BLACK;
				break;
			case line_clear:
				color = olc::WHITE;
				break;
			case garbage:
				color = olc::GREY;
				break;
			case number_of_ColorTypes:
			default:
				color = olc::WHITE;
				break;
			}
			for(int x = 0; x < (swRemainder / 5);x++)
				for(int y = 0; y < (swRemainder / 5); y++)
					Draw(xPos +x, yPos +y, color);
		}
		
	}
	void renderPiece(const Piece& piece) {

		int sw = ScreenWidth() * (0.90);
		int sh = ScreenHeight() * (0.90);
		int cellSize = sw / BOARDWIDTH;
		for (const auto& mino : piece.piecedef) {
			for (uint_fast8_t w = 0; w < (sw / BOARDWIDTH); w++)
			{
				for (uint_fast8_t h = 0; h < (sh / VISUALBOARDHEIGHT); h++)
				{
					auto clr = PieceTypeToColorType(piece.kind);
					olc::Pixel color = olc::WHITE;
					switch (clr)
					{
					case S:
						color = olc::GREEN;
						break;
					case Z:
						color = olc::RED;
						break;
					case J:
						color = olc::BLUE;
						break;
					case L:
						color = { 255,165,0 };
						break;
					case T:
						color = olc::MAGENTA;
						break;
					case O:
						color = olc::YELLOW;
						break;
					case I:
						color = olc::CYAN;
						break;
					case empty:
						color = olc::BLACK;
						break;
					case line_clear:
						color = olc::WHITE;
						break;
					case garbage:
						color = olc::GREY;
						break;
					case number_of_ColorTypes:
					default:
						color = olc::WHITE;
						break;
					}
					auto y = (mino.y + piece.y);
					Draw((mino.x + piece.x) * cellSize + w, (VISUALBOARDHEIGHT - (mino.y + piece.y) - 1) * cellSize + h, color);
				}
			}
		}

	}
	void renderBoard(Board& board) {

		int sw = ScreenWidth() * (0.90);
		int sh = ScreenHeight() * (0.90);
		int cellSize = sw / BOARDWIDTH;
		for (int_fast8_t height = VISUALBOARDHEIGHT - 1; height >= 0; height--) {
			for (uint_fast8_t bw = 0; bw < BOARDWIDTH; bw++)
			{
				for (uint_fast8_t w = 0; w < (sw / BOARDWIDTH); w++)
				{
					for (uint_fast8_t h = 0; h < (sh / VISUALBOARDHEIGHT); h++)
					{
						auto piece = board.board[bw][height];
						olc::Pixel color = olc::WHITE;
						switch (piece)
						{
						case S:
							color = olc::GREEN;
							break;
						case Z:
							color = olc::RED;
							break;
						case J:
							color = olc::BLUE;
							break;
						case L:
							color = { 255,165,0 };
							break;
						case T:
							color = olc::MAGENTA;
							break;
						case O:
							color = olc::YELLOW;
							break;
						case I:
							color = olc::CYAN;
							break;
						case empty:
							color = olc::BLACK;
							break;
						case line_clear:
							color = olc::WHITE;
							break;
						case garbage:
							color = olc::GREY;
							break;
						case number_of_ColorTypes:
						default:
							color = olc::WHITE;
							break;
						}
						Draw(bw * cellSize + w, (VISUALBOARDHEIGHT - height - 1) * cellSize + h, color);
					}
				}
			}
		}
	}

	// get all the possible piece placements, playing as the piece you pass in 
	constexpr void fromPlay(const Piece& piece) {

		using namespace std::chrono;

		high_resolution_clock::time_point t1 = high_resolution_clock::now();
		moveSelected = 0;

		tet.moveBoard->find_moves(tet.board, piece);

		high_resolution_clock::time_point t2 = high_resolution_clock::now();
		auto time_span = duration_cast<microseconds>((t2 - t1));

		std::cout << time_span << std::endl;
	}
	
	constexpr void queueShenanigans(const Board  board,const std::array<Piece, 5> &queue) {
		int curPiece = 0;
		Piece emulatedHold = PieceType::empty;
		uint_fast8_t bitmask = 0;
		uint_fast8_t bitmask2 = 0;
		for (int_fast8_t i = 0; i < 4; i++)
		{
			bitmask |= (1 << i);
		}
		std::array<Piece, 5> queueCopy = queue;
		
		while (bitmask2 != bitmask)
		{
			// if piece is out of bounds
			if (bitmask & (1 << curPiece)) {
				curPiece = 0;
				bitmask2++;
				queueCopy = queue;
			}
			//if hold
			if (bitmask2 & (1 << curPiece))
			{
				std::swap(queueCopy.at(curPiece), emulatedHold);
				if (queueCopy.at(curPiece).kind == PieceType::empty)
				{
					curPiece++;
					continue;
				}
			}
			tet.moveBoard->find_moves(board, queueCopy.at(curPiece));
			curPiece++;
		}

	}
	int moveSelected = 0;
	spin spinToCheckFor = None;
public:
	Visualizer() {
		sAppName = "tester";
	}

	bool OnUserCreate() override
	{

		tet.piece = Piece(PieceType::T, 1, 0);
		DrawRect(0,0, ScreenWidth() * (0.90), ScreenHeight() * (0.90));
		renderBoard(tet.board);
		return true;
	}
	bool OnUserUpdate(float fElapsedTime) override {
		auto mouseL = GetMouse(olc::Mouse::LEFT);
		auto mouseR = GetMouse(olc::Mouse::RIGHT);
		std::array<Piece, 5> queue = { PieceType::T,PieceType::I,PieceType::O,PieceType::L,PieceType::J };
		auto mousePos = GetMousePos();
		if (mouseL.bHeld) {
			if (mousePos.x > 0 && mousePos.x < (ScreenWidth()  * (0.90)))
				if (mousePos.y > 0 && mousePos.y < (ScreenHeight() * (0.90)))
				{
					int sw = ScreenWidth() * (0.90);
					int cellSize = sw / BOARDWIDTH;

					int y = VISUALBOARDHEIGHT - (mousePos.y / cellSize) - 1;
					int x = mousePos.x / cellSize;
					tet.board.board[x][y] = O;
				}
			renderBoard(tet.board);

		}
		else if (mouseR.bHeld) {
			if (mousePos.x > 0 && mousePos.x < (ScreenWidth()  * (0.90)))
				if (mousePos.y > 0 && mousePos.y < (ScreenHeight() * (0.90)))
				{
					int sw = ScreenWidth() * (0.90);
					int cellSize = sw / BOARDWIDTH;
					
					int y = VISUALBOARDHEIGHT - (mousePos.y / cellSize) - 1;
					int x = mousePos.x / cellSize;
					tet.board.board[x][y] = empty;
				}
			renderBoard(tet.board);

		}
		if (auto key = GetKey(olc::RIGHT); key.bPressed) {
			moveSelected++;
		}
		else if (auto key = GetKey(olc::LEFT); key.bPressed) {
			moveSelected--;
		}
		else if (auto key = GetKey(olc::SHIFT); key.bPressed) {

			using namespace std::chrono;

			high_resolution_clock::time_point t1 = high_resolution_clock::now();
			int holder = go(tet.board, 0);

			high_resolution_clock::time_point t2 = high_resolution_clock::now();
			auto time_span = duration_cast<microseconds>((t2 - t1));

			std::cout << time_span << std::endl;
		}
		else if (auto key = GetKey(olc::ENTER); key.bPressed) {
			fromPlay(PieceType::T);
			constexpr bool onePieceAtATime = false;

			if (onePieceAtATime) {
				if (tet.moveBoard->pieces.size() != 0)
					renderPiece(tet.moveBoard->pieces.at(moveSelected %= (tet.moveBoard->pieces.size())).piece);
			}
			else {
				if (tet.moveBoard->pieces.size() != 0)
					for (const auto& piece : tet.moveBoard->pieces)
						renderPiece(piece.piece);
			}
		}

		if (auto key = GetKey(olc::K1); key.bPressed)
		{
			spinToCheckFor = Full;
		}
		else if (auto key = GetKey(olc::K2); key.bPressed)
		{
			spinToCheckFor = Mini;
		}
		else if (auto key = GetKey(olc::K3); key.bPressed)
		{
			spinToCheckFor = None;
		}
		for(int i = 0; i < queue.size(); i++)
			renderQueuePiece(queue[i], i);

		return true;
	}
};

int main() {
	Visualizer app;
	if (app.Construct(SCREENWIDTH, SCREENHEIGHT, 4, 4)) {
		app.Start();
	}
	return 0;
}