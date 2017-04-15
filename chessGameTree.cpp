#include "chessAI.h"
#include <iostream>

static int totalABMaxCalls = 0;
static int totalABMinCalls = 0;
static int totalBetaCutoffs = 0;
static int totalAlphaCutoffs = 0;

/*	
	=========================================================================
	Helper-Data-Structure Constructors, Destructors, and Overloaded Operators
	=========================================================================
*/

action::action(const action& obj)
{
	this->bestCategory = obj.bestCategory;
	this->destC = obj.destC;
	this->destR = obj.destR;
	this->heuristic = obj.heuristic;
	for (auto itr = obj.moveTypeList.begin(); itr != obj.moveTypeList.end(); ++itr)
		this->moveTypeList.push_back(*itr);
	this->origC = obj.origC;
	this->origR = obj.origR;
	this->piece = obj.piece;
}

void action::operator=(const action& obj)
{
	this->bestCategory = obj.bestCategory;
	this->destC = obj.destC;
	this->destR = obj.destR;
	this->heuristic = obj.heuristic;
	this->moveTypeList.clear();
	for (auto itr = obj.moveTypeList.begin(); itr != obj.moveTypeList.end(); ++itr)
		this->moveTypeList.push_back(*itr);
	this->origC = obj.origC;
	this->origR = obj.origR;
	this->piece = obj.piece;
}

void gameStateNode::init(gameStateNode* node)
{
	this->isMaxNode = !node->isMaxNode;
	this->isMinNode = !this->isMaxNode;
	this->depthLevel = node->depthLevel + 1;
	this->markedForDelete = true;
	this->moveHistory = node->moveHistory;
	this->previous = node;
	this->gameState.init();
	gameState = node->gameState;
}

gameStateNode::~gameStateNode()
{
	while (!actionList.empty())
	{
		action* actionPtr = actionList.top();
		actionList.pop();
		delete actionPtr;
	}

	// delete this->gameState;
}

/*
	========================================
	chessGameTree Constructor and Destructor
	========================================
*/

//	Constructor for chessGameTree, sets the root gameStateNode's board to be equal to the passed board.
chessGameTree::chessGameTree(chessBoardClass& board, int maxD)
{
	root = new gameStateNode();

	root->isMaxNode = true;
	root->isMinNode = false;

	root->markedForDelete = false;
	root->depthLevel = 0;

	root->gameState.init();
	root->gameState = board;
	root->previous = NULL;

	currentGameState = root;
	maxDepth = maxD;
}

//	Destructor for chessGameTree - calls cleanUpTree with a follow-up delete on the root.
chessGameTree::~chessGameTree()
{
	cleanUpTree(root, true);

	delete root;
}

//	Deletes all gameStateNode objects it comes across while traversing the tree that have their markedForDelete flags set to true.
void chessGameTree::cleanUpTree(gameStateNode*& node, bool destroyAll)
{
	if (node->next.size() > 1)
		std::cout << "DEBUG:: Node being cleaned has more than one pointer to another node!" << std::endl;

	while (!node->actionList.empty())
	{
		delete node->actionList.top();
		node->actionList.pop();
	}

	if (node->markedForDelete)
	{
		while (!node->moveHistory.empty())
		{
			delete node->moveHistory.front();
			node->moveHistory.pop_front();
		}
	}
}

/*
	===========================================
	The Meat of the AI - negamax and its caller
	===========================================
*/

action chessGameTree::findBestMove(bool isMaxPlayer)
{
	gameStateNode* node = this->getCurrentNode();

	int color;

	if (node->isMaxNode)
		color = 1;
	else
		color = -1;

	int rootScore = negamax(node, -100000000, 100000000, maxDepth, color);

	return node->bestAction;
}

// Generates the game tree, returning the alpha-beta value for determining the best move.
int chessGameTree::negamax(gameStateNode* node, int alpha, int beta, int remainingDepth, int color)
{
	//	Uses a variation of the minimax algorithm, utilizing alpha-beta pruning, to build and analyze the game-tree 
	//	to a certain depth relative to currentState's depth, returning the best move in the form of an action struct.

	//	Terminal node!
	if (node->gameState.getCheckmate() == true)
	{
		return -color * INT_MAX;
	}
			
	
	//	Return board evaluation value.
	if (remainingDepth == 0 && (maxDepth % 2) == 1)
	{
		return color * evaluatePosition(node);	//	Not entirely sure why, but AI performance is better with this when the difficulty is set to an odd number.
	}
	else if (remainingDepth == 0 && (maxDepth % 2) == 0)
	{
		return -color * evaluatePosition(node);	//	This causes AI performance to increase when difficulty is set to an even number.
	}

		
	///	GENERATE ALL POSSIBLE MOVES AND ORDER THEM FROM MOST PROMISING TO LEAST PROMISING.
	node->actionList = generateActionList(node);

	if (node->actionList.size() == 0)
	{
		std::cout << "DEBUG:: NO ACTIONS HAVE BEEN GENERATED!" << std::endl;

		node->actionList = generateActionList(node);
	}

	//	If generateActionList isn't able to generate valid actions, a placeholder action is pushed onto the stack.
	//	The stack is used as a message carrier to notify the AI that a draw has occured as a result of the move sequence.
	if (node->actionList.top()->bestCategory == action::DRAW)
	{
		return 0;
	}
	
	int bestScore = INT_MIN;

	int score;

	static int noCutOff = 0;
	static int cutOff = 0;
	static int totalMoves = 0;

	totalMoves += node->actionList.size();

	while (!node->actionList.empty())
	{
		/// GENERATE GAMESTATE NODE FOR NEXT MOST PROMISING MOVE.
		gameStateNode* childNode = generateChildNode(node, node->actionList.top());

		//	Generate a new game state for each action.
		if (childNode != NULL)
			node->next.push(childNode);

		if (childNode == NULL)
		{
			action& actionData = *node->actionList.top();

			//	Try the action again so we can trace the bug.
			childNode = generateChildNode(node, node->actionList.top());

			std::cout << "Next child to evaluate is NULL!!!" << std::endl;

			std::cout << "Action Data: ";

			if (actionData.piece->getColor() == WHITE)
				std::cout << "WHITE ";
			else
				std::cout << "BLACK ";

			switch (actionData.piece->getType())
			{
			case PAWN:
				std::cout << "PAWN:\t";
				break;
			case KNIGHT:
				std::cout << "KNIGHT:\t";
				break;
			case BISHOP:
				std::cout << "BISHOP:\t";
				break;
			case ROOK:
				std::cout << "ROOK:\t";
				break;
			case QUEEN:
				std::cout << "QUEEN:\t";
				break;
			case KING:
				std::cout << "KING:\t";
			}

			std::cout << "(" << actionData.origC << ", " << actionData.origR << ") -> "
				<< "(" << actionData.destC << ", " << actionData.destR << ").\n" << std::endl;
		}

		///	RECURSE DOWN THE GAME TREE
		score = -negamax(node->next.top(), -beta, -alpha, remainingDepth - 1, -color);

		/// IF THE GENERATED MOVES THAT LEAD TO LEAF NODE ARE BETTER THAN CURRENT BEST MOVE, UPDATE THE CURRENT BEST MOVE AND ALPHA VALUE.
		if (score > alpha)
		{
			node->bestAction = *node->actionList.top();
			alpha = score;
		}

		delete node->actionList.top();
		node->actionList.pop();
		delete node->next.top();
		node->next.pop();

		///	IF THE LEAF NODE SCORED LARGER THAN THE BETA VALUE, STOP SEARCHING.
		if (score >= beta)
		{
			cutOff++;
			//std::cout << "\n\nDEBUG:: CUT-OFF VALUE = " << cutOff << "\n\n" << std::endl;
			return beta;		// Beta cut-off
		}

		
	}

	//std::cout << "\n\nDEBUG:: NO CUT-OFF VALUE = " << noCutOff << "\n\n" << std::endl;
	//std::cout << "\n\nDEBUG:: CUT-OFF VALUE = " << cutOff << "\n\n" << std::endl;
	//std::cout << "\n\nDEBUG:: TOTAL MOVES = " << totalMoves << "\n\n" << std::endl;

	return alpha;
}

/*
	=========================================
	Game Tree Construction - Worker Functions
	=========================================
*/

gameStateNode* chessGameTree::generateChildNode(gameStateNode* node, action* actionData)
{
	gameStateNode* child = new gameStateNode(node);

	//for (std::list<action*>::iterator itr = node->moveHistory.begin(); itr != node->moveHistory.end(); ++itr)
	//	child->moveHistory.push_back((*itr));	

	//child->gameState = node->gameState;

	/*	Catch up child's game state to the current game state.
	for (std::list<action*>::iterator itr = child->moveHistory.begin(); itr != child->moveHistory.end(); ++itr)
	{
	int origC = (*itr)->origC, origR = (*itr)->origR;
	int destC = (*itr)->destC, destR = (*itr)->destR;

	child->gameState.move(origC, origR, destC, destR);
	}
	*/

	//for (std::list<action*>::iterator itr = child->moveHistory.begin(); itr != child->moveHistory.end(); ++itr)
	//	child->gameState.move((*itr)->origC, (*itr)->origR, (*itr)->destC, (*itr)->destR, false, true);

	if (node->isMaxNode)
		child->gameState.setTurn(WHITE);
	else
		child->gameState.setTurn(BLACK);

	//	If the move is valid, create a child for the move.  Safeguard against invalid actions.
	if (child->gameState.move(actionData->origC, actionData->origR, actionData->destC, actionData->destR, true))
		child->gameState.move(actionData->origC, actionData->origR, actionData->destC, actionData->destR);
	else
	{
		delete child;
		return NULL;
	}

	child->moveHistory.push_back(actionData);

	child->markedForDelete = true;

	child->depthLevel = node->depthLevel + 1;

	child->isMaxNode = !node->isMaxNode;
	child->isMinNode = !node->isMinNode;

	child->previous = node;

	return child;
}

