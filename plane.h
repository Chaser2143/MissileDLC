#ifndef PLANE
#define PLANE

#include "display.h"
#include "missile.h"

// Initialize the plane state machine
// Pass in a pointer to the missile struct (the plane will only have one
// missile)
void plane_init(missile_t *plane_missile);

// State machine tick function
void plane_tick();

// Trigger the plane to expode
void plane_explode();

// Get the XY location of the plane
display_point_t plane_getXY();

#endif /* PLANE */
