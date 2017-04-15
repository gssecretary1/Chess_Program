#pragma once

#include "chessPieceClasses.h"



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
	out of bounds (pawn to column 49, row 57 is not a valid move).

	Other movement checks, e.g., ensuring a king isn't moved into check, making sure pieces can't
	"phase through" each other (aside from the knight), ensuring there is no "friendly fire," etc.,
	will be performed by the chessBoard class methods.  This is necessary since information about the current
	state of the board is not handled by chessType classes (it's private to the chessBoard class) and this
	information is required to check for such conditions.
*/

//	Pawn movement properties:
/*
	Can move forward 1 space.
	Can move forward 2 spaces if row == 6 (Black pawn starting position - where row count starts at 0).
	Can move diagonally up-left or up-right a city-block distance of 2.
*/
bool blackPawn::validMovement(int c, int r)
{
	// Check to make sure movement is within bounds
	if (!inBounds(c, r))
		return false;
	if (!isMove(c, r))
		return false;

	if (column == c && row == 6 && r == 4)		// Forward 2 from starting position case.
		return true;
	else if (column == c && (row - r) == 1)		// Forward 1 case
		return true;
	else if (((row - r) == 1) && (std::abs(column - c) == 1))	// Diagonal movement case.
		return true;
	else
		return false;
}
//	Remaining movement checks to be performed by chessBoard class:
/*
	-	Friendly fire
	-	Blocked path
	-	Pinned
*/



// Knight movement properties
/*
	Knight movement can be separated into 4 quadrant cases,
	each quadrant with 2 of its own cases

			Quadrant 2 		    X   X		        Quadrant 1
	(r > row, c < column)	  X       X       (r > row, c > column)
								  N
							  X       X
			Quadrant 3 		    X   X		        Quadrant 4
	(r < row, c < column)				      (r < row, c > column)

	Quadrant 1:
	Up 1, Right 2
	Up 2, Right 1
	Quadrant 2:
	Up 2, Left 1
	Up 1, Left 2
	Quadrant 3:
	Down 1, Left 2
	Down 2, Left 1
	Quadrant 4:
	Down 2, Right 1
	Down 1, Right 2

	Another way of looking at this is by considering city-block distance;
	knight movement always has a city-block distance of 3.

	The only possible ways to travel 3 city blocks are the 8 ways listed above
	and travel that is purely horizontal or vertical.

										X
									 O     O
								   O		 O
								 X		N	   X
								   O		 O
									 O	   O
										X

	N is the knight's starting position
	O's represent the knight's valid final positions after traveling 3 city-blocks
	X's represent the knight's invalid final positions after traveling 3 city-blocks

				  ( movement over city-block distance of 3 )    && (!horiz) && (!vertical)
	So long as ((std::abs(column - c) + std::abs(row - r) == 3) && r != row && c != column)
	is true, then the knight's movement is valid.
*/
bool blackKnight::validMovement(int c, int r)
{
	using std::abs;

	if (!inBounds(c, r))
		return false;
	if (!isMove(c, r))
		return false;

	return (((abs(column - c) + abs(row - r)) == 3) && r != row && c != column);
}
//	Remaining movement checks to be performed by chessBoard class:
/*
	-	Friendly fire
	-	Pinned
*/



// Bishop movement properties
/*
	A bishop's movement is a diagonal line, whose slope is equal to +/- 1, or slope == abs(r/c) == 1.
	It's possible that c is equal to 0 (if the bishop moves to the leftmost column) so this simple
	check does not work in all cases.

	Another approach is this:  the absolute value of the difference between the bishop's
	new row (r) and old row (row) should be equal to the absolute value of the difference
	between the bishop's new column (c) and the old column (column).

	In other words:  The slope is equal to 1 when abs(r-row) == abs(c-column).

	Bishop movement is valid when (std::abs(r - row) == std::abs(c - column)) is true.

	This ensures that diagonal movements are only valid if the slope is strictly +/- 1, and
	it rules out bishops being able to move horizontally or vertically.
*/
bool blackBishop::validMovement(int c, int r)
{
	using std::abs;

	if (!inBounds(c, r))
		return false;
	if (!isMove(c, r))
		return false;

	return (abs(r - row) == abs(c - column));
}
//	Remaining movement checks to be performed by chessBoard class:
/*
	-	Friendly fire
	-	Blocked path
	-	Pinned
*/



// Rook movement properties
/*
	If a rook's movement is vertical, then c == column.
	If a rook's movement is horizontal, then r == row.
	Movement that is both horizontal and vertical is impossible.

	In other words:

	( (c == column && r != row) || (r == row && c != column) )
			Vertical case				Horizontal case

	The horizontal or vertical distance the rook covers
	need only be limited by the boundaries of the chess board itself.

	Diagonal and diagonal-esque movements don't pass this test, since that would require
	a change in both column AND row values, and either case requires some constancy in
	either the column or the row.
*/
bool blackRook::validMovement(int c, int r)
{
	if (!inBounds(c, r))
		return false;
	if (!isMove(c, r))
		return false;

	return ((c == column && r != row) || (r == row && c != column));
}
//	Remaining movement checks to be performed by chessBoard class:
/*
-	Friendly fire
-	Blocked path
-	Pinned
*/



// Queen movement properties
/*
	Since the movement properties have already been defined for the bishop and rook,
	it can simply be said that a queen, for any movement, must behave either as a
	bishop or as a rook.  You cannot behave like both a bishop AND a rook simultaneously 
	(you cannot travel non-diagonally and diagonally at the same time), so we don't 
	have to worry about this kind of case.

	A queen must pass either the bishop movement test or the rook movement test.

	(abs(r - row) == abs(c - column)) || ((c == column && r != row) || (r == row && c != column));
*/
bool blackQueen::validMovement(int c, int r)
{
	if (!inBounds(c, r))
		return false;
	if (!isMove(c, r))
		return false;

	//			(Bishop movement test)		 ||					  (Rook movement test)
	return (abs(r - row) == abs(c - column)) || ((c == column && r != row) || (r == row && c != column));
}
//	Remaining movement checks to be performed by chessBoard class:
/*
	-	Friendly fire
	-	Blocked path
	-	Pinned
*/



// King movement properties
/*
	If traveling non-diagonally, a king can only travel a distance of 1.
	If travelling diagonally, a king can only travel a city-block distance of 2.

	if (r == row || c == column)
		return (std::abs(r - row) + std::abs(c - column) == 1);
	else
		return (std::abs(r - row) + std::abs(c - column) == 2);

	* Special Case - Castling *

	If a player tries to move the king two spaces to the left or the right, 
	and the king has not been moved yet, then the king is eligible to castle
	with another eligible rook to its left or right, corresponding to the
	direction of the move.

	Note - Castling can only occur under the following conditions:

	=	The king is eligible for castling (has not been moved)

	=	The king is not in check

	=	The rook involved (left rook for left move, right rook for right move)
		is eligible (has not been moved)

	=	There can be no pieces occupying the space between the king and the rook

	=	If castling left (queen-side), then the square just to the left of the
		king must not be under attack.

	=	If casting right (king-side), then the square just to the right of the
		king must not be under attack.

	Note:  This method only checks the 1st case.
*/
bool blackKing::validMovement(int c, int r)
{
	using std::abs;

	if (!inBounds(c, r))
		return false;
	if (!isMove(c, r))
		return false;

	// Castling Movement
	if (abs(c - column) == 2 && r == row && row == 7)
	{
		if (canCastle)
			return true;
		else
			return false;
	}
		
	// Regular movement
	if (r == row || c == column)
		return (abs(r - row) + abs(c - column) == 1);
	else
		return (abs(r - row) + abs(c - column) == 2);
}
//	Remaining movement checks to be performed by chessBoard class:
/*
	-	Friendly fire
	-	Blocked path
	-	Is target square under attack
	-	Remaining castling conditions (if move is to castle, see above)
*/


// The following validMovement definitions are all the same as above, but defined for white pieces.


bool whitePawn::validMovement(int c, int r)
{
	// Check to make sure movement is within bounds
	if (!inBounds(c, r))
		return false;
	if (!isMove(c, r))
		return false;

	// Note: Pawn movement direction has changed from the blackPawn definition above.

	if (column == c && row == 1 && r == 3)		// Forward 2 from starting position case.
		return true;
	else if (column == c && (r - row) == 1)		// Forward 1 case
		return true;
	else if (((r - row) == 1) && (std::abs(column - c) == 1))	// Diagonal movement case.
		return true;
	else
		return false;
}


bool whiteKnight::validMovement(int c, int r)
{
	using std::abs;

	if (!inBounds(c, r))
		return false;
	if (!isMove(c, r))
		return false;

	return (((abs(column - c) + abs(row - r)) == 3) && r != row && c != column);
}


bool whiteBishop::validMovement(int c, int r)
{
	using std::abs;

	if (!inBounds(c, r))
		return false;
	if (!isMove(c, r))
		return false;

	return (abs(r - row) == abs(c - column));
}


bool whiteRook::validMovement(int c, int r)
{
	if (!inBounds(c, r))
		return false;
	if (!isMove(c, r))
		return false;

	return ((c == column && r != row) || (r == row && c != column));
}


bool whiteQueen::validMovement(int c, int r)
{
	if (!inBounds(c, r))
		return false;
	if (!isMove(c, r))
		return false;

	//			(Bishop movement test)		 ||					  (Rook movement test)
	return (abs(r - row) == abs(c - column)) || ((c == column && r != row) || (r == row && c != column));
}


bool whiteKing::validMovement(int c, int r)
{
	using std::abs;

	if (!inBounds(c, r))
		return false;
	if (!isMove(c, r))
		return false;

	// Castling Movement
	if (abs(c - column) == 2 && r == row && row == 0)
	{
		if (canCastle)
			return true;
		else
			return false;
	}

	if (r == row || c == column)
		return (abs(r - row) + abs(c - column) == 1);
	else
		return (abs(r - row) + abs(c - column) == 2);
}