// Populates the a vector of valid actions, sorts the vector in descending order (by heuristic value), and returns it.
std::stack<action*> chessGameTree::generateActionList(gameStateNode* node)
{
	std::vector<action*> actions;

	// Max node == white player
	if (node->isMaxNode)
	{
		// Set pointers to various piece vectors for easy access.
		std::vector<whitePawn>* pawns = node->gameState.getWhitePawns();
		std::vector<whiteKnight>* knights = node->gameState.getWhiteKnights();
		std::vector<whiteBishop>* bishops = node->gameState.getWhiteBishops();
		std::vector<whiteRook>* rooks = node->gameState.getWhiteRooks();
		std::vector<whiteQueen>* queens = node->gameState.getWhiteQueens();
		std::vector<whiteKing>* king = node->gameState.getWhiteKing();

		// Push valid moves into the actionList.
		for (int i = 0; i < pawns->size(); ++i)
			buildActions(node, (*pawns)[i], actions);
		for (int i = 0; i < knights->size(); ++i)
			buildActions(node, (*knights)[i], actions);
		for (int i = 0; i < bishops->size(); ++i)
			buildActions(node, (*bishops)[i], actions);
		for (int i = 0; i < rooks->size(); ++i)
			buildActions(node, (*rooks)[i], actions);
		for (int i = 0; i < queens->size(); ++i)
			buildActions(node, (*queens)[i], actions);

		buildActions(node, (*king)[0], actions);
	}
	else // (node->isMinNode)
	{
		// Set pointers to various piece vectors for easy access.
		std::vector<blackPawn>* pawns = node->gameState.getBlackPawns();
		std::vector<blackKnight>* knights = node->gameState.getBlackKnights();
		std::vector<blackBishop>* bishops = node->gameState.getBlackBishops();
		std::vector<blackRook>* rooks = node->gameState.getBlackRooks();
		std::vector<blackQueen>* queens = node->gameState.getBlackQueens();
		std::vector<blackKing>* king = node->gameState.getBlackKing();

		// Push valid moves into the actionList.
		for (int i = 0; i < pawns->size(); ++i)
			buildActions(node, (*pawns)[i], actions);
		for (int i = 0; i < knights->size(); ++i)
			buildActions(node, (*knights)[i], actions);
		for (int i = 0; i < bishops->size(); ++i)
			buildActions(node, (*bishops)[i], actions);
		for (int i = 0; i < rooks->size(); ++i)
			buildActions(node, (*rooks)[i], actions);
		for (int i = 0; i < queens->size(); ++i)
			buildActions(node, (*queens)[i], actions);

		buildActions(node, (*king)[0], actions);
	}

	// Now that we have our actions, we need to break them up and organize them into separate storage containers.
	std::list<action*> backwardActions;
	std::list<action*> forwardActions;
	std::list<action*> checkActions;
	std::list<action*> doubleCheckActions;
	std::list<action*> castleActions;
	std::list<action*> captureActions;
	std::list<action*> promotionActions;

	//	Organize actions based on their best quality, where...
	//  backwardActions < forwardActions < checkActions < doubleCheckActions < castleActions < captureActions < promotionActions
	while (!actions.empty())
	{
		action::MOVE_TYPE maxMoveType = action::BACKWARD;

		for (std::list<action::MOVE_TYPE>::iterator itr = actions.back()->moveTypeList.begin(); itr != actions.back()->moveTypeList.end(); ++itr)
		{
			if (maxMoveType < (*itr))
				maxMoveType = (*itr);
		}

		actions.back()->bestCategory = maxMoveType;

		switch (maxMoveType)
		{
		case action::BACKWARD:
			backwardActions.push_back(actions.back());
			break;
		case action::FORWARD:
			forwardActions.push_back(actions.back());
			break;
		case action::CHECK:
			checkActions.push_back(actions.back());
			break;
		case action::DOUBLE_CHECK:
			doubleCheckActions.push_back(actions.back());
			break;
		case action::CASTLE:
			castleActions.push_back(actions.back());
			break;
		case action::CAPTURE:
			captureActions.push_back(actions.back());
			break;
		case action::PROMOTION:
			promotionActions.push_back(actions.back());
		}

		actions.pop_back();
	}

	//	The actions are organized, now we need to sort each separate container by ascending order of their heuristic values.
	backwardActions.sort([](action* a1, action* a2)->bool { return a1->heuristic < a2->heuristic; });
	forwardActions.sort([](action* a1, action* a2)->bool { return a1->heuristic < a2->heuristic; });
	checkActions.sort([](action* a1, action* a2)->bool { return a1->heuristic < a2->heuristic; });
	doubleCheckActions.sort([](action* a1, action* a2)->bool { return a1->heuristic < a2->heuristic; });
	castleActions.sort([](action* a1, action* a2)->bool { return a1->heuristic < a2->heuristic; });
	captureActions.sort([](action* a1, action* a2)->bool { return a1->heuristic < a2->heuristic; });
	promotionActions.sort([](action* a1, action* a2)->bool { return a1->heuristic < a2->heuristic; });

	std::stack<action*> actionStack;

	//	Move category containers are sorted, now to push them onto the stack in ascending categorical order (backwardActions first -> promotionActions last)
	while (!backwardActions.empty())
	{
		actionStack.push(backwardActions.front());
		backwardActions.pop_front();
	}

	while (!forwardActions.empty())
	{
		actionStack.push(forwardActions.front());
		forwardActions.pop_front();
	}

	while (!checkActions.empty())
	{
		actionStack.push(checkActions.front());
		checkActions.pop_front();
	}

	while (!doubleCheckActions.empty())
	{
		actionStack.push(doubleCheckActions.front());
		doubleCheckActions.pop_front();
	}

	while (!castleActions.empty())
	{
		actionStack.push(castleActions.front());
		castleActions.pop_front();
	}

	while (!captureActions.empty())
	{
		actionStack.push(captureActions.front());
		captureActions.pop_front();
	}

	while (!promotionActions.empty())
	{
		actionStack.push(promotionActions.front());
		promotionActions.pop_front();
	}

	// It's possible for actionStack to be empty, if the last move resulted in the king and the remaining pieces having no valid moves (like a trapped king with blocked pawns).
	if (actionStack.empty() && node->gameState.getEscapeVector()->empty())
	{
		action* drawData = new action(NULL, -1, -1, -1, -1);
		drawData->bestCategory = action::DRAW;
		actionStack.push(drawData);
	}

	return actionStack;
}

