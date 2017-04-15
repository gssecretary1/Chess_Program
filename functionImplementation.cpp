#pragma once

#include "functionPrototypes.h"
#include "chessBoardClass.h"
#include "chessAI.h"
#include "GlobalVars.h"
#include <GL/glut.h>

#include <iostream>
#include <stdlib.h>
#include <fstream>

using namespace std;

bool mouseDown = false;
int oldMouseX = 0;
int currentMouseX = 0;
int oldMouseY = 0;
int currentMouseY = 0;

int playMode = 0;


CBitmapFont chessFontBitmap;
chessBoardClass chessBoard;	// Default constructor does not initialize!  Must have follow up call to init() method.
chessAIClass* chessAI;
gameStateNode* root;
gameStateNode* currentState;

chessAIClass* debugWhiteAI;
chessAIClass* debugBlackAI;

/*	========================
	I/O FUNCTION DEFINITIONS 
	========================	*/

// Callback function to provide limited keyboard functionality to program.
void keyboardInput(unsigned char key, int x, int y)
{
	if (key == char(ESCAPE_KEY))
		exit(0);

	if (key == 'r' || key == 'R')
	{
		chessBoard.init();
		delete chessAI;
		delete debugWhiteAI;
		delete debugBlackAI;
		playMode = 0;
		printInstructions();
	}

	//	Emulate mouse input to advance AI vs AI play
	if (key == 'p' || key == 'P')
		mouseInput(GLUT_LEFT_BUTTON, GLUT_UP, 1, 1);
}

void arrowKeyInput(int key, int x, int y)
{
	if (playMode == 2)
	{
		if (key == GLUT_KEY_LEFT)
			chessAI->traverseHistory(LEFT);
		else if (key == GLUT_KEY_RIGHT)
			chessAI->traverseHistory(RIGHT);
	}
	else if (playMode == 3)
	{
		if (key == GLUT_KEY_LEFT)
		{
			debugWhiteAI->traverseHistory(LEFT);
			debugBlackAI->traverseHistory(LEFT);
		}
		else if (key == GLUT_KEY_RIGHT)
		{
			debugWhiteAI->traverseHistory(RIGHT);
			debugBlackAI->traverseHistory(RIGHT);
		}	
	}
	
}

