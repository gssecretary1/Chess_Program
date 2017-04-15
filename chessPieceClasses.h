#pragma once

#ifndef CHESS_PIECE_CLASSES
#define CHESS_PIECE_CLASSES

#include <string>



//  ====== Design Remarks and Thoughts ======
/*
	This is a redesign for the chess/chessboard interface.  This will hopefully 
	make it more programmer friendly as the initial version is difficult to read and reason about.

	The abstract base class shows the general properties and interface that is common to all
	chess pieces, regardless of type or color.  The following class definitions get less and less
	general, until the end result is separate classes for each color/type of chess piece.

	It should be noted that the only difference between a blackPawn and a whitePawn (or classes pertaining
	to another type and their color counterpart) is that the std::string color variable is set to the
	appropriate value upon construction.  The classes themselves are in all other respects defined the same.
	This is mainly done so the code is more self-documenting.

	1st Level:  Chess Piece
	2nd Level:  <Color> Chess Piece (<Color> == (Black || White))
	3rd Level:  <Color> <Type> (<Type> == (Pawn || Knight || Bishop || Rook || Queen || King))

	The third level classes will be instance variables of chessBoardClass:

	vector<blackPawn*> bPawnVector();
	vector<blackKnight*> bKnightVector();
	vector<blackBishop*> bBishopVector();
	vector<blackRook*> bRookVector();
	vector<blackQueen*> bQueenVector();
	vector<blackKing*> bKing();
	(etc., for white pieces)

	Pieces will be handled with pointers stored in vectors for easy deletion upon capture.
	Vectors and pointers also allow for major pieces to be easily added upon pawn promotions.
	The pointer to king is kept in a vector for the sake of consistency.
*/

enum PIECE_COLOR { WHITE, BLACK };
enum PIECE_TYPE { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING };


// Abstract base class - establishes interface common to all derived classes of chessPiece
class chessPiece
{
protected:
	int column;			// Note:  column count starts at 0 (chess board is represented w/ a 2d array)
	int row;			// Note:  row count starts at 0 (chess board is represented w/ a 2d array).
	enum PIECE_COLOR color;
	enum PIECE_TYPE type;
	bool captured;

	virtual void setColumn(int c) = 0;
	virtual void setRow(int r) = 0;
	bool inBounds(int c, int r) { return !(c > 7 || c < 0 || r > 7 || r < 0); } // Pieces must stay on the board
	bool isMove(int c, int r) { return !(c == column && r == row); }			// New square cannot be the old square 

	/* Both inBounds() and isMove() are basic checks that every validMovement() method of each type must perform */

public:
	virtual int getColumn() = 0;
	virtual int getRow() = 0;
	virtual PIECE_COLOR getColor() = 0;
	virtual PIECE_TYPE getType() = 0;
	virtual bool validMovement(int c, int r) = 0;	// Must be defined per type.  See note below.
	virtual void moveTo(int c, int r) = 0;			// If move is completely valid, then moveTo may be called.
	virtual void setCaptured(bool flag) = 0;
	virtual bool getCaptured() const = 0;
	chessPiece(const chessPiece& piece) { column = piece.column; row = piece.row; color = piece.color; type = piece.type; captured = piece.captured; }
	chessPiece() { }
	bool operator==(const chessPiece& piece) { return (column == piece.column && row == piece.row && color == piece.color && type == piece.type && captured == piece.captured); }
};

