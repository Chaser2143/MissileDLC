#ifndef POWERUP
#define POWERUP

#include "display.h"
#include "missile.h"

// Initialize the plane state machine
// Pass in a pointer to the missile struct (the plane will only have one
// missile)
void powerup_init();

// State machine tick function
void powerup_tick();

// Trigger the plane to expode
void powerup_explode();

// Get the XY location of the plane
display_point_t powerup_getXY();

#endif /* PLANE */
