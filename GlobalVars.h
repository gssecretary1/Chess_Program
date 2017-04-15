#pragma once

#ifndef GLOBAL_VARS_
#define GLOBAL_VARS_

#include "BitmapFontClass.h"

// Debug flags for console window output blocks.
const bool DEBUG = false;
const bool AI_DEBUG = false;



// For use with keyboard input
const int ESCAPE_KEY = 27;		// ASCII value for the ESC key, used to exit program.
const int LEFT_ARROW = 75;
const int RIGHT_ARROW = 77;

// Desired window size.
const int BOARD_OFFSET = 30;
const int WINDOW_WIDTH = 512 + BOARD_OFFSET;
const int WINDOW_HEIGHT = WINDOW_WIDTH;


#endif