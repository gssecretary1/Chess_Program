#pragma once

#include "chessBoardClass.h"
#include <iostream>


///				To-Dos (Ordered from vital to superfluous)
/*  
	-	The ability to "pick up" pieces on click and drag would be nice.
	-	Outputting the game's movement history for each move would be nice.
*/

///					Misc Notes, Thoughts, and Observations.
/*
	-	The size of a chessBoardClass object at this point in time is about
		512 bytes, or 1/2 KB.  Each node in the AI's game tree will need a copy
		of its own chessBoardClass object, since the object represents the current
		game state for that move (though a more space-friendly representation
		of the game state could be viable).

	-	Let's say the game is going on for a while, but all of the original
		pieces are still on the board.  The worst case (though impossible, 
		it sets an upper bound), is that each piece for a particular color can
		perform any of its possible moves.  The maximum number of possible moves that
		can be performed by each piece is:  pawn(4), knight(8), bishop(13), rook(14),
		queen(27), and king(8).  

	-	Maximum number of moves on a given turn with all of the original pieces is
		8*4 + 2*8 + 2*13 + 2*14 + 27 + 8 = 137.  -> This is siginificantly greater
		than the real maximum that occurs throughout the course of a chess game.

	-	Since the AI's game tree will, at each depth level, consist of nodes that
		represent the game state for each move played up to that point, the upper-bound 
		on the amount of children each node can have is 137 (real is much lower).

	-	At the beginning of the game, the total possible moves that white can perform
		is actually only 20.  As the board opens up, more moves become possible, but
		more pieces are also taken off of the board as the game progresses.  This will probably
		lead to the middle of the tree being bloated, while the top and bottom of the game
		tree (as the game approaches its end) will be thinner (less possible moves = less children).

	-	Let's wager a guess and say over the course of the game, the average amount of possible
		moves that could be performed by either player for their given turn is 50.

	-	If each node has 50 children, then the total number of nodes in the game tree at depth
		level D is the sum, from i = 0 to D, of 50^i, which is (50^(D+1)-1)/49

	-	So, if the AI is looking 4 moves ahead, then the tree's depth D = 4.  The number of
		nodes in the game tree ~ 6,250,000.  Assuming the nodes themselves don't require
		much more space than the chessBoardClass objects they store, then a tree of depth 4
		will take up 3,125,000 KB ~ 3GB. A tree of depth 5 will take up about 150 GB.
		A tree of depth 6 has 15,625,000,000 nodes -> 7,812,500,000 KB ~ 7.4 TB.  This is 
		assuming the 50 children per node growth, which is probable.

	-	If each node has 25 children, then depth 4 ~ 200KB, depth 5 ~ 4.9MB, depth 6 ~ 120MB,
		and depth 7 ~ 3GB.

	-	Just for kicks, if each node has our worst-case number of children (137) then depth 4 ~ 170GB,
		depth 5 ~ 23TB.  The number of possible moves for each game state will have a huge impact
		on how big we can really make our tree.  A tree in the MB range isn't bad, but the GB range
		is best avoided.

	-	In general, if the average number of children per node in the tree = C, and the depth of the
		tree = D, then the number of nodes in the tree will be ~ (C^(D+1) - 1) / (C-1) 

	-	Memory will clearly be an issue if this AI will be any good at chess.  The game tree will
		need to be expanded as moves are made, but nodes (and the children of the nodes) pertaining to
		moves that were not selected can be deleted after a move has been made.  This will ensure
		that the memory the tree takes up is close to constant, given that the depth is kept constant.
		As stated above, C won't always be constant, as the number of possible moves increases in mid-game,
		so the tree may need to be a depth level shorter during mid-game.  Perhaps the AI could
		track this information and use it to know when to stop looking ahead?

	-	A tree with a C of 40 and a D of 3 takes up about 32MB, with ~ 66,000 nodes.  Once the AI
		is actually implemented, I'll need to see just how intelligent it appears to be for different
		D values.  If 3 provides a sufficient challenge (probably not), then there may be no need to let
		D go higher.  C = 40, D = 4 -> 1.25GB - undesirable amount of memory usage.

	-	Alpha-Beta Pruning will definitely be of help here, but it's efficacy relies on how I order
		the moves for use during tree expansion.  Check and capture moves should probably be favored,
		as they are most likely to result in a more radical change in game state (and move closer to
		goal state).  Beyond placing check and/or capture moves first, I'm (at this moment of writing) unsure
		as to what other types of moves I could define and give a place in the hierarchy.  Regardless, a
		good move order at each node could expand the tree depth by close to 2 times the original amount.

	-	Coming up with a more space-friendly format than having each node hold a chessBoardClass object
		could prove beneficial.  While the true issue is the sheer amount of nodes in the tree, reducing
		the size of each node will certainly help.  An 8x8 array of chars would take up 64 bytes of space. 
		With p, n, b, r, q, k representing all of the white pieces from pawn, knight, bishop, rook, queen, 
		and king, and the capital counter-parts representing the pieces for black, and x's for empty squares,
		the game state could be saved in each node in probably as close to minimal space as possible.
	
	-	If the AI inhereits from the chessBoardClass, then it could call the necessary movement check methods.
		Using the 2D-char format, it could create the necessary chessPiece objects by reading from the char-board,
		and pass the necessary data to the movement check methods.  Valid movements can lead to created nodes that
		represent the state of the game given the corresponding move.  The issue with this is how to check for valid
		movements that are deeper into the game than just one move ahead.  Perhaps I could derive the new board from
		the movement check methods somehow?  I may need to rewrite the movement logic methods so that I can use them 
		with a char-board.  
	
	-	The AI class should contain a pointer that points to the current game state, as well as a pointer that 
		points to the initial game state.

	-	Space can be saved by having each node contain a flag that indicates whether it is to be deleted or not.
		All flags will initially start off as true, but a move that corresponds to its respective move-node will result
		in that node's deletion flag being set to false.  Once the flag is set to false, a separate thread will be launched 
		that will clean up the rest of the tree, disposing of nodes that ended up not being used.  A recursive, post-order 
		traversal that checks the deletion flags should be sufficient; nodes that have their deletion flag set to false
		will not be traversed/deleted.  More details can wait until the implementation is attempted.

*/

/*
	================================================================
	chessBoardClass Constructor, Destructor, and Overloaded Operator
	================================================================
*/

// Initializes the chess board object, setting it to the initial game state.
void chessBoardClass::init()
{
// Clear piece vectors

	// Black vectors
	bPawns.clear(); bKnights.clear(); bBishops.clear();
	bRooks.clear(); bQueens.clear(); bKing.clear();
	// White vectors
	wPawns.clear(); wKnights.clear(); wBishops.clear();
	wRooks.clear(); wQueens.clear(); wKing.clear();

// Populate piece vectors, sync board to vector elements.

	bPawns.resize(8); bKnights.resize(5); bBishops.resize(5);
	bRooks.resize(5); bQueens.resize(5); bKing.resize(1);

	wPawns.resize(8); wKnights.resize(5); wBishops.resize(5);
	wRooks.resize(5); wQueens.resize(5); wKing.resize(1);

	// White pawns

	whitePawn wPawn(0, 1);
	wPawns.assign(8, wPawn);
	for (int col = 0, row = 1; col < 8; ++col)
	{
		wPawns[col].moveTo(col, row);
		board[col][row] = &wPawns[col];
	}

	// Black pawns
	blackPawn bPawn(0, 6);
	bPawns.assign(8, bPawn);
	for (int col = 0, row = 6; col < 8; ++col)
	{
		bPawns[col].moveTo(col, row);
		board[col][row] = &bPawns[col];
	}
	// White knights
	{
		whiteKnight knight(1, 0);
		wKnights.assign(2, knight);
		board[1][0] = &wKnights.front();
		wKnights[1].moveTo(6, 0);
		board[6][0] = &wKnights.back();
	}
	// Black knights
	{
		blackKnight knight(1, 7);
		bKnights.assign(2, knight);
		board[1][7] = &bKnights.front();
		bKnights[1].moveTo(6, 7);
		board[6][7] = &bKnights.back();
	}
	// White bishops
	{
		whiteBishop bishop(2, 0);
		wBishops.assign(2, bishop);
		board[2][0] = &wBishops.front();
		wBishops[1].moveTo(5, 0);
		board[5][0] = &wBishops.back();
	}
	// Black bishops
	{
		blackBishop bishop(2, 7);
		bBishops.assign(2, bishop);
		board[2][7] = &bBishops.front();
		bBishops[1].moveTo(5, 7);
		board[5][7] = &bBishops.back();
	}
	// White rooks
	{
		whiteRook rook(0, 0);
		wRooks.assign(2, rook);
		board[0][0] = &wRooks.front();
		wRooks[1].moveTo(7, 0);
		wRooks[1].setCastle(true);
		board[7][0] = &wRooks.back();
	}
	// Black rooks
	{
		blackRook rook(0, 7);
		bRooks.assign(2, rook);
		board[0][7] = &bRooks.front();
		bRooks[1].moveTo(7, 7);
		bRooks[1].setCastle(true);
		board[7][7] = &bRooks.back();
	}
	// White queen
	{
		whiteQueen queen(3, 0);
		wQueens.assign(1, queen);
		board[3][0] = &wQueens.front();
	}
	// Black queen
	{
		blackQueen queen(3, 7);
		bQueens.assign(1, queen);
		board[3][7] = &bQueens.front();
	}
	// White king
	{
		whiteKing king(4, 0);
		wKing.assign(1, king);
		board[4][0] = &wKing.front();
	}		
	// Black king
	{
		blackKing king(4, 7);
		bKing.assign(1, king);
		board[4][7] = &bKing.front();
	}
	// White goes first.
	turn = WHITE;

	// The game doesn't start in the "gameover" state
	checkmate = false;

	// Set rest of board elements to NULL (4 middle rows have no pieces at start)
	for (int row = 2; row < 6; ++row)
		for (int col = 0; col < 8; ++col)
			board[col][row] = NULL;

	// Clear other vectors
	checkVector.clear(); attackVector.clear(); saviorVector.clear(); escapeVector.clear();

	//	Set AI ownership flag, which is used for pawn promotion logic to bypass console window input.
	//	This is set to true if the chessBoardClass object is set by a copy constructor or assignment operator.
	ownedByAI = false;

	// Debug output that prints the position of the pieces,
	// starting from the a1 (bottom-left) and going to h8 (top-right).
	/*
	if (DEBUG)
		for (int row = 0; row < 8; ++row)
		{
			using std::cout;
			using std::endl;
			for (int col = 0; col < 8; ++col)
			{
				if (board[col][row])
					cout << "(" << col << ", " << row << ")"
						<< " == " << board[col][row]->getColor() << " "
						<< board[col][row]->getType() << endl;
			}
			cout << endl;
		}
	*/

	// Prints out the size of the object, in bytes.
	if (DEBUG)
	{
		using std::cout;
		using std::endl;

		cout << "\nNewly initilized chessBoardClass object size == " << sizeof(*this) << " bytes\n\n" << endl;
	}
}

// Copy Constructor - performs a deep copy of the game state from the passed obj.
chessBoardClass::chessBoardClass(const chessBoardClass& obj)
{
	///	Technically the AI only uses the assignment operator, since the copy constructor breaks things.
	//	When the AI creates the game tree, it will use the copy constructor or the overloaded assignment op.
	ownedByAI = true;

	// Set turn, useful for vector copies.
	turn = obj.turn;

//	Copy black obj piece positions and create new pieces.
//	These loops also populate saviorVector and checkVector appropriately.

	// Initialize the board.
	for (int i = 0; i < 8; ++i)
		for (int j = 0; j < 8; ++j)
			board[i][j] = NULL;

	// Copy black pawns and sync to board.
	for (int i = 0; i < obj.bPawns.size(); ++i)
	{
		bPawns.push_back(obj.bPawns[i]);
		board[bPawns.back().getColumn()][bPawns.back().getRow()] = &bPawns.back();

		// Deep copy any pawns that are part of obj.saviorVector
		if (turn == BLACK)
		{
			for (int j = 0; j < obj.saviorVector.size(); ++j)
				if (&obj.bPawns[i] == obj.saviorVector[j])
				{
					saviorVector.push_back(&bPawns[i]);
					break;
				}
		}
		else // if (turn == WHITE) deep copy any pawns that are part of obj.checkVector
		{
			for (int j = 0; j < obj.checkVector.size(); ++j)
				if (&obj.bPawns[i] == obj.checkVector[j])
				{
					checkVector.push_back(&bPawns[i]);
					break;
				}
		}
	}

	// Copy black knights and sync to board.
	for (int i = 0; i < obj.bKnights.size(); ++i)
	{
		bKnights.push_back(obj.bKnights[i]);
		board[bKnights.back().getColumn()][bKnights.back().getRow()] = &bKnights.back();

		if (turn == BLACK)
		{
			for (int j = 0; j < obj.saviorVector.size(); ++j)
				if (&obj.bKnights[i] == obj.saviorVector[j])
				{
					saviorVector.push_back(&bKnights[i]);
					break;
				}
		}
		else
		{
			for (int j = 0; j < obj.checkVector.size(); ++j)
				if (&obj.bKnights[i] == obj.checkVector[j])
				{
					checkVector.push_back(&bKnights[i]);
					break;
				}
		}
	}

	// Copy black bishops and sync to board.
	for (int i = 0; i < obj.bBishops.size(); ++i)
	{
		bBishops.push_back(obj.bBishops[i]);
		board[bBishops.back().getColumn()][bBishops.back().getRow()] = &bBishops.back();

		if (turn == BLACK)
		{
			for (int j = 0; j < obj.saviorVector.size(); ++j)
				if (&obj.bBishops[i] == obj.saviorVector[j])
				{
					saviorVector.push_back(&bBishops[i]);
					break;
				}
		}
		else
		{
			for (int j = 0; j < obj.checkVector.size(); ++j)
				if (&obj.bBishops[i] == obj.checkVector[j])
				{
					checkVector.push_back(&bBishops[i]);
					break;
				}
		}
	}

	// Copy black rooks and sync to board.
	for (int i = 0; i < obj.bRooks.size(); ++i)
	{
		bRooks.push_back(obj.bRooks[i]);
		board[bRooks.back().getColumn()][bRooks.back().getRow()] = &bRooks.back();

		if (turn == BLACK)
		{
			for (int j = 0; j < obj.saviorVector.size(); ++j)
				if (&obj.bRooks[i] == obj.saviorVector[j])
				{
					saviorVector.push_back(&bRooks[i]);
					break;
				}
		}
		else
		{
			for (int j = 0; j < obj.checkVector.size(); ++j)
				if (&obj.bRooks[i] == obj.checkVector[j])
				{
					checkVector.push_back(&bRooks[i]);
					break;
				}
		}
	}

	// Copy black queens and sync to board.
	for (int i = 0; i < obj.bQueens.size(); ++i)
	{
		bQueens.push_back(obj.bQueens[i]);
		board[bQueens.back().getColumn()][bQueens.back().getRow()] = &bQueens.back();

		if (turn == BLACK)
		{
			for (int j = 0; j < obj.saviorVector.size(); ++j)
				if (&obj.bQueens[i] == obj.saviorVector[j])
				{
					saviorVector.push_back(&bQueens[i]);
					break;
				}
		}
		else
		{
			for (int j = 0; j < obj.checkVector.size(); ++j)
				if (&obj.bQueens[i] == obj.checkVector[j])
				{
					checkVector.push_back(&bQueens[i]);
					break;
				}
		}
	}

	// Copy black king and sync to board.
	bKing.push_back(obj.bKing.front());
	board[bKing.front().getColumn()][bKing.front().getRow()] = &bKing.front();

	// Note:  Kings can be their own saviors, but cannot put another king in check.
	if (turn == BLACK)
	{
		for (int j = 0; j < obj.saviorVector.size(); ++j)
			if (&obj.bKing.front() == obj.saviorVector[j])
			{
				saviorVector.push_back(&bKing.front());
				break;
			}
	}


// Rinse and repeat for white pieces

	// Copy white pawns and sync to board.
	for (int i = 0; i < obj.wPawns.size(); ++i)
	{
		wPawns.push_back(obj.wPawns[i]);
		board[wPawns.back().getColumn()][wPawns.back().getRow()] = &wPawns.back();

		// Deep copy any pawns that are part of obj.saviorVector
		if (turn == WHITE)
		{
			for (int j = 0; j < obj.saviorVector.size(); ++j)
				if (&obj.wPawns[i] == obj.saviorVector[j])
				{
					saviorVector.push_back(&wPawns[i]);
					break;
				}
		}
		else // if turn == "black" deep copy any pawns that are part of obj.checkVector
		{
			for (int j = 0; j < obj.checkVector.size(); ++j)
				if (&obj.wPawns[i] == obj.checkVector[j])
				{
					checkVector.push_back(&wPawns[i]);
					break;
				}
		}
	}

	// Copy white knights and sync to board.
	for (int i = 0; i < obj.wKnights.size(); ++i)
	{
		wKnights.push_back(obj.wKnights[i]);
		board[wKnights.back().getColumn()][wKnights.back().getRow()] = &wKnights.back();

		if (turn == WHITE)
		{
			for (int j = 0; j < obj.saviorVector.size(); ++j)
				if (&obj.wKnights[i] == obj.saviorVector[j])
				{
					saviorVector.push_back(&wKnights[i]);
					break;
				}
		}
		else
		{
			for (int j = 0; j < obj.checkVector.size(); ++j)
				if (&obj.wKnights[i] == obj.checkVector[j])
				{
					checkVector.push_back(&wKnights[i]);
					break;
				}
		}
	}

	// Copy white bishops and sync to board.
	for (int i = 0; i < obj.wBishops.size(); ++i)
	{
		wBishops.push_back(obj.wBishops[i]);
		board[wBishops.back().getColumn()][wBishops.back().getRow()] = &wBishops.back();

		if (turn == WHITE)
		{
			for (int j = 0; j < obj.saviorVector.size(); ++j)
				if (&obj.wBishops[i] == obj.saviorVector[j])
				{
					saviorVector.push_back(&wBishops[i]);
					break;
				}
		}
		else
		{
			for (int j = 0; j < obj.checkVector.size(); ++j)
				if (&obj.wBishops[i] == obj.checkVector[j])
				{
					checkVector.push_back(&wBishops[i]);
					break;
				}
		}
	}

	// Copy white rooks and sync to board.
	for (int i = 0; i < obj.wRooks.size(); ++i)
	{
		wRooks.push_back(obj.wRooks[i]);
		board[wRooks.back().getColumn()][wRooks.back().getRow()] = &wRooks.back();

		if (turn == WHITE)
		{
			for (int j = 0; j < obj.saviorVector.size(); ++j)
				if (&obj.wRooks[i] == obj.saviorVector[j])
				{
					saviorVector.push_back(&wRooks[i]);
					break;
				}
		}
		else
		{
			for (int j = 0; j < obj.checkVector.size(); ++j)
				if (&obj.wRooks[i] == obj.checkVector[j])
				{
					checkVector.push_back(&wRooks[i]);
					break;
				}
		}
	}

	// Copy white queens and sync to board.
	for (int i = 0; i < obj.wQueens.size(); ++i)
	{
		wQueens.push_back(obj.wQueens[i]);
		board[wQueens.back().getColumn()][wQueens.back().getRow()] = &wQueens.back();

		if (turn == WHITE)
		{
			for (int j = 0; j < obj.saviorVector.size(); ++j)
				if (&obj.wQueens[i] == obj.saviorVector[j])
				{
					saviorVector.push_back(&wQueens[i]);
					break;
				}
		}
		else
		{
			for (int j = 0; j < obj.checkVector.size(); ++j)
				if (&obj.wQueens[i] == obj.checkVector[j])
				{
					checkVector.push_back(&wQueens[i]);
					break;
				}
		}
	}

	// Copy white king and sync to board.
	wKing.push_back(obj.wKing.front());
	board[wKing.front().getColumn()][wKing.front().getRow()] = &wKing.front();

	// Note:  Kings can be their own saviors, but cannot put another king in check.
	if (turn == WHITE)
	{
		for (int j = 0; j < obj.saviorVector.size(); ++j)
			if (&obj.wKing.front()== obj.saviorVector[j])
			{
				saviorVector.push_back(&wKing.front());
				break;
			}
	}

	// Copy escapeVector
	for (int i = 0; i < obj.escapeVector.size(); ++i)
		escapeVector.push_back(obj.escapeVector[i]);

	// Copy attackVector
	for (int i = 0; i < obj.attackVector.size(); ++i)
		attackVector.push_back(obj.attackVector[i]);

	// Probably superfluous, but what the hell!
	checkmate = obj.checkmate;

	// Debug message
	/*
	if (DEBUG)
	{
		using std::cout;
		using std::endl;
		cout << "==================" << endl
			<< "copyBoard Contents" << endl
			<< "==================" << endl;
		for (int row = 0; row < 8; ++row)
		{
			using std::cout;
			using std::endl;
			for (int col = 0; col < 8; ++col)
			{
				if (board[col][row])
					cout << "(" << col << ", " << row << ")"
					<< " == " << board[col][row]->getColor() << " "
					<< board[col][row]->getType() << endl;
			}
			cout << endl;
		}
		cout << "=================" << endl;
	}
	*/
}