// Allows for click-and-drag placement of chess pieces on board.
void mouseInput(int button, int state, int x, int y)
{
	static int x0 = 0, y0 = 0, x1 = 0, y1= 0;

	if (button == GLUT_RIGHT_BUTTON)
		return;

	// Don't allow piece movement if the game is over.
	if (chessBoard.getCheckmate() == false)
	{
		if ((state == GLUT_DOWN) && (button == GLUT_LEFT_BUTTON))
		{
			x0 = x;
			y0 = WINDOW_HEIGHT - y;

			//if (DEBUG)
			//	cout << "Click has occurred at coordinate (" << x0 << ", " << y0 << ")." << endl;
		}
		if ((state == GLUT_UP))
		{
			x1 = x;
			y1 = WINDOW_HEIGHT - y;

			//if (DEBUG)
			//	cout << "Release has occured at coordinate (" << x1 << ", " << y1 << ")." << endl;

			// Adjust x and y coords to column/row values
			x0 -= BOARD_OFFSET;
			x1 -= BOARD_OFFSET;
			y0 -= BOARD_OFFSET;
			y1 -= BOARD_OFFSET;

			if (x0 >= 0 && x1 >= 0 && y0 >= 0 && y1 >= 0)
			{
				x0 /= 64;
				y0 /= 64;
				x1 /= 64;
				y1 /= 64;

				// Don't both processing non-move clicks on the chessBoard.
				if (x0 == x1 && y0 == y1)
					return;

				//	Use pointer to global chessBoard for Human vs Human play mode.
				if (playMode == 1)
				{
					if (chessBoard.getCheckmate())
					{
						// Print a message or something.
						cout << "======================" << endl
							<< "CHECKMATE HAS OCCURED!" << endl
							<< "======================" << endl;

						return;
					}

					if (chessBoard.move(x0, y0, x1, y1, true))
						chessBoard.move(x0, y0, x1, y1, false, true);

					if (chessBoard.getCheckmate())
					{
						// Print a message or something.
						cout << "======================" << endl
							<< "CHECKMATE HAS OCCURED!" << endl
							<< "======================" << endl;

						return;
					}

					return;
				}
				else if (playMode == 2)	//	Use pointer to current chessAI's chessBoard for Human vs Computer play mode.
				{

					// If the attempted move is invalid, don't allow the AI to process it.
					if (!chessAI->getCurrentState()->gameState.move(x0, y0, x1, y1, true))
						return;

					chessAI->signal(x0, y0, x1, y1);

					//playerVsAI->move(x0, y0, x1, y1, false, true);

					if (chessAI->getCurrentState()->gameState.getCheckmate())
					{
						// Print a message or something.
						cout << "======================" << endl
							<< "CHECKMATE HAS OCCURED!" << endl
							<< "======================" << endl;

						return;
					}

					//cout << "DEBUG:: Piece moved - (" << x0 << ", " << y0 << ") -> (" << x1 << ", " << y1 << ")\n" << endl;

					//	DEBUG:	Print potential memory leak information to console window.
					if (AI_DEBUG)
					{
						currentState = chessAI->getCurrentState();

						cout << "\n\nNumber of missed children (memeory leak issue): ";

						cout << currentState->previous->next.size() - 1;

						cout << "\n\n\n";
					}

					action moveData = chessAI->think();

					chessAI->play(moveData);

					if (chessAI->getCurrentState()->gameState.getCheckmate())
					{
						// Print a message or something.
						cout << "======================" << endl
							<< "CHECKMATE HAS OCCURED!" << endl
							<< "======================" << endl;

						return;
					}

					//	DEBUG:	Print potential memory leak information to console window.
					if (AI_DEBUG)
					{
						currentState = chessAI->getCurrentState();

						cout << "\n\nNumber of missed children (memeory leak issue): ";

						cout << currentState->previous->next.size() - 1;

						cout << "\n\n\n";
					}

					//	DEBUG:  Print AI move information to console window.
					/*
					cout << "DEBUG:: Piece moved - (" << moveData.origC << ", " << moveData.origR
						<< ") -> (" << moveData.destC << ", " << moveData.destR << ")\n" << endl;

					cout << "Heuristic Value: " << moveData.heuristic << '\n' << std::endl;
					*/
				}
				else if (playMode == 3)
				{

					action moveData = debugWhiteAI->think();

					debugWhiteAI->play(moveData);
					debugBlackAI->play(moveData);

					//playerVsAI->move(x0, y0, x1, y1, false, true);

					if (debugWhiteAI->getCurrentState()->gameState.getCheckmate())
					{
						// Print a message or something.
						cout << "======================" << endl
							<< "CHECKMATE HAS OCCURED!" << endl
							<< "======================" << endl;

						return;
					}

					cout << "DEBUG:: Piece moved - (" << moveData.origC << ", " << moveData.origR
						<< ") -> (" << moveData.destC << ", " << moveData.destR << ")\n" << endl;

					cout << "Heuristic Value: " << moveData.heuristic << '\n' << std::endl;

					//	DEBUG:	Print potential memory leak information to console window.
					if (AI_DEBUG)
					{
						currentState = chessAI->getCurrentState();

						cout << "\n\nNumber of missed children (memeory leak issue): ";

						cout << currentState->previous->next.size() - 1;

						cout << "\n\n\n";
					}

					moveData = debugBlackAI->think();

					debugBlackAI->play(moveData);
					debugWhiteAI->play(moveData);

					//	DEBUG:	Print potential memory leak information to console window.
					if (AI_DEBUG)
					{
						currentState = chessAI->getCurrentState();

						cout << "\n\nNumber of missed children (memeory leak issue): ";

						cout << currentState->previous->next.size() - 1;

						cout << "\n\n\n";
					}

					//	DEBUG:  Print AI move information to console window.
					cout << "DEBUG:: Piece moved - (" << moveData.origC << ", " << moveData.origR
						<< ") -> (" << moveData.destC << ", " << moveData.destR << ")\n" << endl;

					cout << "Heuristic Value: " << moveData.heuristic << '\n' << std::endl;
				}
			}
		}
	}
}

