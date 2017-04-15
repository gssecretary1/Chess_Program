#pragma once

#include "chessBoardClass.h"
#include <algorithm>

///	 Various Notes and Thoughts  \\\

/*
=========================
Various Note and Thoughts
=========================
In the game tree, the points associated with each node will be determined by a heuristic function that will
assign some sort of integer value to a new game state, based on how it has changed from the current game state.

The heurisitic function will likely need some tweaking as development goes on to ensure that the AI thinks correctly
about what moves to make, but it will essentially assign an ordering to the moves that are first considered, so as to
more easily and effectively implement alpha-beta pruning for the minimax algorithm.

A game-tree of depth 0 determines the best move, only looking 1 move into the game.  A game-tree of depth 1 determines
the best move , looking 2 moves into the game.

If the root node of the game-tree is for the black player's current position, then all of the edges leading out of the
node represent moves that can be performed.  So at depth 1 we have black's position, black's moves, and white's position.
However, we also have white's moves from that position, since we can generate the action list from a given position without
having to create nodes in the tree.  These actions all have heuristic values associated with their moves, which can be used
to determine how good a move is.  If we go to depth 2, we have black's position/moves, white's positions/counter-moves, and
black's positions/responses to white's counter moves.


=======================
Movement Prioritization
=======================
The moves that are most likely to increase a player's chances of winning are checks and captures, a move that both
captures a piece and checks the king is (possibly) even better.  However, we don't want the AI to consider all possible checks,
since many checks that can be performed over the course of the game tend to be nothing more than "spite" checks or foolish piece
sacrifices that don't amount to anything other than a loss of material.  As such, checks should increase the heuristic value,
but not by too much.

Moves that put a piece closer to the center of the board tend to be good as well, but only to a certain extent.  There are
plenty of occassions in chess where it is better not to hog the center of the board, such as when needing to capture a hanging
piece, or setting up pieces closer to king so that it might be better defended.  A balance needs to be struck between recognizing
the value of moving forward toward the enemy lines and moving toward the center.  Initially these are indistinguishable from one
another, but as more pieces begin to dominate the center the need to push into the enemy ranks may outweigh the need to control
the center of the board.  The heuristic values that I place on these two goals may have a significant impact on how aggressively
or defensively the AI tends to play.  Overall, you cannot win a game of chess by defense alone, so I will likely place a greater
heuristic value on pushing towards the enemy ranks than the pieces change of distance from the center of the board.  An important
thing to note here is that a heuristic penalty will likely need to be placed on moves that act as retreats - moves that pull away
from enemy ranks.

Another useful heuristic may be to track the number of defenders a king has.  This will be useful to promote a defensive play-style,
as an undefended king is a quick way to lead yourself to defeat.  Since a king may only have a total number of eight defenders, and
even eight defenders is overkill, I think a heuristic of (# of defenders / 8) would be a good place to start.  Putting in a bonus
for whether or not the king is castled would be a good way to incentivize such a move.

==============================
Heuristic Function Rough Draft
==============================
Here is a rough sketch of the heuristic function:

H(g1, g2) = (capturedPieceValue) + (isKingCheckedValue) + (changeIn#OfPiecesAttacking/[someConstant]) + (forwardMovementValue)
+ (changeInDistanceFromCenter/[someConstant]) + (kingDefenseRating) + (pawnPromotion)
where g1 is the original game state and g2 is the game state after the move.

The "capturedPieceValue" is pretty straight-forward; each piece has a value assigned to it that, if captured, is added to the heuristic function.
-	A worthy note on this is that a piece's value can change over the course of the game.  See the notes on Dynamic Piece Values below.
The "isKingCheckedValue" may also be augmented by determining how many escape squares the king has available, which might help the AI more accurately checkmate the king.
The "changeIn#OfPiecesAttacking" is self-explanatory.  Changing this to reflect the mean value of the total pieces being attacked may increase aggressiveness.
The "forwardMovementValue" will be a raw calculation that will determine the change in a piece's row value, divided by some constant.
The "changeInDistanceFromCenter" is going to need to be scaled down by [someConstant] to keep it from dominating more important moves, like captures and checks.
The "kingDefenseRating" value will be the addition of values like (# of defenders / 8).
The "pawnPromotion" value will take the value of (promotedPieceValue - pawnValue).

Other heuristics may be added if they are deemed necessary (if the AI makes obvious tactical blunders).  For instance, it is often ill-advised - though sometimes necessary -
to put multiple pawns on the same column.  This can make the end-game a lot more difficult, as it prevents pawns from being able to effectively advance up the board while
protecting adjacent pawns - something that the AI will not know if it decides to stack pawns on the same column early on in the game.

====================
Dynamic Piece Values
====================
Depending on the threat that a piece poses toward the other player, its value can increase.  This is most notably true for pawns; the further they advance
up the board and get closer to being promoted, the more valuable they become.  A pawn on the precipice of promotion can pose a greater threat than a queen that
isn't doing much of anything.  As such, pawns should have their own value modified as they advance from their starting position.  The value of other pieces as the
game goes on is a bit more complicated than that.  There is the common heuristic that two bishops in mid-to-end game are more valuable than two knights, but the
change in value of pieces, especially their value as it is related to their tactical position on the board, is a very difficult thing to analyze and quantify.
As such, I will likely only settle on adding to a pawn's value based on the absolute value of their (currentRow - originRow), divided by some constant.  This should
give the AI the necessary incentive to capture or otherwise stop pawns from advancing.

===============================
Evaluation Function Rough Draft
===============================
Whereas the heuristic function will be used to determine move ordering, the evaluation function will be used at the leaves of the game-tree of depth D to determine the
net change in the game, after the moves represented by the path from the root node (current game state) to the leaf nodes (possible game states after D moves) occur.

It will look something like this:

E(g1, gN) = pawnValue * (numWPawns - numBPawns) + knightValue * (numWKnights - numBKnights) + ... (other pieces) ... + queenValue * (numWQueens - numBQueens)
Where g0 is the current game state and gN is the game state after N moves.

Note:	The function returning a negative value indicates that the moves from g1 to gN result in a net gain in material for black.

This rough draft will very likely be scrapped for something that is more capable of fully analyzing the state of a game.  There is so much more to chess than
just keeping track of net material; if the AI is not capable of accurately evaluating moves, it will seek to capture pieces while disregarding positional strength.

=====================================================
Encouraging Positional Play - Where Should Pieces Go?
=====================================================
Different pieces have different strengths, and their strengths are better used at different positions than others.  For instance, pawns are generally in a better 
position as they move up the board, but pawns near the corners should be encouraged to stay to provide defense to the king.  Knights are best close to the center
of the board, but fare poorly near the edges or in corners.  Bishops are good near the middle, but lack attack value as they get close to the edge or too close
to the enemies back ranks.  Rooks excel at the two extremes of the boards, either the first row or the two enemy back rows (they become useful in assisting with checkmate
in this position), and they also do well if they have a clear shot up a file from the first row, especially through the center.  Queens do best in the center, where their
freedom of movement and attack strength is best used.  Kings and pawns are a bit different in their movement, depending on the state of the game.  During the beginning
and mid-game, kings and pawns near the corners should be encouraged to form defensive structures, but during end-game the king and pawns should be pushing to the other
side of the board in order to promote the remaining pawns.  It is likely that pawns will push forward anyway, since during end-game there are a limited number of pieces,
so positional value may not need to be changed for pawns during the end-game.
Any other pieces that are still on the board during the end-game can likely keep their behavior, as center control is still important.

This implies that a table of values needs to be kept that are used for each type, mapping board positions to positional values for each piece type.

For this project, I will be using the piece-position values that are listed here:  https://chessprogramming.wikispaces.com/Simplified+evaluation+function

Instead of having two tables for each type (one for each color), I'll just look up the table for a type "backward".  So for a white pawn, the table will
be referenced by pawnPosValue[7 - wPawn.row][wPawn.col], while a black pawn will cause the table to be referenced by pawnPosValue[bPawn.row][bPawn.col].
The tables are vertically symmetric through the center, so bType.col doesn't need an modifier to go with it.
*/

