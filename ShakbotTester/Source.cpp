#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include <chrono>

#include "Tetris.hpp"
#include <cassert>
constexpr auto SCREENHEIGHT = VISUALBOARDHEIGHT * 10;
constexpr auto SCREENWIDTH = BOARDWIDTH * 10;

class Visualizer : public olc::PixelGameEngine {
	Tetris tet;

	void renderPiece(const Piece& piece) {
		for (const auto& mino : piece.piecedef) {
			for (uint_fast8_t w = 0; w < (100 / BOARDWIDTH); w++)
			{
				for (uint_fast8_t h = 0; h < (200 / VISUALBOARDHEIGHT); h++)
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
						color = (0xff, 165, 0);
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
					Draw((mino.x + piece.x) * 10 + w, (VISUALBOARDHEIGHT - (mino.y + piece.y) - 1) * 10 + h, color);
				}
			}
		}

	}
	void renderBoard(Board& board) {
		for (int_fast8_t height = VISUALBOARDHEIGHT - 1; height >= 0; height--) {
			for (uint_fast8_t bw = 0; bw < BOARDWIDTH; bw++)
			{
				for (uint_fast8_t w = 0; w < (100 / BOARDWIDTH); w++)
				{
					for (uint_fast8_t h = 0; h < (200 / VISUALBOARDHEIGHT); h++)
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
							color = (255, 165, 0);
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
						Draw(bw * 10 + w, (VISUALBOARDHEIGHT - height - 1) * 10 + h, color);
					}
				}
			}
		}
	}

	// get all the possible piece placements, playing as the piece you pass in 
	const void fromPlay(const Piece& piece) {

		using namespace std::chrono;

		high_resolution_clock::time_point t1 = high_resolution_clock::now();
		moveSelected = 0;

		tet.moveBoard->find_moves(tet.board, piece);

		high_resolution_clock::time_point t2 = high_resolution_clock::now();
		auto time_span = duration_cast<microseconds>((t2 - t1));

		std::cout << time_span << std::endl;
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

		return true;
	}
	bool OnUserUpdate(float fElapsedTime) override {
		auto mouseL = GetMouse(olc::Mouse::LEFT);
		auto mouseR = GetMouse(olc::Mouse::RIGHT);
		auto mousePos = GetMousePos();
		if (mouseL.bHeld) {
			if (mousePos.x > 0 && mousePos.x < ScreenWidth())
				if (mousePos.y > 0 && mousePos.y < ScreenHeight())
				{
					int y = VISUALBOARDHEIGHT - (mousePos.y / 10) - 1;
					int x = mousePos.x / BOARDWIDTH;
					tet.board.board[x][y] = O;
				}
		}
		else if (mouseR.bHeld) {
			if (mousePos.x > 0 && mousePos.x < ScreenWidth())
				if (mousePos.y > 0 && mousePos.y < ScreenHeight())
				{
					int y = VISUALBOARDHEIGHT - (mousePos.y / 10) - 1;
					int x = mousePos.x / BOARDWIDTH;
					tet.board.board[x][y] = empty;
				}
		}
		if (auto key = GetKey(olc::RIGHT); key.bPressed) {
			moveSelected++;
		}
		else if (auto key = GetKey(olc::LEFT); key.bPressed) {
			moveSelected--;
		}
		else if (auto key = GetKey(olc::ENTER); key.bPressed) {
			fromPlay(PieceType::T);
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

		renderBoard(tet.board);
		constexpr bool onePieceAtATime = false;
		if (onePieceAtATime) {
			if (tet.moveBoard->pieces.size() != 0)
				renderPiece(tet.moveBoard->pieces.at(moveSelected %= (tet.moveBoard->pieces.size())));
		}
		else {
			if (tet.moveBoard->pieces.size() != 0)
				for (const auto& piece : tet.moveBoard->pieces)
					renderPiece(piece);
		}
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