//	====================================
//	Notes on validMovement(int c, int r)		[ c and r represent target column and row, respectively ]
//	====================================
/*	
	This is not a "solve-all" test to determine if the final move should be carried out.
	This only determines if the movement of any calling chess piece follows the 
	most fundamental rules of the piece itself.

	For instance, if called on a pawn object, it will only check if the pawn would be moving
	forward 1, diagonally up-left/up-right, or forward 2 if the pawn is in the starting position.

	The function will not determine if the pawn's movement would be blocked by another
	piece, or if there is a piece to capture in the event of diagonal movement, or if the piece is pinned.

	This is essentially a "first-wave" check on attempted movement to determine if, as stated,
	the movement follows the most fundamental rules of piece movement.  For example, it will
	rule out ridiculous moves, like trying to move a king 3 forward and 1 right, or moving pieces
	out of bounds (pawn to x57 is not a valid move).

	Other movement checks, e.g., ensuring a king isn't moved into check, making sure pieces can't
	"phase through" each other (aside from the knight), ensuring there is no "friendly fire," etc.,
	will be performed by the chessBoard class.  This is necessary since information about the current
	state of the board is not handled by chessType classes, and is private to the chessBoard class.
*/

class blackChessPiece : public chessPiece
{
protected:
	void setColumn(int c) { column = c; }
	void setRow(int r) { row = r; }

public:
	int getColumn() { return column; }
	int getRow() { return row; }
	bool getCaptured() const { return captured; }
	void setCaptured(bool flag) { captured = flag; }
	blackChessPiece(const blackChessPiece& piece) { column = piece.column; row = piece.row; color = piece.color; type = piece.type; captured = piece.captured; }
	blackChessPiece() { }

	PIECE_COLOR getColor() { return color; }
};

class whiteChessPiece : public chessPiece
{
protected:
	void setColumn(int c) { column = c; }
	void setRow(int r) { row = r; }
public:
	int getColumn() { return column; }
	int getRow() { return row; }
	bool getCaptured() const { return captured; }
	void setCaptured(bool flag) { captured = flag; }
	whiteChessPiece(const whiteChessPiece& piece) { column = piece.column; row = piece.row; color = piece.color; type = piece.type; captured = piece.captured; }
	whiteChessPiece() { }
	
	PIECE_COLOR getColor() { return color; }
};


class blackPawn : public blackChessPiece
{
private:
	bool enPassant;
public: 
	blackPawn(int c, int r) : enPassant(false)
	{ setColumn(c); setRow(r); color = BLACK; type = PAWN; setCaptured(false); }

	PIECE_TYPE getType() { return type; }
	bool validMovement(int c, int r);
	void moveTo(int c, int r) { if (abs(getRow() - r) == 2) setEnPassant(true); setColumn(c); setRow(r); }
	void setEnPassant(bool flag) { enPassant = flag; }
	bool getEnPassant() { return enPassant; }
	blackPawn() { }
};


class blackKnight : public blackChessPiece
{ 
public: 
	blackKnight(int c, int r) 
	{ setColumn(c); setRow(r); color = BLACK; type = KNIGHT; setCaptured(false); }

	PIECE_TYPE getType() { return type; }
	bool validMovement(int c, int r);
	void moveTo(int c, int r) { setColumn(c); setRow(r); }
	blackKnight() { }
};


class blackBishop : public blackChessPiece
{
public:
	blackBishop(int c, int r)
	{ setColumn(c); setRow(r); color = BLACK; type = BISHOP; setCaptured(false); }

	PIECE_TYPE getType() { return type; }
	bool validMovement(int c, int r);
	void moveTo(int c, int r) { setColumn(c); setRow(r); }
	blackBishop() { }
};


class blackRook : public blackChessPiece
{
private:
	bool canCastle;
public:
	blackRook(int c, int r) : canCastle(true) 
	{ setColumn(c); setRow(r); color = BLACK; type = ROOK; setCaptured(false); }

	PIECE_TYPE getType() { return type; }
	bool validMovement(int c, int r);
	void moveTo(int c, int r) { setColumn(c); setRow(r); setCastle(false); }
	void setCastle(bool flag) { canCastle = flag; }
	bool getCastle() { return canCastle; }
	blackRook() { } 
};


class blackQueen : public blackChessPiece
{
public:
	blackQueen(int c, int r) 
	{ setColumn(c); setRow(r); color = BLACK; type = QUEEN; setCaptured(false); }

	PIECE_TYPE getType() { return type; }
	bool validMovement(int c, int r);
	void moveTo(int c, int r) { setColumn(c); setRow(r); }
	blackQueen() { }
};