//	Overloaded Assignment Operator - essentially identical to what the copy constructor does.
void chessBoardClass::operator=(const chessBoardClass& obj)
{
	//	Sets the flag, which is used in pawnPromotion to bypass console window input commands.
	ownedByAI = true;

	//	Clear calling object's piece vectors.
	bPawns.clear(); bKnights.clear(); bBishops.clear(); bRooks.clear(); bQueens.clear(); bKing.clear();
	wPawns.clear(); wKnights.clear(); wBishops.clear(); wRooks.clear(); wQueens.clear(); wKing.clear();

	//	Clear game-state data vectors.
	checkVector.clear(); escapeVector.clear(); attackVector.clear();
	saviorVector.clear(); pinVector.clear(); defenderVector.clear();

	// Set turn, useful for vector copies.
	turn = obj.turn;

	//	Copy black obj piece positions and create new pieces.
	//	These loops also populate saviorVector and checkVector appropriately.

	// Initialize the board.
	for (int i = 0; i < 8; ++i)
		for (int j = 0; j < 8; ++j)
			board[i][j] = NULL;

	// Copy black pawns and sync to board.
	for (int i = 0; i < obj.bPawns.size(); ++i)
	{
		//	Don't bother copying over captured pieces.
		if (obj.bPawns[i].getCaptured())
			continue;
		
			bPawns.push_back(obj.bPawns[i]);
		board[bPawns.back().getColumn()][bPawns.back().getRow()] = &bPawns.back();

		// Deep copy any pawns that are part of obj.saviorVector
		if (turn == BLACK)
		{
			for (int j = 0; j < obj.saviorVector.size(); ++j)
				if (&obj.bPawns[i] == obj.saviorVector[j])
				{
					saviorVector.push_back(&bPawns.back());
					break;
				}

			for (int j = 0; j < obj.pinVector.size(); ++j)
				if (&obj.bPawns[i] == obj.pinVector[j].first.first)
				{
					chessPiece* pinner = obj.pinVector[j].first.second;
					PIN_DIR pinDirection = obj.pinVector[j].second;
					pinVector.push_back(std::make_pair(std::make_pair(&bPawns.back(), pinner), pinDirection));
				}

			for (auto itr = obj.defenderVector.begin(); itr != obj.defenderVector.end(); ++itr)
				if (&obj.bPawns[i] == itr->first)
					defenderVector.push_back(std::make_pair(&bPawns.back(), itr->second));
		}
		else // if (turn == WHITE) deep copy any pawns that are part of obj.checkVector
		{
			for (int j = 0; j < obj.checkVector.size(); ++j)
				if (&obj.bPawns[i] == obj.checkVector[j])
				{
					checkVector.push_back(&bPawns.back());
					break;
				}
		}
	}

	// Copy black knights and sync to board.
	for (int i = 0; i < obj.bKnights.size(); ++i)
	{
		if (obj.bKnights[i].getCaptured())
			continue;

		bKnights.push_back(obj.bKnights[i]);
		board[bKnights.back().getColumn()][bKnights.back().getRow()] = &bKnights.back();

		if (turn == BLACK)
		{
			for (int j = 0; j < obj.saviorVector.size(); ++j)
				if (&obj.bKnights[i] == obj.saviorVector[j])
				{
					saviorVector.push_back(&bKnights.back());
					break;
				}

			for (int j = 0; j < obj.pinVector.size(); ++j)
				if (&obj.bKnights[i] == obj.pinVector[j].first.first)
				{
					chessPiece* pinner = obj.pinVector[j].first.second;
					PIN_DIR pinDirection = obj.pinVector[j].second;
					pinVector.push_back(std::make_pair(std::make_pair(&bKnights.back(), pinner), pinDirection));
				}

			for (auto itr = obj.defenderVector.begin(); itr != obj.defenderVector.end(); ++itr)
				if (&obj.bKnights[i] == itr->first)
					defenderVector.push_back(std::make_pair(&bKnights.back(), itr->second));
		}
		else
		{
			for (int j = 0; j < obj.checkVector.size(); ++j)
				if (&obj.bKnights[i] == obj.checkVector[j])
				{
					checkVector.push_back(&bKnights.back());
					break;
				}
		}
	}

	// Copy black bishops and sync to board.
	for (int i = 0; i < obj.bBishops.size(); ++i)
	{
		if (obj.bBishops[i].getCaptured())
			continue;

		bBishops.push_back(obj.bBishops[i]);
		board[bBishops.back().getColumn()][bBishops.back().getRow()] = &bBishops.back();

		if (turn == BLACK)
		{
			for (int j = 0; j < obj.saviorVector.size(); ++j)
				if (&obj.bBishops[i] == obj.saviorVector[j])
				{
					saviorVector.push_back(&bBishops.back());
					break;
				}

			for (int j = 0; j < obj.pinVector.size(); ++j)
				if (&obj.bBishops[i] == obj.pinVector[j].first.first)
				{
					chessPiece* pinner = obj.pinVector[j].first.second;
					PIN_DIR pinDirection = obj.pinVector[j].second;
					pinVector.push_back(std::make_pair(std::make_pair(&bBishops.back(), pinner), pinDirection));
				}

			for (auto itr = obj.defenderVector.begin(); itr != obj.defenderVector.end(); ++itr)
				if (&obj.bBishops[i] == itr->first)
					defenderVector.push_back(std::make_pair(&bBishops.back(), itr->second));
		}
		else
		{
			for (int j = 0; j < obj.checkVector.size(); ++j)
				if (&obj.bBishops[i] == obj.checkVector[j])
				{
					checkVector.push_back(&bBishops.back());
					break;
				}
		}
	}

	// Copy black rooks and sync to board.
	for (int i = 0; i < obj.bRooks.size(); ++i)
	{
		if (obj.bRooks[i].getCaptured())
			continue;

		bRooks.push_back(obj.bRooks[i]);
		board[bRooks.back().getColumn()][bRooks.back().getRow()] = &bRooks.back();

		if (turn == BLACK)
		{
			for (int j = 0; j < obj.saviorVector.size(); ++j)
				if (&obj.bRooks[i] == obj.saviorVector[j])
				{
					saviorVector.push_back(&bRooks.back());
					break;
				}

			for (int j = 0; j < obj.pinVector.size(); ++j)
				if (&obj.bRooks[i] == obj.pinVector[j].first.first)
				{
					chessPiece* pinner = obj.pinVector[j].first.second;
					PIN_DIR pinDirection = obj.pinVector[j].second;
					pinVector.push_back(std::make_pair(std::make_pair(&bRooks.back(), pinner), pinDirection));
				}

			for (auto itr = obj.defenderVector.begin(); itr != obj.defenderVector.end(); ++itr)
				if (&obj.bRooks[i] == itr->first)
					defenderVector.push_back(std::make_pair(&bRooks.back(), itr->second));
		}
		else
		{
			for (int j = 0; j < obj.checkVector.size(); ++j)
				if (&obj.bRooks[i] == obj.checkVector[j])
				{
					checkVector.push_back(&bRooks.back());
					break;
				}
		}
	}

	// Copy black queens and sync to board.
	for (int i = 0; i < obj.bQueens.size(); ++i)
	{
		if (obj.bQueens[i].getCaptured())
			continue;

		bQueens.push_back(obj.bQueens[i]);
		board[bQueens.back().getColumn()][bQueens.back().getRow()] = &bQueens.back();

		if (turn == BLACK)
		{
			for (int j = 0; j < obj.saviorVector.size(); ++j)
				if (&obj.bQueens[i] == obj.saviorVector[j])
				{
					saviorVector.push_back(&bQueens.back());
					break;
				}

			for (int j = 0; j < obj.pinVector.size(); ++j)
				if (&obj.bQueens[i] == obj.pinVector[j].first.first)
				{
					chessPiece* pinner = obj.pinVector[j].first.second;
					PIN_DIR pinDirection = obj.pinVector[j].second;
					pinVector.push_back(std::make_pair(std::make_pair(&bQueens.back(), pinner), pinDirection));
				}

			for (auto itr = obj.defenderVector.begin(); itr != obj.defenderVector.end(); ++itr)
				if (&obj.bQueens[i] == itr->first)
					defenderVector.push_back(std::make_pair(&bQueens.back(), itr->second));
		}
		else
		{
			for (int j = 0; j < obj.checkVector.size(); ++j)
				if (&obj.bQueens[i] == obj.checkVector[j])
				{
					checkVector.push_back(&bQueens.back());
					break;
				}
		}
	}

	// Copy black king and sync to board.
	bKing.push_back(obj.bKing.front());
	board[bKing.front().getColumn()][bKing.front().getRow()] = &bKing.front();

	// Note:  Kings can be their own saviors, but cannot put another king in check.
	if (turn == BLACK)
	{
		for (int j = 0; j < obj.saviorVector.size(); ++j)
			if (&obj.bKing.front() == obj.saviorVector[j])
			{
				saviorVector.push_back(&bKing.front());
				break;
			}
	}


	// Rinse and repeat for white pieces

	// Copy white pawns and sync to board.
	for (int i = 0; i < obj.wPawns.size(); ++i)
	{
		if (obj.wPawns[i].getCaptured())
			continue;

		wPawns.push_back(obj.wPawns[i]);
		board[wPawns.back().getColumn()][wPawns.back().getRow()] = &wPawns.back();

		// Deep copy any pawns that are part of obj.saviorVector
		if (turn == WHITE)
		{
			for (int j = 0; j < obj.saviorVector.size(); ++j)
				if (&obj.wPawns[i] == obj.saviorVector[j])
				{
					saviorVector.push_back(&wPawns.back());
					break;
				}

			for (int j = 0; j < obj.pinVector.size(); ++j)
				if (&obj.wPawns[i] == obj.pinVector[j].first.first)
				{
					chessPiece* pinner = obj.pinVector[j].first.second;
					PIN_DIR pinDirection = obj.pinVector[j].second;
					pinVector.push_back(std::make_pair(std::make_pair(&wPawns.back(), pinner), pinDirection));
				}

			for (auto itr = obj.defenderVector.begin(); itr != obj.defenderVector.end(); ++itr)
				if (&obj.wPawns[i] == itr->first)
					defenderVector.push_back(std::make_pair(&wPawns.back(), itr->second));
		}
		else // if turn == "black" deep copy any pawns that are part of obj.checkVector
		{
			for (int j = 0; j < obj.checkVector.size(); ++j)
				if (&obj.wPawns[i] == obj.checkVector[j])
				{
					checkVector.push_back(&wPawns.back());
					break;
				}
		}
	}

	// Copy white knights and sync to board.
	for (int i = 0; i < obj.wKnights.size(); ++i)
	{
		if (obj.wKnights[i].getCaptured())
			continue;

		wKnights.push_back(obj.wKnights[i]);
		board[wKnights.back().getColumn()][wKnights.back().getRow()] = &wKnights.back();

		if (turn == WHITE)
		{
			for (int j = 0; j < obj.saviorVector.size(); ++j)
				if (&obj.wKnights[i] == obj.saviorVector[j])
				{
					saviorVector.push_back(&wKnights.back());
					break;
				}

			for (int j = 0; j < obj.pinVector.size(); ++j)
				if (&obj.wKnights[i] == obj.pinVector[j].first.first)
				{
					chessPiece* pinner = obj.pinVector[j].first.second;
					PIN_DIR pinDirection = obj.pinVector[j].second;
					pinVector.push_back(std::make_pair(std::make_pair(&wKnights.back(), pinner), pinDirection));
				}

			for (auto itr = obj.defenderVector.begin(); itr != obj.defenderVector.end(); ++itr)
				if (&obj.wKnights[i] == itr->first)
					defenderVector.push_back(std::make_pair(&wKnights.back(), itr->second));
		}
		else
		{
			for (int j = 0; j < obj.checkVector.size(); ++j)
				if (&obj.wKnights[i] == obj.checkVector[j])
				{
					checkVector.push_back(&wKnights.back());
					break;
				}
		}
	}

	// Copy white bishops and sync to board.
	for (int i = 0; i < obj.wBishops.size(); ++i)
	{
		if (obj.wBishops[i].getCaptured())
			continue;

		wBishops.push_back(obj.wBishops[i]);
		board[wBishops.back().getColumn()][wBishops.back().getRow()] = &wBishops.back();

		if (turn == WHITE)
		{
			for (int j = 0; j < obj.saviorVector.size(); ++j)
				if (&obj.wBishops[i] == obj.saviorVector[j])
				{
					saviorVector.push_back(&wBishops.back());
					break;
				}

			for (int j = 0; j < obj.pinVector.size(); ++j)
				if (&obj.wBishops[i] == obj.pinVector[j].first.first)
				{
					chessPiece* pinner = obj.pinVector[j].first.second;
					PIN_DIR pinDirection = obj.pinVector[j].second;
					pinVector.push_back(std::make_pair(std::make_pair(&wBishops.back(), pinner), pinDirection));
				}

			for (auto itr = obj.defenderVector.begin(); itr != obj.defenderVector.end(); ++itr)
				if (&obj.wBishops[i] == itr->first)
					defenderVector.push_back(std::make_pair(&wBishops.back(), itr->second));
		}
		else
		{
			for (int j = 0; j < obj.checkVector.size(); ++j)
				if (&obj.wBishops[i] == obj.checkVector[j])
				{
					checkVector.push_back(&wBishops.back());
					break;
				}
		}
	}

	// Copy white rooks and sync to board.
	for (int i = 0; i < obj.wRooks.size(); ++i)
	{
		if (obj.wRooks[i].getCaptured())
			continue;

		wRooks.push_back(obj.wRooks[i]);
		board[wRooks.back().getColumn()][wRooks.back().getRow()] = &wRooks.back();

		if (turn == WHITE)
		{
			for (int j = 0; j < obj.saviorVector.size(); ++j)
				if (&obj.wRooks[i] == obj.saviorVector[j])
				{
					saviorVector.push_back(&wRooks.back());
					break;
				}

			for (int j = 0; j < obj.pinVector.size(); ++j)
				if (&obj.wRooks[i] == obj.pinVector[j].first.first)
				{
					chessPiece* pinner = obj.pinVector[j].first.second;
					PIN_DIR pinDirection = obj.pinVector[j].second;
					pinVector.push_back(std::make_pair(std::make_pair(&wRooks.back(), pinner), pinDirection));
				}

			for (auto itr = obj.defenderVector.begin(); itr != obj.defenderVector.end(); ++itr)
				if (&obj.wRooks[i] == itr->first)
					defenderVector.push_back(std::make_pair(&wRooks.back(), itr->second));
		}
		else
		{
			for (int j = 0; j < obj.checkVector.size(); ++j)
				if (&obj.wRooks[i] == obj.checkVector[j])
				{
					checkVector.push_back(&wRooks.back());
					break;
				}
		}
	}

	// Copy white queens and sync to board.
	for (int i = 0; i < obj.wQueens.size(); ++i)
	{
		if (obj.wQueens[i].getCaptured())
			continue;

		wQueens.push_back(obj.wQueens[i]);
		board[wQueens.back().getColumn()][wQueens.back().getRow()] = &wQueens.back();

		if (turn == WHITE)
		{
			for (int j = 0; j < obj.saviorVector.size(); ++j)
				if (&obj.wQueens[i] == obj.saviorVector[j])
				{
					saviorVector.push_back(&wQueens.back());
					break;
				}

			for (int j = 0; j < obj.pinVector.size(); ++j)
				if (&obj.wQueens[i] == obj.pinVector[j].first.first)
				{
					chessPiece* pinner = obj.pinVector[j].first.second;
					PIN_DIR pinDirection = obj.pinVector[j].second;
					pinVector.push_back(std::make_pair(std::make_pair(&wQueens.back(), pinner), pinDirection));
				}

			for (auto itr = obj.defenderVector.begin(); itr != obj.defenderVector.end(); ++itr)
				if (&obj.wQueens[i] == itr->first)
					defenderVector.push_back(std::make_pair(&wQueens.back(), itr->second));
		}
		else
		{
			for (int j = 0; j < obj.checkVector.size(); ++j)
				if (&obj.wQueens[i] == obj.checkVector[j])
				{
					checkVector.push_back(&wQueens.back());
					break;
				}
		}
	}

	// Copy white king and sync to board.
	wKing.push_back(obj.wKing.front());
	board[wKing.front().getColumn()][wKing.front().getRow()] = &wKing.front();

	// Note:  Kings can be their own saviors, but cannot put another king in check.
	if (turn == WHITE)
	{
		for (int j = 0; j < obj.saviorVector.size(); ++j)
			if (&obj.wKing.front() == obj.saviorVector[j])
			{
				saviorVector.push_back(&wKing.front());
				break;
			}
	}

	// Copy escapeVector
	for (int i = 0; i < obj.escapeVector.size(); ++i)
		escapeVector.push_back(obj.escapeVector[i]);

	// Copy attackVector
	for (int i = 0; i < obj.attackVector.size(); ++i)
		attackVector.push_back(obj.attackVector[i]);

	// Probably superfluous, but what the hell!
	checkmate = obj.checkmate;

	//	This is to counteract a bug related to pawnPromotion.


	// Debug message
	/*
	if (DEBUG)
	{
	using std::cout;
	using std::endl;
	cout << "==================" << endl
	<< "copyBoard Contents" << endl
	<< "==================" << endl;
	for (int row = 0; row < 8; ++row)
	{
	using std::cout;
	using std::endl;
	for (int col = 0; col < 8; ++col)
	{
	if (board[col][row])
	cout << "(" << col << ", " << row << ")"
	<< " == " << board[col][row]->getColor() << " "
	<< board[col][row]->getType() << endl;
	}
	cout << endl;
	}
	cout << "=================" << endl;
	}
	*/
}