struct action
{
	chessPiece* piece;					// Piece to be moved, points to a chessPiece object that is NOT allocated with new.
	int origC, origR, destC, destR;		// Original and destination movement coordinates.
	int heuristic;						// Heuristic value of move.

	enum MOVE_TYPE { BACKWARD, FORWARD, CHECK, DOUBLE_CHECK, CASTLE, CAPTURE, PROMOTION, DRAW };

	std::list<MOVE_TYPE> moveTypeList;

	MOVE_TYPE bestCategory;

	// Constructor for convenience.
	action(chessPiece* p, int c0, int r0, int c1, int r1, double h = 0)
	{
		piece = p; origC = c0; origR = r0; destC = c1; destR = r1; heuristic = h;
	}

	~action() { }
	action() { }
	action(const action& obj);
	void operator=(const action& obj);

};

struct gameStateNode
{
	chessBoardClass gameState;	//	Representation of the chessboard.

	bool isMinNode;	// Evaluated from max player perspective if true, for alpha-beta pruning.
	bool isMaxNode;	// Evaluated from min player perspective if true, for alpha-beta pruning.

	int depthLevel;						//	Will mark the node's depth level in the game tree.
	gameStateNode* previous;			//	Points to the previous game state (parent).
	action bestAction;					//	Will contain the best action that is available for the given game state.
	std::stack<action*> actionList;		//	Will contain the possible moves from the node's game state.
	std::stack<gameStateNode*> next;	//	Points to all of the possible gameStateNodes that represent the board after all possible moves.
	std::list<action*> moveHistory;		//	Points to all of the action structs that occur from the path from the root to the node.