// This will attempt various moves on a passed piece to determine if they should be added to the actionList.
void chessGameTree::buildActions(gameStateNode* node, chessPiece& piece, std::vector<action*>& actionList)
{
	// Notes on buildActions
	/*
	If possible, we will avoid creating a copy of the game-board to see if a move is valid.  As discussed in chessBoardClass.cpp,
	this is really slow, and will be especially costly given that the AI will be building (hundreds of?) thousands of these actions each turn.

	This can be avoided by obtaining the needed data for node->gameState's (chessBoardClass) accessor methods.  Things like
	the checkVector, saviorVector, etc.  We'll essentially be needing to perform the various board checks that chessBoardClass
	performs while seeing if a move is valid, without actually performing the move.

	Alternatively, modifying chessBoardClass's move() method to take a boolean flag that causes it to simply return whether
	or not a move is legal and not actually perform the move could work.  This would perform all of the movement tests
	to ensure the move is legal, while not having to re-copy the gameState of the node for all of the potential children it will create.

	There are some optimizations that can be done to save a bit of time:

	-	If piece->type != "king" && checkVector.size() > 1, then we can immediately return.
	-	If piece->type == "king", then the only actions the king can take are moves in escapeVector.

	-	If (checkVector.size() == 1), then the only pieces that can be moved are those that are in
	saviorVector, and the only squares they can move to are in attackVector and escapeVector (for king only).

	-	A special case should be made for a piece that is an element of pinVector:  only moves that are in the direction of the pin
	should be attempted.  It would be a waste of time to do any others.
	*-*	Would it?  The multi-test movement logic already checks for this.  Is the work testing illegal moves greater than doing the
	pin test twice?  Unsure, as it depends on the number of illegal moves, but I'm going to assume it's better to minimize the
	problem space (we'll test "twice" for pins).

	*/

	chessBoardClass board;
	board.init();

	board = node->gameState;

	PIECE_TYPE type = piece.getType();
	std::vector<chessPiece*>* checkVector = board.getCheckVector();
	std::vector<chessPiece*>* saviorVector = board.getSaviorVector();
	std::vector<std::pair<std::pair<chessPiece*, chessPiece*>, PIN_DIR>>* pinVector = board.getPinVector();
	std::vector<std::pair<int, int>>* attackVector = board.getAttackVector();
	std::vector<std::pair<int, int>>* escapeVector = board.getEscapeVector();

	bool kingInCheck = board.getCheck();

	// If more than two pieces are checking the king, then only the king can be moved.
	if (checkVector->size() > 1 && type != KING)
		return;

	// If one piece is checking the king, only savior pieces or the king may be moved.
	if (kingInCheck && type != KING)
	{
		bool found = false;
		for (int i = 0; i < saviorVector->size() && !found; ++i)
		{
			chessPiece* savior = (*saviorVector)[i];
			if (piece == *savior)
				found = true;
		}

		if (!found)
			return;
	}

	bool isPinned = false;
	PIN_DIR pinDirection = PIN_DIR(-1);

	// See if piece is pinned.
	for (int i = 0; i < pinVector->size(); ++i)
	{
		chessPiece* pinnedPiece = (*pinVector)[i].first.first;
		if (piece == *pinnedPiece)
		{
			isPinned = true;
			pinDirection = (*pinVector)[i].second;
			break;
		}
	}

	/// DEBUG DATA STRUCTURES
	//  Used to store pieces that would be captured if the move occurred, to root out a friendly fire bug.
	std::stack<std::pair<chessPiece*, action*>> debugStack;
	chessPiece* debugPtr;

	//	Flag will be used with chessBoardClass move() calls to test for move legality without actually performing move.
	const bool checkLegality = true;

	int origC = piece.getColumn();
	int origR = piece.getRow();

	action* newAction = NULL;

	//	Test for various cases, adding to actionList where actions are valid.
	if (kingInCheck)
	{
		//	If the piece is the king, then try to move to coordinates in escapeVector.
		//	If the piece is anything else, try to move to coordinates in attackVector.
		//	Note:  attackVector is typically very small, so don't bother checking for pin here.

		if (type == KING)
		{
			for (int i = 0; i < escapeVector->size(); ++i)
			{
				if (board.move(origC, origR, (*escapeVector)[i].first, (*escapeVector)[i].second, checkLegality))
				{

					int destC = (*escapeVector)[i].first, destR = (*escapeVector)[i].second;

					newAction = new action(&piece, origC, origR, destC, destR);
					newAction->heuristic = evaluateHeuristic(board, newAction);

					debugPtr = board.getSquareContents(destC, destR);
					if (debugPtr != NULL)
						debugStack.push(std::make_pair(debugPtr, newAction));

					actionList.push_back(newAction);
				}
			}
		}
		else // The piece in this case is a savior.
		{
			for (int i = 0; i < attackVector->size(); ++i)
			{
				if (board.move(origC, origR, (*attackVector)[i].first, (*attackVector)[i].second, checkLegality))
				{
					int destC = (*attackVector)[i].first, destR = (*attackVector)[i].second;

					newAction = new action(&piece, origC, origR, destC, destR);
					newAction->heuristic = evaluateHeuristic(board, newAction);

					debugPtr = board.getSquareContents(destC, destR);
					if (debugPtr != NULL)
						debugStack.push(std::make_pair(debugPtr, newAction));

					actionList.push_back(newAction);
				}
			}
		}
	}
	else if (type == PAWN)
	{
		//	Possible moves for pawn are very small, so don't bother to check for pin here.
		//	board->move() will do the check for us.

		// White pieces move "up" the board (movement in positive direction across rows).
		if (piece.getColor() == WHITE)
		{
			//	Test movement cases:  If they pass, then create an action struct for it and push it onto the back of actionList.

			//	Case 1:  Forward Movement - 1
			if (board.move(origC, origR, origC, origR + 1, checkLegality))
			{
				newAction = new action(&piece, origC, origR, origC, origR + 1);
				newAction->heuristic = evaluateHeuristic(board, newAction);

				debugPtr = board.getSquareContents(origC, origR + 1);
				if (debugPtr != NULL)
					debugStack.push(std::make_pair(debugPtr, newAction));

				actionList.push_back(newAction);
			}

			//	Case 2:  Forward Movement - 2
			if (board.move(origC, origR, origC, origR + 2, checkLegality))
			{
				newAction = new action(&piece, origC, origR, origC, origR + 2);
				newAction->heuristic = evaluateHeuristic(board, newAction);

				debugPtr = board.getSquareContents(origC, origR + 2);
				if (debugPtr != NULL)
					debugStack.push(std::make_pair(debugPtr, newAction));

				actionList.push_back(newAction);
			}

			//	Case 3:  Diagonal Attack - Up Left
			if (board.move(origC, origR, origC - 1, origR + 1, checkLegality))
			{
				newAction = new action(&piece, origC, origR, origC - 1, origR + 1);
				newAction->heuristic = evaluateHeuristic(board, newAction);

				debugPtr = board.getSquareContents(origC - 1, origR + 1);
				if (debugPtr != NULL)
					debugStack.push(std::make_pair(debugPtr, newAction));

				actionList.push_back(newAction);
			}

			//	Case 4:  Diagonal Attack - Up Right
			if (board.move(origC, origR, origC + 1, origR + 1, checkLegality))
			{
				newAction = new action(&piece, origC, origR, origC + 1, origR + 1);
				newAction->heuristic = evaluateHeuristic(board, newAction);

				debugPtr = board.getSquareContents(origC + 1, origR + 1);
				if (debugPtr != NULL)
					debugStack.push(std::make_pair(debugPtr, newAction));

				actionList.push_back(newAction);
			}

		}	// Black pieces move "down" the board (movement in negative direction across rows).
		else if (piece.getColor() == BLACK)
		{
			// Test movement cases:  If they pass, then create an action struct for it and push it onto the back of actionList.

			//	Case 1:  Forward Movement - 1
			if (board.move(origC, origR, origC, origR - 1, checkLegality))
			{
				newAction = new action(&piece, origC, origR, origC, origR - 1);
				newAction->heuristic = evaluateHeuristic(board, newAction);

				debugPtr = board.getSquareContents(origC, origR - 1);
				if (debugPtr != NULL)
					debugStack.push(std::make_pair(debugPtr, newAction));

				actionList.push_back(newAction);
			}

			//	Case 2:  Forward Movement - 2
			if (board.move(origC, origR, origC, origR - 2, checkLegality))
			{
				newAction = new action(&piece, origC, origR, origC, origR - 2);
				newAction->heuristic = evaluateHeuristic(board, newAction);

				debugPtr = board.getSquareContents(origC, origR - 2);
				if (debugPtr != NULL)
					debugStack.push(std::make_pair(debugPtr, newAction));

				actionList.push_back(newAction);
			}

			//	Case 3:  Diagonal Attack - Down Left
			if (board.move(origC, origR, origC - 1, origR - 1, checkLegality))
			{
				newAction = new action(&piece, origC, origR, origC - 1, origR - 1);
				newAction->heuristic = evaluateHeuristic(board, newAction);

				debugPtr = board.getSquareContents(origC - 1, origR - 1);
				if (debugPtr != NULL)
					debugStack.push(std::make_pair(debugPtr, newAction));

				actionList.push_back(newAction);
			}

			//	Case 4:  Diagonal Attack - Down Right
			if (board.move(origC, origR, origC + 1, origR - 1, checkLegality))
			{
				newAction = new action(&piece, origC, origR, origC + 1, origR - 1);
				newAction->heuristic = evaluateHeuristic(board, newAction);

				debugPtr = board.getSquareContents(origC + 1, origR - 1);
				if (debugPtr != NULL)
					debugStack.push(std::make_pair(debugPtr, newAction));

				actionList.push_back(newAction);
			}
		}
	}
	else if (type == KNIGHT)
	{
		// Knights cannot be moved as pinned pieces, so if the knight is pinned just return.

		if (isPinned)
			return;

		//	Test movement cases

		//	Case 1:  Up 1 - Right 2
		if (board.move(origC, origR, origC + 2, origR + 1, checkLegality))
		{
			newAction = new action(&piece, origC, origR, origC + 2, origR + 1);
			newAction->heuristic = evaluateHeuristic(board, newAction);

			debugPtr = board.getSquareContents(origC + 2, origR + 1);
			if (debugPtr != NULL)
				debugStack.push(std::make_pair(debugPtr, newAction));

			actionList.push_back(newAction);
		}

		//	Case 2:  Up 2 - Right 1
		if (board.move(origC, origR, origC + 1, origR + 2, checkLegality))
		{
			newAction = new action(&piece, origC, origR, origC + 1, origR + 2);
			newAction->heuristic = evaluateHeuristic(board, newAction);

			debugPtr = board.getSquareContents(origC + 1, origR + 2);
			if (debugPtr != NULL)
				debugStack.push(std::make_pair(debugPtr, newAction));

			actionList.push_back(newAction);
		}

		//	Case 3:  Up 2 - Left 1
		if (board.move(origC, origR, origC - 1, origR + 2, checkLegality))
		{
			newAction = new action(&piece, origC, origR, origC - 1, origR + 2);
			newAction->heuristic = evaluateHeuristic(board, newAction);

			debugPtr = board.getSquareContents(origC - 1, origR + 2);
			if (debugPtr != NULL)
				debugStack.push(std::make_pair(debugPtr, newAction));

			actionList.push_back(newAction);
		}

		//	Case 4:  Up 1 - Left 2
		if (board.move(origC, origR, origC - 2, origR + 1, checkLegality))
		{
			newAction = new action(&piece, origC, origR, origC - 2, origR + 1);
			newAction->heuristic = evaluateHeuristic(board, newAction);

			debugPtr = board.getSquareContents(origC - 2, origR + 1);
			if (debugPtr != NULL)
				debugStack.push(std::make_pair(debugPtr, newAction));

			actionList.push_back(newAction);
		}

		//	Case 5:  Down 1 - Left 2
		if (board.move(origC, origR, origC - 2, origR - 1, checkLegality))
		{
			newAction = new action(&piece, origC, origR, origC - 2, origR - 1);
			newAction->heuristic = evaluateHeuristic(board, newAction);

			debugPtr = board.getSquareContents(origC - 2, origR - 1);
			if (debugPtr != NULL)
				debugStack.push(std::make_pair(debugPtr, newAction));

			actionList.push_back(newAction);
		}

		//	Case 6:  Down 2 - Left 1
		if (board.move(origC, origR, origC - 1, origR - 2, checkLegality))
		{
			newAction = new action(&piece, origC, origR, origC - 1, origR - 2);
			newAction->heuristic = evaluateHeuristic(board, newAction);

			debugPtr = board.getSquareContents(origC - 1, origR - 2);
			if (debugPtr != NULL)
				debugStack.push(std::make_pair(debugPtr, newAction));

			actionList.push_back(newAction);
		}

		//	Case 7:  Down 2 - Right 1
		if (board.move(origC, origR, origC + 1, origR - 2, checkLegality))
		{
			newAction = new action(&piece, origC, origR, origC + 1, origR - 2);
			newAction->heuristic = evaluateHeuristic(board, newAction);

			debugPtr = board.getSquareContents(origC + 1, origR - 2);
			if (debugPtr != NULL)
				debugStack.push(std::make_pair(debugPtr, newAction));

			actionList.push_back(newAction);
		}

		//	Case 8:  Down 1 - Right 2
		if (board.move(origC, origR, origC + 2, origR - 1, checkLegality))
		{
			newAction = new action(&piece, origC, origR, origC + 2, origR - 1);
			newAction->heuristic = evaluateHeuristic(board, newAction);

			debugPtr = board.getSquareContents(origC + 2, origR - 1);
			if (debugPtr != NULL)
				debugStack.push(std::make_pair(debugPtr, newAction));

			actionList.push_back(newAction);
		}

	}
	else if (type == BISHOP)
	{
		//	Bishop movement is strictly diagonal, so if pin direction comes from a non-diagonal direction we can just return.
		if (pinDirection == UP || pinDirection == DOWN || pinDirection == LEFT || pinDirection == RIGHT)
			return;

		//	Case 1:  Up - Right Movement
		for (int destC = origC + 1, destR = origR + 1; destC <= 7 && destR <= 7; ++destC, ++destR)
		{
			//	Movement range for bishop is potentially large, so potential pin direction must be checked.
			if (isPinned && pinDirection != UP_RIGHT)
				break;

			if (board.move(origC, origR, destC, destR, checkLegality))
			{
				newAction = new action(&piece, origC, origR, destC, destR);
				newAction->heuristic = evaluateHeuristic(board, newAction);

				debugPtr = board.getSquareContents(destC, destR);
				if (debugPtr != NULL)
					debugStack.push(std::make_pair(debugPtr, newAction));

				actionList.push_back(newAction);
			}
			else // The moment a move becomes illegal, we can stop scanning.
				break;
		}

		//	Case 2:  Up - Left Movement
		for (int destC = origC - 1, destR = origR + 1; destC >= 0 && destR <= 7; --destC, ++destR)
		{
			if (isPinned && pinDirection != UP_LEFT)
				break;

			if (board.move(origC, origR, destC, destR, checkLegality))
			{
				newAction = new action(&piece, origC, origR, destC, destR);
				newAction->heuristic = evaluateHeuristic(board, newAction);

				debugPtr = board.getSquareContents(destC, destR);
				if (debugPtr != NULL)
					debugStack.push(std::make_pair(debugPtr, newAction));

				actionList.push_back(newAction);
			}
			else
				break;
		}

		//	Case 3:  Down - Left Movement
		for (int destC = origC - 1, destR = origR - 1; destC >= 0 && destR >= 0; --destC, --destR)
		{
			if (isPinned && pinDirection != DOWN_LEFT)
				break;

			if (board.move(origC, origR, destC, destR, checkLegality))
			{
				newAction = new action(&piece, origC, origR, destC, destR);
				newAction->heuristic = evaluateHeuristic(board, newAction);

				debugPtr = board.getSquareContents(destC, destR);
				if (debugPtr != NULL)
					debugStack.push(std::make_pair(debugPtr, newAction));

				actionList.push_back(newAction);
			}
			else
				break;
		}

		//	Case 4:  Down - Right Movement
		for (int destC = origC + 1, destR = origR - 1; destC <= 7 && destR >= 0; ++destC, --destR)
		{
			if (isPinned && pinDirection != DOWN_RIGHT)
				break;

			if (board.move(origC, origR, destC, destR, checkLegality))
			{
				newAction = new action(&piece, origC, origR, destC, destR);
				newAction->heuristic = evaluateHeuristic(board, newAction);

				debugPtr = board.getSquareContents(destC, destR);
				if (debugPtr != NULL)
					debugStack.push(std::make_pair(debugPtr, newAction));

				actionList.push_back(newAction);
			}
			else
				break;
		}
	}
	else if (type == ROOK)
	{
		//	Rook movement is strictly non-diagonal, so if pin direction comes from a diagonal direction we can just return.
		if (pinDirection == UP_RIGHT || pinDirection == UP_LEFT || pinDirection == DOWN_LEFT || pinDirection == DOWN_RIGHT)
			return;

		//	Case 1:  Right Movement
		for (int destC = origC + 1, destR = origR; destC <= 7; ++destC)
		{
			//	Movement range for rook is potentially large, so potential pin direction must be checked.
			if (isPinned && pinDirection != RIGHT)
				break;

			if (board.move(origC, origR, destC, destR, checkLegality))
			{
				newAction = new action(&piece, origC, origR, destC, destR);
				newAction->heuristic = evaluateHeuristic(board, newAction);

				debugPtr = board.getSquareContents(destC, destR);
				if (debugPtr != NULL)
					debugStack.push(std::make_pair(debugPtr, newAction));

				actionList.push_back(newAction);
			}
			else // The moment a move becomes illegal, we can stop scanning.
				break;
		}

		//	Case 2:  Upward Movement
		for (int destC = origC, destR = origR + 1; destR <= 7; ++destR)
		{
			if (isPinned && pinDirection != UP)
				break;

			if (board.move(origC, origR, destC, destR, checkLegality))
			{
				newAction = new action(&piece, origC, origR, destC, destR);
				newAction->heuristic = evaluateHeuristic(board, newAction);

				debugPtr = board.getSquareContents(destC, destR);
				if (debugPtr != NULL)
					debugStack.push(std::make_pair(debugPtr, newAction));

				actionList.push_back(newAction);
			}
			else
				break;
		}

		//	Case 3:  Left Movement
		for (int destC = origC - 1, destR = origR; destC >= 0; --destC)
		{
			if (isPinned && pinDirection != LEFT)
				break;

			if (board.move(origC, origR, destC, destR, checkLegality))
			{
				newAction = new action(&piece, origC, origR, destC, destR);
				newAction->heuristic = evaluateHeuristic(board, newAction);

				debugPtr = board.getSquareContents(destC, destR);
				if (debugPtr != NULL)
					debugStack.push(std::make_pair(debugPtr, newAction));

				actionList.push_back(newAction);
			}
			else
				break;
		}

		//	Case 4:  Downward Movement
		for (int destC = origC, destR = origR - 1; destR >= 0; --destR)
		{
			if (isPinned && pinDirection != DOWN)
				break;

			if (board.move(origC, origR, destC, destR, checkLegality))
			{
				newAction = new action(&piece, origC, origR, destC, destR);
				newAction->heuristic = evaluateHeuristic(board, newAction);

				debugPtr = board.getSquareContents(destC, destR);
				if (debugPtr != NULL)
					debugStack.push(std::make_pair(debugPtr, newAction));

				actionList.push_back(newAction);
			}
			else
				break;
		}
	}
	else if (type == QUEEN)
	{
		//	Case 1:  Right Movement
		for (int destC = origC + 1, destR = origR; destC <= 7; ++destC)
		{
			//	Movement range for queen is potentially large, so potential pin direction must be checked.
			if (isPinned && pinDirection != RIGHT)
				break;

			if (board.move(origC, origR, destC, destR, checkLegality))
			{
				newAction = new action(&piece, origC, origR, destC, destR);
				newAction->heuristic = evaluateHeuristic(board, newAction);

				debugPtr = board.getSquareContents(destC, destR);
				if (debugPtr != NULL)
					debugStack.push(std::make_pair(debugPtr, newAction));

				actionList.push_back(newAction);
			}
			else // The moment a move becomes illegal, we can stop scanning.
				break;
		}

		//	Case 2:  Up - Right Movement
		for (int destC = origC + 1, destR = origR + 1; destC <= 7 && destR <= 7; ++destC, ++destR)
		{
			if (isPinned && pinDirection != UP_RIGHT)
				break;

			if (board.move(origC, origR, destC, destR, checkLegality))
			{
				newAction = new action(&piece, origC, origR, destC, destR);
				newAction->heuristic = evaluateHeuristic(board, newAction);

				debugPtr = board.getSquareContents(destC, destR);
				if (debugPtr != NULL)
					debugStack.push(std::make_pair(debugPtr, newAction));

				actionList.push_back(newAction);
			}
			else
				break;
		}

		//	Case 3:  Upward Movement
		for (int destC = origC, destR = origR + 1; destR <= 7; ++destR)
		{
			if (isPinned && pinDirection != UP)
				break;

			if (board.move(origC, origR, destC, destR, checkLegality))
			{
				newAction = new action(&piece, origC, origR, destC, destR);
				newAction->heuristic = evaluateHeuristic(board, newAction);

				debugPtr = board.getSquareContents(destC, destR);
				if (debugPtr != NULL)
					debugStack.push(std::make_pair(debugPtr, newAction));

				actionList.push_back(newAction);
			}
			else
				break;
		}

		//	Case 4:  Up - Left Movement
		for (int destC = origC - 1, destR = origR + 1; destC >= 0 && destR <= 7; --destC, ++destR)
		{
			if (isPinned && pinDirection != UP_LEFT)
				break;

			if (board.move(origC, origR, destC, destR, checkLegality))
			{
				newAction = new action(&piece, origC, origR, destC, destR);
				newAction->heuristic = evaluateHeuristic(board, newAction);

				debugPtr = board.getSquareContents(destC, destR);
				if (debugPtr != NULL)
					debugStack.push(std::make_pair(debugPtr, newAction));

				actionList.push_back(newAction);
			}
			else
				break;
		}

		//	Case 5:  Left Movement
		for (int destC = origC - 1, destR = origR; destC >= 0; --destC)
		{
			if (isPinned && pinDirection != LEFT)
				break;

			if (board.move(origC, origR, destC, destR, checkLegality))
			{
				newAction = new action(&piece, origC, origR, destC, destR);
				newAction->heuristic = evaluateHeuristic(board, newAction);

				debugPtr = board.getSquareContents(destC, destR);
				if (debugPtr != NULL)
					debugStack.push(std::make_pair(debugPtr, newAction));

				actionList.push_back(newAction);
			}
			else
				break;
		}

		//	Case 6:  Down - Left Movement
		for (int destC = origC - 1, destR = origR - 1; destC >= 0 && destR >= 0; --destC, --destR)
		{
			if (isPinned && pinDirection != DOWN_LEFT)
				break;

			if (board.move(origC, origR, destC, destR, checkLegality))
			{
				newAction = new action(&piece, origC, origR, destC, destR);
				newAction->heuristic = evaluateHeuristic(board, newAction);

				debugPtr = board.getSquareContents(destC, destR);
				if (debugPtr != NULL)
					debugStack.push(std::make_pair(debugPtr, newAction));

				actionList.push_back(newAction);
			}
			else
				break;
		}

		//	Case 7:  Downward Movement
		for (int destC = origC, destR = origR - 1; destR >= 0; --destR)
		{
			if (isPinned && pinDirection != DOWN)
				break;

			if (board.move(origC, origR, destC, destR, checkLegality))
			{
				newAction = new action(&piece, origC, origR, destC, destR);
				newAction->heuristic = evaluateHeuristic(board, newAction);

				debugPtr = board.getSquareContents(destC, destR);
				if (debugPtr != NULL)
					debugStack.push(std::make_pair(debugPtr, newAction));

				actionList.push_back(newAction);
			}
			else
				break;
		}

		//	Case 8:  Down - Right Movement
		for (int destC = origC + 1, destR = origR - 1; destC <= 7 && destR >= 0; ++destC, --destR)
		{
			if (isPinned && pinDirection != DOWN_RIGHT)
				break;

			if (board.move(origC, origR, destC, destR, checkLegality))
			{
				newAction = new action(&piece, origC, origR, destC, destR);
				newAction->heuristic = evaluateHeuristic(board, newAction);

				debugPtr = board.getSquareContents(destC, destR);
				if (debugPtr != NULL)
					debugStack.push(std::make_pair(debugPtr, newAction));

				actionList.push_back(newAction);
			}
			else
				break;
		}
	}
	else // (type == "king")
	{
		//	King cannot be pinned, so don't bother checking for pin case.

		//	Special Cases:  Castling

		//	Castle Queen-side
		if (board.move(origC, origR, origC - 2, origR, checkLegality))
		{
			newAction = new action(&piece, origC, origR, origC - 2, origR);
			newAction->heuristic = evaluateHeuristic(board, newAction);

			debugPtr = board.getSquareContents(origC - 2, origR);
			if (debugPtr != NULL)
				debugStack.push(std::make_pair(debugPtr, newAction));

			actionList.push_back(newAction);
		}

		//	Castle King-side
		if (board.move(origC, origR, origC + 2, origR, checkLegality))
		{
			newAction = new action(&piece, origC, origR, origC + 2, origR);
			newAction->heuristic = evaluateHeuristic(board, newAction);

			debugPtr = board.getSquareContents(origC + 2, origR);
			if (debugPtr != NULL)
				debugStack.push(std::make_pair(debugPtr, newAction));

			actionList.push_back(newAction);
		}

		//	In these cases, the king is not in check, so all eight movements are potentially safe.

		//	Case 1:  Right Movement
		if (board.move(origC, origR, origC + 1, origR, checkLegality))
		{
			newAction = new action(&piece, origC, origR, origC + 1, origR);
			newAction->heuristic = evaluateHeuristic(board, newAction);

			debugPtr = board.getSquareContents(origC + 1, origR);
			if (debugPtr != NULL)
				debugStack.push(std::make_pair(debugPtr, newAction));

			actionList.push_back(newAction);
		}

		//	Case 2:  Up - Right Movement
		if (board.move(origC, origR, origC + 1, origR + 1, checkLegality))
		{
			newAction = new action(&piece, origC, origR, origC + 1, origR + 1);
			newAction->heuristic = evaluateHeuristic(board, newAction);

			debugPtr = board.getSquareContents(origC + 1, origR + 1);
			if (debugPtr != NULL)
				debugStack.push(std::make_pair(debugPtr, newAction));

			actionList.push_back(newAction);
		}

		//	Case 3:  Upward Movement
		if (board.move(origC, origR, origC, origR + 1, checkLegality))
		{
			newAction = new action(&piece, origC, origR, origC, origR + 1);
			newAction->heuristic = evaluateHeuristic(board, newAction);

			debugPtr = board.getSquareContents(origC, origR + 1);
			if (debugPtr != NULL)
				debugStack.push(std::make_pair(debugPtr, newAction));

			actionList.push_back(newAction);
		}

		//	Case 4:  Up - Left Movement
		if (board.move(origC, origR, origC - 1, origR + 1, checkLegality))
		{
			newAction = new action(&piece, origC, origR, origC - 1, origR + 1);
			newAction->heuristic = evaluateHeuristic(board, newAction);

			debugPtr = board.getSquareContents(origC - 1, origR + 1);
			if (debugPtr != NULL)
				debugStack.push(std::make_pair(debugPtr, newAction));

			actionList.push_back(newAction);
		}

		//	Case 5:  Left Movement
		if (board.move(origC, origR, origC - 1, origR, checkLegality))
		{
			newAction = new action(&piece, origC, origR, origC - 1, origR);
			newAction->heuristic = evaluateHeuristic(board, newAction);

			debugPtr = board.getSquareContents(origC - 1, origR);
			if (debugPtr != NULL)
				debugStack.push(std::make_pair(debugPtr, newAction));

			actionList.push_back(newAction);
		}

		//	Case 6:  Down - Left Movement
		if (board.move(origC, origR, origC - 1, origR - 1, checkLegality))
		{
			newAction = new action(&piece, origC, origR, origC - 1, origR - 1);
			newAction->heuristic = evaluateHeuristic(board, newAction);

			debugPtr = board.getSquareContents(origC - 1, origR - 1);
			if (debugPtr != NULL)
				debugStack.push(std::make_pair(debugPtr, newAction));

			actionList.push_back(newAction);
		}

		//	Case 7:  Downward Movement
		if (board.move(origC, origR, origC, origR - 1, checkLegality))
		{
			newAction = new action(&piece, origC, origR, origC, origR - 1);
			newAction->heuristic = evaluateHeuristic(board, newAction);

			debugPtr = board.getSquareContents(origC, origR - 1);
			if (debugPtr != NULL)
				debugStack.push(std::make_pair(debugPtr, newAction));

			actionList.push_back(newAction);
		}

		//	Case 8:  Down - Right Movement
		if (board.move(origC, origR, origC + 1, origR - 1, checkLegality))
		{
			newAction = new action(&piece, origC, origR, origC + 1, origR - 1);
			newAction->heuristic = evaluateHeuristic(board, newAction);

			debugPtr = board.getSquareContents(origC + 1, origR - 1);
			if (debugPtr != NULL)
				debugStack.push(std::make_pair(debugPtr, newAction));

			actionList.push_back(newAction);
		}
	}

	while (!debugStack.empty())
	{
		debugPtr = debugStack.top().first;
		action* aPtr = debugStack.top().second;

		if (debugPtr->getColor() != piece.getColor())
		{
			debugStack.pop();
			continue;
		}

		std::cout << "DEBUG:: FRIENDLY FIRE ACTION GENERATED!!!" << std::endl;
		std::cout << "Piece: ";

		switch (piece.getColor())
		{
		case WHITE:
			std::cout << "WHITE ";
			break;
		case BLACK:
			std::cout << "BLACK ";
		}

		switch (piece.getType())
		{
		case PAWN:
			std::cout << "PAWN";
			break;
		case KNIGHT:
			std::cout << "KNIGHT";
			break;
		case BISHOP:
			std::cout << "BISHOP";
			break;
		case ROOK:
			std::cout << "ROOK";
			break;
		case QUEEN:
			std::cout << "QUEEN";
			break;
		case KING:
			std::cout << "KING";
			break;
		}

		std::cout << std::endl << "Target: ";

		chessPiece* target = board.getSquareContents(aPtr->destC, aPtr->destR);

		switch (target->getColor())
		{
		case WHITE:
			std::cout << "WHITE ";
			break;
		case BLACK:
			std::cout << "BLACK ";
		}

		switch (target->getType())
		{
		case PAWN:
			std::cout << "PAWN";
			break;
		case KNIGHT:
			std::cout << "KNIGHT";
			break;
		case BISHOP:
			std::cout << "BISHOP";
			break;
		case ROOK:
			std::cout << "ROOK";
			break;
		case QUEEN:
			std::cout << "QUEEN";
			break;
		case KING:
			std::cout << "KING";
			break;
		}

		std::cout << std::endl;
		std::cout << "(" << aPtr->origC << ", " << aPtr->origR << ") -> (" << aPtr->destC << ", " << aPtr->destR << ")." << std::endl;

		if (board.move(aPtr->origC, aPtr->origR, aPtr->destC, aPtr->destR, true))
			std::cout << "Re-evaluation has returned true!!!" << std::endl;

		std::cout << std::endl;
	}

	//	There's been a problem where pieces are allowed to capture the king.
	//	Solution:  Discard moves where the destination square is occupied by a king.
	//	This is really not the core problem, as the king shouldn't even be able to move to a square that is under attack.
	//	This will be addressed above in kingInCheck.

	return;
}