class blackKing : public blackChessPiece
{
private:
	bool canCastle;
public:
	blackKing(int c, int r) : canCastle(true)
	{ setColumn(c); setRow(r); color = BLACK; type = KING; setCaptured(false); }

	PIECE_TYPE getType() { return type; }
	bool validMovement(int c, int r);
	void moveTo(int c, int r) { setColumn(c); setRow(r); canCastle = false; }
	bool getCastle() { return canCastle; }
	blackKing() { }
};

/*
	Below are the classes for the white variants of the above black<Type> classes.
	They are essentially identical, with the only difference being the std::string color
	value of each class is set to it's appropriate value ("white").  In every other respect,
	each class below behaves identically to its color counterpart.

	Note:	White pawn movement is changed to account for opposite movement direction.
*/

class whitePawn : public whiteChessPiece
{
private:
	bool enPassant;
public:
	whitePawn(int c, int r) : enPassant(false) 
	{ setColumn(c); setRow(r); color = WHITE; type = PAWN; setCaptured(false); }

	whitePawn() { enPassant = false; color = WHITE; type = PAWN; }

	whitePawn(const whitePawn& obj) { this->setColumn(obj.column); setRow(obj.row); color = WHITE; type = PAWN; setCaptured(obj.captured); setEnPassant(obj.enPassant); }

	PIECE_TYPE getType() { return type; }
	bool validMovement(int c, int r);
	void moveTo(int c, int r) { if (abs(getRow() - r) == 2) setEnPassant(true); setColumn(c); setRow(r); }
	void setEnPassant(bool flag) { enPassant = flag; }
	bool getEnPassant() { return enPassant; }

};


class whiteKnight : public whiteChessPiece
{
public:
	whiteKnight(int c, int r) 
	{ setColumn(c); setRow(r); color = WHITE; type = KNIGHT; setCaptured(false); }

	PIECE_TYPE getType() { return type; }
	bool validMovement(int c, int r);
	void moveTo(int c, int r) { setColumn(c); setRow(r); }
	whiteKnight() { }
};


class whiteBishop : public whiteChessPiece
{
public:
	whiteBishop(int c, int r) 
	{ setColumn(c); setRow(r); color = WHITE; type = BISHOP; setCaptured(false); }

	PIECE_TYPE getType() { return type; }
	bool validMovement(int c, int r);
	void moveTo(int c, int r) { setColumn(c); setRow(r); }
	whiteBishop() { }
};


class whiteRook : public whiteChessPiece
{
private:
	bool canCastle;
public:
	whiteRook(int c, int r) : canCastle(true)
	{ setColumn(c); setRow(r); color = WHITE; type = ROOK; setCaptured(false); }

	PIECE_TYPE getType() { return type; }
	bool validMovement(int c, int r);
	void moveTo(int c, int r) { setColumn(c); setRow(r); setCastle(false); }
	void setCastle(bool flag) { canCastle = flag; }
	bool getCastle() { return canCastle; }
	whiteRook() { }
};


class whiteQueen : public whiteChessPiece
{
public:
	whiteQueen(int c, int r) 
	{ setColumn(c); setRow(r); color = WHITE; type = QUEEN; setCaptured(false); }

	PIECE_TYPE getType() { return type; }
	bool validMovement(int c, int r);
	void moveTo(int c, int r) { setColumn(c); setRow(r); }
	whiteQueen() { }
};


class whiteKing : public whiteChessPiece
{
private:
	bool canCastle;
public:
	whiteKing(int c, int r) : canCastle(true)
	{ setColumn(c); setRow(r); color = WHITE; type = KING; setCaptured(false); }

	PIECE_TYPE getType() { return type; }
	bool validMovement(int c, int r);
	void moveTo(int c, int r) { setColumn(c); setRow(r); canCastle = false; }
	bool getCastle() { return canCastle; }
	whiteKing() { }
};

#endif