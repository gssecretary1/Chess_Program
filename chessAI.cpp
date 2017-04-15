#include "chessAI.h"



chessAIClass::chessAIClass(chessBoardClass& board, PIECE_COLOR c, int difficulty)
{
	gameStateModel = new chessGameTree(board, difficulty);

	color = c;

	if (color == WHITE) 
	{
		isMaxPlayer = true;
		isMinPlayer = false;
	}
	else // color == BLACK
	{
		isMaxPlayer = false;
		isMinPlayer = true;
	}
}

void chessAIClass::play(action moveData)
{
	gameStateModel->signalMove(moveData);
}

action chessAIClass::think()
{
	bestMove = gameStateModel->findBestMove(isMaxPlayer);

	return bestMove;
}

void chessAIClass::signal(int origC, int origR, int destC, int destR)
{
	chessPiece* piece = gameStateModel->getGameState().getSquareContents(origC, origR);

	action moveData(piece, origC, origR, destC, destR);

	gameStateModel->signalMove(moveData);
}

void chessAIClass::signal(const chessBoardClass& chessBoard, action moveData)
{
	gameStateModel->signalMove(chessBoard, moveData);
}