//	Evaluates a leaf node of the game tree, returning an integer value that represents how favorable of a game state the moves that lead to the leaf lead to.
int chessGameTree::evaluatePosition(gameStateNode* node)
{
	//	Goal state.
	if (node->gameState.getCheckmate())
		return INT_MAX;

	// Evaluate difference in material in potential game state.
	chessBoardClass& potentialGame = node->gameState;

	int potentialPawnDiff = (potentialGame.getWhitePawns()->size() - potentialGame.getBlackPawns()->size());
	int potentialKnightDiff = (potentialGame.getWhiteKnights()->size() - potentialGame.getBlackKnights()->size());
	int potentialBishopDiff = (potentialGame.getWhiteBishops()->size() - potentialGame.getBlackBishops()->size());
	int potentialRookDiff = (potentialGame.getWhiteRooks()->size() - potentialGame.getBlackRooks()->size());
	int potentialQueenDiff = (potentialGame.getWhiteQueens()->size() - potentialGame.getBlackQueens()->size());

	int potentialDiff = val.queen * (potentialQueenDiff)+val.rook * (potentialRookDiff)+val.bishop * (potentialBishopDiff)
		+val.knight * (potentialKnightDiff)+val.pawn * (potentialPawnDiff);


	int netMaterialChange = potentialDiff;

	/*
		We'll differentiate between early/midgame and endgame by the following:

		Endgame:	The following remaining-piece scenarios mean the game is in its endgame state.

		1)	One queen on either/both sides and all minor pieces gone (knights, bishops, and rooks).
		2)	No queens and only two minor pieces or less on either side (knights, bishops, and rooks).

		The king uses a difference position value array for midgame and endgame, to encourage the king to push up to
		support pawn promotion.
	*/

	bool midgame = false;
	bool endgame = false;

	if (potentialGame.getWhiteQueens()->size() >= 1 || potentialGame.getBlackQueens()->size() >= 1)
	{
		if (potentialGame.getWhiteKnights()->empty() && potentialGame.getWhiteBishops()->empty() && potentialGame.getWhiteRooks()->empty())
		{
			if (potentialGame.getBlackKnights()->empty() && potentialGame.getBlackBishops()->empty() && potentialGame.getBlackRooks()->empty())
				endgame = true;
		}
	}
	else if (potentialGame.getWhiteQueens()->empty() && potentialGame.getBlackQueens()->empty())
	{
		if ((potentialGame.getWhiteKnights()->size() + potentialGame.getWhiteBishops()->size() + potentialGame.getWhiteRooks()->size()) <= 2)
			if ((potentialGame.getBlackKnights()->size() + potentialGame.getBlackBishops()->size() + potentialGame.getBlackRooks()->size()) <= 2)
				endgame = true;
	}

	if (!endgame) midgame = true;

	//	Will hold white positional values.
	int whitePosValue = 0;

	//	Will assess pawn positional value, for each pawn.
	for (auto itr = potentialGame.getWhitePawns()->begin(); itr != potentialGame.getWhitePawns()->end(); ++itr)
	{
		int col = (*itr).getColumn();
		int row = (*itr).getRow();

		//	I designed the chessBoard in a way that its indexed by [col][row], but I don't feel like transpoing all of the piecePosValue arrays.
		//	Because of this, the arrays will need to be indexed by row first, then column.
		whitePosValue += pawnPosValue[7 - row][col];
	}

	//	Will assess knight positional value, for each knight.
	for (auto itr = potentialGame.getWhiteKnights()->begin(); itr != potentialGame.getWhiteKnights()->end(); ++itr)
	{
		int col = (*itr).getColumn();
		int row = (*itr).getRow();

		whitePosValue += knightPosValue[7 - row][col];
	}

	//	Will assess bishop positional value, for each bishop.
	for (auto itr = potentialGame.getWhiteBishops()->begin(); itr != potentialGame.getWhiteBishops()->end(); ++itr)
	{
		int col = (*itr).getColumn();
		int row = (*itr).getRow();

		whitePosValue += bishopPosValue[7 - row][col];
	}

	//	Will assess rooks positional value, for each rooks.
	for (auto itr = potentialGame.getWhiteRooks()->begin(); itr != potentialGame.getWhiteRooks()->end(); ++itr)
	{
		int col = (*itr).getColumn();
		int row = (*itr).getRow();

		whitePosValue += rookPosValue[7 - row][col];
	}

	//	Will assess queens positional value, for each queens.
	for (auto itr = potentialGame.getWhiteQueens()->begin(); itr != potentialGame.getWhiteQueens()->end(); ++itr)
	{
		int col = (*itr).getColumn();
		int row = (*itr).getRow();

		whitePosValue += queenPosValue[7 - row][col];
	}

	//	Will assess king's positional value, for king.
	if (midgame)
	{
		whitePosValue += kingPosValue_MID[7 - potentialGame.getWhiteKing()->front().getRow()][potentialGame.getWhiteKing()->front().getColumn()];
	}
	else if (endgame)
	{
		whitePosValue += kingPosValue_END[7 - potentialGame.getWhiteKing()->front().getRow()][potentialGame.getWhiteKing()->front().getColumn()];
	}

	int blackPosValue = 0;

	//	Rinse and repeat for black vectors.
	for (auto itr = potentialGame.getBlackPawns()->begin(); itr != potentialGame.getBlackPawns()->end(); ++itr)
	{
		int col = (*itr).getColumn();
		int row = (*itr).getRow();

		blackPosValue += pawnPosValue[row][col];
	}

	for (auto itr = potentialGame.getBlackKnights()->begin(); itr != potentialGame.getBlackKnights()->end(); ++itr)
	{
		int col = itr->getColumn();
		int row = itr->getRow();

		blackPosValue += knightPosValue[row][col];
	}

	for (auto itr = potentialGame.getBlackBishops()->begin(); itr != potentialGame.getBlackBishops()->end(); ++itr)
	{
		int col = itr->getColumn();
		int row = itr->getRow();

		blackPosValue += bishopPosValue[row][col];
	}

	for (auto itr = potentialGame.getBlackRooks()->begin(); itr != potentialGame.getBlackRooks()->end(); ++itr)
	{
		int col = itr->getColumn();
		int row = itr->getRow();

		blackPosValue += rookPosValue[row][col];
	}

	for (auto itr = potentialGame.getBlackQueens()->begin(); itr != potentialGame.getBlackQueens()->end(); ++itr)
	{
		int col = itr->getColumn();
		int row = itr->getRow();

		blackPosValue += queenPosValue[row][col];
	}

	if (midgame)
	{
		blackPosValue += kingPosValue_MID[potentialGame.getBlackKing()->front().getRow()][potentialGame.getBlackKing()->front().getColumn()];
	}
	else if (endgame)
	{
		blackPosValue += kingPosValue_END[potentialGame.getBlackKing()->front().getRow()][potentialGame.getBlackKing()->front().getColumn()];
	}

	int color;

	if (potentialGame.getTurn() == WHITE)
		color = 1;
	else
		color = -1;

	int netPosValue = whitePosValue - blackPosValue;


	return color * (netMaterialChange + netPosValue);
}

