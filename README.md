Disclaimer
==========
All of the code that was used for the project is here.

The only code that I did not create is the code found in BitmapFontClass.h and BitmapFontClass.cpp.
Credit for this code goes to Codehead, which was found in the help section of Codehead's program,
CBFG - Codehead's Bitmap Font Generator, which is used in conjunction with this program to help draw
the chess pieces and markers on the screen.

Codehead's CBFG Webpage: http://www.codehead.co.uk/cbfg/

The DiagramTTAlpha2.bff file, generated using CBFG, originally came from a font file.
The font used in conjunction with CBFG is courtesy of dacelle, a user on http://www.chess.com/.
The font was retrieved from:  https://www.chess.com/download/view/fritz-fonts.

Application Instructions
========================
In order to run the .exe, you'll need to include the DiagramTTAlpha2.bff and freeglut.dll files in the
same directory, otherwise it'll crash.

The .exe will open up two windows - a GUI-based window and a basic console window.  The GUI-based window
will be blank until you enter in the necessary commands into the console window to start a game - instructions
are provided in the console window.  Note:  higher numbers may be entered for AI difficulty than those listed,
but result in long wait-times for AI play.  This is due to the fact that search time increases exponentially with
search depth (how many moves ahead are considered).  5 seems to be the maximum - 6 results in long wait-times.

There are some additional keyboard commands that are not listed in the console window that I included before 
completing the project - all commands are listed below.  
The GUI window must be selected to register the following keyboard commands:

ESC - 	Closes the program.
R	-	Restarts the application (you must re-enter the initial console commands after this).
Left Arrow	-	Scrolls the game back to the previous moved that was played.
Right Arrow	-	Scrolls the game forward to a move that was played after the current state.

Note:
If you are able to push a pawn to the opposite end of the board, you must enter in the piece type into the console
window that you wish to promote the pawn to.  The game will not proceed until this is done, so it may seem like it
has locked up.


File Overview
=============
There are about 8,500 lines of code that were written for this project, most of which are documented.  The following
is a short list describing what each file contains:

BitmapFontClass.cpp:  	
Class method definitions that are used for rendering the chess pieces and board markers onto the
			screen, using the GLUT OpenGL library.
			
						
BitmapFontClass.h:	
Class declaration for the object used to render chess pieces and board markers.


chessAI.cpp:	
Class method definitions for the AI.  These are mostly wrapper functions that call functions
		that are contained in chessGameTree.cpp.


chessAI.h:	
Class declaration for the AI.


chessBoardClass.cpp:	
Class method definitions for the chessboard.  These contain all of the necessary code to store
			the current state of the game, as well as some important data structures that are used to analyze
			the current state of the game.  Most of the functions that are used to determine the legality of
			a move are defined here; the rest are contained within chessPieceClasses.cpp.
	
	
chessBoardClass.h:	
Class declaration for the chessboard.


chessGameTree.cpp:	
Class method definitons for the chess game-tree.  This contain all of the functions that 					effectively act as the "brain" of the AI.  These functions work by starting with the current 					game-state, then simulating the most promising sequence of moves, but only so far ahead, for 					(mostly) each game-state.  
After a sequence of moves has been tested (the end of the game tree is reached), the current 					state of the simulated game is evaluated, and a score is given to this game-state.  The move 					sequence that scores the highest is the one that the AI uses to move next (it'll play the first 				move of the best move sequence).
In reality, not ALL moves are attempted, as this would take to long.  The heart of this process 				uses an algorithm call Minimax, which utilizes an optimization call Alpha-Beta pruning.  This 					optimization allows large sections of the game-tree to be skipped without impacting the quality 				of the AI's decision making.  It's complicated stuff, but the better the moves are initially 					ordered, the most of the tree gets skipped, which means the AI can think faster.


chessGameTree.h:	
Class delcaration for the chess game-tree.  Also contains various helper data structures, that 					are used to make the implementation simpler, as well as 8x8 integer arrays that are used to 					evaluate piece-dependent positional strength.
	
	
chessPieceClasses.cpp:	
Class method definitons for all of the individual chessPiece classes.  This contains all of the 				validMovement method definitions for each type and color of chess piece.  This is the simplest and least			costly test that can be performed to see if a move is legal, but it is not the only test that is needed.			Essentially, validMovement tests to see if the move being attempted on a piece is follows the most 				fundamental piece movement rules of chess.  For instance, it'll determine that a pawn moving strictly 				horizontally is illegal, but it doesn't determine if a pinned piece may be moved or not.  Various other 			tests are done to test for more complex movement rules - see chessBoardClass.cpp.
	
	
chessBoardClasses.h:	
Class declaration for all of the individuial chessPiece classes, as well as some base classes that the 				individual chess piece classes inherit from.  Extremely basic class methods are defined here, since they			require only one to a few lines to do so.


functionImplementation.cpp:	
Contains various functions that are used for the GLUT main loop (display/render functions,					keyboard/mouse IO functions, etc.).  Also contains some global variables that are used in 					various functions throughout the file.


functionPrototypes.h:	
Contains function prototypes for various functions that are defined in functionImplementation.cpp


GlobalVars.h:	
Contains some global variables that are used in various files.


main.cpp:	
Contains the main function definition that contains a call to the initial OpenGL function, as well as a call to 		the GLUT main loop.
						


