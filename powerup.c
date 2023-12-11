#include "display.h"
#include "missile.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "config.h"
#include <math.h>
#include <stdlib.h>
#include "sound.h"

#define SCREEN_WIDTH 320 //Display Width
#define PLANE_HEIGHT 70 //Height of plane

#define DEBUG_FLAG true

#define TEN_SECONDS 10
#define TWO_SECONDS 2

#define PLANE_TRIANGLE_LENGTH 20
#define HALF_PLANE_TRIANGLE_LENGTH 10
#define PLANE_TRIANGLE_HEIGHT 3
#define UFO_TOP_HEIGHT 3
#define UFO_TOP_RADIUS 2


#define PLANE_INIT_MSG "In Plane Init State \n"
#define PLANE_MOVE_MSG "In Plane Move State \n"
#define PLANE_DEAD_MSG "In Plane Dead State \n"

//States
enum powerup_st {
    powerup_init_st, //Init_st
    powerup_move_st, //Moving
    powerup_dead_st, //Dead
};

static int32_t resetTicks = 0; //Counts to ticks we need to reset
static int32_t resetPowerupTicks = 0; //Counts up to the number of ticks it needs to reset

static int32_t moveTicks = 0; //Counts to ticks we need to move powerup
static int32_t movePowerupTicks = 0;

// Current state 
static int32_t currentState;

// Starting x,y of place
static uint16_t x_origin = SCREEN_WIDTH;
static uint16_t y_origin = PLANE_HEIGHT;

// Used to track the current x,y of plane
static int16_t x_current;
static int16_t y_current;

static bool isExploded = false; //Whether or not the plane is caught in an explosion

int16_t random_x() {
    return ((rand()%300) + 5);
}
int16_t random_y() {
    return ((rand()%100) + 30);
}

// Initialize the plane state machine

void powerup_init(){
    currentState = powerup_init_st;
    x_current = random_x();
    y_current = random_y();
    resetPowerupTicks = 0;
    resetTicks = TEN_SECONDS/CONFIG_GAME_TIMER_PERIOD;
    movePowerupTicks = 0;
    moveTicks = TWO_SECONDS/CONFIG_GAME_TIMER_PERIOD;
}

// Trigger the plane to expode
void powerup_explode(){
    isExploded = true;
}

// Get the XY location of the plane
display_point_t powerup_getXY(){
    display_point_t newDisplayPoint;
    newDisplayPoint.x = x_current;
    newDisplayPoint.y = y_current;
    return newDisplayPoint;
}

//Draws and Erases the Powerup
void drawPowerup(bool erase){
    if(erase){
        display_fillTriangle(x_current+12, y_current, x_current-12, y_current, x_current, y_current+10, DISPLAY_BLACK);
        display_fillTriangle(x_current+12, y_current, x_current-12, y_current, x_current, y_current-10, DISPLAY_BLACK);
    }
    else{
        display_fillTriangle(x_current+12, y_current, x_current-12, y_current, x_current, y_current+10, rand()%0xFFFF);
        display_fillTriangle(x_current+12, y_current, x_current-12, y_current, x_current, y_current-10, rand()%0xFFFF);
    }
}

// State machine tick function
void powerup_tick(){
    switch(currentState){ //State Update
        case powerup_init_st:
            currentState = powerup_move_st;
            break;
        case powerup_move_st: //Keeping these two conditions separate for scoring purposes
            if(isExploded){ //if there is a collision
                drawPowerup(true); //Erase plane
                sound_powerup();
                resetPowerupTicks = 50;
                currentState = powerup_dead_st;
                break;
            }
            if(movePowerupTicks > moveTicks){//if we haven't reached the destination
                drawPowerup(true); //Erase plane
                currentState = powerup_dead_st;
                break;
            }
            break;
        case powerup_dead_st:
            x_current = 400;
            y_current = 400;
            if(resetPowerupTicks >= resetTicks){ //Reset all the stats for dead planes
                resetPowerupTicks = 0;
                movePowerupTicks = 0;
                currentState = powerup_init_st;
                x_current = random_x();
                y_current = random_y();
                isExploded = false;            
            }
            break;
        default:
            break;
    }

    switch(currentState){ //State Update
        case powerup_init_st:
            break;
        case powerup_move_st:
            drawPowerup(true); //Erase plane
            drawPowerup(false);
            movePowerupTicks = movePowerupTicks + 1;
            break;
        case powerup_dead_st:
            resetPowerupTicks = resetPowerupTicks + 1; //Increment Ticks Each Pass
            break;
        default:
            break;
    }
}