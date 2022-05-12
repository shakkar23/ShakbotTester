#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"


#include "Tetris.hpp"
#include <cassert>
#include <algorithm>
#include <chrono>
#include <thread>
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
	int BoardScreenWidth() {
		return boardScreenWidth;
	}
	int BoardScreenHeight() {
		return boardScreenHeight;
	}
	void renderQueuePiece(const Piece& piece, uint_fast8_t queueNumber) {
		int sw = BoardScreenWidth();
		int swRemainder = ScreenWidth() - sw;
		int sh = BoardScreenHeight();
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

		int sw = BoardScreenWidth();
		int sh = BoardScreenHeight();
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

		int sw = BoardScreenWidth();
		int sh = BoardScreenHeight();
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
	void queuePlay(std::vector<Piece> queue, Piece hold) {
		auto oldb = tet.board;
		for (int inputIter = 0, pieceIter = 0; (inputIter < tet.botReturnInput.size()) && (pieceIter < queue.size()); ++inputIter, ++pieceIter) {
			spin tSpinned = spin::None;
			// play current piece
			Piece play = queue[pieceIter];
			
			// check if the inputs are hold
			// an entire input sequence is hold
			if (*tet.botReturnInput[inputIter].begin() == inputs::Hold)
			{
				// swap current piece and hold
				std::swap(play, hold);
				// if the current piece is empty then we need to skip it
				if (play.kind == PieceType::empty)
				{
					play = queue[++pieceIter];
				}
				// if the next piece is empty then we need to skip the current inputs as well
				++inputIter;
			}
			Piece piece = playPiece(play, tet.botReturnInput[inputIter], tet.board, tSpinned);
			tet.board.setPiece(piece);
			tet.board.clearLines();
		}

		if(oldb.board == tet.board.board)
		{
			tet.board = oldb;
		}
		renderBoard(tet.board);
	}

	void renderInt(int score) {
		int sw = ScreenWidth();
		int sh = BoardScreenHeight();
		int xPos = 0;
		int yPos = sh+4;
		
		//erase last score
		FillRect(xPos, yPos, sw, sh, olc::BLACK);
		//draw new score
		DrawString(xPos, yPos, std::to_string(score), olc::YELLOW,1);
	}
	

	// get all the possible piece placements, playing as the piece you pass in 
	void bitPlay(const Piece& piece) {

		using namespace std::chrono;

		high_resolution_clock::time_point t1 = high_resolution_clock::now();
		moveSelected = 0;
		BitBoard bitty;
		bitty = bitty.fromBoard(tet.board);

		for (int i = 0; i < 10000; ++i)
			tet.moveBoard->find_moves(bitty, piece);

		high_resolution_clock::time_point t2 = high_resolution_clock::now();
		auto time_span = duration_cast<microseconds>((t2 - t1));

		std::cout << time_span / 10000 << std::endl;
	}
	void normalPlay(const Piece& piece) {

		using namespace std::chrono;

		high_resolution_clock::time_point t1 = high_resolution_clock::now();
		moveSelected = 0;
		BitBoard bitty;
		bitty = bitty.fromBoard(tet.board);

		for (int i = 0; i < 10000; ++i)
			tet.moveBoard->find_moves(bitty, piece);

		high_resolution_clock::time_point t2 = high_resolution_clock::now();
		auto time_span = duration_cast<microseconds>((t2 - t1));

		std::cout << time_span / 10000 << std::endl;
	}
	
	int moveSelected = 0;
	int boardScreenWidth;
	int boardScreenHeight;
	spin spinToCheckFor = None;
public:
	Visualizer() {
		//sAppName = "tester";

	}

	bool OnUserCreate() override
	{
		sAppName = "Shot";
		boardScreenWidth = int(ScreenWidth() * 0.9);
		boardScreenHeight = int(ScreenHeight() * 0.9);

		//tet.piece = Piece(PieceType::T, 1, 0);
		DrawRect(0,0, BoardScreenWidth(), BoardScreenHeight());
		renderBoard(tet.board); 
		
		tet.queue = { PieceType::T,PieceType::I,PieceType::O,PieceType::L,PieceType::J };
		for (int i = 0; i < tet.queue.size(); i++)
			renderQueuePiece(tet.queue[i], i);
		return true;
	}
	bool OnUserUpdate(float fElapsedTime) override {
		auto mouseL = GetMouse(olc::Mouse::LEFT);
		auto mouseR = GetMouse(olc::Mouse::RIGHT);
		auto &mousePos = GetMousePos();
		if (mouseL.bHeld) {
			if (mousePos.x > 0 && mousePos.x < (BoardScreenWidth()))
				if (mousePos.y > 0 && mousePos.y < (BoardScreenHeight()))
				{
					int sw = BoardScreenWidth();
					int cellSize = sw / BOARDWIDTH;

					int y = VISUALBOARDHEIGHT - (mousePos.y / cellSize) - 1;
					int x = mousePos.x / cellSize;
					tet.board.board[x][y] = O;
				}
			renderBoard(tet.board);

		}
		else if (mouseR.bHeld) {
			if (mousePos.x > 0 && mousePos.x < (BoardScreenWidth()))
				if (mousePos.y > 0 && mousePos.y < (BoardScreenHeight()))
				{
					int sw = BoardScreenWidth();
					int cellSize = sw / BOARDWIDTH;
					
					int y = VISUALBOARDHEIGHT - (mousePos.y / cellSize) - 1;
					int x = mousePos.x / cellSize;
					tet.board.board[x][y] = empty;
				}
			renderBoard(tet.board);

		}
		if (GetKey(olc::RIGHT).bPressed) {
			moveSelected++;
		}
		else if (GetKey(olc::LEFT).bPressed) {
			moveSelected--;
		}
		else if (GetKey(olc::SHIFT).bPressed) {
			// stop bot
			tet.EventEnd();
			// sheesh, ima play deez pieces

			for( const auto & pieceInputs :tet.botReturnInput){
				spin tSpinned = spin::None;
				Piece piece = playPiece(tet.piece, pieceInputs, tet.board, tSpinned);
				tet.board.setPiece(piece);
				tet.board.clearLines();
			}
			renderBoard(tet.board);
			
			
		}
		else if (GetKey(olc::ENTER).bPressed) {
			// start bot
			tet.EventStart();
		}

		if (GetKey(olc::K1).bPressed)
		{
			// move the std cout cursor up one line
			std::cout << "\033[1A";
			// clear cli
			std::cout << "\033[2K";
			std::cout << tet.eval(BitBoard::fromBoard(tet.board), 0, 0) << std::endl;
		}
		else if (GetKey(olc::K2).bPressed)
			//for (int i = 0; i < 20; i++) 
			{
			using namespace std::chrono_literals;
			auto start = std::chrono::high_resolution_clock::now();

			std::chrono::seconds timespan(1);

			std::jthread guh([&]() {tet.concurrentThread(); });

			std::this_thread::sleep_for(timespan);
			tet.needPlays = true;
			guh.join();
			// if the bot dies on the first piece the bool for need plays is still true
			tet.needPlays = false;
			renderInt(tet.damage);

			auto end = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double, std::milli> elapsed = end - start;

			std::cout << "Waited " << elapsed.count() << " ms\n";

			queuePlay(tet.queue, tet.hold);
			for (int i = 0; i < tet.queue.size(); i++)
			{
				tet.queue[i] = PieceType(rand() % 7);
				renderQueuePiece(tet.queue[i], i);
			}
			renderBoard(tet.board);
		}
		else if (GetKey(olc::K3).bPressed)
			for (int i = 0; i < 20; i++) {
				using namespace std::chrono_literals;
			auto start = std::chrono::high_resolution_clock::now();
			
			tet.EventStart();
			
			std::chrono::milliseconds timespan(16);
			std::this_thread::sleep_for(timespan);

			tet.EventEnd();

			auto end = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double, std::milli> elapsed = end - start;
			
			std::cout << "Waited " << elapsed.count() << " ms\n";
			
			queuePlay(tet.queue, tet.hold);
			//for (const auto& pieceInputs : tet.botReturnInput) {
			//	spin tSpinned = spin::None;
			//	Piece piece = playPiece(tet.piece, pieceInputs, tet.board, tSpinned);
			//	tet.board.setPiece(piece);
			//	tet.board.clearLines();
			//}
			for (int i = 0; i < tet.queue.size(); i++)
			{
				tet.queue[i] = PieceType(rand()%7);
				renderQueuePiece(tet.queue[i], i);
			}
			renderBoard(tet.board);
		}
		else if (GetKey(olc::K4).bPressed) {
			tet.board.clear();
			renderBoard(tet.board);
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