	bool markedForDelete = true;

	~gameStateNode();
	gameStateNode() { }
	gameStateNode(chessBoardClass& board) { gameState = board; }
	gameStateNode(gameStateNode* node) { init(node); }
	void init(gameStateNode* node);
};

//	Contains pure, standard piece values, independent of position.
const struct pieceValues
{
	int pawn = 100;
	int knight = 300;
	int bishop = 300;
	int rook = 500;
	int queen = 900;
	int king = 100000;	// Might not be needed, but included for consistency's sake.
} val;

//	Below are the positional values for each piece type.  They are used for evaluating the quality of a sequence of moves, based on the strength of the piece positions.
const int pawnPosValue[8][8] = { 0,  0,  0,  0,  0,  0,  0,  0,
								50, 50, 50, 50, 50, 50, 50, 50,
								10, 10, 20, 30, 30, 20, 10, 10,
								5,  5, 10, 25, 25, 10,  5,  5,
								0,  0,  0, 20, 20,  0,  0,  0,
								5, -5,-10,  0,  0,-10, -5,  5,
								5, 10, 10,-20,-20, 10, 10,  5,
								0,  0,  0,  0,  0,  0,  0,  0 };

const int knightPosValue[8][8] = {	-50,-40,-30,-30,-30,-30,-40,-50,
									-40,-20,  0,  0,  0,  0,-20,-40,
									-30,  0, 10, 15, 15, 10,  0,-30,
									-30,  5, 15, 20, 20, 15,  5,-30,
									-30,  0, 15, 20, 20, 15,  0,-30,
									-30,  5, 10, 15, 15, 10,  5,-30,
									-40,-20,  0,  5,  5,  0,-20,-40,
									-50,-40,-30,-30,-30,-30,-40,-50 };

const int bishopPosValue[8][8] = {	-20,-10,-10,-10,-10,-10,-10,-20,
									-10,  0,  0,  0,  0,  0,  0,-10,
									-10,  0,  5, 10, 10,  5,  0,-10,
									-10,  5,  5, 10, 10,  5,  5,-10,
									-10,  0, 10, 10, 10, 10,  0,-10,
									-10, 10, 10, 10, 10, 10, 10,-10,
									-10,  5,  0,  0,  0,  0,  5,-10,
									-20,-10,-10,-10,-10,-10,-10,-20 };

const int rookPosValue[8][8] = {	0,  0,  0,  0,  0,  0,  0,  0,
									5, 10, 10, 10, 10, 10, 10,  5,
								   -5,  0,  0,  0,  0,  0,  0, -5,
								   -5,  0,  0,  0,  0,  0,  0, -5,
								   -5,  0,  0,  0,  0,  0,  0, -5,
								   -5,  0,  0,  0,  0,  0,  0, -5,
								   -5,  0,  0,  0,  0,  0,  0, -5,
									0,  0,  0,  5,  5,  0,  0,  0 };

const int queenPosValue[8][8] = {	-20,-10,-10, -5, -5,-10,-10,-20,
									-10,  0,  0,  0,  0,  0,  0,-10,
									-10,  0,  5,  5,  5,  5,  0,-10,
									-5,  0,  5,  5,  5,  5,  0, -5,
									 0,  0,  5,  5,  5,  5,  0, -5,
									-10,  5,  5,  5,  5,  5,  0,-10,
									-10,  0,  5,  0,  0,  0,  0,-10,
									-20,-10,-10, -5, -5,-10,-10,-20 };


const int kingPosValue_MID[8][8] = {	-30,-40,-40,-50,-50,-40,-40,-30,
										-30,-40,-40,-50,-50,-40,-40,-30,
										-30,-40,-40,-50,-50,-40,-40,-30,
										-30,-40,-40,-50,-50,-40,-40,-30,
										-20,-30,-30,-40,-40,-30,-30,-20,
										-10,-20,-20,-20,-20,-20,-20,-10,
										 20, 20,  0,  0,  0,  0, 20, 20,
										 20, 30, 10,  0,  0, 10, 30, 20 };

