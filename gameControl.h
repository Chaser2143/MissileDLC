#ifndef GAMECONTROL
#define GAMECONTROL

#include <stdbool.h>

// Initialize the game control logic
// This function will initialize all missiles, stats, plane, etc.
void gameControl_init();

// Tick the game control logic
//
// This function should tick the missiles, handle screen touches, collisions,
// and updating statistics.
void gameControl_tick();

//Returns a bool of whether or not the game is over
bool getGameStatus();

bool didYouWin();

#endif /* GAMECONTROL */
