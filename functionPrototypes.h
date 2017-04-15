#pragma once

#ifndef FUNCTION_PROTOS_
#define FUNCTION_PROTOS_

#include <string>
#include <GL/glut.h>

// I/O Functions
void keyboardInput(unsigned char key, int x, int y);
void mouseInput(int button, int state, int x, int y);
void mouseMotion(int x, int y);
void printInstructions();

// Display Functions
char* translatePieceType(std::string c, std::string t);
void renderChessBoard();
void display();
void drawPixel(int x0, int x1, int y0, int y1);
void drawLine(int x0, int x1, int y0, int y1);

// Initialization Functions
void init(int argc, char** argv);

#endif