//	King positional values change during the end game to encourage the AI to move the king up the board to support in pawn promotion.
const int kingPosValue_END[8][8] = {	-50,-40,-30,-20,-20,-30,-40,-50,
										-30,-20,-10,  0,  0,-10,-20,-30,
										-30,-10, 20, 30, 30, 20,-10,-30,
										-30,-10, 30, 40, 40, 30,-10,-30,
										-30,-10, 30, 40, 40, 30,-10,-30,
										-30,-10, 20, 30, 30, 20,-10,-30,
										-30,-30,  0,  0,  0,  0,-30,-30,
										-50,-30,-30,-30,-30,-30,-30,-50 };

class chessGameTree
{
protected:
	

	gameStateNode* root;				// Points to the initial game state node.
	gameStateNode* currentGameState;	// Points to the current (real) game state node.
	int maxDepth;						// Maximum depth level the tree is allowed to be built to, relative to the depth of the currentGameState node.


	//	Generates the game tree and searches it, using a variation of the minimax algorithm that utilizes alpha-beta pruning.
	int negamax(gameStateNode* node, int alpha, int beta, int remainingDepth, int color);

	int getMax(double lhs, double rhs) { if (lhs >= rhs) return lhs; else return rhs; }

	//	Helper function to negamax - creates a child node for the passed node, returning the address of the new gameStateNode.
	gameStateNode* generateChildNode(gameStateNode* node, action* actionData);

	//	Helper function to negamax - returns a vector of actions (an actionList), sorted into descending order from their heuristic values.
	std::stack<action*> generateActionList(gameStateNode* node);

	//	Helper function to negamax - takes a node, chessPiece, and action list and pushes valid actions into the passed action vector.
	void buildActions(gameStateNode* node, chessPiece& piece, std::vector<action*>& actionList);

	//	Helper function to buildActions.  Takes a pointer-to-chessBoard object and pointer-to-action struct and returns a double.
	//	This is an implementation of the heuristic function noted above under Heuristic Function Rough Draft.
	int evaluateHeuristic(chessBoardClass& board, action* moveData);

	//	Helper function to evaluateHeuristic.  Takes a pointer-to-board and pointer-to-action struct and returns a vector containing
	//	pieces that the piece pointed to by the action is attacking.
	std::vector<chessPiece*> getTargets(chessBoardClass& board, action* moveData);

	//	Helper function to evaluateHeuristic.  Takes a pointer-to-board and pointer-to-action struct and returns a vector containing
	//	any pieces that are attacking the enemy king.
	std::vector<chessPiece*> getCheckers(chessBoardClass& board, action* moveData);

	//	Helper function to evaluateHeuristic.  Scans the board to see if the new move would affect the new state's defenderVector if executed.
	//	Returns a vector of pointers-to-chessPieces that are the defenders in the new state.
	std::vector<chessPiece*> getDefenders(chessBoardClass& board, action* moveData, std::vector<std::pair<chessPiece*, PIN_DIR>>* origDefenders);

	double getNetPawnPromotion(chessBoardClass& board, action* moveData);

	//	Helper function to negamax - returns a value indicative of net change in material from currentGameState to a leaf node in the game tree.
	//	This is an implementation of the evaluation function noted above under Evaluation Function Rough Draft.
	//	Note:	Returns a negative value if black gains more material (or loses less) than white from currentGameState to node->gameState.
	//			This should be taken into account when evaluating from the perspective of the min player (black).
	int evaluatePosition(gameStateNode* node);

	//	By default, this will delete all of the gameStateNodes that have their markedForDelete flag set to true.  Uses post-order traversal.
	//  If the 2nd parameter takes a true value, all gameStateNodes will be deleted (will be called in the destructor and upon game reset).
	void cleanUpTree(gameStateNode*& node, bool destroyAll = false);

public:

	//	Builds the game tree and returns the best move the depth of the tree allows it to determine.
	action findBestMove(bool isMaxPlayer);

	//	Signals that a move (moveData) has occured, and to update the game tree accordingly.
	void signalMove(action moveData);
	void signalMove(const chessBoardClass& board, action moveData);

	void traverseGameHistory(PIN_DIR DIR) 
	{ 
		if (DIR == LEFT && currentGameState->previous != NULL) currentGameState = currentGameState->previous; 
		else if (DIR == RIGHT && !currentGameState->next.empty())  currentGameState = currentGameState->next.top(); 
	}

	chessGameTree(chessBoardClass& board, int maxD = 5);

	chessBoardClass& getGameState() { return currentGameState->gameState; }

	gameStateNode* getRootNode() { return root; }
	gameStateNode* getCurrentNode() { return currentGameState; }

	~chessGameTree();
};


