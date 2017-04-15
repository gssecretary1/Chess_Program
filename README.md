# Chess_Program
Chess program that was created for Computer Graphics (CS3600) and Artificial Intelligence (CS4480).

I made a basic chess program for my Artificial Intelligence course (CS4480). The program incorporates most of the 
rules of chess (excluding some rules related to draw; e.g., draw through perpetual check). The AI component of the 
application is fairly basic, but offered a good introduction into chess AI. The computer player utilizes a variation 
of the Minimax algorithm, Negamax with alpha-beta pruning, which is built on top of a limited-depth-first-search 
framework. In layman's terms, the AI evaluates a set of move sequences, starting with the most promising moves first 
while ruling out sequences that are guaranteed to be bad.

The program uses a graphical representation of the chessboard. The chess pieces and board markers are rendered from 
a bitmap file that was created from a font file (https://www.chess.com/download/view/fritz-fonts) using Codehead's 
Bitmap Font Generator (CBFG) (http://www.codehead.co.uk/cbfg/). The code to render the chess pieces and the chessboard's 
rank and file markers come directly from the CBFG's help files and uses OpenGL - no modifications were made. 
Other than the chess pieces and markers, the rest of the graphical portion of the program uses the OpenGL GLUT library.
