#pragma once

#ifndef CHESS_BOARD_CLASS_
#define CHESS_BOARD_CLASS_

#include "GlobalVars.h"
#include "chessPieceClasses.h"
#include <vector>
#include <stack>
#include <list>



//	enum type that will be used to log pin directions in the pinVector below.
//	Needed for some AI related methods, so it is kept in global space.
enum PIN_DIR { RIGHT, UP_RIGHT, UP, UP_LEFT, LEFT, DOWN_LEFT, DOWN, DOWN_RIGHT };

// chessBoardClass notes
/*
	The chessBoard will be implemented as a 2d array (8 x 8) of pointer-to-chessPiece.
	Since all of the specific chessPiece classes (like blackPawn, or whiteBishop) are all
	derived from the chessPiece class, it will be a valid assignment to point the array 
	elements to the chess piece that occupies the square, whatever piece that may be. 
	A square is empty if the pointer is set to NULL.

	These will be instance variables of chessBoardClass:

	vector<blackPawn*> bPawns;
	vector<blackKnight*> bKnights;
	vector<blackBishop*> bBishops;
	vector<blackRook*> bRooks;
	vector<blackQueen*> bQueens;
	vector<blackKing*> bKing;
	(etc., for white pieces)

	Pieces will be handled with pointers stored in vectors for easy deletion upon capture.
	Vectors also allow for major pieces to be easily added upon pawn promotions.
	The pointer to king is kept in a vector for the sake of consistency.

	chessBoardClass itself will do all of the necessary checking to ensure that valid moves
	are performed.  The validMovement method of each chess piece object will be called as a
	"first wave" check to rule out the most obvious erroneous movements, but more subtle
	movement checks must be performed by chessBoardClass, since these checks require knowledge
	of the current (or future) state of the board.*/
/*

	Here is a list of the more subtle movement checks chessBoardClass will perform:

	-	Friendly fire:	Check to make sure target attack square is not occupied by friendly piece.

	-	Blocked path:	Scan path to target square to see if there is an obstructing piece.			<-- Does not apply to knights.

	-	Pinned:			Check to see if moving the selected piece would result in putting its team's
						king in check.  There are a few ways to do this:  
						1)	Allow the move and then detect if the king is in check or not, reverting to 
							previous game state if true.
						2)	Scan from the pieces original position to see if there is an enemy "within eye
							sight" that could present a threat to the king if the piece were moved.

	-	En Passant:		If a pawn moves forward 2, then it is eligible to be taken via the en passant rule, but
						only on the next turn.  It loses its eligibility after the opposing teams turn, if not taken.
						After each team's turn, the opposing team's pawns must have their enPassant flags set to false.
						If a pawn is to take another pawn that is eligible for en passant, then the en passant pawn 
						must be to the left or right of the attacking pawn, and the attacking pawn must move diagonally 
						in the direction of the en passant pawn to take it.

	-	Castling:		The requirements for castling to occur are as follows:
						=	The king is eligible for castling (has not been moved)		<-- Checked by king's validMovement method.
						=	The king is not in check
						=	The rook involved (left rook for left move, right rook for right move)
							is eligible (has not been moved)							<-- Checked by rook's validMovement method.
						=	There can be no pieces occupying the space between the king and the rook
						=	If castling left (queen-side), then the square just to the left of the
							king must not be under attack.
						=	If casting right (king-side), then the square just to the right of the
							king must not be under attack.

	-	Promotion:		If a pawn reaches the opposite end of the board, then it can be promoted.
						=	Can be promoted to knight, bishop, rook, or queen.

	-	King Movement:	The king cannot be moved into a square that is under attack,
						effectively putting itself in check.
*/

class chessBoardClass
{
protected:
// Black vectors
	std::vector<blackPawn> bPawns;
	std::vector<blackKnight> bKnights;
	std::vector<blackBishop> bBishops;
	std::vector<blackRook> bRooks;
	std::vector<blackQueen> bQueens;
	std::vector<blackKing> bKing;

// White vectors
	std::vector<whitePawn> wPawns;
	std::vector<whiteKnight> wKnights;
	std::vector<whiteBishop> wBishops;
	std::vector<whiteRook> wRooks;
	std::vector<whiteQueen> wQueens;
	std::vector<whiteKing> wKing;

///	No longer needed.
// Helper function that will erase piece vector elements that have their erase flag set to true.
//	void cleanUpPieceVectors(PIECE_COLOR color, PIECE_TYPE type);