// Destructor
chessBoardClass::~chessBoardClass()
{
	//	Nothing is allocated off of the heap, so this is empty for now.
}

/*
	================
	Helper Functions
	================
*/

// Returns the address of the chessPiece type that is on the board at coordinates (c, r).
chessPiece* chessBoardClass::getSquareContents(int c, int r)
{
	if (c < 0 || c > 7 || r < 0 || r > 7)
		return NULL;
	else
		return board[c][r];
}

// Function will prepare the next game state for the other player
void chessBoardClass::swapTurn()
{
	// Flip the turn variable
	if (turn == WHITE)
		turn = BLACK;
	else
		turn = WHITE;

	// Clear all vectors for next turn.
	checkVector.clear();
	pinVector.clear();
	escapeVector.clear();
	attackVector.clear();
	saviorVector.clear();

	// Clear the en passant flags for the appropriate side.
	if (turn == WHITE)
	{
		for (int i = 0; i < wPawns.size(); ++i)
		{
			whitePawn* pawn = &wPawns[i];
			
			if (pawn != NULL)
				pawn->setEnPassant(false);
		}
	}
	else if (turn == BLACK)
	{
		for (int i = 0; i < bPawns.size(); ++i)
		{
			blackPawn* pawn = &bPawns[i];

			if (pawn != NULL)
				pawn->setEnPassant(false);
		}
	}
}

//	Allows the turn to be set manually, setting up the board and various data structures correctly.  Useful for AI related functions.
void chessBoardClass::setTurn(PIECE_COLOR c)
{
	turn = c;

	chessPiece* king;
	if (turn == WHITE)
		king = &wKing.front();
	else
		king = &bKing.front();

	// Populate checkVector with any pieces that are putting otherKing in check.
	checkVector = scanForCheck(*king);

	// Populate escape vector with pair<int, int> board coordinates that represent
	// squares that the king can safely move to.
	escapeVector = scanForEscapeSquares(*king);

	// Populate pinVector with any pieces that are currently pinned to their king.
	pinVector = scanForPins(*king);

	// The king is in check, populate the appropriate vectors.
	if (!checkVector.empty())
	{
		// Populate attackVector with pair<int,int> board coordinates that represent
		// squares between the checking piece(s) and the king.
		attackVector = setAttackVector(*king);

		// Populate saviorVector with chessPiece* elements that point to pieces that
		// can take the king out of check by being moved to a coordinate in attackVector
		saviorVector = scanForSaviors();
	}

	// If the king is in check,
	// AND there are no escape squares,
	// AND there are no savior pieces,
	// then checkmate has occured.
	if (checkVector.size() > 0 && escapeVector.empty() && saviorVector.empty())
		checkmate = true;

}

// Function will implement pawn promotion.  If a pawn reaches the end of the board,
// then it can be replaced with a knight, bishop, rook, or queen.
void chessBoardClass::pawnPromotion(chessPiece& pawn)
{
	PIECE_COLOR color = pawn.getColor();
	PIECE_TYPE type = pawn.getType();


	if (!this->getAI())
	{
		/// This should be swapped out with a GUI implementation at some point.
		///	This interrupts the flow of the game, sucks having to stop and type something out.
		while (!(type == KNIGHT || type == BISHOP || type == ROOK || type == QUEEN))
		{
			std::cout << "Enter piece to promote to (1. knight, 2. bishop, 3. rook, or 4. queen): ";
			int promotion;
			std::cin >> promotion;

			switch (promotion)
			{
			case 1: type = KNIGHT;
				break;
			case 2: type = BISHOP;
				break;
			case 3: type = ROOK;
				break;
			case 4:	type = QUEEN;
				break;
			}
		}
	}

	if (this->getAI())
		type = QUEEN;

	int col = pawn.getColumn();
	int row = pawn.getRow();

	// Get board ready to point at newly created piece.
	board[col][row] = NULL;

	/*
	The conditional blocks below will create the appropriate piece, with the right type and color.
	The piece will be pushed back onto the appropriate vector and the board will be set to point to
	this newly created piece.
	*/

	// Black piece promotion
	if (color == BLACK)
	{
		// Knight promotion
		if (type == KNIGHT)
		{
			blackKnight knight(col, row);
			bKnights.push_back(knight);
			board[col][row] = &bKnights.back();
		} // Bishop promotion
		else if (type == BISHOP)
		{
			blackBishop bishop(col, row);
			bBishops.push_back(bishop);
			board[col][row] = &bBishops.back();
		} // Rook promotion
		else if (type == ROOK)
		{
			blackRook rook(col, row);
			bRooks.push_back(rook);
			board[col][row] = &bRooks.back();
		}
		else // (type == "queen") Queen promotion
		{
			blackQueen queen(col, row);
			bQueens.push_back(queen);
			board[col][row] = &bQueens.back();
		}
	} // White promotion
	else // (color == "white")
	{	// Knight promotion
		if (type == KNIGHT)
		{
			whiteKnight knight(col, row);
			wKnights.push_back(knight);
			board[col][row] = &wKnights.back();
		}	// Bishop promotion
		else if (type == BISHOP)
		{
			whiteBishop bishop(col, row);
			wBishops.push_back(bishop);
			board[col][row] = &wBishops.back();
		}	// Rook promotion
		else if (type == ROOK)
		{
			whiteRook rook(col, row);
			wRooks.push_back(rook);
			board[col][row] = &wRooks.back();
		}
		else // (type == "queen") Queen promotion
		{
			whiteQueen queen(col, row);
			wQueens.push_back(queen);
			board[col][row] = &wQueens.back();
		}
	}

	//	There's been a recurring issue with the board losing track of pieces upon promotion, which I believe to be caused by the vector containers
	//	allocating additional space elsewhere to store the newly pushed-back pieces.  This will readjust the board to point to the pieces in the vectors.
	if (color == WHITE)
	{
		switch (type)
		{
		case KNIGHT:
			for (auto itr = wKnights.begin(); itr != wKnights.end(); ++itr)
				board[(*itr).getColumn()][(*itr).getRow()] = &(*itr);
			break;
		case BISHOP:
			for (auto itr = wBishops.begin(); itr != wBishops.end(); ++itr)
				board[(*itr).getColumn()][(*itr).getRow()] = &(*itr);
			break;
		case ROOK:
			for (auto itr = wRooks.begin(); itr != wRooks.end(); ++itr)
				board[(*itr).getColumn()][(*itr).getRow()] = &(*itr);
			break;
		case QUEEN:
			for (auto itr = wQueens.begin(); itr != wQueens.end(); ++itr)
				board[(*itr).getColumn()][(*itr).getRow()] = &(*itr);
		}
	}
	else if (color == BLACK)
	{
		switch (type)
		{
		case KNIGHT:
			for (auto itr = bKnights.begin(); itr != bKnights.end(); ++itr)
				board[(*itr).getColumn()][(*itr).getRow()] = &(*itr);
			break;
		case BISHOP:
			for (auto itr = bBishops.begin(); itr != bBishops.end(); ++itr)
				board[(*itr).getColumn()][(*itr).getRow()] = &(*itr);
			break;
		case ROOK:
			for (auto itr = bRooks.begin(); itr != bRooks.end(); ++itr)
				board[(*itr).getColumn()][(*itr).getRow()] = &(*itr);
			break;
		case QUEEN:
			for (auto itr = bQueens.begin(); itr != bQueens.end(); ++itr)
				board[(*itr).getColumn()][(*itr).getRow()] = &(*itr);
		}
	}
}

// Function will return a vector of pointer-to-chessPiece elements, which contains
// all of the pieces that are currently checking the king.
std::vector<chessPiece*> chessBoardClass::scanForCheck(chessPiece& king)
{
	std::vector<chessPiece*> attackers;

	int kingCol = king.getColumn(), kingRow = king.getRow();

	attackers = getAttackers(&king, kingCol, kingRow);

	return attackers;
}

// Function will return a vector of paired pointer-to-chessPiece/PIN_DIR pairs, which will represent
// the pieces that are currently pinned to the king passed in as an argument.
std::vector<std::pair<std::pair<chessPiece*, chessPiece*>, PIN_DIR>> chessBoardClass::scanForPins(chessPiece& king)
{
	// Pieces that are potentiall pinned are the ones that are acting as defenders to the king.
	std::vector<std::pair<chessPiece*, PIN_DIR>> pinCandidates;
	pinCandidates = scanForDefenders(king);

	std::vector<std::pair<std::pair<chessPiece*, chessPiece*>, PIN_DIR>> pinVector;

	// If the king has no defenders, then no pieces can be pinned.
	if (pinCandidates.empty())
		return pinVector;

	PIECE_COLOR friendly = king.getColor();

	// Go through each potentially pinned piece and assess if the piece is indeed pinned.
	// Horizontal and vertical pins come from enemy queens or rooks.
	// Diagonal pins come from enemy queens or bishops.
	// Pawns, knights, and kings cannot pin pieces.
	for (int i = 0; i < pinCandidates.size(); ++i)
	{
		chessPiece* defender = pinCandidates[i].first;
		PIN_DIR pinDirection = pinCandidates[i].second;

		int col = defender->getColumn();
		int row = defender->getRow();

		// Switch control structure ensures the correct scan is made for the current defender.
		switch (pinDirection)
		{
		case RIGHT:	// Scan for enemy to the right of the defender.
			for (int tracerC = col + 1; tracerC <= 7; ++tracerC)
			{
				chessPiece* occupier = getSquareContents(tracerC, row);

				if (occupier != NULL)
				{
					if (occupier->getColor() != friendly)
					{
						if (occupier->getType() == ROOK || occupier->getType() == QUEEN)
							pinVector.push_back(std::make_pair(std::make_pair(defender, occupier), pinDirection));
						// Bit of a "mouth-full" here, but its essentially a ( (chessPiece, chessPiece), pinDirection )
						// pair that is pushed to the back of pinVector.  The first element of the "outer" pair just 
						// happens to be a pair of elements itself.
					}

					break;
				}
			}
			break;

		case UP_RIGHT:	// Scan for enemy to the up-right of the defender.
			for (int tracerC = col + 1, tracerR = row + 1; tracerC <= 7 && tracerR <= 7; ++tracerC, ++tracerR)
			{
				chessPiece* occupier = getSquareContents(tracerC, tracerR);

				if (occupier != NULL)
				{
					if (occupier->getColor() != friendly)
					{
						if (occupier->getType() == BISHOP || occupier->getType() == QUEEN)
							pinVector.push_back(std::make_pair(std::make_pair(defender, occupier), pinDirection));
					}

					break;
				}
			}
			break;

		case UP:	// Scan for enemy above the defender.
			for (int tracerR = row + 1; tracerR <= 7; ++tracerR)
			{
				chessPiece* occupier = getSquareContents(col, tracerR);

				if (occupier != NULL)
				{
					if (occupier->getColor() != friendly)
					{
						if (occupier->getType() == ROOK || occupier->getType() == QUEEN)
							pinVector.push_back(std::make_pair(std::make_pair(defender, occupier), pinDirection));
					}

					break;
				}
			}
			break;

		case UP_LEFT:	// Scan for enemy to the up-left of the defender.
			for (int tracerC = col - 1, tracerR = row + 1; tracerC >= 0 && tracerR <= 7; --tracerC, ++tracerR)
			{
				chessPiece* occupier = getSquareContents(tracerC, tracerR);

				if (occupier != NULL)
				{
					if (occupier->getColor() != friendly)
					{
						if (occupier->getType() == BISHOP || occupier->getType() == QUEEN)
							pinVector.push_back(std::make_pair(std::make_pair(defender, occupier), pinDirection));
					}

					break;
				}
			}
			break;

		case LEFT:	// Scan for enemy to the left of the defender.
			for (int tracerC = col - 1; tracerC >= 0; --tracerC)
			{
				chessPiece* occupier = getSquareContents(tracerC, row);

				if (occupier != NULL)
				{
					if (occupier->getColor() != friendly)
					{
						if (occupier->getType() == ROOK || occupier->getType() == QUEEN)
							pinVector.push_back(std::make_pair(std::make_pair(defender, occupier), pinDirection));
					}

					break;
				}
			}
			break;

		case DOWN_LEFT:	// Scan for enemy to the down-left of the defender.
			for (int tracerC = col - 1, tracerR = row - 1; tracerC >= 0 && tracerR >= 0; --tracerC, --tracerR)
			{
				chessPiece* occupier = getSquareContents(tracerC, tracerR);

				if (occupier != NULL)
				{
					if (occupier->getColor() != friendly)
					{
						if (occupier->getType() == BISHOP || occupier->getType() == QUEEN)
							pinVector.push_back(std::make_pair(std::make_pair(defender, occupier), pinDirection));
					}

					break;
				}
			}
			break;

		case DOWN:	// Scan for enemy below the defender.
			for (int tracerR = row - 1; tracerR >= 0; --tracerR)
			{
				chessPiece* occupier = getSquareContents(col, tracerR);

				if (occupier != NULL)
				{
					if (occupier->getColor() != friendly)
					{
						if (occupier->getType() == ROOK || occupier->getType() == QUEEN)
							pinVector.push_back(std::make_pair(std::make_pair(defender, occupier), pinDirection));
					}

					break;
				}
			}
			break;

		case DOWN_RIGHT:	// Scan for enemy to the down-right of the defender.
			for (int tracerC = col + 1, tracerR = row - 1; tracerC <= 7 && tracerR >= 0; ++tracerC, --tracerR)
			{
				chessPiece* occupier = getSquareContents(tracerC, tracerR);

				if (occupier != NULL)
				{
					if (occupier->getColor() != friendly)
					{
						if (occupier->getType() == BISHOP || occupier->getType() == QUEEN)
							pinVector.push_back(std::make_pair(std::make_pair(defender, occupier), pinDirection));
					}

					break;
				}
			}
			break;
		}

	}

	// All pinned pieces are now in pinVector.
	return pinVector;

}

// Function will return a vector of std::pair<chessPiece*, PIN_DIR> objects that will represent pieces that are currently
// defending the king.  Defending pieces are pieces that are on the same row, column, or diagonal as the king, such that
// there are no other pieces - friendly or enemy - in between the defender and the king.
std::vector<std::pair<chessPiece*, PIN_DIR>> chessBoardClass::scanForDefenders(chessPiece& king)
{
	int col = king.getColumn();
	int row = king.getRow();
	PIECE_COLOR friendly = king.getColor();

	// Will hold all of the chessPieces that are within 8-directional "eye-sight" of the king, as well as
	// their position relative to the king.
	std::vector<std::pair<chessPiece*, PIN_DIR>> defenders;

	// Value will reflect defender's position relative to the king's position.
	PIN_DIR defensivePosition;

	/*
	Eight-Directional Scan For Defenders
	- First friendly piece encountered in each direction is a defender -
	*/

	// Right Scan - Scan King's row to the right for a defender.
	defensivePosition = RIGHT;
	for (int tracerC = col + 1; tracerC <= 7; ++tracerC)
	{
		chessPiece* occupier = getSquareContents(tracerC, row);

		if (occupier != NULL)
		{
			// Occupier is friendly - a defender.
			if (occupier->getColor() == friendly)
				defenders.push_back(std::make_pair(occupier, defensivePosition));
			else // Occupier is an enemy - no defenders are present in this direction.
				break;

			break;
		}
	}

	// Up-Right Scan - Scan King's diagonal to the up-right for a defender.
	defensivePosition = UP_RIGHT;
	for (int tracerC = col + 1, tracerR = row + 1; tracerC <= 7 && tracerR <= 7; ++tracerC, ++tracerR)
	{
		chessPiece* occupier = getSquareContents(tracerC, tracerR);

		if (occupier != NULL)
		{
			// Occupier is friendly - a defender.
			if (occupier->getColor() == friendly)
				defenders.push_back(std::make_pair(occupier, defensivePosition));
			else // Occupier is an enemy - no defenders are present in this direction.
				break;

			break;
		}
	}

	// Up Scan - Scan King's column upward for a defender.
	defensivePosition = UP;
	for (int tracerR = row + 1; tracerR <= 7; ++tracerR)
	{
		chessPiece* occupier = getSquareContents(col, tracerR);

		if (occupier != NULL)
		{
			// Occupier is friendly - a defender.
			if (occupier->getColor() == friendly)
				defenders.push_back(std::make_pair(occupier, defensivePosition));
			else // Occupier is an enemy - no defenders are present in this direction.
				break;

			break;
		}
	}

	// Up-Left Scan - Scan King's diagonal to the up-left for a defender.
	defensivePosition = UP_LEFT;
	for (int tracerC = col - 1, tracerR = row + 1; tracerC >= 0 && tracerR <= 7; --tracerC, ++tracerR)
	{
		chessPiece* occupier = getSquareContents(tracerC, tracerR);

		if (occupier != NULL)
		{
			// Occupier is friendly - a defender.
			if (occupier->getColor() == friendly)
				defenders.push_back(std::make_pair(occupier, defensivePosition));
			else // Occupier is an enemy - no defenders are present in this direction.
				break;

			break;
		}
	}

	// Left Scan - Scan King's row to the left for a defender.
	defensivePosition = LEFT;
	for (int tracerC = col - 1; tracerC >= 0; --tracerC)
	{
		chessPiece* occupier = getSquareContents(tracerC, row);

		if (occupier != NULL)
		{
			// Occupier is friendly - a defender.
			if (occupier->getColor() == friendly)
				defenders.push_back(std::make_pair(occupier, defensivePosition));
			else // Occupier is an enemy - no defenders are present in this direction.
				break;

			break;
		}
	}

	// Down-Left Scan - Scan King's diagonal to the down-left for a defender.
	defensivePosition = DOWN_LEFT;
	for (int tracerC = col - 1, tracerR = row - 1; tracerC >= 0 && tracerR >= 0; --tracerC, --tracerR)
	{
		chessPiece* occupier = getSquareContents(tracerC, tracerR);

		if (occupier != NULL)
		{
			// Occupier is friendly - a defender.
			if (occupier->getColor() == friendly)
				defenders.push_back(std::make_pair(occupier, defensivePosition));
			else // Occupier is an enemy - no defenders are present in this direction.
				break;

			break;
		}
	}

	// Down Scan - Scan King's column downward for a defender.
	defensivePosition = DOWN;
	for (int tracerR = row - 1; tracerR >= 0; --tracerR)
	{
		chessPiece* occupier = getSquareContents(col, tracerR);

		if (occupier != NULL)
		{
			// Occupier is friendly - a defender.
			if (occupier->getColor() == friendly)
				defenders.push_back(std::make_pair(occupier, defensivePosition));
			else // Occupier is an enemy - no defenders are present in this direction.
				break;

			break;
		}
	}

	// Down-Right Scan - Scan King's diagonal to the down-right for a defender.
	defensivePosition = DOWN_RIGHT;
	for (int tracerC = col + 1, tracerR = row - 1; tracerC <= 7 && tracerR >= 0; ++tracerC, --tracerR)
	{
		chessPiece* occupier = getSquareContents(tracerC, tracerR);

		if (occupier != NULL)
		{
			// Occupier is friendly - a defender.
			if (occupier->getColor() == friendly)
				defenders.push_back(std::make_pair(occupier, defensivePosition));
			else // Occupier is an enemy - no defenders are present in this direction.
				break;

			break;
		}
	}

	// Scans complete - assign to object's defenderVector and return vector.

	this->defenderVector = defenders;

	return defenders;
}

