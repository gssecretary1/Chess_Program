#pragma once

#include "chessGameTree.h"

class chessAIClass
{
	PIECE_COLOR color;	// Color that the AI will play as.
	bool isMinPlayer;	// If the AI is playing black, it will be the min player.
	bool isMaxPlayer;	// If the AI is playing white, it will be the max player.

	chessGameTree* gameStateModel;

	action bestMove;

public:
	//	Calling play() will cause the AI to play it's determined best move.
	void play(action moveData);

	//	Calling think() will cause the AI to build the game tree.
	action think();

	//	Calling signal() will alert the AI that the player has moved, and the move data is passed.
	void signal(int origC, int origR, int destC, int destR);
	void signal(const chessBoardClass& chessBoard, action moveData);

	chessAIClass(chessBoardClass& board, PIECE_COLOR C = BLACK, int difficulty = 3);

	gameStateNode* getInitialState() { return gameStateModel->getRootNode(); }
	gameStateNode* getCurrentState() { return gameStateModel->getCurrentNode(); }

	void traverseHistory(PIN_DIR direction) { gameStateModel->traverseGameHistory(direction); }

	~chessAIClass() { delete gameStateModel; }

};