	bool ownedByAI;

// Check vector.  Will contain pointers to pieces that are currently checking the king.
// Will be cleared after each turn.
	std::vector<chessPiece*> checkVector;

// Escape square vector.  Will hold std::pairs of board coordinates where king can move safely.
// Will be cleared after each turn.
	std::vector<std::pair<int, int>> escapeVector;

// Attack vector.  Will hold std::pairs of board coordinates that are in between king and attacker
	std::vector<std::pair<int, int>> attackVector;

// Savior vector.  Will hold pieces that can be moved between a checking piece and the king, as well
// as pieces that can be used to attack a checking piece.
	std::vector<chessPiece*> saviorVector;

// Defender vector.  Will hold pieces that are within direct, eight-directional "eye-sight" of the king.
	std::vector<std::pair<chessPiece*, PIN_DIR>> defenderVector;

// Pin vector.  Will hold std::pairs of pieces that are pinned to their king, as well as an enum value
// that will represent what direction the attack is coming from.
//	Format:  ( (Pinned Piece, Pinning Piece), PIN_DIRECTION )
	std::vector<std::pair<std::pair<chessPiece*, chessPiece*>, PIN_DIR>> pinVector;

// Defines board size.
	static const int numColumns = 8;
	static const int numRows = 8;

// board is an 8x8 array of pointer-to-chessPiece elements
	chessPiece* board[numColumns][numRows];

// WHITE if it's white's turn, BLACK if it's black's turn.
	PIECE_COLOR turn;

//	If a king cannot be taken out of check, then this is true.
	bool checkmate;
	
// Piece-type dependent move logic to determine move legality.
	bool movementLogic(chessPiece& piece, int destC, int destR);

// Scans for enemy attacks on the king's current square, returning a vector that contains
// the pieces that are attacking the king.
// After black's move a scan for white's king being check should occur, and vice-versa.
	std::vector<chessPiece*> scanForCheck(chessPiece& king);

// Scans for pieces that are currently pinned, returning a vector of std::pairs that contain pairs of pointers 
//	to the pieces that are pinned and causing the pin, and an enum value that represents the direction of the pinning attack.
	std::vector<std::pair<std::pair<chessPiece*, chessPiece*>, PIN_DIR>> scanForPins(chessPiece& king);

// Populates attackVector with the coordinates of the squares between the king and the piece or pieces
// that are in checkVector, as well as the coordinate of the checking piece.
	std::vector<std::pair<int, int>> setAttackVector(chessPiece& king);

// Populates saviorVector with the pieces that can take the king out of check by
// being placed between the checking piece and the king by scanning through attackVector
// and determining which pieces on the board can move to those squares.
	std::vector<chessPiece*> scanForSaviors();

// Scans for potential escape squares for the king to move to if in check.
	std::vector<std::pair<int, int>> scanForEscapeSquares(chessPiece& king);
// Helper function for scanForEscapeSquares.  Returns true if location is not under attack.
	std::vector<chessPiece*> getAttackers(int col, int row);
//	Similar, but specially designed for the king to handle check conditions when attacked by minor pieces.
	std::vector<chessPiece*> getAttackers(chessPiece* king, int col, int row);
	
// Helper function to swap turn indicator and clear the check, escape, and attack vectors
	void swapTurn();

// Helper function that performs necessary work to complete a chess move.
	void performMove(chessPiece& piece, int destC, int destR, bool forceMove = false);

// Helper function that will return false if a moving piece is obstructed.
	bool isPathClear(chessPiece& piece, int destC, int destR);

// Helper function that will handle pawn promotions.
	void pawnPromotion(chessPiece& pawn);

// Helper function that will assist scanForPins() by finding potential pin candidates.
	std::vector<std::pair<chessPiece*, PIN_DIR>> scanForDefenders(chessPiece& king);


public:
	// Constructor
	chessBoardClass() { }
	// Copy Constructor
	chessBoardClass(const chessBoardClass& obj);
	// Destructor
	~chessBoardClass();
	// Sets the board and various instance variables to initial state
	void init();
	// Moves piece at (origC, origR) to (destC, destR) if such a move is legal.
	bool move(int origC, int origR, int destC, int destR, bool noMove = false, bool forceMove = false);
	// Returns the address of the piece at coordinates (c, r), or NULL if no piece is present.
	chessPiece* getSquareContents(int c, int r);

	// Returns true if the game is over.
	bool getCheckmate() { return checkmate; }

	// Returns true if the king is in check.
	bool getCheck() { return checkVector.size() > 0; }

	//	Returns true if the object has been created by an AI.
	bool getAI() { return ownedByAI; }