// Function will return a vector of pair<int, int> elements that are squares on the board
// that a "savior" piece can put itself on to block a check, putting itself between the king
// and an attacking (checking) piece, as well as the square of the attacker itself.
std::vector<std::pair<int, int>> chessBoardClass::setAttackVector(chessPiece& king)
{
	int kingCol = king.getColumn();
	int kingRow = king.getRow();

	// Will hold all of the coordinates involved in a check.
	std::vector<std::pair<int, int>> coordinateVector;

	for (int i = 0; i < checkVector.size(); ++i)
	{
		chessPiece* attacker = checkVector[i];

		int attackerCol = attacker->getColumn();
		int attackerRow = attacker->getRow();

		// Add the attacker's coordinates to coordinateVector
		coordinateVector.push_back(std::make_pair(attackerCol, attackerRow));

		if (kingCol == attackerCol) // If king and attacker are on the same column, it is a vertical attack
		{
			// King is above the attacker
			if (kingRow > attackerRow)
				for (int tracerR = kingRow - 1; tracerR != attackerRow; --tracerR)
					coordinateVector.push_back(std::make_pair(attackerCol, tracerR));
			else // King is below the attacker.
				for (int tracerR = kingRow + 1; tracerR != attackerRow; ++tracerR)
					coordinateVector.push_back(std::make_pair(attackerCol, tracerR));
		}
		else if (kingRow == attackerRow) // If king and attacker are on the same row, it is a horizontal attack
		{
			// King is to the right of the attacker
			if (kingCol > attackerCol)
				for (int tracerC = kingCol - 1; tracerC != attackerCol; --tracerC)
					coordinateVector.push_back(std::make_pair(tracerC, attackerRow));
			else // King is to the left of the attacker
				for (int tracerC = kingCol + 1; tracerC != attackerCol; ++tracerC)
					coordinateVector.push_back(std::make_pair(tracerC, attackerRow));
		}
		else if (abs(kingCol - attackerCol) == abs(kingRow - attackerRow))	// Diagonal attack case
		{

			if (kingCol < attackerCol) // King is to the left of the attacker
			{
				if (kingRow < attackerRow) // King is to the left and below the attacker
					for (int tracerC = kingCol + 1, tracerR = kingRow + 1; tracerC != attackerCol && tracerR != attackerRow; ++tracerC, ++tracerR)
						coordinateVector.push_back(std::make_pair(tracerC, tracerR));
				else if (kingRow > attackerRow) // King is to the left and above the attacker
					for (int tracerC = kingCol + 1, tracerR = kingRow - 1; tracerC != attackerCol && tracerR != attackerRow; ++tracerC, --tracerR)
						coordinateVector.push_back(std::make_pair(tracerC, tracerR));
			}
			else if (kingCol > attackerCol) // King is to the right of the attacker
			{
				if (kingRow < attackerRow) // King is to the right and below the attacker
					for (int tracerC = kingCol - 1, tracerR = kingRow + 1; tracerC != attackerCol && tracerR != attackerRow; --tracerC, ++tracerR)
						coordinateVector.push_back(std::make_pair(tracerC, tracerR));
				else if (kingRow > attackerRow) // King is to the right and above the attacker
					for (int tracerC = kingCol - 1, tracerR = kingRow - 1; tracerC != attackerCol && tracerR != attackerRow; --tracerC, --tracerR)
						coordinateVector.push_back(std::make_pair(tracerC, tracerR));
			}
		}
		else // Only other attack case is a knight, which cannot be blocked.
		{
			continue;
		}
	}

	return coordinateVector;
}

// Function will return a vector of chessPiece* elements that are the pieces that
// can save the king by placing itself between a checking piece and the king.
std::vector<chessPiece*> chessBoardClass::scanForSaviors()
{
	/*
	This will function similarly to getAttackers.

	The method will scan for friendly pieces that can be moved to
	coordinates that are in attackVector.
	*/
	std::pair<int, int> coordinate;

	chessPiece* piece;

	PIECE_COLOR friendlyColor = turn;

	std::vector<chessPiece*> saviors;

	// Uses coordinates stored in attackVector to scan for savior pieces.
	for (int i = 0; i < attackVector.size(); ++i)
	{
		coordinate = attackVector[i];

		// Scan for friendly pieces that can move to the square (coordinate).


		//		===============
		//		Horizontal Scan
		//		===============
		/*
		Saviors are friendly pieces that can move horizontally to get to coordinate.

		Savior Types:  Rook, Queen
		*/
		// Horizontal Scan Left
		for (int tracerC = coordinate.first - 1; tracerC >= 0; --tracerC)
		{
			piece = getSquareContents(tracerC, coordinate.second);

			if (piece == NULL)
				continue;
			else if (piece != NULL)
			{
				if (piece->getColor() == friendlyColor && (piece->getType() == QUEEN || piece->getType() == ROOK))
					saviors.push_back(piece);

				break;
			}
		}
		// Horizontal Scan Right
		for (int tracerC = coordinate.first + 1; tracerC <= 7; ++tracerC)
		{
			piece = getSquareContents(tracerC, coordinate.second);

			if (piece == NULL)
				continue;
			else if (piece != NULL)
			{
				if (piece->getColor() == friendlyColor && (piece->getType() == QUEEN || piece->getType() == ROOK))
					saviors.push_back(piece);

				break;
			}
		}


		//		=============
		//		Vertical Scan
		//		=============
		/*
		Saviors are friendly pieces that can move vertically to get to coordinate.

		Savior Types:  Pawn, Rook, Queen

		Special Note on Pawn Case:
		Pawns that end up being selected should be tested before being added to the saviors vector.
		If friendlyColor == "black", then a black pawn shouldn't be added to saviors in the scan down
		case (it can't move backward to coord).
		Same goes for if friendlyColor == "white"; the white pawn shouldn't be added to saviors in the
		scan up case, since the pawn can't move back down to the coordinate.
		Note:  Pawns cannot move vertically onto square if it is occupied (by the attacker)
		*/
		// Vertical Scan Up
		for (int tracerR = coordinate.second + 1; tracerR <= 7; ++tracerR)
		{
			piece = getSquareContents(coordinate.first, tracerR);

			if (piece == NULL)
				continue;
			else if (piece != NULL)
			{
				if (piece->getColor() == friendlyColor && (piece->getType() == PAWN || piece->getType() == ROOK || piece->getType() == QUEEN))
				{
					if (piece->getType() == PAWN)	// Pawns have limited range and direction of movement.
					{
						if (!piece->validMovement(coordinate.first, coordinate.second))	// Check range -> can pawn move to coordinates?
							break;
						else if (getSquareContents(coordinate.first, coordinate.second) != NULL) // Pawn cannot vertically capture.
							break;
						else
							saviors.push_back(piece);
					}
					else
						saviors.push_back(piece);	// Queens and rooks have no restrictions.

				}

				break;
			}
		}
		// Vertical Scan Down
		for (int tracerR = coordinate.second - 1; tracerR >= 0; --tracerR)
		{
			piece = getSquareContents(coordinate.first, tracerR);

			if (piece == NULL)
				continue;
			else if (piece != NULL)
			{
				if (piece->getColor() == friendlyColor && (piece->getType() == PAWN || piece->getType() == ROOK || piece->getType() == QUEEN))
				{
					if (piece->getType() == PAWN)	// Pawns have limited range and direction of movement.
					{
						if (!piece->validMovement(coordinate.first, coordinate.second))	// Check range -> can pawn move to coordinates?
							break;
						else if (getSquareContents(coordinate.first, coordinate.second) != NULL) // Pawn cannot vertically capture.
							break;
						else
							saviors.push_back(piece);
					}
					else
						saviors.push_back(piece);	// Queens and rooks have no restrictions.

				}

				break;
			}
		}

		//		=============
		//		Diagonal Scan
		//		=============
		/*
		Saviors are friendly pieces that can move diagonally to coordinate.

		Savior Types:  Pawn, Bishop, Queen

		Pawns should be checked to make sure they are only considered if they are in range.
		*/

		// Up-Right Scan
		for (int tracerC = coordinate.first + 1, tracerR = coordinate.second + 1; tracerC <= 7 && tracerR <= 7; ++tracerC, ++tracerR)
		{
			piece = getSquareContents(tracerC, tracerR);

			if (piece == NULL)
				continue;
			else if (piece != NULL)
			{
				if (piece->getColor() == friendlyColor && (piece->getType() == PAWN || piece->getType() == BISHOP || piece->getType() == QUEEN))
				{
					if (piece->getType() == PAWN) // Pawns have limited range and direction of movement.
					{
						if (!piece->validMovement(coordinate.first, coordinate.second))	// Check range -> can pawn move to coordinates?
							break;
						else if (getSquareContents(coordinate.first, coordinate.second) == NULL) // Can only diagonally move on occupied square (if enemy occupied).
							break;
						else
							saviors.push_back(piece);
					}
					else
						saviors.push_back(piece);	// Queens and bishops have no restrictions.
				}

				break;
			}
		}
		// Up-Left Scan
		for (int tracerC = coordinate.first - 1, tracerR = coordinate.second + 1; tracerC >= 0 && tracerR <= 7; --tracerC, ++tracerR)
		{
			piece = getSquareContents(tracerC, tracerR);

			if (piece == NULL)
				continue;
			else if (piece != NULL)
			{
				if (piece->getColor() == friendlyColor && (piece->getType() == PAWN || piece->getType() == BISHOP || piece->getType() == QUEEN))
				{
					if (piece->getType() == PAWN) // Pawns have limited range and direction of movement.
					{
						if (!piece->validMovement(coordinate.first, coordinate.second))	// Check range -> can pawn move to coordinates?
							break;
						else if (getSquareContents(coordinate.first, coordinate.second) == NULL) // Can only diagonally move on occupied square (if enemy occupied).
							break;
						else
							saviors.push_back(piece);
					}
					else
						saviors.push_back(piece);	// Queens and bishops have no restrictions.
				}

				break;
			}
		}
		// Down-Left Scan
		for (int tracerC = coordinate.first - 1, tracerR = coordinate.second - 1; tracerC >= 0 && tracerR >= 0; --tracerC, --tracerR)
		{
			piece = getSquareContents(tracerC, tracerR);

			if (piece == NULL)
				continue;
			else if (piece != NULL)
			{
				if (piece->getColor() == friendlyColor && (piece->getType() == PAWN || piece->getType() == BISHOP || piece->getType() == QUEEN))
				{
					if (piece->getType() == PAWN) // Pawns have limited range and direction of movement.
					{
						if (!piece->validMovement(coordinate.first, coordinate.second))	// Check range -> can pawn move to coordinates?
							break;
						else if (getSquareContents(coordinate.first, coordinate.second) == NULL) // Can only diagonally move on occupied square (if enemy occupied).
							break;
						else
							saviors.push_back(piece);
					}
					else
						saviors.push_back(piece);	// Queens and bishops have no restrictions.
				}

				break;
			}
		}
		// Down-Right Scan
		for (int tracerC = coordinate.first + 1, tracerR = coordinate.second - 1; tracerC <= 7 && tracerR >= 0; ++tracerC, --tracerR)
		{
			piece = getSquareContents(tracerC, tracerR);

			if (piece == NULL)
				continue;
			else if (piece != NULL)
			{
				if (piece->getColor() == friendlyColor && (piece->getType() == PAWN || piece->getType() == BISHOP || piece->getType() == QUEEN))
				{
					if (piece->getType() == PAWN) // Pawns have limited range and direction of movement.
					{
						if (!piece->validMovement(coordinate.first, coordinate.second))	// Check range -> can pawn move to coordinates?
							break;
						else if (getSquareContents(coordinate.first, coordinate.second) == NULL) // Can only diagonally move on occupied square (if enemy occupied).
							break;
						else
							saviors.push_back(piece);
					}
					else
						saviors.push_back(piece);	// Queens and bishops have no restrictions.
				}

				break;
			}
		}

		//		===========
		//		Knight Scan
		//		===========
		/*
		Saviors are friendly knights that can be moved to coordinate.

		Savior Type:  Obvious
		*/

		// Up 1 - Right 2 Case
		piece = getSquareContents(coordinate.first + 2, coordinate.second + 1);
		if (piece != NULL)
			if (piece->getColor() == friendlyColor && piece->getType() == KNIGHT)
				saviors.push_back(piece);

		// Up 2 - Right 1 Case
		piece = getSquareContents(coordinate.first + 1, coordinate.second + 2);
		if (piece != NULL)
			if (piece->getColor() == friendlyColor && piece->getType() == KNIGHT)
				saviors.push_back(piece);

		// Up 2 - Left 1 Case
		piece = getSquareContents(coordinate.first - 1, coordinate.second + 2);
		if (piece != NULL)
			if (piece->getColor() == friendlyColor && piece->getType() == KNIGHT)
				saviors.push_back(piece);

		// Up 1 - Left 2 Case
		piece = getSquareContents(coordinate.first - 2, coordinate.second + 1);
		if (piece != NULL)
			if (piece->getColor() == friendlyColor && piece->getType() == KNIGHT)
				saviors.push_back(piece);

		// Down 1 - Left 2 Case
		piece = getSquareContents(coordinate.first - 2, coordinate.second - 1);
		if (piece != NULL)
			if (piece->getColor() == friendlyColor && piece->getType() == KNIGHT)
				saviors.push_back(piece);

		// Down 2 - Left 1 Case
		piece = getSquareContents(coordinate.first - 1, coordinate.second - 2);
		if (piece != NULL)
			if (piece->getColor() == friendlyColor && piece->getType() == KNIGHT)
				saviors.push_back(piece);

		// Down 2 - Right 1 Case
		piece = getSquareContents(coordinate.first + 1, coordinate.second - 2);
		if (piece != NULL)
			if (piece->getColor() == friendlyColor && piece->getType() == KNIGHT)
				saviors.push_back(piece);

		// Down 1 - Right 2 Case
		piece = getSquareContents(coordinate.first + 2, coordinate.second - 1);
		if (piece != NULL)
			if (piece->getColor() == friendlyColor && piece->getType() == KNIGHT)
				saviors.push_back(piece);
	}

	// All potential saviors have been scanned for.
	return saviors;
}

// Determines if board coordinates (col, row) are under attack by any
// pieces that are the opposite of the current turn color
std::vector<chessPiece*> chessBoardClass::getAttackers(int col, int row)
{
	std::vector<chessPiece*> attackers;

	PIECE_COLOR friendlyColor = turn;
	PIECE_COLOR enemyColor;

	if (turn == WHITE)
		enemyColor = BLACK;
	else
		enemyColor = WHITE;

	chessPiece* scanner;
	PIECE_COLOR color;
	PIECE_TYPE type;

	/*
	=================
	Horizontal Attack
	=================
	Threats:  Queen, Rook, or King
	*/

	// Horizontal Scan Left
	for (int tracerC = col - 1; tracerC >= 0; --tracerC)
	{
		scanner = getSquareContents(tracerC, row);

		if (scanner == NULL)
			continue;
		else
		{
			color = scanner->getColor();
			type = scanner->getType();

			if (color == enemyColor)
				if (type == QUEEN || type == ROOK || (type == KING && tracerC == col - 1))
					attackers.push_back(scanner);

			break;
		}
	}
	// Horizontal Scan Right
	for (int tracerC = col + 1; tracerC <= 7; ++tracerC)
	{
		scanner = getSquareContents(tracerC, row);

		if (scanner == NULL)
			continue;
		else
		{
			color = scanner->getColor();
			type = scanner->getType();

			if (color == enemyColor)
				if (type == QUEEN || type == ROOK || (type == KING && tracerC == col + 1))
					attackers.push_back(scanner);

			break;
		}
	}

	/*
	===============
	Vertical Attack
	===============
	Threats:  Queen, Rook, or King
	*/

	// Vertical Scan Down
	for (int tracerR = row - 1; tracerR >= 0; --tracerR)
	{
		scanner = getSquareContents(col, tracerR);

		if (scanner == NULL)
			continue;
		else
		{
			color = scanner->getColor();
			type = scanner->getType();

			if (color == enemyColor)
				if (type == QUEEN || type == ROOK || (type == KING && tracerR == row - 1))
					attackers.push_back(scanner);

			break;
		}
	}
	// Vertical Scan Up
	for (int tracerR = row + 1; tracerR <= 7; ++tracerR)
	{
		scanner = getSquareContents(col, tracerR);

		if (scanner == NULL)
			continue;
		else
		{
			color = scanner->getColor();
			type = scanner->getType();

			if (color == enemyColor)
				if (type == QUEEN || type == ROOK || (type == KING && tracerR == row + 1))
					attackers.push_back(scanner);

			break;
		}
	}

	/*
	===============
	Diagonal Attack
	===============
	Threats:  Queen, Bishop, Pawn, or King
	*/

	// Diagonal Scan Up-Right
	for (int tracerC = col + 1, tracerR = row + 1; tracerC <= 7 && tracerR <= 7; ++tracerC, ++tracerR)
	{
		scanner = getSquareContents(tracerC, tracerR);

		if (scanner == NULL)
			continue;
		else
		{
			color = scanner->getColor();
			type = scanner->getType();

			if (color == enemyColor)						// Make sure pawn is within attack range to register a check
				if (type == QUEEN || type == BISHOP || ((type == PAWN && tracerC == col + 1 && tracerR == row + 1)) || (type == KING && tracerC == col + 1 && tracerR == row + 1))
					if (type == PAWN && color == WHITE)
						break;
					else
						attackers.push_back(scanner);

			break;
		}
	}
	// Diagonal Scan Up-Left
	for (int tracerC = col - 1, tracerR = row + 1; tracerC >= 0 && tracerR <= 7; --tracerC, ++tracerR)
	{
		scanner = getSquareContents(tracerC, tracerR);

		if (scanner == NULL)
			continue;
		else
		{
			color = scanner->getColor();
			type = scanner->getType();

			if (color == enemyColor)						    // Make sure pawn is within attack range to register a check
				if (type == QUEEN || type == BISHOP || ((type == PAWN && tracerC == col - 1 && tracerR == row + 1)) || (type == KING && tracerC == col - 1 && tracerR == row + 1))
					if (type == PAWN && color == WHITE)
						break;
					else
						attackers.push_back(scanner);

			break;
		}
	}
	// Diagonal Scan Down-Left
	for (int tracerC = col - 1, tracerR = row - 1; tracerC >= 0 && tracerR >= 0; --tracerC, --tracerR)
	{
		scanner = getSquareContents(tracerC, tracerR);

		if (scanner == NULL)
			continue;
		else
		{
			color = scanner->getColor();
			type = scanner->getType();
			if (color == enemyColor)						 // Make sure pawn is within attack range to register a check
				if (type == QUEEN || type == BISHOP || ((type == PAWN && tracerC == col - 1 && tracerR == row - 1)) || (type == KING && tracerC == col - 1 && tracerR == row - 1))
					if (type == PAWN && color == BLACK)
						break;
					else
						attackers.push_back(scanner);

			break;
		}
	}
	// Diagonal Scan Down-Right
	for (int tracerC = col + 1, tracerR = row - 1; tracerC <= 7 && tracerR >= 0; ++tracerC, --tracerR)
	{
		scanner = getSquareContents(tracerC, tracerR);

		if (scanner == NULL)
			continue;
		else
		{
			color = scanner->getColor();
			type = scanner->getType();

			if (color == enemyColor)						 // Make sure pawn is within attack range to register a check
				if (type == QUEEN || type == BISHOP || ((type == PAWN && tracerC == col + 1 && tracerR == row - 1)) || (type == KING && tracerC == col + 1 && tracerR == row - 1))
					if (type == PAWN && color == BLACK)
						break;
					else
						attackers.push_back(scanner);

			break;
		}
	}

	/*
	=============
	Knight Attack
	=============
	Threat:  Obvious
	*/
	// Note: getSquareContents returns NULL if arguments are outside of array boundaries.

	// Scan around board[col][row] for an attacking enemy knight.

	// Up 1, Right 2
	scanner = getSquareContents(col + 2, row + 1);
	if (scanner != NULL)
		if (scanner->getColor() == enemyColor && scanner->getType() == KNIGHT)
			attackers.push_back(scanner);

	// Up 2, Right 1
	scanner = getSquareContents(col + 1, row + 2);
	if (scanner != NULL)
		if (scanner->getColor() == enemyColor && scanner->getType() == KNIGHT)
			attackers.push_back(scanner);

	// Up 2, Left 1
	scanner = getSquareContents(col - 1, row + 2);
	if (scanner != NULL)
		if (scanner->getColor() == enemyColor && scanner->getType() == KNIGHT)
			attackers.push_back(scanner);

	// Up 1, Left 2
	scanner = getSquareContents(col - 2, row + 1);
	if (scanner != NULL)
		if (scanner->getColor() == enemyColor && scanner->getType() == KNIGHT)
			attackers.push_back(scanner);

	// Down 1, Left 2
	scanner = getSquareContents(col - 2, row - 1);
	if (scanner != NULL)
		if (scanner->getColor() == enemyColor && scanner->getType() == KNIGHT)
			attackers.push_back(scanner);

	// Down 2, Left 1
	scanner = getSquareContents(col - 1, row - 2);
	if (scanner != NULL)
		if (scanner->getColor() == enemyColor && scanner->getType() == KNIGHT)
			attackers.push_back(scanner);

	// Down 2, Right 1
	scanner = getSquareContents(col + 1, row - 2);
	if (scanner != NULL)
		if (scanner->getColor() == enemyColor && scanner->getType() == KNIGHT)
			attackers.push_back(scanner);

	// Down 1, Right 2
	scanner = getSquareContents(col + 2, row - 1);
	if (scanner != NULL)
		if (scanner->getColor() == enemyColor && scanner->getType() == KNIGHT)
			attackers.push_back(scanner);

	// All possible threats scanned for, return results.
	return attackers;
}

