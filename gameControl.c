#include <stdbool.h>
#include <stdio.h>
#include "config.h"
#include "display.h"
#include "interrupts.h"
#include "intervalTimer.h"
#include "missile.h"
#include "touchscreen.h"
#include <math.h>
#include "plane.h"
#include <stdlib.h>

missile_t missiles[CONFIG_MAX_TOTAL_MISSILES]; //Init missiles
missile_t *enemy_missiles = &(missiles[0]); //Start of enemy missiles
missile_t *player_missiles = &(missiles[CONFIG_MAX_ENEMY_MISSILES]); //Start of player missiles

#define FIRST_HALF_ENEMY_MISSILES 3
#define SECOND_HALF_ENEMY_MISSILES 7
#define FIRST_HALF_PLAYER_MISSILES 7
#define SECOND_HALF_PLAYER_MISSILES 9
#define END_PLAYER_MISSILES 11
#define PLANE_MISSILE 11

#define START_HEIGHT 0 //Start text height
#define START_WIDTH 20 //Start text width
#define SECOND_WIDTH 160 //Second Width for stats
#define TEXT_SIZE 1 //Text size
#define SHOT_TEXT "Shot: " //Shot msg
#define IMPACTED_TEXT "Impacted: " //Impacted msg

static bool first_half = true;

static uint16_t number_player_missiles_shot = 0;
static uint16_t number_enemy_missiles_impacted = 0;

//Draw the stats at the top of the screen
void drawStats(uint16_t color){
    display_setCursor(START_WIDTH, START_HEIGHT);
    display_setTextColor(color);
    display_setTextWrap(true);
    display_setTextSize(TEXT_SIZE);
    display_print(SHOT_TEXT);
    display_printlnDecimalInt(number_player_missiles_shot);

    display_setCursor(SECOND_WIDTH, START_HEIGHT);
    display_print(IMPACTED_TEXT);
    display_printlnDecimalInt(number_enemy_missiles_impacted);
}

//Compute and return the total length between 2 points, can be used for double and total length
double computeDistance(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2){
    double total_dist = sqrt((pow(y2-y1,2)) + (pow(x2-x1,2)));
    return total_dist;
}

//Detect Collision Function Here
bool detectCollision(missile_t *enemy_missile, missile_t *any_missile){
    //Checks if enemy missile is within any_missile explode radius
    double distance = computeDistance(enemy_missile->x_current, enemy_missile->y_current, any_missile->x_current, any_missile->y_current);
    if(any_missile->radius > distance){
        return true;
    }
    return false;
}

// Initialize the game control logic
// This function will initialize all missiles, stats, plane, etc.
void gameControl_init(){
    // Initialize missiles to dead
  for (uint16_t i = 0; i < CONFIG_MAX_TOTAL_MISSILES; i++){
    missile_init_dead(&missiles[i]);
  }

  #ifdef LAB8_M3
  plane_init(&missiles[PLANE_MISSILE]);//Init the plane
  #endif

  //Set background color ---MAYBE needs to be taken out
  display_fillScreen(CONFIG_BACKGROUND_COLOR);
}

// Tick the game control logic
//
// This function should tick the missiles, handle screen touches, collisions,
// and updating statistics.
void gameControl_tick(){
    if(first_half){
        //   Tick first half of enemy missiles
        for (uint16_t i = 0; i < FIRST_HALF_ENEMY_MISSILES; i++){
            missile_tick(&missiles[i]);
        } // Tick first half of player missiles
        for (uint16_t i = FIRST_HALF_PLAYER_MISSILES; i < SECOND_HALF_PLAYER_MISSILES; i++){
            missile_tick(&missiles[i]);
        }
        first_half = false;
    }
    else{
        //   Tick second half of enemy missiles
        for (uint16_t i = FIRST_HALF_ENEMY_MISSILES; i < SECOND_HALF_ENEMY_MISSILES; i++){
            missile_tick(&missiles[i]);
        } //Tick second half of player missiles
        for (uint16_t i = SECOND_HALF_PLAYER_MISSILES; i < CONFIG_MAX_TOTAL_MISSILES; i++){
            missile_tick(&missiles[i]);
        }
        first_half = true;
    }

    drawStats(DISPLAY_BLACK); //Draw stats on top of screen (Erase)
    #ifdef LAB8_M3
    plane_tick(); //Tick the plane
    #endif

    //Read enemy missiles impacted
    for(uint16_t i=0; i < CONFIG_MAX_TOTAL_MISSILES; i++){
        if(missiles[i].impacted){ //Only enemy and plane missiles are ever set to impacted, so I can count them
            missiles[i].impacted = false; //Reset this
            number_enemy_missiles_impacted++;
        }
    }

    // • If enemy missile is dead, relaunch it (call init again)
    for (uint16_t i = 0; i < CONFIG_MAX_ENEMY_MISSILES; i++)
        if (missile_is_dead(&enemy_missiles[i])) {
        missile_init_enemy(&enemy_missiles[i]);
    }

    // • If touchscreen touched, launch player missile (if one is available)
    // Check for dead player missiles and re-initialize
    if(touchscreen_get_status() == TOUCHSCREEN_RELEASED){

        for (uint16_t i = 0; i < CONFIG_MAX_PLAYER_MISSILES; i++){
            if (missile_is_dead(&player_missiles[i])) {
                missile_init_player(&player_missiles[i], touchscreen_get_location().x, touchscreen_get_location().y);
                touchscreen_ack_touch();
                number_player_missiles_shot++; //Increment our count
                break; //Only do it for one missile
            }
        }
        touchscreen_ack_touch();
    }

    // • Detect collisions
    // // Check if missile i should explode, caused by an exploding missile j
    for (uint16_t i = 0; i < CONFIG_MAX_ENEMY_MISSILES; i++){
        for (uint16_t j = 0; j < CONFIG_MAX_TOTAL_MISSILES; j++){
            if(!missile_is_flying(&missiles[i])){ //If flying, skip
                continue;
            }
            if(!missile_is_exploding(&missiles[j])){ //If exploding, skip
                continue;
            }

            if(detectCollision(&missiles[i], &missiles[j])){ //Overall missile collision detection
                missile_trigger_explosion(&missiles[i]);
            }

            if(detectCollision(&missiles[PLANE_MISSILE], &missiles[j])){ //Plane missile detection
                missile_trigger_explosion(&missiles[PLANE_MISSILE]);
            }
        }
    }
    #ifdef LAB8_M3
    display_point_t planeCoords = plane_getXY(); //Gets plane coords
    //Detect Plane Collision
    for(uint16_t i=0; i < CONFIG_MAX_TOTAL_MISSILES; i++){
        if(missiles[i].radius > computeDistance(planeCoords.x, planeCoords.y, missiles[i].x_current, missiles[i].y_current)){
            plane_explode(); //Set the plane to explode and move on
            break;
        }
    }
    #endif
    //Stat Counter Section
    drawStats(DISPLAY_WHITE); //Draw Stats
}