// Tracks x, y coordinate when mouse is pressed down, for line drawing.
void mouseMotion(int x, int y)
{
	glutPostRedisplay();

	currentMouseX = x;
	currentMouseY = WINDOW_HEIGHT - y;
}

// Prints instructions for user to console window.
void printInstructions()
{
	using std::cout;
	using std::endl;

	cout << "===== MOUSE INSTRUCTIONS =====" << endl << endl;

	cout << "Click and drag pieces to play." << endl << endl;

	cout << "===== KEYBOARD INSTRUCTIONS ======" << endl << endl;
	cout << "Reset Game: r" << endl;
	cout << "Exit Program: ESC" << endl;

	cout << "\n\n";

	while (!(playMode == 1 || playMode == 2 || playMode == 3))
	{
		cout << "Select Mode of Play (1 - Human vs. Human; 2 - Human vs. Computer): ";
		cin >> playMode;
	}

	if (playMode == 2)
	{
		chessBoardClass* ptr = chessBoard.getGameState();

		int difficulty = 5;

		cout << "Enter difficulty setting for computer (1 - 5):  ";
		cin >> difficulty;

		chessAI = new chessAIClass(chessBoard, BLACK, difficulty);

		root = chessAI->getInitialState();
	}
	
	if (playMode == 3)
	{
		int difficulty = 2;

		debugWhiteAI = new chessAIClass(chessBoard, WHITE, difficulty);
		debugBlackAI = new chessAIClass(chessBoard, BLACK, difficulty);

		root = debugWhiteAI->getInitialState();
	}
}



/*	============================ 
	DISPLAY FUNCTION DEFINITIONS 
	============================	*/

char* translatePieceType(PIECE_COLOR c, PIECE_TYPE t)
{
	char* whitePieces[6] = { "p", "n", "l", "r", "q", "k" };
	char* blackPieces[6] = { "P", "N", "L", "R", "Q", "K" };
	if (c == BLACK)
	{
		if (t == PAWN)
			return blackPieces[0];
		else if (t == KNIGHT)
			return blackPieces[1];
		else if (t == BISHOP)
			return blackPieces[2];
		else if (t == ROOK)
			return blackPieces[3];
		else if (t == QUEEN)
			return blackPieces[4];
		else if (t == KING)
			return blackPieces[5];
	}
	else if (c == WHITE)
	{
		if (t == PAWN)
			return whitePieces[0];
		else if (t == KNIGHT)
			return whitePieces[1];
		else if (t == BISHOP)
			return whitePieces[2];
		else if (t == ROOK)
			return whitePieces[3];
		else if (t == QUEEN)
			return whitePieces[4];
		else if (t == KING)
			return whitePieces[5];
	}

	return NULL;
}