// Function will return a vector of pair<int, int> elements that are squares in proximity
// to the king that it may move to escape check.
std::vector<std::pair<int, int>> chessBoardClass::scanForEscapeSquares(chessPiece& king)
{
	int kingCol = king.getColumn();
	int kingRow = king.getRow();

	std::vector<std::pair<int, int>> coordinateVector;

	chessPiece* occupier;

	// Scan through the 8 squares adjacent to the king, storing the
	// coordinates to squares that the king can move to to escape check.
	for (int tracerC = kingCol - 1; (tracerC <= kingCol + 1) && (tracerC <= 7); ++tracerC)
	{
		if (tracerC < 0)	// Don't start the scan out of bounds.
			continue;

		for (int tracerR = kingRow - 1; (tracerR <= kingRow + 1) && (tracerR <= 7); ++tracerR)
		{
			if (tracerR < 0)	// Don't start the scan out of bounds.
				continue;

			// Check to see if square is currently occupied.
			occupier = getSquareContents(tracerC, tracerR);

			if (occupier != NULL)
			{
				if (occupier->getColor() == king.getColor())	// Square is currently occupied by friendly piece.
					continue;
				else if (occupier->getColor() != king.getColor())	// Square is occupied by an enemy piece.
				{
					std::vector<chessPiece*> attackers = getAttackers(&king, tracerC, tracerR);
					if (attackers.size() == 0)
						coordinateVector.push_back(std::make_pair(tracerC, tracerR));	// If enemy is undefended, it's a safe square to take.
					else
						continue;	// If enemy is defended, then square can't be taken.
				}
			}
			else if (occupier == NULL)
			{
				std::vector<chessPiece*> attackers = getAttackers(&king, tracerC, tracerR);
				if (attackers.size() == 0)
					coordinateVector.push_back(std::make_pair(tracerC, tracerR));	// Square is unoccupied and not under attack
				else
					continue;	// Square is unoccupied, but under attack (not safe).
			}
		}
	}

	return coordinateVector;
}

//	Specially designed for use in the scanForEscapeSquares method, to test if a square is safe under non-check king movement.
//	The issue:  The reason why this function is necessary is due to the fact that the original version has a significant oversight.
//				The oversight is that it the other getAttackers method fails to take into account that the king is in the way during
//				the eight-directional scanForEscapeSquares scan, so it won't detect bishops, rooks, and queens if the conditions are right.
//	The solution:	Temporarily modify the board so that the appropriate square no longer points to the king.  After the scans, point the
//					board back to the king.
std::vector<chessPiece*> chessBoardClass::getAttackers(chessPiece* king, int col, int row)
{
	std::vector<chessPiece*> attackers;

	board[king->getColumn()][king->getRow()] = NULL;

	PIECE_COLOR friendlyColor = turn;
	PIECE_COLOR enemyColor;

	if (turn == WHITE)
		enemyColor = BLACK;
	else
		enemyColor = WHITE;

	chessPiece* scanner;
	PIECE_COLOR color;
	PIECE_TYPE type;

	/*
	=================
	Horizontal Attack
	=================
	Threats:  Queen, Rook, or King
	*/

	// Horizontal Scan Left
	for (int tracerC = col - 1; tracerC >= 0; --tracerC)
	{
		scanner = getSquareContents(tracerC, row);

		if (scanner == NULL)
			continue;
		else
		{
			color = scanner->getColor();
			type = scanner->getType();

			if (color == enemyColor)
				if (type == QUEEN || type == ROOK || (type == KING && tracerC == col - 1))
					attackers.push_back(scanner);

			break;
		}
	}
	// Horizontal Scan Right
	for (int tracerC = col + 1; tracerC <= 7; ++tracerC)
	{
		scanner = getSquareContents(tracerC, row);

		if (scanner == NULL)
			continue;
		else
		{
			color = scanner->getColor();
			type = scanner->getType();

			if (color == enemyColor)
				if (type == QUEEN || type == ROOK || (type == KING && tracerC == col + 1))
					attackers.push_back(scanner);

			break;
		}
	}

	/*
	===============
	Vertical Attack
	===============
	Threats:  Queen, Rook, or King
	*/

	// Vertical Scan Down
	for (int tracerR = row - 1; tracerR >= 0; --tracerR)
	{
		scanner = getSquareContents(col, tracerR);

		if (scanner == NULL)
			continue;
		else
		{
			color = scanner->getColor();
			type = scanner->getType();

			if (color == enemyColor)
				if (type == QUEEN || type == ROOK || (type == KING && tracerR == row - 1))
					attackers.push_back(scanner);

			break;
		}
	}
	// Vertical Scan Up
	for (int tracerR = row + 1; tracerR <= 7; ++tracerR)
	{
		scanner = getSquareContents(col, tracerR);

		if (scanner == NULL)
			continue;
		else
		{
			color = scanner->getColor();
			type = scanner->getType();

			if (color == enemyColor)
				if (type == QUEEN || type == ROOK || (type == KING && tracerR == row + 1))
					attackers.push_back(scanner);

			break;
		}
	}

	/*
	===============
	Diagonal Attack
	===============
	Threats:  Queen, Bishop, Pawn, or King
	*/

	// Diagonal Scan Up-Right
	for (int tracerC = col + 1, tracerR = row + 1; tracerC <= 7 && tracerR <= 7; ++tracerC, ++tracerR)
	{
		scanner = getSquareContents(tracerC, tracerR);

		if (scanner == NULL)
			continue;
		else
		{
			color = scanner->getColor();
			type = scanner->getType();

			if (color == enemyColor)						// Make sure pawn is within attack range to register a check
				if (type == QUEEN || type == BISHOP || ((type == PAWN && tracerC == col + 1 && tracerR == row + 1)) || (type == KING && tracerC == col + 1 && tracerR == row + 1))
					if (type == PAWN && color == WHITE)
						break;
					else
						attackers.push_back(scanner);

			break;
		}
	}
	// Diagonal Scan Up-Left
	for (int tracerC = col - 1, tracerR = row + 1; tracerC >= 0 && tracerR <= 7; --tracerC, ++tracerR)
	{
		scanner = getSquareContents(tracerC, tracerR);

		if (scanner == NULL)
			continue;
		else
		{
			color = scanner->getColor();
			type = scanner->getType();

			if (color == enemyColor)						    // Make sure pawn is within attack range to register a check
				if (type == QUEEN || type == BISHOP || ((type == PAWN && tracerC == col - 1 && tracerR == row + 1)) || (type == KING && tracerC == col - 1 && tracerR == row + 1))
					if (type == PAWN && color == WHITE)
						break;
					else
						attackers.push_back(scanner);

			break;
		}
	}
	// Diagonal Scan Down-Left
	for (int tracerC = col - 1, tracerR = row - 1; tracerC >= 0 && tracerR >= 0; --tracerC, --tracerR)
	{
		scanner = getSquareContents(tracerC, tracerR);

		if (scanner == NULL)
			continue;
		else
		{
			color = scanner->getColor();
			type = scanner->getType();
			if (color == enemyColor)						 // Make sure pawn is within attack range to register a check
				if (type == QUEEN || type == BISHOP || ((type == PAWN && tracerC == col - 1 && tracerR == row - 1)) || (type == KING && tracerC == col - 1 && tracerR == row - 1))
					if (type == PAWN && color == BLACK)
						break;
					else
						attackers.push_back(scanner);

			break;
		}
	}
	// Diagonal Scan Down-Right
	for (int tracerC = col + 1, tracerR = row - 1; tracerC <= 7 && tracerR >= 0; ++tracerC, --tracerR)
	{
		scanner = getSquareContents(tracerC, tracerR);

		if (scanner == NULL)
			continue;
		else
		{
			color = scanner->getColor();
			type = scanner->getType();

			if (color == enemyColor)						 // Make sure pawn is within attack range to register a check
				if (type == QUEEN || type == BISHOP || ((type == PAWN && tracerC == col + 1 && tracerR == row - 1)) || (type == KING && tracerC == col + 1 && tracerR == row - 1))
					if (type == PAWN && color == BLACK)
						break;
					else
						attackers.push_back(scanner);

			break;
		}
	}

	/*
	=============
	Knight Attack
	=============
	Threat:  Obvious
	*/
	// Note: getSquareContents returns NULL if arguments are outside of array boundaries.

	// Scan around board[col][row] for an attacking enemy knight.

	// Up 1, Right 2
	scanner = getSquareContents(col + 2, row + 1);
	if (scanner != NULL)
		if (scanner->getColor() == enemyColor && scanner->getType() == KNIGHT)
			attackers.push_back(scanner);

	// Up 2, Right 1
	scanner = getSquareContents(col + 1, row + 2);
	if (scanner != NULL)
		if (scanner->getColor() == enemyColor && scanner->getType() == KNIGHT)
			attackers.push_back(scanner);

	// Up 2, Left 1
	scanner = getSquareContents(col - 1, row + 2);
	if (scanner != NULL)
		if (scanner->getColor() == enemyColor && scanner->getType() == KNIGHT)
			attackers.push_back(scanner);

	// Up 1, Left 2
	scanner = getSquareContents(col - 2, row + 1);
	if (scanner != NULL)
		if (scanner->getColor() == enemyColor && scanner->getType() == KNIGHT)
			attackers.push_back(scanner);

	// Down 1, Left 2
	scanner = getSquareContents(col - 2, row - 1);
	if (scanner != NULL)
		if (scanner->getColor() == enemyColor && scanner->getType() == KNIGHT)
			attackers.push_back(scanner);

	// Down 2, Left 1
	scanner = getSquareContents(col - 1, row - 2);
	if (scanner != NULL)
		if (scanner->getColor() == enemyColor && scanner->getType() == KNIGHT)
			attackers.push_back(scanner);

	// Down 2, Right 1
	scanner = getSquareContents(col + 1, row - 2);
	if (scanner != NULL)
		if (scanner->getColor() == enemyColor && scanner->getType() == KNIGHT)
			attackers.push_back(scanner);

	// Down 1, Right 2
	scanner = getSquareContents(col + 2, row - 1);
	if (scanner != NULL)
		if (scanner->getColor() == enemyColor && scanner->getType() == KNIGHT)
			attackers.push_back(scanner);

	// All possible threats scanned for, return results.

	board[king->getColumn()][king->getRow()] = king;

	return attackers;
}

//	Returns true if the path from the piece to a destination square is free of obstructions.
bool chessBoardClass::isPathClear(chessPiece& piece, int destC, int destR)
{
	int origC = piece.getColumn(), origR = piece.getRow();

	chessPiece* scanner;

	// Non-diagonal movement cases
	if (origR == destR)	// Horizontal Movement
	{
		if (origC > destC) // Leftward Movement
		{
			for (int tracerC = origC - 1; tracerC != destC; --tracerC)
			{
				scanner = getSquareContents(tracerC, destR);
				if (scanner != NULL)
					return false;
			}
		}
		else if (origC < destC) // Rightward Movement
		{
			for (int tracerC = origC + 1; tracerC != destC; ++tracerC)
			{
				scanner = getSquareContents(tracerC, destR);
				if (scanner != NULL)
					return false;
			}
		}
	}
	else if (origC == destC) // Vertical Movement
	{
		if (origR < destR) // Upward Movement
		{
			for (int tracerR = origR + 1; tracerR != destR; ++tracerR)
			{
				scanner = getSquareContents(destC, tracerR);
				if (scanner != NULL)
					return false;
			}
		}
		else if (origR > destR) // Downward Movement
		{
			for (int tracerR = origR - 1; tracerR != destR; --tracerR)
			{
				scanner = getSquareContents(destC, tracerR);
				if (scanner != NULL)
					return false;
			}
		}
	}

	// Diagonal movement cases
	if (origC > destC) // Leftward Movement
	{
		if (origR > destR) // Left-Down Movement
		{
			for (int tracerC = origC - 1, tracerR = origR - 1; tracerC != destC && tracerR != destR; --tracerC, --tracerR)
			{
				scanner = getSquareContents(tracerC, tracerR);
				if (scanner != NULL)
					return false;
			}
		}
		else if (origR < destR) // Left-Up Movement
		{
			for (int tracerC = origC - 1, tracerR = origR + 1; tracerC != destC && tracerR != destR; --tracerC, ++tracerR)
			{
				scanner = getSquareContents(tracerC, tracerR);
				if (scanner != NULL)
					return false;
			}
		}
	}
	else if (origC < destC) // Rightward Movement
	{
		if (origR > destR) // Right-Down Movement
		{
			for (int tracerC = origC + 1, tracerR = origR - 1; tracerC != destC && tracerR != destR; ++tracerC, --tracerR)
			{
				scanner = getSquareContents(tracerC, tracerR);
				if (scanner != NULL)
					return false;
			}
		}
		else if (origR < destR) // Right-Up Movement
		{
			for (int tracerC = origC + 1, tracerR = origR + 1; tracerC != destC && tracerR != destR; ++tracerC, ++tracerR)
			{
				scanner = getSquareContents(tracerC, tracerR);
				if (scanner != NULL)
					return false;
			}
		}
	}

	// Path from (origC, origR) to (destC, destR) is not obstructed.
	return true;
}

/*
	======================================================
	Movement Related Methods - Movement Handling and Logic
	======================================================
*/