// Returns an int value that represents how promising a move is based on the heuristic function from the notes in chessAI.h
int chessGameTree::evaluateHeuristic(chessBoardClass& board, action* moveData)
{
	//double captureVal, attackVal, checkVal, forwardMovementVal, centerControlVal, kingDefenseVal, pawnPromotion;
	int captureVal, checkVal, kingDefenseVal, forwardMovementVal, pawnPromotion, positionVal;

	captureVal = 0;

	//	Determine captureVal
	if (board.getSquareContents(moveData->destC, moveData->destR) != NULL)
	{
		chessPiece* capture = board.getSquareContents(moveData->destC, moveData->destR);
		PIECE_TYPE type = capture->getType();

		moveData->moveTypeList.push_back(action::CAPTURE);

		//	Threat of pawn incrases as it approaches promotion.
		if (type == PAWN)
		{
			if (capture->getColor() == WHITE)
			{
				if (capture->getRow() == 5)
					captureVal = 4 * val.pawn;
				else if (capture->getRow() == 6)
					captureVal = 6 * val.pawn;
			}
			else if (capture->getColor() == BLACK)
			{
				if (capture->getRow() == 2)
					captureVal = 4 * val.pawn;
				else if (capture->getRow() == 1)
					captureVal = 6 * val.pawn;
			}
		}
		else
		{
			if (type == KNIGHT)
				captureVal = val.knight;
			else if (type == BISHOP)
				captureVal = val.bishop;
			else if (type == ROOK)
				captureVal = val.rook;
			else if (type == QUEEN)
				captureVal = val.queen;
		}

		//	We want to avoid mindlessly capturing pieces (we don't want our queen to capture pawns, only to get captured herself), so
		//	we'll augment captureVal by subtracting from it a weighted value of the piece that is being moved.
		int pieceVal;

		switch (moveData->piece->getType())
		{
		case PAWN:
			pieceVal = val.pawn;
			break;
		case KNIGHT:
			pieceVal = val.knight;
			break;
		case BISHOP:
			pieceVal = val.bishop;
			break;
		case ROOK:
			pieceVal = val.rook;
			break;
		case QUEEN:
			pieceVal = val.queen;
			break;
		case KING:
			pieceVal = val.king / 75;	//	King value is huge, so we'll scale it down a bit so as to not discourage the offensive use of the king too strongly.
		}

		captureVal -= pieceVal;
	}

	//	attackVal has been phased out for the moment.
	/*
	//	Determine attackVal

	//	Get number of pieces being attacked from original position.
	std::vector<chessPiece*> origTargets = getTargets(board, moveData);
	//	Get number of pieces being attacked from new position.


	action* postMoveData = new action(moveData->piece, moveData->destC, moveData->destR, -1, -1);
	std::vector<chessPiece*> newTargets = getTargets(board, postMoveData);
	delete postMoveData;

	if (origTargets.size() != 0)
	{
	attackVal = int(newTargets.size() - origTargets.size()) / 2.0;

	if (AI_DEBUG)
	{
	std::cout << "\n\n==========DEBUG==========\norigTargets.size() != 0\n";
	std::cout << "newTargets.size() == " << newTargets.size() << std::endl;
	std::cout << "origTargets.size() == " << origTargets.size() << std::endl;
	std::cout << "attackVal = " << attackVal << "\n\n" << std::endl;
	}

	}
	else
	{
	attackVal = newTargets.size();

	if (AI_DEBUG)
	{
	std::cout << "\n\n==========DEBUG==========\norigTargets.size() == 0\n";
	std::cout << "newTargets.size() == " << newTargets.size() << std::endl;
	std::cout << "attackVal = " << attackVal << "\n\n" << std::endl;
	}

	}
	if (AI_DEBUG && attackVal > 10)
	{
	std::cout << "\n\n========DEBUG=========\nattackVal has large value!!! (" << attackVal << ")\n\n" << std::endl;
	std::cin.get();
	}
	*/

	//	Determine checkValue

	//	Idea:	Determine enemy king's position relative to piece's original position, as well as to the new position.
	//			From here, if the original position was within "eye-sight" of the enemy king, scan behind the piece being moved for
	//			a discover check.  Next, determine if the piece would be attacking the king from its new position.  If the attacker
	//			is a knight, scan to see if the attacker is within a 3-city block distance of the king, excluding purely
	//			horizontal or vertical travel.  This should be enough to determine check.  Return a vector containing the checking
	//			pieces, so the checkValue can be larger when there are multiple checking pieces.

	std::vector<chessPiece*> checkingPieces = getCheckers(board, moveData);

	if (checkingPieces.size() == 1)
	{
		moveData->moveTypeList.push_back(action::CHECK);
		checkVal = 2;
	}
	else if (checkingPieces.size() > 1)
	{
		moveData->moveTypeList.push_back(action::DOUBLE_CHECK);
		checkVal = 5;
	}
	else
		checkVal = 0;

	if (captureVal >= 0 && checkVal > 0)
		return captureVal + checkVal;

	//	Determine forwardMovementVal
	if (moveData->piece->getColor() == WHITE)
	{
		//	White moves up the board, so if destR > origR, then from white's perspective it has moved forward.
		forwardMovementVal = double(moveData->destR - moveData->origR);
	}
	else if (moveData->piece->getColor() == BLACK)
	{
		//	Black moves down the board, so if origR > destR, then from black's perspective it has moved forward.
		forwardMovementVal = double(moveData->origR - moveData->destR);
	}

	if (forwardMovementVal >= 0.0)
		moveData->moveTypeList.push_back(action::FORWARD);
	else
		moveData->moveTypeList.push_back(action::BACKWARD);

	//	This has been phased out in favor of piece-position value tables, as implemented below.
	/*
	//	Determine centerControlVal
	double origCenterControl, destCenterControl;
	//	The greater the distance from the center, the greater the penalty - except for the king.
	//	Distance = abs(position - 3.5)

	origCenterControl = abs(moveData->origC - 3.5) + abs(moveData->origR - 3.5);
	destCenterControl = abs(moveData->destC - 3.5) + abs(moveData->destR - 3.5);

	//	If the destination square is further away from the orig square, then a negative value is appropriate.
	centerControlVal = (origCenterControl - destCenterControl) / 2;

	//	It can be important for the king to move to the center at times (like near the end game), so lessen the penalty.
	if (moveData->piece->getType() == KING)
	centerControlVal = -centerControlVal / 4;

	*/

	//	Determine kingDefenseVal
	std::vector<std::pair<chessPiece*, PIN_DIR>>* defenders = board.getDefenderVector();

	double origDefenseVal = defenders->size() / 8.0;

	double destDefenseVal = getDefenders(board, moveData, defenders).size() / 8.0;

	double castleMultiplier = 1.0;

	if (moveData->piece->getType() == KING && abs(moveData->origC - moveData->destC) == 2)
	{
		moveData->moveTypeList.push_back(action::CASTLE);
		castleMultiplier = 10.0;
	}


	kingDefenseVal = 100 * castleMultiplier * (destDefenseVal + origDefenseVal) / 2;

	//	Determine pawnPromotion
	pawnPromotion = getNetPawnPromotion(board, moveData);

	if (int(pawnPromotion) > 0)
		moveData->moveTypeList.push_back(action::PROMOTION);


	//	Determine positional value.
	bool midgame = false;
	bool endgame = false;

	if (board.getWhiteQueens()->size() >= 1 || board.getBlackQueens()->size() >= 1)
	{
		if (board.getWhiteKnights()->empty() && board.getWhiteBishops()->empty() && board.getWhiteRooks()->empty())
		{
			if (board.getBlackKnights()->empty() && board.getBlackBishops()->empty() && board.getBlackRooks()->empty())
				endgame = true;
		}
	}
	else if (board.getWhiteQueens()->empty() && board.getBlackQueens()->empty())
	{
		if ((board.getWhiteKnights()->size() + board.getWhiteBishops()->size() + board.getWhiteRooks()->size()) <= 2)
			if ((board.getBlackKnights()->size() + board.getBlackBishops()->size() + board.getBlackRooks()->size()) <= 2)
				endgame = true;
	}

	PIECE_COLOR color = moveData->piece->getColor();
	PIECE_TYPE type = moveData->piece->getType();

	if (!endgame) midgame = true;

	if (color == WHITE)
		switch (type)
		{
		case PAWN:
			positionVal = pawnPosValue[7 - moveData->destR][moveData->destC] - pawnPosValue[7 - moveData->origR][moveData->origC];
			break;
		case KNIGHT:
			positionVal = knightPosValue[7 - moveData->destR][moveData->destC] - knightPosValue[7 - moveData->origR][moveData->origC];
			break;
		case BISHOP:
			positionVal = bishopPosValue[7 - moveData->destR][moveData->destC] - bishopPosValue[7 - moveData->origR][moveData->origC];
			break;
		case ROOK:
			positionVal = rookPosValue[7 - moveData->destR][moveData->destC] - rookPosValue[7 - moveData->origR][moveData->origC];
			break;
		case QUEEN:
			positionVal = queenPosValue[7 - moveData->destR][moveData->destC] - queenPosValue[7 - moveData->origR][moveData->origC];
			break;
		case KING:
			if (midgame) positionVal = kingPosValue_MID[7 - moveData->destR][moveData->destC] - kingPosValue_MID[7 - moveData->origR][moveData->origC];
			else if (endgame) positionVal = kingPosValue_END[7 - moveData->destR][moveData->destC] - kingPosValue_END[7 - moveData->origR][moveData->origC];
		}
	else if (color == BLACK)
		switch (type)
		{
		case PAWN:
			positionVal = pawnPosValue[moveData->destR][moveData->destC] - pawnPosValue[moveData->origR][moveData->origC];
			break;
		case KNIGHT:
			positionVal = knightPosValue[moveData->origR][moveData->origC] - knightPosValue[moveData->origR][moveData->origC];
			break;
		case BISHOP:
			positionVal = bishopPosValue[moveData->origR][moveData->origC] - bishopPosValue[moveData->origR][moveData->origC];
			break;
		case ROOK:
			positionVal = rookPosValue[moveData->origR][moveData->origC] - rookPosValue[moveData->origR][moveData->origC];
			break;
		case QUEEN:
			positionVal = queenPosValue[moveData->origR][moveData->origC] - queenPosValue[moveData->origR][moveData->origC];
			break;
		case KING:
			if (midgame) positionVal = kingPosValue_MID[moveData->origR][moveData->origC] - kingPosValue_MID[moveData->origR][moveData->origC];
			else if (endgame) positionVal = kingPosValue_END[moveData->origR][moveData->origC] - kingPosValue_END[moveData->origR][moveData->origC];
		}

	//	Return heuristic value
	int total = 100 * forwardMovementVal + kingDefenseVal + pawnPromotion + 100 * positionVal;
	return total;

}