	// Returns the current state of the game, in the form of a pointer to the calling chessBoardClass object.
	// To (maybe) be used (responsibly) with the AI player to easily copy the current game state.
	chessBoardClass* getGameState() { return this; }

	void setTurn(PIECE_COLOR c);

	PIECE_COLOR getTurn() { return turn; }

	void operator=(const chessBoardClass& obj);

	//	Accessor methods that return points to the various piece vectors.
	
	//	Black piece accessor methods.
	std::vector<blackPawn>* getBlackPawns() { return &bPawns; }
	std::vector<blackKnight>* getBlackKnights() { return &bKnights; }
	std::vector<blackBishop>* getBlackBishops() { return &bBishops; }
	std::vector<blackRook>* getBlackRooks() { return &bRooks; }
	std::vector<blackQueen>* getBlackQueens() { return &bQueens; }
	std::vector<blackKing>* getBlackKing() { return &bKing; }

	//	White piece accessor methods.
	std::vector<whitePawn>* getWhitePawns() { return &wPawns; }
	std::vector<whiteKnight>* getWhiteKnights() { return &wKnights; }
	std::vector<whiteBishop>* getWhiteBishops() { return &wBishops; }
	std::vector<whiteRook>* getWhiteRooks() { return &wRooks; }
	std::vector<whiteQueen>* getWhiteQueens() { return &wQueens; }
	std::vector<whiteKing>* getWhiteKing() { return &wKing; }

	//	Get board-analysis vectors
	std::vector<chessPiece*>* getCheckVector() { return &checkVector; }
	std::vector<std::pair<int, int>>* getEscapeVector() { return &escapeVector; }
	std::vector<std::pair<int, int>>* getAttackVector() { return &attackVector; }
	std::vector<chessPiece*>* getSaviorVector() { return &saviorVector; }
	std::vector<std::pair<std::pair<chessPiece*, chessPiece*>, PIN_DIR>>* getPinVector() { return &pinVector; }
	std::vector<std::pair<chessPiece*, PIN_DIR>>* getDefenderVector() { return &defenderVector; }

};


// Below is the old interface/implementation.  It's not very good.

/*
struct chessPiece
{
	char* pieceType;
	bool isBlack;
	bool isWhite;

//	Only used for king pieces.
	int xCoord;
	int yCoord;
};

struct chessBoardSquare
{
	const int size = 64;
	bool containsPiece;
	chessPiece occupier;
	void setOccupier(char* type, bool isBlk) { occupier.pieceType = type; occupier.isBlack = isBlk; 
										       occupier.isWhite = !isBlk; containsPiece = true; }
};

class chessBoardClass  
{
private:
	// The chessboard, made up of an 8x8 2d array of chessBoardSquare structs.
	chessBoardSquare board[8][8];
	bool isWTurn;
	bool isBTurn;
	bool kingInCheck;
//  There are a lot of checks that involve the king's position on the board, so it's better to store
//	the position, rather than search for the king each time.
	chessPiece bKing;
	chessPiece wKing;

	// Checks if move made by corresponding piece is valid 
	// Note:  Valid Queen Move == true, if( rook && bishop are valid ) 
	bool isMoveValid(char* piece, int fromC, int fromR, int toC, int toR);
	bool kingMoveValid(int fromC, int fromR, int toC, int toR);
	bool knightMoveValid(int fromC, int fromR, int toC, int toR);
	bool bishopMoveValid(int fromC, int fromR, int toC, int toR);
	bool pawnMoveValid(int fromC, int fromR, int toC, int toR);
	bool rookMoveValid(int fromC, int fromR, int toC, int toR);
	bool queenMoveValid(int fromC, int fromR, int toC, int toR) { return (bishopMoveValid(fromC, fromR, toC, toR)
														  		  || rookMoveValid(fromC, fromR, toC, toR)); }

	void swapTurns() { isWTurn = !isWTurn; isBTurn = !isBTurn; }

public:
//  Constructor, initializes board by calling resetBoard.
	chessBoardClass() { resetBoard(); }
//  Sets all pieces up on the board in their correct positions, sets private data values to proper starting values.
	void resetBoard();
//	Called by display() to render the chessboard and the various pieces on the board, based on their position
	void renderChessBoard();
//  Handles the movement of pieces around the board.  Called by mouseInput(). 
	bool movePiece(int fromC, int fromR, int toC, int toR);
//  Returns piece type of occupier at (C,R) coordinates, if present (returns nullptr if not) 
	char* getPieceType(int c, int r) { if (board[r][c].containsPiece) return board[r][c].occupier.pieceType; 
									   else return nullptr; }
	



};

*/
#endif