void renderChessBoard()
{
	char* numBorderArr[8] = { "1", "2", "3", "4", "5", "6", "7", "8" };
	char* alphaBorderArr[8] = { "a", "b", "c", "d", "e", "f", "g", "h" };
	//  Draws the chess board itself
	for (int i = 0; i < 8; i++)
		for (int j = 0; j < 8; j++)
		{
			if (i == j)
			{	// Draws the column/row markers
				chessFontBitmap.Select();
				chessFontBitmap.Print(numBorderArr[i], -5, BOARD_OFFSET + i*64 + 3);
				chessFontBitmap.Print(alphaBorderArr[i], BOARD_OFFSET + 4 + i*64, -20);
			}
			glDisable(GL_TEXTURE_2D);
			glBegin(GL_QUADS);

			//	Selects color for light square
			if ((i + j) % 2)
				glColor3f(0.5, 0.5, 0.68);
			else // Selects color for dark square
				glColor3f(.25, .25, .25);

			// Draws the square at the correct position on the board.
			glVertex2i(BOARD_OFFSET + (i * 64), BOARD_OFFSET + (j * 64));
			glVertex2i(BOARD_OFFSET + (i * 64), BOARD_OFFSET + (j + 1) * 64);
			glVertex2i(BOARD_OFFSET + ((i + 1) * 64), BOARD_OFFSET + (j + 1) * 64);
			glVertex2i(BOARD_OFFSET + ((i + 1) * 64), BOARD_OFFSET + j * 64);

			glEnd();
		}

	//	Draws all of the chess pieces, row by row.
	for (int row = 0; row < 8; ++row)
	{
		for (int col = 0; col < 8; ++col)
		{
			chessPiece* piece;
			// Get piece from square (is NULL if no piece)
			if (playMode == 1)
			{
				piece = chessBoard.getSquareContents(col, row);
			}
			else if (playMode == 2)
			{
				piece = chessAI->getCurrentState()->gameState.getSquareContents(col, row);
			}
			else if (playMode == 3)
			{
				piece = debugWhiteAI->getCurrentState()->gameState.getSquareContents(col, row);
			}
			
			if (piece != NULL)
			{
				PIECE_COLOR color = piece->getColor();
				PIECE_TYPE type = piece->getType();

				chessFontBitmap.Select();
				//chessFontBitmap.Print(*character to render*, *X Coord*, *Y Coord*)
				chessFontBitmap.Print(translatePieceType(color, type),
					(BOARD_OFFSET + 6 + (col * 64)), BOARD_OFFSET + (row * 64));
			}
		}
	}

}

// Performs the work to display the chess board and pieces
void display()
{
	// Slows frame drawing down so it doesn't eat up so much CPU time.
	Sleep(33);
	// glClear() required here to render the scene properly
	glClear(GL_COLOR_BUFFER_BIT);

	// Does the actually scene drawing, chess board AND pieces.
	renderChessBoard();

	glutSwapBuffers();
}

/*
// Draws the line from the click-point to the current mouse position, on left-mouse-button hold.
   void drawLine(int x0, int x1, int y0, int y1)
{
	glColor3f(0.75, 0.2, 1.0);

	glBegin(GL_LINES);
	glVertex2i(x0, y0);
	glVertex2i(x1, y1);

	glEnd();
}
*/

void windowResizeEvent(int x, int y)
{
	glutReshapeWindow(WINDOW_WIDTH, WINDOW_HEIGHT);
}

/*	===================================
	INITIALIZATION FUNCTION DEFINITIONS
	===================================	*/

// Initializes values needed for program.
void init(int argc, char** argv)
{
	// Various obligatory GLUT setup stuff
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutInitWindowPosition(50, 400);
	glutCreateWindow("CS 4480 - Chess AI Project");
	glutDisplayFunc(display);

	glutKeyboardFunc(keyboardInput);
	glutSpecialFunc(arrowKeyInput);
	glutMouseFunc(mouseInput);
	glutMotionFunc(mouseMotion);
	glutReshapeFunc(windowResizeEvent);

	glutSetCursor(GLUT_CURSOR_RIGHT_ARROW);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// Setup the coordinate system based on the window's properties.
	gluOrtho2D(0.0, GLdouble(WINDOW_WIDTH), 0.0, GLdouble(WINDOW_HEIGHT));

	// Load the bitmap font file, set the color.
	// Pure white is a bit harsh, so it's toned down to 0.9
	chessFontBitmap.Load("DiagramTTAlpha2.bff");	// Make sure file is located in same directory as .exe
	chessFontBitmap.SetColor(.9, .9, .9);

	chessBoard.init();
}