/*
	====================================
	evaluateHeuristic - Helper Functions
	====================================
*/

//	Returns a vector containing all of the pieces that moveData->piece is attacking from (moveData->origC, moveData->origR).
std::vector<chessPiece*> chessGameTree::getTargets(chessBoardClass& board, action* moveData)
{
	PIECE_COLOR color = moveData->piece->getColor();
	PIECE_TYPE type = moveData->piece->getType();
	std::vector<chessPiece*> targets;

	if (type == PAWN)
	{
		if (color == WHITE)
		{
			chessPiece* target1 = board.getSquareContents(moveData->origC - 1, moveData->origR + 1);
			chessPiece* target2 = board.getSquareContents(moveData->origC + 1, moveData->origR + 1);

			if (target2 != NULL && color != target2->getColor())
				targets.push_back(target1);

			if (target2 != NULL && color != target2->getColor())
				targets.push_back(target2);
		}
		else if (color == BLACK)
		{
			chessPiece* target1 = board.getSquareContents(moveData->origC - 1, moveData->origR - 1);
			chessPiece* target2 = board.getSquareContents(moveData->origC + 1, moveData->origR - 1);

			if (target1 != NULL && color != target1->getColor())
				targets.push_back(target1);

			if (target2 != NULL && color != target2->getColor())
				targets.push_back(target2);
		}
	}
	else if (type == KNIGHT)
	{
		std::vector<chessPiece*> potentialTargets;

		//	Case 1:  Up 1 - Right 2
		potentialTargets.push_back(board.getSquareContents(moveData->origC + 2, moveData->origR + 1));
		//	Case 2:  Up 2 - Right 1
		potentialTargets.push_back(board.getSquareContents(moveData->origC + 1, moveData->origR + 2));
		//	Case 3:  Up 2 - Left 1
		potentialTargets.push_back(board.getSquareContents(moveData->origC - 1, moveData->origR + 2));
		//	Case 4:  Up 1 - Left 2
		potentialTargets.push_back(board.getSquareContents(moveData->origC - 2, moveData->origR + 1));
		//	Case 5:  Down 1 - Left 2
		potentialTargets.push_back(board.getSquareContents(moveData->origC - 2, moveData->origR - 1));
		//	Case 6:	 Down 2 - Left 1
		potentialTargets.push_back(board.getSquareContents(moveData->origC - 1, moveData->origR - 2));
		//	Case 7:  Down 2 - Right 1
		potentialTargets.push_back(board.getSquareContents(moveData->origC + 1, moveData->origR - 2));
		//	Case 8:  Down 1 - Right 2
		potentialTargets.push_back(board.getSquareContents(moveData->origC + 2, moveData->origR - 1));

		for (int i = 0; i < potentialTargets.size(); ++i)
		{
			//	Only add enemy targets to vector - null values and friendlies are ignored.
			if (potentialTargets[i] != NULL && color != potentialTargets[i]->getColor())
				targets.push_back(potentialTargets[i]);
		}
	}
	else if (type == BISHOP)
	{
		//	Case 1:  Up-Right
		for (int tracerC = moveData->origC + 1, tracerR = moveData->origR + 1; tracerC <= 7 && tracerR <= 7; ++tracerC, ++tracerR)
		{
			chessPiece* scanner = board.getSquareContents(tracerC, tracerR);

			if (scanner != NULL)
			{
				if (color != scanner->getColor())
					targets.push_back(scanner);
				break;
			}
		}

		//	Case 2:  Up-Left
		for (int tracerC = moveData->origC - 1, tracerR = moveData->origR + 1; tracerC >= 0 && tracerR <= 7; --tracerC, ++tracerR)
		{
			chessPiece* scanner = board.getSquareContents(tracerC, tracerR);

			if (scanner != NULL)
			{
				if (color != scanner->getColor())
					targets.push_back(scanner);
				break;
			}
		}

		//	Case 3:  Down-Left
		for (int tracerC = moveData->origC - 1, tracerR = moveData->origR - 1; tracerC >= 0 && tracerR >= 0; --tracerC, --tracerR)
		{
			chessPiece* scanner = board.getSquareContents(tracerC, tracerR);

			if (scanner != NULL)
			{
				if (color != scanner->getColor())
					targets.push_back(scanner);
				break;
			}
		}

		//	Case 4:  Down-Right
		for (int tracerC = moveData->origC + 1, tracerR = moveData->origR - 1; tracerC <= 7 && tracerR >= 0; ++tracerC, -tracerR)
		{
			chessPiece* scanner = board.getSquareContents(tracerC, tracerR);

			if (scanner != NULL)
			{
				if (color != scanner->getColor())
					targets.push_back(scanner);
				break;
			}
		}
	}
	else if (type == ROOK)
	{
		//	Case 1:  Right
		for (int tracerC = moveData->origC + 1, tracerR = moveData->origR; tracerC <= 7; ++tracerC)
		{
			chessPiece* scanner = board.getSquareContents(tracerC, tracerR);

			if (scanner != NULL)
			{
				if (color != scanner->getColor())
					targets.push_back(scanner);
				break;
			}
		}

		//	Case 2:  Up
		for (int tracerC = moveData->origC, tracerR = moveData->origR + 1; tracerR <= 7; ++tracerR)
		{
			chessPiece* scanner = board.getSquareContents(tracerC, tracerR);

			if (scanner != NULL)
			{
				if (color != scanner->getColor())
					targets.push_back(scanner);
				break;
			}
		}

		//	Case 3:  Left
		for (int tracerC = moveData->origC - 1, tracerR = moveData->origR; tracerC >= 0; --tracerC)
		{
			chessPiece* scanner = board.getSquareContents(tracerC, tracerR);

			if (scanner != NULL)
			{
				if (color != scanner->getColor())
					targets.push_back(scanner);
				break;
			}
		}

		//	Case 4:  Down
		for (int tracerC = moveData->origC, tracerR = moveData->origR - 1; tracerR >= 0; --tracerR)
		{
			chessPiece* scanner = board.getSquareContents(tracerC, tracerR);

			if (scanner != NULL)
			{
				if (color != scanner->getColor())
					targets.push_back(scanner);
				break;
			}
		}
	}
	else if (type == QUEEN)
	{

		//	Case 1:  Right
		for (int tracerC = moveData->origC + 1, tracerR = moveData->origR; tracerC <= 7; ++tracerC)
		{
			chessPiece* scanner = board.getSquareContents(tracerC, tracerR);

			if (scanner != NULL)
			{
				if (color != scanner->getColor())
					targets.push_back(scanner);
				break;
			}
		}
		//	Case 2:  Up-Right
		for (int tracerC = moveData->origC + 1, tracerR = moveData->origR + 1; tracerC <= 7 && tracerR <= 7; ++tracerC, ++tracerR)
		{
			chessPiece* scanner = board.getSquareContents(tracerC, tracerR);

			if (scanner != NULL)
			{
				if (color != scanner->getColor())
					targets.push_back(scanner);
				break;
			}
		}
		//	Case 3:  Up
		for (int tracerC = moveData->origC, tracerR = moveData->origR + 1; tracerR <= 7; ++tracerR)
		{
			chessPiece* scanner = board.getSquareContents(tracerC, tracerR);

			if (scanner != NULL)
			{
				if (color != scanner->getColor())
					targets.push_back(scanner);
				break;
			}
		}
		//	Case 4:  Up-Left
		for (int tracerC = moveData->origC - 1, tracerR = moveData->origR + 1; tracerC >= 0 && tracerR <= 7; --tracerC, ++tracerR)
		{
			chessPiece* scanner = board.getSquareContents(tracerC, tracerR);

			if (scanner != NULL)
			{
				if (color != scanner->getColor())
					targets.push_back(scanner);
				break;
			}
		}
		//	Case 5:  Left
		for (int tracerC = moveData->origC - 1, tracerR = moveData->origR; tracerC >= 0; --tracerC)
		{
			chessPiece* scanner = board.getSquareContents(tracerC, tracerR);

			if (scanner != NULL)
			{
				if (color != scanner->getColor())
					targets.push_back(scanner);
				break;
			}
		}
		//	Case 6:  Down-Left
		for (int tracerC = moveData->origC - 1, tracerR = moveData->origR - 1; tracerC >= 0 && tracerR >= 0; --tracerC, --tracerR)
		{
			chessPiece* scanner = board.getSquareContents(tracerC, tracerR);

			if (scanner != NULL)
			{
				if (color != scanner->getColor())
					targets.push_back(scanner);
				break;
			}
		}
		//	Case 7:  Down
		for (int tracerC = moveData->origC, tracerR = moveData->origR - 1; tracerR >= 0; --tracerR)
		{
			chessPiece* scanner = board.getSquareContents(tracerC, tracerR);

			if (scanner != NULL)
			{
				if (color != scanner->getColor())
					targets.push_back(scanner);
				break;
			}
		}
		//	Case 8:  Down-Right
		for (int tracerC = moveData->origC + 1, tracerR = moveData->origR - 1; tracerC <= 7 && tracerR >= 0; ++tracerC, -tracerR)
		{
			chessPiece* scanner = board.getSquareContents(tracerC, tracerR);

			if (scanner != NULL)
			{
				if (color != scanner->getColor())
					targets.push_back(scanner);
				break;
			}
		}
	}
	else if (type == KING)
	{
		//	For now we'll leave this blank, to add some discouragement for the AI using the king offensively.
	}


	return targets;
}

