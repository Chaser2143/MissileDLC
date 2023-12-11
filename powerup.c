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

// // Ending x,y of plane, and the total length from origin to destination.
// static uint16_t x_dest = 0;
// static uint16_t y_dest = PLANE_HEIGHT;
// static uint16_t total_length = SCREEN_WIDTH;

// Used to track the current x,y of plane
static int16_t x_current;
static int16_t y_current;

// While flying, this tracks the current length of the flight path
// static double length = 0;

// static missile_t *missile; //Plane's missile

static bool isExploded = false; //Whether or not the plane is caught in an explosion

// static int16_t missile_launch_x = 0; //Launch coord of plane's missile

// static bool missile_launched = false; //If we launched the missile

int16_t random_x() {
    return ((rand()%300) + 5);
}
int16_t random_y() {
    return ((rand()%100) + 30);
}

// Initialize the plane state machine
// Pass in a pointer to the missile struct (the plane will only have one
// missile)
void powerup_init(){
    currentState = powerup_init_st;
    //missile = plane_missile;
    x_current = random_x();
    y_current = random_y();
    // missile.type = plane_missile;
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

// //Returns the percentage of distance the plane has traveled
// double planeGetPercentage(){
//     return (length/total_length);
// }

// //Updates the total length the plane has traveled based off the tick
// void planeUpdateLength(){
//     length = length + CONFIG_PLANE_DISTANCE_PER_TICK;
// }

// //Updates the X Coordinate of the plane
// void planeUpdateLocation(){
//     x_current = (x_origin + (planeGetPercentage() * (x_dest - x_origin)));
// }

//Draws and Erases the Plan
void draw2Plane(bool erase){
    if(erase){
        display_fillCircle(x_current+HALF_PLANE_TRIANGLE_LENGTH, y_current-UFO_TOP_HEIGHT, UFO_TOP_RADIUS, DISPLAY_BLACK);//Draw the UFO TOP
        display_fillTriangle(x_current, y_current, x_current+PLANE_TRIANGLE_LENGTH, y_current, x_current+HALF_PLANE_TRIANGLE_LENGTH, y_current-PLANE_TRIANGLE_HEIGHT, DISPLAY_BLACK);//Erase the plane
    }
    else{
        display_fillTriangle(x_current, y_current, x_current+PLANE_TRIANGLE_LENGTH, y_current, x_current+HALF_PLANE_TRIANGLE_LENGTH, y_current-PLANE_TRIANGLE_HEIGHT, DISPLAY_WHITE);//Draw the plane
        display_fillCircle(x_current+HALF_PLANE_TRIANGLE_LENGTH, y_current-UFO_TOP_HEIGHT, UFO_TOP_RADIUS, DISPLAY_GREEN);//Draw the UFO TOP
    }
}

// //Debug plane state
// void plane_debug_tick(){
//     static enum plane_st previousState;
//     static bool firstPass = true;
//     if (previousState != currentState || firstPass) {
//         firstPass = false;                // previousState will be defined, firstPass is false.
//         previousState = currentState; 
//         switch(currentState){ //State Update
//             case plane_init_st:
//                 printf(PLANE_INIT_MSG);
//                 break;
//             case plane_move_st:
//                 printf(PLANE_MOVE_MSG);
//                 break;
//             case plane_dead_st:
//                 printf(PLANE_DEAD_MSG);
//                 break;
//             default:
//                 break;
//         }
//     }
// }


// State machine tick function
void powerup_tick(){
    // if(DEBUG_FLAG){
    //     plane_debug_tick();
    // }
    switch(currentState){ //State Update
        case powerup_init_st:
            //missile_launch_x = rand() % SCREEN_WIDTH;
            // if(DEBUG_FLAG){
            //     printf("%d is the firing point for the missile\n", missile_launch_x);
            // }
            currentState = powerup_move_st;
            break;
        case powerup_move_st: //Keeping these two conditions separate for scoring purposes
            if(isExploded){ //if there is a collision
                draw2Plane(true); //Erase plane
                sound_powerup();
                resetPowerupTicks = 50;
                currentState = powerup_dead_st;
                break;
            }
            if(movePowerupTicks > moveTicks){//if we haven't reached the destination
                draw2Plane(true); //Erase plane
                currentState = powerup_dead_st;
                break;
            }
            // if((x_current <= missile_launch_x) && !missile_launched){ //If we haven't launched yet and we're at the right spot to
            //     if(DEBUG_FLAG){
            //         printf("Launching missile now!\n");
            //     }
            //     missile_init_plane(missile, x_current, y_current); //Launch off the missile
            //     missile_launched = true;
            //     break;
            // }
            break;
        case powerup_dead_st:
            x_current = 400;
            y_current = 400;
            if(resetPowerupTicks >= resetTicks){ //Reset all the stats for dead planes
                resetPowerupTicks = 0;
                movePowerupTicks = 0;
                currentState = powerup_init_st;
                //length = 0; //Reset Plane Specs
                x_current = random_x();
                y_current = random_y();
                isExploded = false;
                //missile_launched = false;
            }
            break;
        default:
            break;
    }

    switch(currentState){ //State Update
        case powerup_init_st:
            break;
        case powerup_move_st:
            draw2Plane(true); //Erase plane
            //planeUpdateLength(); //Updates flight progress
            //planeUpdateLocation(); //Update Location of plane
            draw2Plane(false);
            movePowerupTicks = movePowerupTicks + 1;
            break;
        case powerup_dead_st:
            resetPowerupTicks = resetPowerupTicks + 1; //Increment Ticks Each Pass
            break;
        default:
            break;
    }
}