// Function will move piece at (origC, origR) to (destC, destR), if legal.
/*
	Note: The parameter forceMove has a default value of false.  This is used with the chessGameTree
	class to help when building game state nodes when we know the move is already legal.

	For more info, consult the notes below, titled "Multi-Test Movement Logic Notes."
*/
bool chessBoardClass::move(int origC, int origR, int destC, int destR, bool noMove, bool forceMove)
{
	bool legalMove = false;

	chessPiece* piece = getSquareContents(origC, origR);

	if (!forceMove)
	{
		// Can't move a piece that isn't there.
		if (!piece)
			return false;

		// White player can't move black pieces, vice-versa.
		if (piece->getColor() != turn)
			return false;

		// Can't keep playing a game that's over.
		if (checkmate)
			return false;

		legalMove = movementLogic(*piece, destC, destR);

		if (noMove)
			return legalMove;
	}

	if ((legalMove && noMove == false) || forceMove)
	{
		// Does the pointer manipulation needed to perform the actual move.
		performMove(*piece, destC, destR);
		
		// Note:  This clears checkVector, escapeVector, attackVector, and saviorVector
		swapTurn();
		
		// Move has been performed and it is now the other player's turn.

		// Scan to see if the current player's king is now in check, as a result of the move.
		chessPiece* king;
			if (turn == WHITE)
				king = &wKing.front();
			else
				king = &bKing.front();

		// Populate checkVector with any pieces that are putting otherKing in check.
		checkVector = scanForCheck(*king);

		// Populate escape vector with pair<int, int> board coordinates that represent
		// squares that the king can safely move to.
		escapeVector = scanForEscapeSquares(*king);

		// Populate pinVector with any pieces that are currently pinned to their king.
		pinVector = scanForPins(*king);
		
		// The king is in check, populate the appropriate vectors.
		if (!checkVector.empty())
		{
			// Populate attackVector with pair<int,int> board coordinates that represent
			// squares between the checking piece(s) and the king.
			attackVector = setAttackVector(*king);

			// Populate saviorVector with chessPiece* elements that point to pieces that
			// can take the king out of check by being moved to a coordinate in attackVector
			saviorVector = scanForSaviors();
		}

		// If the king is in check,
		// AND there are no escape squares,
		// AND there are no savior pieces,
		// then checkmate has occured.
		if (checkVector.size() > 0 && escapeVector.empty() && saviorVector.empty())
			checkmate = true;

		// Print out checkVector, attackVector, saviorVector, and escapeVector contents
		if (DEBUG)
		{
			using std::cout;
			using std::endl;

			cout << "\n\n\n\n\n\n\n\n\n\n\n" << endl;

			// checkVector - Displays the checking piece or pieces and their coordinates
			cout << endl;
			cout << "====================" << endl
				 <<	"checkVector Contents" << endl << endl;
		
			if (checkVector.empty())
				cout << "No content" << endl;
			else
				for (int i = 0; i < checkVector.size(); ++i)
				{
					chessPiece* piece = checkVector[i];
					cout << piece->getColor() << " " << piece->getType()
						 << " -> (" << piece->getColumn() << ", " << piece->getRow() << ")" << endl;
				}

			// pinVector - Displays the pieces that are currently pinned and their coordinates,
			//			   as well as the piece causing the pin and its coordinates.
			cout << "====================" << endl
				<< "pinVector Contents" << endl << endl;

			if (pinVector.empty())
				cout << "No content" << endl;
			else
				for (int i = 0; i < pinVector.size(); ++i)
				{
					chessPiece* pinnedPiece = pinVector[i].first.first;
					chessPiece* attackPiece = pinVector[i].first.second;
					cout << pinnedPiece->getColor() << " " << pinnedPiece->getType()
						<< " -> (" << pinnedPiece->getColumn() << ", " << pinnedPiece->getRow() << "), ";

					cout << attackPiece->getColor() << " " << attackPiece->getType()
						<< " -> (" << attackPiece->getColumn() << ", " << attackPiece->getRow() << ") :: ";

					cout << "Pin Direction: ";
					switch (pinVector[i].second)
					{
					case RIGHT: cout << "RIGHT" << endl; break;
					case UP_RIGHT: cout << "UP_RIGHT" << endl; break;
					case UP: cout << "UP" << endl; break;
					case UP_LEFT: cout << "UP_LEFT" << endl; break;
					case LEFT: cout << "LEFT" << endl; break;
					case DOWN_LEFT: cout << "DOWN_LEFT" << endl; break;
					case DOWN: cout << "DOWN" << endl; break;
					case DOWN_RIGHT: cout << "DOWN_RIGHT" << endl; break;
					}
				}

			// attackVector - Coordinates of all the squares that are under attack by checkers.
			cout << "=====================" << endl
			 	 << "attackVector Contents" << endl << endl;

			if (attackVector.empty())
				cout << "No content" << endl;
			else
				for (int i = 0; i < attackVector.size(); ++i)
				{
					std::pair<int, int> coord = attackVector[i];

					cout << "(" << coord.first << ", " << coord.second << ")" << endl;
				}

			// saviorVector - The pieces that can save the king from check and their coordiantes.
			cout << "=====================" << endl
				 << "saviorVector Contents" << endl <<  endl;
		
			if (saviorVector.empty())
				cout << "No content" << endl;
			else
				for (int i = 0; i < saviorVector.size(); ++i)
				{
					chessPiece* piece = saviorVector[i];

					cout << piece->getColor() << " " << piece->getType()
						 << " -> (" << piece->getColumn() << ", " << piece->getRow() << ")" << endl;
				}

			// escapeVector - The coordinates of safe squares that the king can be moved to.
			cout << "=====================" << endl
				 << "escapeVector Contents" << endl << endl;
		
			if (escapeVector.empty())
				cout << "No content" << endl;
			else
				for (int i = 0; i < escapeVector.size(); ++i)
				{
					std::pair<int, int> coord = escapeVector[i];

					cout << "(" << coord.first << ", " << coord.second << ")" << endl;
				}

			cout << "========================" << endl;
			cout << "******************************" << endl;
		}
		
		return true;
	}
	else
		return false;
		
}

//	Helper function to move(), does a lot of the lower-level data handling.
void chessBoardClass::performMove(chessPiece& piece, int destC, int destR, bool forceMove)
{
	int origC = piece.getColumn(), origR = piece.getRow();

	chessPiece* occupier = getSquareContents(destC, destR);

	// Special Movement Case:  En Passant Capture
	// All movement tests have passed at this point.
	if (piece.getType() == PAWN && origC != destC)
	{
		// Attack diagonal is empty, so the target must be an en-passant eligible pawn.
		if (occupier == NULL)
		{
			occupier = getSquareContents(destC, origR);
			
			// Set board square to point to NULL, so display function doesn't 
			// try to access an invalid memory address
			board[destC][origR] = NULL;
		}
		// The rest of the function takes care of the remaining work.
	}

	// Special Movement Case:  Castling
	// All movement tests have been performed -> just move the pieces.
	if (piece.getType() == KING && abs(origC - destC) == 2)
	{
		chessPiece* rook;
		// Castle Queen-Side
		if (destC < origC)
		{
			// Grab the rook pointer from the board.
			rook = getSquareContents(0, origR);

			// Clear the necessary board pointers.
			board[origC][origR] = NULL;
			board[0][origR] = NULL;

			// Set the pieces in place, sync board.
			piece.moveTo(destC, destR);
			board[destC][destR] = &piece;
			rook->moveTo(destC + 1, destR);	// Queen-side castle -> rook is right of the king.
			board[destC + 1][destR] = rook;

		}
		else // Castle King-Side
		{
			// Grab the rook pointer from the board.
			rook = getSquareContents(7, origR);

			// Clear the necessary board pointers.
			board[origC][origR] = NULL;
			board[7][origR] = NULL;

			// Set the pieces in place, sync board.
			piece.moveTo(destC, destR);
			board[destC][destR] = &piece;
			rook->moveTo(destC - 1, destR);	// King-side castle -> rook is right of the king.
			board[destC - 1][destR] = rook;
		}
	}

	// Vanilla Movement Case
	// If target square is occupied, delete the occupier.
	if (occupier != NULL)
	{
		occupier->setCaptured(true);
		PIECE_COLOR color = occupier->getColor();
		PIECE_TYPE type = occupier->getType();

		// A vector element's erase flag is set to true.
		/* This will erase the piece from its vector so it doesn't cause problems.
		// This is needed since deleting a pointer that is an element of a vector
		// doesn't erase it from the vector.  Since the pin-check test in the movementLogic
		// method below relies on copying the piece vectors into a new chessBoardClass
		// instance, we can't be copying dangling pointers that don't actually point
		// to anything. */
	}

	// Move the piece to the target testination, sync the board.
	piece.moveTo(destC, destR);
	board[destC][destR] = &piece;

	// Note:	piece->moveTo() handles setting the enPassant flag or 
	//			canCastle flag of pawn and rook/king pieces appropriately.

	// Original spot is empty after the move.
	board[origC][origR] = NULL;

	// Special Movement Case:  Pawn Promotion
	if (piece.getType() == PAWN && (destR == 0 || destR == 7) && !forceMove)
	{	
		// Flag the pawn for erasure.
		piece.setCaptured(true);

		// Only way for a pawn to reach row 0 (1st row) is for the pawn to be black, which means it needs to be promoted.
		// Same thing applies to white pawns that reach row 7 (the last row).
		pawnPromotion(piece);

		PIECE_COLOR color = piece.getColor();
		PIECE_TYPE type = piece.getType();

		// The promoted pawn needs to be deleted.
		// cleanUpPieceVectors(color, type);
	}

	return;
}

///										Multi-Test Movement Logic Notes											\\\

/* 
	Multiple tests will be done for each move to ensure that the move is legal.
	
	The first test will be performed by the piece's own validMovement method, as defined in chessPieceClass.cpp
	-	Failure to pass this first check will cause the moveLogic method to return false.

	The second test will determine if the move would result in "friendly fire."
	-	You cannot capture your own pieces.

	The third test will ensure that pieces cannot "phase through" other pieces while moving to a destination square.
	-	Knights are exempt from the "no phase through" rule.

	The fourth test will handle the case that the king is in check.
	-	If the king is in check and the number of pieces checking the king is > 1, then any move that
		is not performed by the king is automatically ruled out - the king MUST be moved to safety.

	-	If the king is in check and there is only one checking piece, then any move that is performed
		must be by a savior piece that is moving to a coordinate in attackVector (attacking the checker
		or moving between the checker and king) or the king must be moving to a coordinate in escapeVector.
		The attacker's square can be moved to by the king if the attacker is not defended (and in range of the king), 
		which means that there can be a coordinate in common between attackVector and escapeVector.  
		They're not complementary in this case.

	-	If the king is not in check (checkVector.size() == 0), then the fourth test can be skipped.

	The fifth test will effectively determine if a piece is pinned.  
	When a piece cannot be moved, as it would result in its own king being attacked, the piece is said to be "pinned."
	This will be tested by building a board that reflects the state of the game if the move being tested were allowed.  
	The test will then determine if the king is in check on this new board.

	Storing to and testing with the various vectors (checkVector, attackVector, etc.) is preferable, since they 
	will tend to be quite small (if populated at all) and relatively inexpensive to fill.
	However, these test vectors cannot be used to determine if a piece is pinned.

	Due to this, every move that passes the first four tests will eventually need to be tested on the copy board
	to simulate the move before final approval.  This is necessary to implement pinning; moving any piece
	on the board cannot result in your own king being put in check.  The most intuitive way to check for this
	is to build a test board where this move is allowed to occur, and then see if the king on the test board
	is in check.  I'm sure there are other (and likely better) ways to implement pinning, but this seems the most 
	straight forward to me at the moment.

	The test board will be another instance of the chessBoardClass that takes a deep copy of the calling board.
	After the deep copy, the board will then call move(), with the forceMove parameter set to true.  This bypasses
	the calls to the movement checks and swapTurn(), and allows for the same color's king to be scanned after the move
	to see if it is in check.  If the king is in check on the test board, then this means that the move on the original
	board should not occur, since the piece being moved put the player's own king in check, which is illegal.

								/// NOTE ON POTENTIAL TEST 5 (PINNING TEST) IMPROVEMENT \\\
										   /// THIS HAS NOW BEEN IMPLEMENTED \\\

	Making a copy of the current board and testing a move on it for every move is a bit overkill, since most moves that one
	performs over the course of the game aren't going to be attempted on pieces that are pinned.  In the worst case, the most pieces
	that can ever be pinned is 8.  The only pieces that can possibly be pinned are all friendly pieces that are in 8-directional 
	eye sight of the king.  An eight-directional scan for the first friendly piece encountered in each direction can be done to 
	determine potential pin candidates.  If we add these pieces to a vector each turn (much like the existing vectors we're currently 
	using, e.g., checkVector and saviorVector), then only pieces that are trying to be moved that are containted within this vector 
	will trigger the fifth test.  We could take this a step further and determine what direction of attack would cause a pin for a 
	given piece and scan for an appropriate attack for each piece in the vector, which would allow us to skip having to do a "copyboard-
	and-test" procedure entirely.  I haven't done the math to see if it would be more expensive than the current method, but in the
	worst case doing 8 attack scans with about 3-4 iterations(*) each doesn't seem bad compared to creating and initializing a whole 
	copy of the current game state and then performing a move on it that is probably legal anyways for each turn.

		(*) This worst-case assumes that the king is in the middle of the board and surrounded by friendly pieces on all sides.
			Realistically, the king will likely be in or near a corner with 3 - 4 defenders.  Here the number of iterations for
			attack scans is likely to be 5-6 for each, in the worst case.  15 to 24 attack scan iterations, plus the initial scans to
			populate the vector and determine attack direction, seems much better than the copy-board method.

	Notes on Special Movement:
	A special move is either an en passant pawn capture or castling.

	En Passant
	-	In this case, a pawn moves two space forward from its starting position.  If it lands to the left or right
		of an enemy pawn, that enemy pawn can move diagonally toward the pawn so that it ends up behind the 2-move
		pawn.  This counts as a capture, and the 2-move pawn is taken off of the board.  It is possible that 2-move 
		pawn movement results in a discovery check, where a piece that was hiding behind the pawn now has a clear
		shot at the enemy king.  If this is the case, then the 2-move pawn cannot be taken by en passant, since the
		check needs to be addressed by a savior piece, and the diagonal movement would not block the discovery check.

	-	Thus:	En passant capture cannot occur if the king is in check.  It also cannot occur if the pawn is pinned.
				The fifth test will handle the pinned case and if the fourth test is entered, then we can rule out
				en passant moves in it.  However, if a diagonal capture is attempted, it needs to determine if the
				diagonal move is a normal capture by pawn or an en passant capture.  For example, if the diagonal 
				movement is up-left, then it should check to see if there is a piece in the square to the up-left
				OR if there is a pawn eligible for en passant capture to its left.  Both of these cases cannot be
				true during the same game state, as it would imply that the 2-move pawn phased through a friendly piece.

	-	If the king is in check, the fourth test will rule out en passant moves implicitly.  The validity of the
		en passant movement itself will be checked just before the fourth rule.
		

	Castling
	-	The rules for castling are as follows:
		=	The king is eligible for castling (has not been moved).	<-- Checked by king's validMovement method.
		=	The king is not in check.
		=	The rook involved (left rook for left move, right rook for right move)
			is eligible (has not been moved).
		=	There can be no pieces occupying the space between the king and the rook
		=	If castling left (queen-side), then the two squares immediately to the left of the
			king must not be under attack (cannot castle into or through check).
		=	If casting right (king-side), then the two squares immediately to the right of the
			king must not be under attack (cannot castle into or through check).

	-	So long as these conditions are met, then castling is a legal action.

*/

// Function returns true if the desired move by the piece is legal
bool chessBoardClass::movementLogic(chessPiece& piece, int destC, int destR)
{
	// First Test:  Test Basic Movement Rules
	if (!piece.validMovement(destC, destR))
		return false;

	// Second Test: Test for Friendly Fire
	if (getSquareContents(destC, destR) != NULL)
		if (getSquareContents(destC, destR)->getColor() == piece.getColor())
			return false;

	// Third Test:  Test for Phase-through
	if (piece.getType() != KNIGHT && piece.getType() != PAWN && !isPathClear(piece, destC, destR))
		return false;
	else if (piece.getType() == PAWN)
	{
		//	Pawns get a special test for phase-through, since isPathClear does not test for pawn attack conditions.
		int origC = piece.getColumn(), origR = piece.getRow();

		//	If movement is non-diagonal, then pawn's cannot move to an occupied square.
		if (origC == destC)
		{
			if (piece.getColor() == WHITE)
			{
				if (destR == origR + 1 && getSquareContents(destC, destR) != NULL)
					return false;
				else if (destR == origR + 2)
				{
					if (getSquareContents(destC, destR) != NULL || getSquareContents(destC, destR - 1) != NULL)
						return false;
				}
			}
			else if (piece.getColor() == BLACK)
			{
				if (destR == origR - 1 && getSquareContents(destC, destR) != NULL)
					return false;
				else if (destR == origR - 2)
				{
					if (getSquareContents(destC, destR) != NULL || getSquareContents(destC, destR + 1) != NULL)
						return false;
				}
			}
		}
	}

	// Special Case:  En Passant and Pawn Capturing
	if (piece.getType() == PAWN && (!(piece.getColumn() == destC)))
	{
		// First and Second Tests have already ensured that the pawn movement is valid,
		// so we just need to ensure that its target is valid.

		// Will rule out pawn diagonal movement that doesn't result in a capture.
		if (getSquareContents(destC, destR) == NULL)
		{
			chessPiece* occupier = getSquareContents(destC, piece.getRow());

			if (occupier == NULL)
				return false;
			else if (occupier->getType() != PAWN)
				return false;
			else if (occupier->getType() == PAWN)
			{
				if (occupier->getColor() == BLACK)
				{
					if (!static_cast<blackPawn*>(occupier)->getEnPassant())
						return false;
				}
				else if (occupier->getColor() == WHITE)
				{
					if (!static_cast<whitePawn*>(occupier)->getEnPassant())
						return false;
				}
			}
		}
	}

	// Special Case:  Castling
	if (piece.getType() == KING && abs(piece.getColumn() - destC) == 2)
	{
		// First Test has already checked if the king's castle flag is set to true.
		// Need to check if castling rook is eligible.
		int origC = piece.getColumn();
		int origR = piece.getRow();
		chessPiece* rook;

		// Queen-side Castling
		if (destC < origC)
		{
			chessPiece* scanner;
			// Scan for clear squares
			for (int tracerC = origC - 1; tracerC != 0; --tracerC)
			{
				scanner = getSquareContents(tracerC, origR);
				if (scanner != NULL)
					return false;
			}

			// Squares are clear, but are they under attack?
			for (int tracerC = origC - 1; tracerC != 0; --tracerC)
			{
				if (getAttackers(tracerC, origR).size() > 0)
					return false;
			}

			rook = getSquareContents(0, origR);
		}
		else // King-side Castling
		{
			chessPiece* scanner;
			// Scan for clear squares
			for (int tracerC = origC + 1; tracerC != 7; ++tracerC)
			{
				scanner = getSquareContents(tracerC, origR);
				if (scanner != NULL)
					return false;
			}

			// Squares are clear, but are they under attack?
			for (int tracerC = origC + 1; tracerC != 7; ++tracerC)
			{
				if (getAttackers(tracerC, origR).size() > 0)
					return false;
			}

			rook = getSquareContents(7, origR);
		}

		if (rook == NULL)
			return false;

		// Check for rook's castling eligibility
		if (rook != NULL && (rook->getType() == ROOK))
		{
			if (rook->getColor() == BLACK)
			{
				if (static_cast<blackRook*>(rook)->getCastle() == false)
					return false;
			}
			else // (color == "white")
			{
				if (static_cast<whiteRook*>(rook)->getCastle() == false)
					return false;
			}
		}
	}

	// Special Case:  Non-Check King Movement
	if (piece.getType() == KING)
	{
		//	The king cannot be moved to a square that is under attack.


		//	Note:	Despite the fact that getAttackers() fails to scan in the direction opposite of the
		//			king's attempted movement, the square is still guaranteed to be safe.  The only time
		//			this isn't the case is when the king is currently in check, which is handled by the
		//			fourth test, just below.
		if (!getAttackers(destC, destR).empty())
			return false;
	}

	// Fourth Test:  King in Check
	if (checkVector.size() > 0)
	{
		// If number of attackers is greater than one, then the king must be moved.
		if (checkVector.size() > 1)
		{
			// If number of attackers is greater than two, the king must be moved.
			if (piece.getType() != KING)
				return false;
		}

		else // checkVector.size() == 1
		{
			if (piece.getType() != KING)
			{
				bool isSavior = false;

				// See if piece is a potential savior.
				for (int i = 0; i < saviorVector.size(); ++i)
				{
					if (&piece == saviorVector[i])
						isSavior = true;
				}

				if (!isSavior)
					return false;

				bool isSaving = false;
				std::pair<int, int> coord = std::make_pair(destC, destR);

				// See if piece is moving to save the king.
				for (int i = 0; i < attackVector.size(); ++i)
				{
					if (coord == attackVector[i])
						isSaving = true;
				}

				if (!isSaving)
					return false;
			}
		}

		if (piece.getType() == KING)
		{
			std::pair<int, int> coord = std::make_pair(destC, destR);

			bool safeMove = false;

			for (int i = 0; i < escapeVector.size() && !safeMove; ++i)
			{
				if (coord == escapeVector[i])
					safeMove = true;
			}

			if (!safeMove)
				return false;
		}
	}	// End Fourth Test

	/// An improved version of this is described in Multi-Test Movement Logic Notes and is now implemented below.
	// Fifth Test - Check for Pin
	if (!pinVector.empty())
	{
		bool isPinnedPiece = false;
		chessPiece* pinner;
		PIN_DIR pinDirection;

		// If the piece attempting to be moved is pinned, pinPtr will point to its containing element in pinVector.
		for (auto itr = pinVector.begin(); !isPinnedPiece && itr != pinVector.end(); ++itr)
		{
			if ((*itr).first.first == &piece)
			{
				pinner = (*itr).first.second;
				pinDirection = (*itr).second;
				isPinnedPiece = true;
				break;
			}
				
		}

		if (!isPinnedPiece)
			return true;

		// Check to see if piece is in pinVector.

		/*
			Here's a brief explanation for what is about to follow.

			If a piece is pinned, there are a limited number of moves that the piece is legally allowed to make.

			1)  Arguably, the most common case is that the pinned piece attacks the piece that is causing the pin.
				This is a simple case and easy to check for.  If the function has made it this far, then attacking the
				piece causing the pin would be a legal move under normal circumstances, and it still is here.

			2)  The other case is that the pinned piece is moved to some intermediate position between the king and the pinner.
				This is commonly done to minimize losses and/or to improve ones defensive structure while gaining tempo, but whatever
				the reasoning behind it the test for it is a bit trickier.  Essentially I'll need to scan to see if the move being attempted
				falls on a valid square (one that is between the pinner and the king).
		*/

		//	Case 0 - Knights Cannot Move Under Pin
		if (piece.getType() == KNIGHT)
			return false;

		// Case 1 - Pinned Piece Captures Pinner

		if (getSquareContents(destC, destR) == pinner)
			return true;	// If the pinned piece moves to capture its pinner, then the move is legal.


		// Case 2 - Pinned Piece is Moved to Intermediate Position
		/*
			Given that the attempted move has passed all of the tests so far, we know a few things that will be useful to take into consideration:

			1)  The move obeys the most fundamental rules of the piece itself.
			2)  The path to the target square is clear of obstruction.
			3)	The target square is unoccupied.

			Knowing the above and the direction of the pin (pinPtr->second), as well as the origin square, actually gives us all of the information we need.

			===================
			HORIZONTAL PIN CASE
			===================

			If the pin is occuring from the left or right, then origR must be equal to destR for the move to be legal.

			=================
			VERTICAL PIN CASE
			=================

			If the pin is occuring from above or below, then origC must be equal to destC for the move to be legal.

			=================
			DIAGONAL PIN CASE
			=================

			If the pin is occuring from a diagonal direction, then its slope must be taken into account.

			A pin from a positive slope direction (UP_RIGHT, DOWN_LEFT) means only movement where
			[( (destR - origR) / (destC - origC) ) == 1] is legal.

			A pin from a negative slope direction (UP_LEFT, DOWN_RIGHT) means only momvement where
			[( (destR - origR) / (destC - origC) ) == -1] is legal.
		*/

		int origC = piece.getColumn();
		int origR = piece.getRow();

		if ((pinDirection == LEFT || pinDirection == RIGHT) && origR == destR)
			return true;
		else if ((pinDirection == UP || pinDirection == DOWN) && origC == destC)
			return true;
		else if (pinDirection == UP_RIGHT || pinDirection == DOWN_LEFT)
		{
			if ((destC - origC) == 0)
				return false;

			// Diagonal movement of pinned piece must have a positive slope of 1.
			if (((destR - origR) / (destC - origC)) != 1)
				return false;
		}
		else if (pinDirection == UP_LEFT || pinDirection == DOWN_RIGHT)
		{
			if ((destC - origC) == 0)
				return false;

			// Diagonal movement of pinned piece must have a slope of -1.
			if (((destR - origR) / (destC - origC)) != -1)
				return false;
		}
		else
			return false;

	}


	// All tests have passed - the move is legal (whew!)
	return true;


	// Old implementation for Pin Test
	/*
	// Copy the board and force the move.
	chessBoardClass testBoard(*this);
	bool forceMove = true;

	if (testBoard.move(piece->getColumn(), piece->getRow(), destC, destR, forceMove) == false)
		return false;	// --> piece is pinned, return false.

	*/
}