//	Returns a vector containing all of the pieces that are checking the enemy king after moveData->piece has moved to (moveData->destC, moveData->destR).
std::vector<chessPiece*> chessGameTree::getCheckers(chessBoardClass& board, action* moveData)
{
	//	We'll use PIN_DIR to mark the direction piece is originally, relative to the king.
	PIN_DIR origDirection = PIN_DIR(-1);

	PIECE_COLOR color = moveData->piece->getColor();

	chessPiece* enemyKing;
	if (color == WHITE)
		enemyKing = &(*board.getBlackKing())[0];
	else // (color == "black")
		enemyKing = &(*board.getWhiteKing())[0];

	int kingCol = enemyKing->getColumn();
	int kingRow = enemyKing->getRow();


	if (kingRow == moveData->origR)	//	Check for same row
	{
		if (kingCol < moveData->origC)	// Piece is to the right of the king.
			origDirection = RIGHT;
		else
			origDirection = LEFT;		// Piece is to the left of the king.
	}//	Check for same column
	else if (kingCol == moveData->origC)
	{
		if (kingRow < moveData->origC)	// Piece is above the king.
			origDirection = UP;
		else
			origDirection = DOWN;		// Piece is below the king.
	}//	Check for same diagonal
	else if (abs(kingCol - moveData->origC) == abs(kingRow - moveData->origR))
	{
		/*
		Idea of above conditional check:  horizontal distance from king to piece must be equal to vertical distance from king to piece.
		If the horizontal distance and vertical distance is the same, then a line drawn from king to piece is a line of slope 1, so both
		pieces must be on the same diagonal.

		As a side note, if the piece and the king are resting on the same diagonal, then we know the piece is not a bishop or a queen, since
		the player moving piece could not have the king in check at the start of his move.  This same idea applies to the other cases above.
		*/

		// The piece is to the right of the king.
		if (kingCol < moveData->origC)
		{
			if (kingRow < moveData->origR)
				origDirection = UP_RIGHT;	// The piece is to the up-right of the king
			else
				origDirection = DOWN_RIGHT;	// The piece is to the down-right of the king
		}
		else // The piece is to the left of the king.
		{
			if (kingRow < moveData->origR)
				origDirection = UP_LEFT;	// The piece is to the up-left of the king
			else
				origDirection = DOWN_LEFT;	// The piece is to the down-left of the king
		}
	}

	std::vector<chessPiece*> checkers;

	//	If origDirection has been set to a cardinal direction, then a discovered check is possible and needs to be scanned for.
	if (origDirection == RIGHT || origDirection == UP || origDirection == LEFT || origDirection == DOWN)
	{
		//	Discovered check threats:  Queen, Rook.

		if (origDirection == RIGHT)
		{
			for (int tracerC = moveData->origC + 1, tracerR = moveData->origR; tracerC <= 7; ++tracerC)
			{
				chessPiece* scanner = board.getSquareContents(tracerC, tracerR);

				if (scanner != NULL)
				{
					if (color != scanner->getColor() && (scanner->getType() == ROOK || scanner->getType() == QUEEN))
					{
						checkers.push_back(scanner);
						break;
					}
					else
						break;
				}
			}
		}
		else if (origDirection == UP)
		{
			for (int tracerC = moveData->origC, tracerR = moveData->origR + 1; tracerR <= 7; ++tracerR)
			{
				chessPiece* scanner = board.getSquareContents(tracerC, tracerR);

				if (scanner != NULL)
				{
					if (color != scanner->getColor() && (scanner->getType() == ROOK || scanner->getType() == QUEEN))
					{
						checkers.push_back(scanner);
						break;
					}
					else
						break;
				}
			}
		}
		else if (origDirection == LEFT)
		{
			for (int tracerC = moveData->origC - 1, tracerR = moveData->origR; tracerC >= 0; --tracerC)
			{
				chessPiece* scanner = board.getSquareContents(tracerC, tracerR);

				if (scanner != NULL)
				{
					if (color != scanner->getColor() && (scanner->getType() == ROOK || scanner->getType() == QUEEN))
					{
						checkers.push_back(scanner);
						break;
					}
					else
						break;
				}
			}
		}
		else if (origDirection == DOWN)
		{
			for (int tracerC = moveData->origC, tracerR = moveData->origR - 1; tracerR >= 0; --tracerR)
			{
				chessPiece* scanner = board.getSquareContents(tracerC, tracerR);

				if (scanner != NULL)
				{
					if (color != scanner->getColor() && (scanner->getType() == ROOK || scanner->getType() == QUEEN))
					{
						checkers.push_back(scanner);
						break;
					}
					else
						break;
				}
			}
		}
	}
	else if (origDirection == UP_RIGHT || origDirection == UP_LEFT || origDirection == DOWN_LEFT || origDirection == DOWN_RIGHT)
	{
		//	Discovered check threats:  Queen, Bishop.
		//  Note: Pawns are not a threat, since they cannot be the checker in a discovered check (they'd be out of range).

		if (origDirection == UP_RIGHT)
		{
			for (int tracerC = moveData->origC + 1, tracerR = moveData->origR + 1; tracerC <= 7 && tracerR <= 7; ++tracerC, ++tracerR)
			{
				chessPiece* scanner = board.getSquareContents(tracerC, tracerR);

				if (scanner != NULL)
				{
					if (color != scanner->getColor() && (scanner->getType() == BISHOP || scanner->getType() == QUEEN))
					{
						checkers.push_back(scanner);
						break;
					}
					else
						break;
				}
			}
		}
		else if (origDirection == UP_LEFT)
		{
			for (int tracerC = moveData->origC - 1, tracerR = moveData->origR + 1; tracerC >= 0 && tracerR <= 7; --tracerC, ++tracerR)
			{
				chessPiece* scanner = board.getSquareContents(tracerC, tracerR);

				if (scanner != NULL)
				{
					if (color != scanner->getColor() && (scanner->getType() == BISHOP || scanner->getType() == QUEEN))
					{
						checkers.push_back(scanner);
						break;
					}
					else
						break;
				}
			}
		}
		else if (origDirection == DOWN_LEFT)
		{
			for (int tracerC = moveData->origC - 1, tracerR = moveData->origR - 1; tracerC >= 0 && tracerR >= 0; --tracerC, --tracerR)
			{
				chessPiece* scanner = board.getSquareContents(tracerC, tracerR);

				if (scanner != NULL)
				{
					if (color != scanner->getColor() && (scanner->getType() == BISHOP || scanner->getType() == QUEEN))
					{
						checkers.push_back(scanner);
						break;
					}
					else
						break;
				}
			}
		}
		else if (origDirection == DOWN_RIGHT)
		{
			for (int tracerC = moveData->origC + 1, tracerR = moveData->origR - 1; tracerC <= 7 && tracerR >= 0; ++tracerC, --tracerR)
			{
				chessPiece* scanner = board.getSquareContents(tracerC, tracerR);

				if (scanner != NULL)
				{
					if (color != scanner->getColor() && (scanner->getType() == BISHOP || scanner->getType() == QUEEN))
					{
						checkers.push_back(scanner);
						break;
					}
					else
						break;
				}
			}
		}
	}

	//	Discovered checks are all scanned for.  Now we need to see if the piece's new position leads to a check.

	PIECE_TYPE type = moveData->piece->getType();

	if (type == PAWN)
	{
		if (color == WHITE)
		{
			if (kingCol == moveData->destC - 1 && kingRow == moveData->destR + 1)	//	King is up-left of white pawn.
				checkers.push_back(moveData->piece);
			else if (kingCol == moveData->destC + 1 && kingRow == moveData->destR + 1)	//	King is up-right of white pawn.
				checkers.push_back(moveData->piece);
		}
		else // color == "black"
		{
			if (kingCol == moveData->destC - 1 && kingRow == moveData->destR - 1)	//	King is down-left of black pawn.
				checkers.push_back(moveData->piece);
			else if (kingCol == moveData->destC + 1 && kingRow == moveData->destR - 1)	//	King is down-right of black pawn.
				checkers.push_back(moveData->piece);
		}
	}
	else if (type == KNIGHT)
	{
		//	If the city-block distance between the knight and the king is 3...
		if (((abs(kingRow - moveData->destR)) + (abs(kingCol - moveData->destC))) == 3)
		{
			//	... and if the distance is not purely horizontal or vertical...
			if (kingRow != moveData->destR && kingCol != moveData->destC)
				checkers.push_back(moveData->piece);	//  ... then the knight is checking the king.
		}
	}
	else if (type == BISHOP)
	{
		//	If bishop is moving to the same diagonal as the king...
		if (abs(kingRow - moveData->destR) == abs(kingCol - moveData->destC))
		{
			if (kingRow < moveData->destR)		//	Bishop is to move above the king
			{
				if (kingCol < moveData->destC)	//	Bishop is to move to the right of the king.
				{
					//	Scan for bishop check on king.
					for (int tracerC = kingCol + 1, tracerR = kingRow + 1; tracerC <= 7 && tracerR <= 7; ++tracerC, ++tracerR)
					{
						chessPiece* scanner = board.getSquareContents(tracerC, tracerR);

						if (scanner != NULL)
							break;

						if (tracerC == moveData->destC && tracerR == moveData->destR)
						{
							checkers.push_back(moveData->piece);
							break;
						}
					}
				}
				else if (kingCol > moveData->destC)	//	Bishop is to move to the left of the king.
				{
					for (int tracerC = kingCol - 1, tracerR = kingRow + 1; tracerC >= 0 && tracerR <= 7; --tracerC, ++tracerR)
					{
						chessPiece* scanner = board.getSquareContents(tracerC, tracerR);

						if (scanner != NULL)
							break;

						if (tracerC == moveData->destC && tracerR == moveData->destR)
						{
							checkers.push_back(moveData->piece);
							break;
						}
					}
				}
			}
			else if (kingRow > moveData->destR)	//	Bishop is to move below the king.
			{
				if (kingCol < moveData->destC)	//	Bishop is to move to the right of the king.
				{
					for (int tracerC = kingCol + 1, tracerR = kingRow - 1; tracerC <= 7 && tracerR >= 0; ++tracerC, --tracerR)
					{
						chessPiece* scanner = board.getSquareContents(tracerC, tracerR);

						if (scanner != NULL)
							break;

						if (tracerC == moveData->destC && tracerR == moveData->destR)
						{
							checkers.push_back(moveData->piece);
							break;
						}
					}
				}
				else if (kingCol > moveData->destC)	//	Bishop is to move to the left of the king.
				{
					for (int tracerC = kingCol - 1, tracerR = kingRow - 1; tracerC >= 0 && tracerR >= 0; --tracerC, --tracerR)
					{
						chessPiece* scanner = board.getSquareContents(tracerC, tracerR);

						if (scanner != NULL)
							break;

						if (tracerC == moveData->destC && tracerR == moveData->destR)
						{
							checkers.push_back(moveData->piece);
							break;
						}
					}
				}
			}
		}
	}
	else if (type == ROOK)
	{
		//	If rook is moving to the same row or column as the king...
		if (kingRow == moveData->destR || kingCol == moveData->destC)
		{
			if (kingRow == moveData->destR)	//	//	Rook is on the same row as the king.
			{

				if (kingCol < moveData->destC)	//	Rook is to the right of the king.
				{
					for (int tracerC = kingCol + 1, tracerR = kingRow; tracerC <= 7; ++tracerC)
					{
						chessPiece* scanner = board.getSquareContents(tracerC, tracerR);

						if (scanner != NULL)
							break;

						if (tracerC == moveData->destC && tracerR == moveData->destR)
						{
							checkers.push_back(moveData->piece);
							break;
						}
					}
				}
				else if (kingCol > moveData->destC)	//	Rook is to the left of the king.
				{
					for (int tracerC = kingCol - 1, tracerR = kingRow; tracerC >= 0; --tracerC)
					{
						chessPiece* scanner = board.getSquareContents(tracerC, tracerR);

						if (scanner != NULL)
							break;

						if (tracerC == moveData->destC && tracerR == moveData->destR)
						{
							checkers.push_back(moveData->piece);
							break;
						}
					}
				}
			}
			else if (kingCol == moveData->destC)	//	Rook is on the same column as the king.
			{
				if (kingRow < moveData->destR)	//	Rook is above the king.
				{
					for (int tracerC = kingCol, tracerR = kingRow + 1; tracerR <= 7; ++tracerR)
					{
						chessPiece* scanner = board.getSquareContents(tracerC, tracerR);

						if (scanner != NULL)
							break;

						if (tracerC == moveData->destC && tracerR == moveData->destR)
						{
							checkers.push_back(moveData->piece);
							break;
						}
					}
				}
				else if (kingRow > moveData->destR)	//	Rook is below the king.
				{
					for (int tracerC = kingCol, tracerR = kingRow - 1; tracerR >= 0; --tracerR)
					{
						chessPiece* scanner = board.getSquareContents(tracerC, tracerR);

						if (scanner != NULL)
							break;

						if (tracerC == moveData->destC && tracerR == moveData->destR)
						{
							checkers.push_back(moveData->piece);
							break;
						}
					}
				}
			}
		}
	}
	else if (type == QUEEN)
	{
		//	If queen is moving to the same diagonal as the king...
		if (abs(kingRow - moveData->destR) == abs(kingCol - moveData->destC))
		{
			if (kingRow < moveData->destR)		//	Queen is to move above the king
			{
				if (kingCol < moveData->destC)	//	Queen is to move to the right of the king.
				{
					//	Scan for queen check on king.
					for (int tracerC = kingCol + 1, tracerR = kingRow + 1; tracerC <= 7 && tracerR <= 7; ++tracerC, ++tracerR)
					{
						chessPiece* scanner = board.getSquareContents(tracerC, tracerR);

						if (scanner != NULL)
							break;

						if (tracerC == moveData->destC && tracerR == moveData->destR)
						{
							checkers.push_back(moveData->piece);
							break;
						}
					}
				}
				else if (kingCol > moveData->destC)	//	Queen is to move to the left of the king.
				{
					for (int tracerC = kingCol - 1, tracerR = kingRow + 1; tracerC >= 0 && tracerR <= 7; --tracerC, ++tracerR)
					{
						chessPiece* scanner = board.getSquareContents(tracerC, tracerR);

						if (scanner != NULL)
							break;

						if (tracerC == moveData->destC && tracerR == moveData->destR)
						{
							checkers.push_back(moveData->piece);
							break;
						}
					}
				}
			}
			else if (kingRow > moveData->destR)	//	Queen is to move below the king.
			{
				if (kingCol < moveData->destC)	//	Queen is to move to the right of the king.
				{
					for (int tracerC = kingCol + 1, tracerR = kingRow - 1; tracerC <= 7 && tracerR >= 0; ++tracerC, --tracerR)
					{
						chessPiece* scanner = board.getSquareContents(tracerC, tracerR);

						if (scanner != NULL)
							break;

						if (tracerC == moveData->destC && tracerR == moveData->destR)
						{
							checkers.push_back(moveData->piece);
							break;
						}
					}
				}
				else if (kingCol > moveData->destC)	//	Queen is to move to the left of the king.
				{
					for (int tracerC = kingCol - 1, tracerR = kingRow - 1; tracerC >= 0 && tracerR >= 0; --tracerC, --tracerR)
					{
						chessPiece* scanner = board.getSquareContents(tracerC, tracerR);

						if (scanner != NULL)
							break;

						if (tracerC == moveData->destC && tracerR == moveData->destR)
						{
							checkers.push_back(moveData->piece);
							break;
						}
					}
				}
			}
		}
		else if (kingRow == moveData->destR || kingCol == moveData->destC)	//	If queen is moving to the same row or column as the king...
		{
			if (kingRow == moveData->destR)	//	//	Queen is on the same row as the king.
			{

				if (kingCol < moveData->destC)	//	Queen is to the right of the king.
				{
					for (int tracerC = kingCol + 1, tracerR = kingRow; tracerC <= 7; ++tracerC)
					{
						chessPiece* scanner = board.getSquareContents(tracerC, tracerR);

						if (scanner != NULL)
							break;

						if (tracerC == moveData->destC && tracerR == moveData->destR)
						{
							checkers.push_back(moveData->piece);
							break;
						}
					}
				}
				else if (kingCol > moveData->destC)	//	Queen is to the left of the king.
				{
					for (int tracerC = kingCol - 1, tracerR = kingRow; tracerC >= 0; --tracerC)
					{
						chessPiece* scanner = board.getSquareContents(tracerC, tracerR);

						if (scanner != NULL)
							break;

						if (tracerC == moveData->destC && tracerR == moveData->destR)
						{
							checkers.push_back(moveData->piece);
							break;
						}
					}
				}
			}
			else if (kingCol == moveData->destC)	//	Queen is on the same column as the king.
			{
				if (kingRow < moveData->destR)	//	Queen is above the king.
				{
					for (int tracerC = kingCol, tracerR = kingRow + 1; tracerR <= 7; ++tracerR)
					{
						chessPiece* scanner = board.getSquareContents(tracerC, tracerR);

						if (scanner != NULL)
							break;

						if (tracerC == moveData->destC && tracerR == moveData->destR)
						{
							checkers.push_back(moveData->piece);
							break;
						}
					}
				}
				else if (kingRow > moveData->destR)	//	Queen is below the king.
				{
					for (int tracerC = kingCol, tracerR = kingRow - 1; tracerR >= 0; --tracerR)
					{
						chessPiece* scanner = board.getSquareContents(tracerC, tracerR);

						if (scanner != NULL)
							break;

						if (tracerC == moveData->destC && tracerR == moveData->destR)
						{
							checkers.push_back(moveData->piece);
							break;
						}
					}
				}
			}
		}
	}


	//	All scans complete.  Return vector.
	return checkers;
}