// Below is the old implementation for the old interface.
/*
char* numBorderArr[8] = { "1", "2", "3", "4", "5", "6", "7", "8" };
char* alphaBorderArr[8] = { "a", "b", "c", "d", "e", "f", "g", "h" };

char* WHITE_KING = "k";
char* BLACK_KING = "K";

char* WHITE_QUEEN = "q";
char* BLACK_QUEEN = "Q";

char* WHITE_ROOK = "r";
char* BLACK_ROOK = "R";

char* WHITE_BISHOP = "l";
char* BLACK_BISHOP = "L";

char* WHITE_KNIGHT = "n";
char* BLACK_KNIGHT = "N";

char* WHITE_PAWN = "p";
char* BLACK_PAWN = "P";

// Resets board to default state, called by constructor and keyboardInput()
void chessBoardClass::resetBoard()
{
//	Set up white pieces
	board[0][0].setOccupier(WHITE_ROOK, false);
	board[0][1].setOccupier(WHITE_KNIGHT, false);
	board[0][2].setOccupier(WHITE_BISHOP, false);
	board[0][3].setOccupier(WHITE_QUEEN, false);
	board[0][4].setOccupier(WHITE_KING, false);
	board[0][5].setOccupier(WHITE_BISHOP, false);
	board[0][6].setOccupier(WHITE_KNIGHT, false);
	board[0][7].setOccupier(WHITE_ROOK, false);
	for (int i = 0; i < 8; i++)
		board[1][i].setOccupier(WHITE_PAWN, false);

// Set up black pieces
	board[7][0].setOccupier(BLACK_ROOK, true);
	board[7][1].setOccupier(BLACK_KNIGHT, true);
	board[7][2].setOccupier(BLACK_BISHOP, true);
	board[7][3].setOccupier(BLACK_QUEEN, true);
	board[7][4].setOccupier(BLACK_KING, true);
	board[7][5].setOccupier(BLACK_BISHOP, true);
	board[7][6].setOccupier(BLACK_KNIGHT, true);
	board[7][7].setOccupier(BLACK_ROOK, true);
	for (int i = 0; i < 8; i++)
		board[6][i].setOccupier(BLACK_PAWN, true);

//	Clear midboard section.
	for (int i = 2; i < 6; i++)
		for (int j = 0; j < 8; j++)
			board[i][j].containsPiece = false;

//	Set private data member values
	kingInCheck = isBTurn = !(isWTurn = true);
	
	wKing.isBlack = !(wKing.isWhite = true);
	wKing.pieceType = WHITE_KING;
	wKing.xCoord = 4;
	wKing.yCoord = 0;

	bKing.isWhite = !(bKing.isBlack = true);
	bKing.pieceType = BLACK_KING;
	bKing.xCoord = 4;
	bKing.yCoord = 7;
}

// Called by display(), renders the chess pieces
void chessBoardClass::renderChessBoard()
{
//  Draws the chess board itself
	for (int i = 0; i < 8; i++)
		for (int j = 0; j < 8; j++)
		{
			if (i == j)
			{	// Draws the column/row markers
				chessFontBitmap.Select();
				chessFontBitmap.Print(numBorderArr[i], -5, BOARD_OFFSET + i*board[i][j].size + 3);
				chessFontBitmap.Print(alphaBorderArr[i], BOARD_OFFSET + 4 + i*board[i][j].size, -20);
			}
			glDisable(GL_TEXTURE_2D);
			glBegin(GL_QUADS);
				
			//	Selects color for light square
			if ((i + j) % 2)
				glColor3f(0.6, 0.6, 0.6);
			else // Selects color for dark square
				glColor3f(.25, .25, .25);

			// Draws the square at the correct position on the board.
			glVertex2i(BOARD_OFFSET + (i * board[i][j].size) , BOARD_OFFSET + (j * board[i][j].size));
			glVertex2i(BOARD_OFFSET + (i * board[i][j].size), BOARD_OFFSET + (j + 1) * board[i][j].size);
			glVertex2i(BOARD_OFFSET + ((i + 1) * board[i][j].size), BOARD_OFFSET + (j + 1) * board[i][j].size);
			glVertex2i(BOARD_OFFSET + ((i + 1) * board[i][j].size), BOARD_OFFSET + j * board[i][j].size);

			glEnd();
		}

	// The chess board has been drawn to the frame, now to draw the chess pieces themselves.

//	Draws all of the chess pieces, row by row.
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			if (board[i][j].containsPiece)	// Check flag, render if needed.
			{
				chessFontBitmap.Select();
				// ChessFontBitmap.Print(*Which Piece To Render*, *X Coord*, *Y Coord*)
				chessFontBitmap.Print(board[i][j].occupier.pieceType,
					(BOARD_OFFSET + 6 + (j * board[i][j].size)), BOARD_OFFSET + (i * board[i][j].size));
			}
		}
	}
}

bool chessBoardClass::movePiece(int fromC, int fromR, int toC, int toR)
{
	// Debug Stuff //
	if (DEBUG)
	{
		cout << "============================\n" << endl;

		if (board[fromR][fromC].containsPiece)
		{
			cout << "(fromC fromR toC toR) = (" << fromC << " " << fromR << " " << toC << " " << toR << ")" << endl;
			cout << "The piece at (" << fromC << ", " << fromR << ") is a: " << getPieceType(fromC, fromR) << endl;
		}
		else
		{
			cout << "(fromC fromR toC toR) = (" << fromC << " " << fromR << " " << toC << " " << toR << ")" << endl;
			cout << "There is no piece at (" << fromC << ", " << fromR << ")." << endl;
		}
		cout << "\n============================\n" << endl;

	}

	if ((fromC == toC) && (fromR == toR))
		return false;

	if (board[fromR][fromC].containsPiece)
	{
		char* pieceToMove = getPieceType(fromC, fromR);

		if (isMoveValid(pieceToMove, fromC, fromR, toC, toR))
		{
			board[toR][toC].occupier = board[fromR][fromC].occupier;
			board[toR][toC].containsPiece = true;
			board[fromR][fromC].containsPiece = false;

			swapTurns();

			// Debug Stuff //
			if (DEBUG) {
				cout << "=====================\n" <<
						"VALID MOVE!!!\n" <<
						"=====================" << endl;
			}
		}
		else
			return false;
	}
	else
		return false;
}

bool chessBoardClass::isMoveValid(char* piece, int fromC, int fromR, int toC, int toR)
{
	if (DEBUG)
		cout << "ENTERED isMoveValid\npiece == " << piece << "\nisWTurn == " << isWTurn << "; isBTurn == " << isBTurn << endl << endl;
//	Check that turn order is respected.
	if (isWTurn)
	{
		if ((piece == BLACK_KING) || (piece == BLACK_QUEEN) || (piece == BLACK_ROOK) ||
			(piece == BLACK_BISHOP) || (piece == BLACK_KNIGHT) || (piece == BLACK_PAWN))
			return false;
	}
	else if (isBTurn)
	{
		if ((piece == WHITE_KING) || (piece == WHITE_QUEEN) || (piece == WHITE_ROOK) ||
			(piece == WHITE_BISHOP) || (piece == WHITE_KNIGHT) || (piece == WHITE_PAWN))
			return false;
	}

	if (DEBUG)
		cout << piece << " is entering isMoveValid control block." << endl;
//  Direct control to dedicated movement function.
	if ((piece == BLACK_PAWN) || (piece == WHITE_PAWN))
		return pawnMoveValid(fromC, fromR, toC, toR);
	else if ((piece == BLACK_ROOK) || (piece == WHITE_ROOK))
		return rookMoveValid(fromC, fromR, toC, toR);
	else if ((piece == BLACK_KNIGHT) || (piece == WHITE_KNIGHT))
		return knightMoveValid(fromC, fromR, toC, toR);
	else if ((piece == BLACK_BISHOP) || (piece == WHITE_BISHOP))
		return bishopMoveValid(fromC, fromR, toC, toR);
	else if ((piece == BLACK_QUEEN) || (piece == WHITE_QUEEN))
		return queenMoveValid(fromC, fromR, toC, toR);
	else if ((piece == BLACK_KING) || (piece == WHITE_KING))
		return kingMoveValid(fromC, fromR, toC, toR);

	return false;
}

bool chessBoardClass::pawnMoveValid(int fromC, int fromR, int toC, int toR)
{	// Todo:  Implement en passant pawn rule.
	if (isWTurn)
	{	// Move 1/Move 2 square cases
		if (toC == fromC)
		{	// Move 2 squares up from starting position
			if ((toR == 3) && (board[fromR + 1][toC].containsPiece == false) && (board[toR][toC].containsPiece == false) && (fromR == 1))
				return true;
			else if (((toR - fromR) == 1) && (board[toR][toC].containsPiece == false)) // Move 1 square up
				return true;
			else
				return false;
		} // Diagonal attack case:  Make sure move is either up-left or up-right
		else if ((toC == (fromC - 1) || (toC == (fromC + 1))) && (toR == (fromR + 1)))
		{
			if (board[toR][toC].containsPiece && board[toR][toC].occupier.isBlack)
				return true;
			else
				return false;
		}
		else
			return false;
	}
	else if (isBTurn) // same logic as above, but applied to black pawns.
	{
		if (toC == fromC)
		{
			if ((toR == 4) && (board[fromR - 1][toC].containsPiece == false) && (board[toR][toC].containsPiece == false) && (fromR == 6))
				return true;
			else if (((fromR - toR) == 1) && (board[toR][toC].containsPiece == false))
				return true;
			else
				return false;
		}
		else if ((toC == (fromC - 1) || (toC == (fromC + 1))) && (toR == (fromR - 1)))
		{
			if (board[toR][toC].containsPiece && board[toR][toC].occupier.isWhite)
				return true;
			else
				return false;
		}
		else
			return false;
	}
}

bool chessBoardClass::rookMoveValid(int fromC, int fromR, int toC, int toR)
{
	// Vertical movement case
	if (fromC == toC)	
	{	
		// Downward movement case
		if (fromR > toR) 
		{
			int tracer = fromR - 1;
			while (tracer != toR) // check for mid-movement collision
			{
				if (board[tracer][toC].containsPiece) // There's a piece in the way - invalid move.
					return false;
				else
					tracer--;
			}
			if (!board[toR][toC].containsPiece)
				return true;
			else
			{
				if (isWTurn && board[toR][toC].occupier.isBlack)
					return true;
				else if (isBTurn && board[toR][toC].occupier.isWhite)
					return true;
				else
					return false;
			}
		}
		// Upward movement case
		else if (fromR < toR)
		{
			int tracer = fromR + 1;
			while (tracer != toR) // check for mid-movement collision
			{
				if (board[tracer][toC].containsPiece) // There's a piece in the way - invalid move.
					return false;
				else
					tracer++;
			}
			if (!board[toR][toC].containsPiece)
				return true;
			else
			{
				if (isWTurn && board[toR][toC].occupier.isBlack)
					return true;
				else if (isBTurn && board[toR][toC].occupier.isWhite)
					return true;
				else
					return false;
			}
		}
	}
	// Horizontal movement case
	else if (fromR == toR)
	{
		// Leftward movement case
		if (fromC > toC)
		{
			int tracer = fromC - 1;
			while (tracer != toC)
			{
				if (board[toR][tracer].containsPiece)
					return false;
				else tracer--;
			}
			if (!board[toR][toC].containsPiece)
				return true;
			else
			{
				if (isWTurn && board[toR][toC].occupier.isBlack)
					return true;
				else if (isBTurn && board[toR][toC].occupier.isWhite)
					return true;
				else
					return false;
			}
		}
		// Rightward movement case
		if (fromC < toC)
		{
			int tracer = fromC + 1;
			while (tracer != toC)
			{
				if (board[toR][tracer].containsPiece)
					return false;
				else tracer++;
			}
			if (!board[toR][toC].containsPiece)
				return true;
			else
			{
				if (isWTurn && board[toR][toC].occupier.isBlack)
					return true;
				else if (isBTurn && board[toR][toC].occupier.isWhite)
					return true;
				else
					return false;
			}
		}
	}
	else
		return false;
}

bool chessBoardClass::knightMoveValid(int fromC, int fromR, int toC, int toR)
{
	// If target square is occupied, check for friendly fire.
	if (board[toR][toC].containsPiece) 
	{
		if (isWTurn && board[toR][toC].occupier.isWhite)
			return false;
		else if (isBTurn && board[toR][toC].occupier.isBlack)
			return false;
	}

	if (DEBUG)
		cout << "Knight passed FF check." << endl;

	// Knight movement is separated into 4 potential quadrants.

	// Upward and left/right movement cases
	if (fromR < toR)
	{
		if (DEBUG)
			cout << "Knight entering Upward case." << endl;
		// Leftward movement case
		if (fromC > toC)
		{
			if (DEBUG)
				cout << "Knight entering Upward Left case." << endl;
			// Up 2, Left 1 case
			if (((fromR + 2) == toR) && ((fromC - 1) == toC))
				return true;
			// Up 1, Left 2 case
			else if (((fromR + 1) == toR) && ((fromC - 2) == toC))
				return true;
			else
				return false;
		}
		// Rightward movement case
		else if (fromC < toC)
		{
			if (DEBUG)
				cout << "Knight entering Upward Right case." << endl;
			// Up 2, Right 1 case
			if (((fromR + 2) == toR) && ((fromC + 1) == toC))
				return true;
			// Up 1, Right 2 case
			else if (((fromR + 1) == toR) && ((fromC + 2) == toC))
				return true;
			else
				return false;
		}
		else // Knight cannot move strictly vertical (or horizontal).
			return false;
	}
	// Downward and left/right movement cases
	else if (fromR > toR)
	{
		if (DEBUG)
			cout << "Knight enterting Downward case." << endl;
		// Leftward movement case
		if (fromC > toC)
		{
			if (DEBUG)
				cout << "Knight entering Downward Left case." << endl;
			// Down 2, Left 1 case
			if (((fromR - 2) == toR) && ((fromC - 1) == toC))
				return true;
			// Down 1, Left 2 case
			else if (((fromR - 1) == toR) && ((fromC - 2) == toC))
				return true;
			else
				return false;
		}
		// Rightward movement case
		else if (fromC < toC)
		{
			if (DEBUG)
				cout << "Knight entering Downward Right case." << endl;
			// Down 2, Right 1 case
			if (((fromR - 2) == toR) && ((fromC + 1) == toC))
				return true;
			// Down 1, Right 2 case
			else if (((fromR - 1) == toR) && ((fromC + 2) == toC))
				return true;
			else
				return false;
		}
		else // Knight cannot move strictly vertical (or horizontal).
			return false;

	}
	// Knight cannot move strictly horizontal (or vertical).
	else  
		return false;
	
}

bool chessBoardClass::bishopMoveValid(int fromC, int fromR, int toC, int toR)
{
	// Check for friendly at target square
	if (board[toR][toC].containsPiece)
	{
		if (isWTurn && board[toR][toC].occupier.isWhite)
			return false;
		else if (isBTurn && board[toR][toC].occupier.isBlack)
			return false;
	}

	// Bishops can only move in a 45+(90*c) angle (slope = +/- 1)
	int run = (toC - fromC);
	if (run < 0)
		run = -run;
	int rise = (toR - fromR);
	if (rise < 0)
		rise = -rise;

	if (run != rise)
		return false;

	// Bishop movement follows same 4-quadrant rule as knight, but with freer range of movement.

	// Up and left/right movement cases
	if (fromR < toR)
	{	
		// Up and left movement case
		if (fromC > toC)
		{
			int tracerC = fromC - 1;
			int tracerR = fromR + 1;

			// Trace foward, looking for mid-movement collision
			while ((tracerC != toC) && (tracerR != toR))
			{
				if (board[tracerR][tracerC].containsPiece)
					return false;
		
				tracerC--;
				tracerR++;
			}
			// No friendly at target square, and no mid-movement collision.  Valid move.
			return true;
		}
		// Up and right movement case
		else if (fromC < toC)
		{
			int tracerC = fromC + 1;
			int tracerR = fromR + 1;

			// Trace foward, looking for mid-movement collision
			while ((tracerC != toC) && (tracerR != toR))
			{
				if (board[tracerR][tracerC].containsPiece)
					return false;

				tracerC++;
				tracerR++;
			}
			// No friendly at target square, and no mid-movement collision.  Valid move.
			return true;
		}
		// Bishop cannot move strictly vertical (or horizontal).
		else
			return false;
		
	}
	// Down and left/right movement cases
	else if (fromR > toR)
	{
		// Down and left movement case
		if (fromC > toC)
		{
			int tracerC = fromC - 1;
			int tracerR = fromR - 1;

			// Trace foward, looking for mid-movement collision
			while ((tracerC != toC) && (tracerR != toR))
			{
				if (board[tracerR][tracerC].containsPiece)
					return false;

				tracerC--;
				tracerR--;
			}
			// No friendly at target square, and no mid-movement collision.  Valid move.
			return true;
		}
		// Down and right movement case
		else if (fromC < toC)
		{
			int tracerC = fromC + 1;
			int tracerR = fromR - 1;

			// Trace foward, looking for mid-movement collision
			while ((tracerC != toC) && (tracerR != toR))
			{
				if (board[tracerR][tracerC].containsPiece)
					return false;

				tracerC++;
				tracerR--;
			}
			// No friendly at target square, and no mid-movement collision.  Valid move.
			return true;
		}
		// Bishop cannot move strictly vertical (or horizontal).
		else
			return false;
	}
	// Bishop cannot move strictly horizontal (or vertical).
	else 
		return false;
}

// bool chessBoardClass::queenMoveValid(int fromC, int fromR, int toC, int toR);
// If queen passes either the rookMoveValid test OR the bishopMoveValid test, then the move is valid. 

bool chessBoardClass::kingMoveValid(int fromC, int fromR, int toC, int toR)
{

	if (DEBUG)
		cout << "kingMoveValid has been called." << endl;

	// Check for friendly fire, as always.
	if (board[toR][toC].containsPiece)
	{
		if (isWTurn && board[toR][toC].occupier.isWhite)
			return false;
		else if (isBTurn && board[toR][toC].occupier.isBlack)
			return false;
	}

	if (DEBUG)
		cout << "FF check successful." << endl;

	// Check to see if move is within range

	// King horizontal/vertical movement cannot exceed 1 square.
	if ((toR == fromR) || (toC == fromC))
	{
		if (abs(toR - fromR) + abs(toC - fromC) != 1)
			return false;
	}
	// King diagonal movement cannot exceed a city-block distance of 2.
	else if ( (abs(toR - fromR) + abs(toC - fromC)) > 2)
		return false;

	if (DEBUG)
		cout << "King movement is within range." << endl;

	// King movement is unique:  Cannot move into check 
	// Check to see if target square is under attack 
	// Multiple potential cases:  Horizontal attack, vertical attack, diagonal attack, knight attack 
	int tracerC;
	int tracerR;

	// Horizontal Case #1 - Check Left
	// Threats:  Enemy Queen or Rook
	// Scan from just left of target square to the left-most square, return false if enemy Queen or Rook is encountered.
	tracerC = toC - 1;
	while (tracerC >= 0)
	{
		if (isWTurn)
		{
			if (board[toR][tracerC].containsPiece)
				if ((getPieceType(tracerC, toR) == BLACK_QUEEN) || (getPieceType(tracerC, toR) == BLACK_ROOK))
					return false;
				else
					break;
		}
		else if (isBTurn)
		{
			if (board[toR][tracerC].containsPiece)
				if ((getPieceType(tracerC, toR) == WHITE_QUEEN) || (getPieceType(tracerC, toR) == WHITE_ROOK))
					return false;
				else
					break;
		}
		tracerC--;
	}
	
	if(DEBUG)
		cout << "Horiz #1 passed." << endl;

	// Horizontal Case #2 - Check Right
	// Threats:  Enemy Queen or Rook
	// Same idea as above, but with a right scan.
	tracerC = toC + 1;
	while (tracerC <= 7)
	{
		if (isWTurn)
		{
			if (board[toR][tracerC].containsPiece)
				if ((getPieceType(tracerC, toR) == BLACK_QUEEN) || (getPieceType(tracerC, toR) == BLACK_ROOK))
					return false;
				else
					break;
		}
		else if (isBTurn)
		{
			if (board[toR][tracerC].containsPiece)
				if ((getPieceType(tracerC, toR) == WHITE_QUEEN) || (getPieceType(tracerC, toR) == WHITE_ROOK))
					return false;
				else
					break;
		}
		tracerC++;
	}

	if(DEBUG)
		cout << "Horiz #2 passed." << endl;

	// Vertical Case #1 - Check Up
	// Threats:  Enemy Queen or Rook
	// Vertical upward scan for threat, starting at just above target square.
	tracerR = toR + 1;
	while (tracerR <= 7)
	{
		if (isWTurn)
		{
			if (board[tracerR][toC].containsPiece)
				if ((getPieceType(toC, tracerR) == BLACK_QUEEN) || (getPieceType(toC, tracerR) == BLACK_ROOK))
					return false;
				else
					break;
		}
		else if (isBTurn)
		{
			if (board[tracerR][toC].containsPiece)
				if ((getPieceType(toC, tracerR) == WHITE_QUEEN) || (getPieceType(toC, tracerR) == WHITE_ROOK))
					return false;
				else
					break;
		}
		tracerR++;
	}

	if (DEBUG)
		cout << "Vert #1 passed." << endl;

	// Vertical Case #2 - Check Down
	// Threats:  Enemy Queen or Rook
	// Vertical downward scan for threat, starting just below target square.
	tracerR = toR - 1;
	while (tracerR >= 0)
	{
		if (isWTurn)
		{
			if (board[tracerR][toC].containsPiece)
				if ((getPieceType(toC, tracerR) == BLACK_QUEEN) || (getPieceType(toC, tracerR) == BLACK_ROOK))
					return false;
				else
					break;
		}
		else if (isBTurn)
		{
			if (board[tracerR][toC].containsPiece)
				if ((getPieceType(toC, tracerR) == WHITE_QUEEN) || (getPieceType(toC, tracerR) == WHITE_ROOK))
					return false;
				else
					break;
		}
		tracerR--;
	}

	if (DEBUG)
		cout << "Vert #2 passed." << endl;

	// Diagonal Case #1 - Check Up and Left
	// Threats:  Enemy Pawn, Queen, or Bishop
	// Diagonal, up-left scan for threat, starting at just up and to the left of target square.
	tracerC = toC - 1;
	tracerR = toR + 1;
	while ((tracerC >= 0) && (tracerR <= 7))
	{
		if (isWTurn)
		{
			if (tracerC == (toC - 1))
			{
				if (board[tracerR][tracerC].containsPiece)
					if ((getPieceType(tracerC, tracerR) == BLACK_QUEEN)
						|| (getPieceType(tracerC, tracerR) == BLACK_BISHOP)
						|| (getPieceType(tracerC, tracerR) == BLACK_PAWN))
						return false;
					else
						break;
			}
			else
			{
				if (board[tracerR][tracerC].containsPiece)
					if ((getPieceType(tracerC, tracerR) == BLACK_QUEEN)
						|| (getPieceType(tracerC, tracerR) == BLACK_BISHOP))
						return false;
					else
						break;
			}
		}
		else if (isBTurn)
		{
			if (tracerC == (toC - 1))
			{
				if (board[tracerR][tracerC].containsPiece)
					if ((getPieceType(tracerC, tracerR) == WHITE_QUEEN)
						|| (getPieceType(tracerC, tracerR) == WHITE_BISHOP)
						|| (getPieceType(tracerC, tracerR) == WHITE_PAWN))
						return false;
					else
						break;
			}
			else
			{
				if (board[tracerR][tracerC].containsPiece)
					if ((getPieceType(tracerC, tracerR) == WHITE_QUEEN)
						|| (getPieceType(tracerC, tracerR) == WHITE_BISHOP))
						return false;
					else
						break;
			}
		}
		tracerC--;
		tracerR++;
	}

	if (DEBUG)
		cout << "Diag #1 passed." << endl;

	// Diagonal Case # 2 - Check Up and Right
	// Threats:  Enemy Pawn, Queen, or Bishop
	// Same idea as above, but up-right case.
	tracerC = toC + 1;
	tracerR = toR + 1;
	while ((tracerC <= 7) && (tracerR <= 7))
	{
		if (isWTurn)
		{
			if (tracerC == (toC + 1))
			{
				if (board[tracerR][tracerC].containsPiece)
					if ((getPieceType(tracerC, tracerR) == BLACK_QUEEN)
						|| (getPieceType(tracerC, tracerR) == BLACK_BISHOP)
						|| (getPieceType(tracerC, tracerR) == BLACK_PAWN))
						return false;
					else
						break;
			}
			else
			{
				if (board[tracerR][tracerC].containsPiece)
					if ((getPieceType(tracerC, tracerR) == BLACK_QUEEN)
						|| (getPieceType(tracerC, tracerR) == BLACK_BISHOP))
						return false;
					else
						break;
			}
		}
		else if (isBTurn)
		{
			if (tracerC == (toC + 1))
			{
				if (board[tracerR][tracerC].containsPiece)
					if ((getPieceType(tracerC, tracerR) == WHITE_QUEEN)
						|| (getPieceType(tracerC, tracerR) == WHITE_BISHOP)
						|| (getPieceType(tracerC, tracerR) == WHITE_PAWN))
						return false;
					else
						break;
			}
			else
			{
				if (board[tracerR][tracerC].containsPiece)
					if ((getPieceType(tracerC, tracerR) == WHITE_QUEEN)
						|| (getPieceType(tracerC, tracerR) == WHITE_BISHOP))
						return false;
					else
						break;
			}
		}
		tracerC++;
		tracerR++;
	}

	if (DEBUG)
		cout << "Diag #2 passed." << endl;

	// Diagonal Case #3 - Check Down and Left
	// Threats:  Enemy Pawn, Queen, or Bishop
	// Same as above, but down left.
	tracerC = toC - 1;
	tracerR = toR - 1;
	while ((tracerC >= 0) && (tracerR >= 0))
	{
		if (isWTurn)
		{
			if (tracerC == (toC - 1))
			{
				if (board[tracerR][tracerC].containsPiece)
					if ((getPieceType(tracerC, tracerR) == BLACK_QUEEN)
						|| (getPieceType(tracerC, tracerR) == BLACK_BISHOP)
						|| (getPieceType(tracerC, tracerR) == BLACK_PAWN))
						return false;
					else
						break;
			}
			else
			{
				if (board[tracerR][tracerC].containsPiece)
					if ((getPieceType(tracerC, tracerR) == BLACK_QUEEN)
						|| (getPieceType(tracerC, tracerR) == BLACK_BISHOP))
						return false;
					else
						break;
			}
		}
		else if (isBTurn)
		{
			if (tracerC == (toC - 1))
			{
				if (board[tracerR][tracerC].containsPiece)
					if ((getPieceType(tracerC, tracerR) == WHITE_QUEEN)
						|| (getPieceType(tracerC, tracerR) == WHITE_BISHOP)
						|| (getPieceType(tracerC, tracerR) == WHITE_PAWN))
						return false;
					else
						break;
			}
			else
			{
				if (board[tracerR][tracerC].containsPiece)
					if ((getPieceType(tracerC, tracerR) == WHITE_QUEEN)
						|| (getPieceType(tracerC, tracerR) == WHITE_BISHOP))
						return false;
					else
						break;
			}
		}
		tracerC--;
		tracerR--;
	}

	if (DEBUG)
		cout << "Diag #3 passed." << endl;

	// Diagonal Case #4 - Check Down and Right
	// Threats:  Enemy Pawn, Queen, or Bishop
	// Same as above, but down-right.
	tracerC = toC + 1;
	tracerR = toR - 1;
	while ((tracerC <= 7) && (tracerR >= 0))
	{
		if (isWTurn)
		{
			if (tracerC == (toC + 1))
			{
				if (board[tracerR][tracerC].containsPiece)
					if ((getPieceType(tracerC, tracerR) == BLACK_QUEEN)
						|| (getPieceType(tracerC, tracerR) == BLACK_BISHOP)
						|| (getPieceType(tracerC, tracerR) == BLACK_PAWN))
						return false;
					else
						break;
			}
			else
			{
				if (board[tracerR][tracerC].containsPiece)
					if ((getPieceType(tracerC, tracerR) == BLACK_QUEEN)
						|| (getPieceType(tracerC, tracerR) == BLACK_BISHOP))
						return false;
					else
						break;
			}
		}
		else if (isBTurn)
		{
			if (tracerC == (toC + 1))
			{
				if (board[tracerR][tracerC].containsPiece)
					if ((getPieceType(tracerC, tracerR) == WHITE_QUEEN)
						|| (getPieceType(tracerC, tracerR) == WHITE_BISHOP)
						|| (getPieceType(tracerC, tracerR) == WHITE_PAWN))
						return false;
					else
						break;
			}
			else
			{
				if (board[tracerR][tracerC].containsPiece)
					if ((getPieceType(tracerC, tracerR) == WHITE_QUEEN)
						|| (getPieceType(tracerC, tracerR) == WHITE_BISHOP))
						return false;
					else
						break;
			}
		}
		tracerC++;
		tracerR--;
	}

	if (DEBUG)
		cout << "Diag #4 passed." << endl;

	// Knight Cases - Check All Potential Positions
	// Scan up from target square, minding the bounds.
	// Check for knight attack on target square from all possible threat positions.
	tracerR = toR - 2;
	while (tracerR <= (toR + 2))
	{
		// Current scan level is below the board space.
		if (tracerR < 0)
		{
			tracerR++;
			continue;
		}
		// Current scan level is above the board space.
		if (tracerR > 7)
			break;

		// Low Knight Range
		if (tracerR == (toR - 2))
		{
			if (isWTurn)
			{
				if ((toC - 1) > 0)
				{
					if (board[tracerR][toC - 1].containsPiece && (getPieceType((toC - 1), tracerR) == BLACK_KNIGHT))
						return false;
				}
				else if ((toC + 1) < 7)
				{
					if (board[tracerR][toC + 1].containsPiece && (getPieceType((toC + 1), tracerR) == BLACK_KNIGHT))
						return false;
				}

			}
			else if (isBTurn)
			{
				if ((toC - 1) > 0)
				{
					if (board[tracerR][toC - 1].containsPiece && (getPieceType((toC - 1), tracerR) == WHITE_KNIGHT))
						return false;
				}
				else if ((toC + 1) < 7)
				{
					if (board[tracerR][toC + 1].containsPiece && (getPieceType((toC + 1), tracerR) == WHITE_KNIGHT))
						return false;
				}

			}
		}
		// Mid-Low Knight Range
		else if (tracerR == (toR - 1))
		{
			if (isWTurn)
			{
				if ((toC - 2) > 0)
				{
					if (board[tracerR][toC - 2].containsPiece && (getPieceType((toC - 2), tracerR) == BLACK_KNIGHT))
						return false;
				}
				else if ((toC + 2) < 7)
				{
					if (board[tracerR][toC + 2].containsPiece && (getPieceType((toC + 2), tracerR) == BLACK_KNIGHT))
						return false;
				}

			}
			else if (isBTurn)
			{
				if ((toC - 2) > 0)
				{
					if (board[tracerR][toC - 2].containsPiece && (getPieceType((toC - 2), tracerR) == WHITE_KNIGHT))
						return false;
				}
				else if ((toC + 2) < 7)
				{
					if (board[tracerR][toC + 2].containsPiece && (getPieceType((toC + 2), tracerR) == WHITE_KNIGHT))
						return false;
				}
			}
		}
		// Mid-High Knight Range
		else if (tracerR == (toR + 1))
		{
			if (isWTurn)
			{
				if ((toC - 2) > 0)
				{
					if (board[tracerR][toC - 2].containsPiece && (getPieceType((toC - 2), tracerR) == BLACK_KNIGHT))
						return false;
				}
				else if ((toC + 2) < 7)
				{
					if (board[tracerR][toC + 2].containsPiece && (getPieceType((toC + 2), tracerR) == BLACK_KNIGHT))
						return false;
				}

			}
			else if (isBTurn)
			{
				if ((toC - 2) > 0)
				{
					if (board[tracerR][toC - 2].containsPiece && (getPieceType((toC - 2), tracerR) == WHITE_KNIGHT))
						return false;
				}
				else if ((toC + 2) < 7)
				{
					if (board[tracerR][toC + 2].containsPiece && (getPieceType((toC + 2), tracerR) == WHITE_KNIGHT))
						return false;
				}
			}
		}
		// High Knight Range
		else if (tracerR == (toR + 2))
		{
			if (isWTurn)
			{
				if ((toC - 1) > 0)
				{
					if (board[tracerR][toC - 1].containsPiece && (getPieceType((toC - 1), tracerR) == BLACK_KNIGHT))
						return false;
				}
				else if ((toC + 1) < 7)
				{
					if (board[tracerR][toC + 1].containsPiece && (getPieceType((toC + 1), tracerR) == BLACK_KNIGHT))
						return false;
				}

			}
			else if (isBTurn)
			{
				if ((toC - 1) > 0)
				{
					if (board[tracerR][toC - 1].containsPiece && (getPieceType((toC - 1), tracerR) == WHITE_KNIGHT))
						return false;
				}
				else if ((toC + 1) < 7)
				{
					if (board[tracerR][toC + 1].containsPiece && (getPieceType((toC + 1), tracerR) == WHITE_KNIGHT))
						return false;
				}
			}
		}

		tracerR++;
	}

	if (DEBUG)
		cout << "Knight cases passed." << endl;


	// If all checks pass with no false return, the king movement is valid.
	return true;
}

*/