//	Returns a vector containing all of the pieces that are in eight-direction eye-sight of the king after moveData->piece has moved to (moveData->destC, moveData->destR).
std::vector<chessPiece*> chessGameTree::getDefenders(chessBoardClass& board, action* moveData, std::vector<std::pair<chessPiece*, PIN_DIR>>* origDefenders)
{
	/*
	Idea:	Determine cardinal direction of piece's destination square (destC, destR), relative to king's position.

	Case 1:	No cardinal direction can be determined, i.e. piece is not a defender.  origDefenders will have the first element of each pair
	copied over to a new chessPiece vector, which will contain all of origDefenders with the exception of piece, if present in origDefenders.

	Case 2:	A cardinal direction is determined.  This means that piece's destination lies on the same row, column, or diagonal as the king.
	We must scan in that direction from the king to determine if piece is now defending the king.

	-	2.1)	If piece is the first chessPiece object encountered during the scan, it is flagged as a defender and added to the defenders
	if it is not already present (hence the need for the flag).
	-	2.2)	If anything else is encountered before chessPiece (friendly or enemy), then chessPiece is flagged as not a defender and the
	function will proceed forward as if Case 1 had occured.
	*/

	//	We'll use PIN_DIR to mark the direction piece is originally, relative to the king.
	PIN_DIR destDirection = PIN_DIR(-1);

	PIECE_COLOR color = moveData->piece->getColor();

	chessPiece* king;
	if (color == WHITE)
		king = &(*board.getWhiteKing())[0];
	else // (color == "black")
		king = &(*board.getBlackKing())[0];

	int kingCol = king->getColumn();
	int kingRow = king->getRow();


	if (kingRow == moveData->origR)	//	Check for same row
	{
		if (kingCol < moveData->origC)	// Piece is to the right of the king.
			destDirection = RIGHT;
		else
			destDirection = LEFT;		// Piece is to the left of the king.
	}//	Check for same column
	else if (kingCol == moveData->origC)
	{
		if (kingRow < moveData->origC)	// Piece is above the king.
			destDirection = UP;
		else
			destDirection = DOWN;		// Piece is below the king.
	}//	Check for same diagonal
	else if (abs(kingCol - moveData->origC) == abs(kingRow - moveData->origR))
	{
		/*
		Idea of above conditional check:  horizontal distance from king to piece must be equal to vertical distance from king to piece.
		If the horizontal distance and vertical distance is the same, then a line drawn from king to piece is a line of slope 1, so both
		pieces must be on the same diagonal.

		As a side note, if the piece and the king are resting on the same diagonal, then we know the piece is not a bishop or a queen, since
		the player moving piece could not have the king in check at the start of his move.  This same idea applies to the other cases above.
		*/

		// The piece is to the right of the king.
		if (kingCol < moveData->origC)
		{
			if (kingRow < moveData->origR)
				destDirection = UP_RIGHT;	// The piece is to the up-right of the king
			else
				destDirection = DOWN_RIGHT;	// The piece is to the down-right of the king
		}
		else // The piece is to the left of the king.
		{
			if (kingRow < moveData->origR)
				destDirection = UP_LEFT;	// The piece is to the up-left of the king
			else
				destDirection = DOWN_LEFT;	// The piece is to the down-left of the king
		}
	}

	bool isDefender = false;

	//	If a cardinal direction could not be determined, then piece is not a defender.
	if (destDirection == PIN_DIR(-1))
		isDefender = false;
	else //	If cardinal direction can be determined, see if piece is defending king.
	{
		if (destDirection == RIGHT)
		{
			for (int tracerC = kingCol + 1, tracerR = kingRow; tracerC <= 7; ++tracerC)
			{
				chessPiece* scanner = board.getSquareContents(tracerC, tracerR);

				if (scanner != NULL)
				{
					if (scanner == moveData->piece)
						isDefender = true;

					break;
				}
			}
		}
		else if (destDirection == UP_RIGHT)
		{
			for (int tracerC = kingCol + 1, tracerR = kingRow + 1; tracerC <= 7 && tracerR <= 7; ++tracerC, ++tracerR)
			{
				chessPiece* scanner = board.getSquareContents(tracerC, tracerR);

				if (scanner != NULL)
				{
					if (scanner == moveData->piece)
						isDefender = true;

					break;
				}
			}
		}
		else if (destDirection == UP)
		{
			for (int tracerC = kingCol, tracerR = kingRow + 1; tracerR <= 7; ++tracerR)
			{
				chessPiece* scanner = board.getSquareContents(tracerC, tracerR);

				if (scanner != NULL)
				{
					if (scanner == moveData->piece)
						isDefender = true;

					break;
				}
			}
		}
		else if (destDirection == UP_LEFT)
		{
			for (int tracerC = kingCol - 1, tracerR = kingRow + 1; tracerC >= 0 && tracerR <= 7; --tracerC, ++tracerR)
			{
				chessPiece* scanner = board.getSquareContents(tracerC, tracerR);

				if (scanner != NULL)
				{
					if (scanner == moveData->piece)
						isDefender = true;

					break;
				}
			}
		}
		else if (destDirection == LEFT)
		{
			for (int tracerC = kingCol - 1, tracerR = kingRow; tracerC >= 0; --tracerC)
			{
				chessPiece* scanner = board.getSquareContents(tracerC, tracerR);

				if (scanner != NULL)
				{
					if (scanner == moveData->piece)
						isDefender = true;

					break;
				}
			}
		}
		else if (destDirection == DOWN_LEFT)
		{
			for (int tracerC = kingCol - 1, tracerR = kingRow - 1; tracerC >= 0 && tracerR >= 0; --tracerC, --tracerR)
			{
				chessPiece* scanner = board.getSquareContents(tracerC, tracerR);

				if (scanner != NULL)
				{
					if (scanner == moveData->piece)
						isDefender = true;

					break;
				}
			}
		}
		else if (destDirection == DOWN)
		{
			for (int tracerC = kingCol, tracerR = kingRow - 1; tracerR >= 0; --tracerR)
			{
				chessPiece* scanner = board.getSquareContents(tracerC, tracerR);

				if (scanner != NULL)
				{
					if (scanner == moveData->piece)
						isDefender = true;

					break;
				}
			}
		}
		else if (destDirection == DOWN_RIGHT)
		{
			for (int tracerC = kingCol + 1, tracerR = kingRow - 1; tracerC <= 7 && tracerR >= 0; ++tracerC, --tracerR)
			{
				chessPiece* scanner = board.getSquareContents(tracerC, tracerR);

				if (scanner != NULL)
				{
					if (scanner == moveData->piece)
						isDefender = true;

					break;
				}
			}
		}
	}

	std::vector<chessPiece*> defenders;

	if (isDefender)
		defenders.push_back(moveData->piece);

	for (int i = 0; i < origDefenders->size(); ++i)
	{
		chessPiece* defender = (*origDefenders)[i].first;
		PIN_DIR origDirection = (*origDefenders)[i].second;

		//	Note:	If piece has been determined to be a defender, it is possible that piece has 'replaced' a defender
		//			by moving between the original defender and the king.  This should be checked for (destDirection != origDirection).

		if (isDefender && destDirection != origDirection && defender != moveData->piece)
			defenders.push_back(defender);
	}

	if (defenders.empty())
	{
		for (int i = 0; i < origDefenders->size(); ++i)
			defenders.push_back((*origDefenders)[i].first);
	}

	return defenders;
}

//	Returns a double value conveying the net gain in material from promoting a pawn.
///	NOTE:  AI WILL ALWAYS PROMOTE TO QUEEN, UNLESS PROMOTION IS SPECIFICALLY IMPLEMENTED AS AN ACTION LATER ON.
double chessGameTree::getNetPawnPromotion(chessBoardClass& board, action* moveData)
{
	if (moveData->piece->getType() != PAWN)
		return 0.0;

	PIECE_COLOR color = moveData->piece->getColor();
	int destR = moveData->piece->getRow();

	//	Technically this is frivolous, since only white pawns can reach row 7 (and only black pawns can reach row 0),
	//	but it helps to convey meaning and increase readability.
	if (color == WHITE)
	{
		if (destR == 7)
			return val.queen - 6.0;
		else
			return 0.0;
	}
	else // color == "black"
	{
		if (destR == 0)
			return val.queen - 6.0;
		else
			return 0.0;
	}

}

/*
	=======================
	Miscellaneous Functions
	=======================
*/
//	Updates the currentGameState pointer.
void chessGameTree::signalMove(action moveData)
{
	cleanUpTree(currentGameState);

	gameStateNode* newState = new gameStateNode(currentGameState);

	if (currentGameState->isMaxNode)
		newState->gameState.setTurn(WHITE);
	else
		newState->gameState.setTurn(BLACK);

	while (!currentGameState->next.empty())
		currentGameState->next.pop();
	while (!currentGameState->actionList.empty())
		currentGameState->actionList.pop();

	//	Play the new move.
	newState->gameState.move(moveData.origC, moveData.origR, moveData.destC, moveData.destR);
	newState->moveHistory.push_back(new action(moveData));
	newState->previous = currentGameState;

	currentGameState->next.push(newState);
	currentGameState->bestAction = moveData;
	newState->markedForDelete = false;

	currentGameState = newState;
	

}

//	Updates the currentGameState pointer.
void chessGameTree::signalMove(const chessBoardClass& board, action moveData)
{
	cleanUpTree(currentGameState);

	gameStateNode* newState = new gameStateNode(currentGameState);

	if (currentGameState->isMaxNode)
		newState->gameState.setTurn(WHITE);
	else
		newState->gameState.setTurn(BLACK);

	while (!currentGameState->next.empty())
		currentGameState->next.pop();
	while (!currentGameState->actionList.empty())
		currentGameState->actionList.pop();


}

