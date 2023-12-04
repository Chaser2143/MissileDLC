#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "config.h"
#include <math.h>
#include <stdlib.h>
#include "missile.h"
#include "display.h"

#define SCREEN_WIDTH 320 //Display Width
#define SCREEN_HEIGHT 240 //Display Height
// Launch site coordinates
#define LAUNCH_SITE_ONE 80
#define LAUNCH_SITE_TWO 160
#define LAUNCH_SITE_THREE 240

#define ENEMY_MISSILE_START_HEIGHT 30 //Start height of enemy missiles

#define TOP_FOURTH 4 //Used for randomizing enemy missile start height

#define DEBUG_FLAG false //True if we want debug print statements

// All printed messages for states are provided here.
#define init_st_msg "Init State\n" //Init_st
#define move_st_msg "Moving State\n" //Moving
#define explode_grow_st_msg "Grow State\n" //ExplodeGrow
#define explode_shrink_st_msg "Shrink State\n" //ExplodeShrink
#define dead_st_msg "Dead State\n" //Dead

#define DOUBLE_SPEED 2 //Used for doubling the speed as we only tick our missiles half as often
#define TRIPLE_SPEED 3

//States
enum missile_st {
    init_st, //Init_st
    move_st, //Moving
    explode_grow_st, //ExplodeGrow
    explode_shrink_st, //ExplodeShrink
    dead_st, //Dead
};

// Return whether the given missile is dead.
bool missile_is_dead(missile_t *missile){
    if(missile->currentState == dead_st){
        return true;
    }
    return false;
}

// Return whether the given missile is exploding.  This is needed when detecting
// whether a missile hits another exploding missile.
bool missile_is_exploding(missile_t *missile){
    if((missile->currentState == explode_grow_st) || (missile->currentState == explode_shrink_st)){
        return true;
    }
    return false;
}

// Return whether the given missile is flying.
bool missile_is_flying(missile_t *missile){
    if(missile->currentState == move_st){
        return true;
    }
    return false;
}

// Used to indicate that a flying missile should be detonated.  This occurs when
// an enemy or plane missile is located within an explosion zone.
void missile_trigger_explosion(missile_t *missile){
    missile->explode_me = true;
}

//Compute and return the total length between 2 points, can be used for double and total length
double computeLength(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2){
    double total_dist = sqrt((pow(y2-y1,2)) + (pow(x2-x1,2)));
    return total_dist;
}

void init_general(missile_t *missile){
    //General initialization steps needed for every missile type
    missile->length=0;
    missile->explode_me = false;
    missile->total_length = computeLength(missile->x_origin,missile->y_origin,missile->x_dest,missile->y_dest); //Computes total end length
    missile->x_current = missile->x_origin;
    missile->y_current = missile->y_origin;
    missile->impacted = false;
}

////////// State Machine INIT Functions //////////
// Unlike most state machines that have a single `init` function, our missile
// will have different initializers depending on the missile type.

// Initialize the missile as a dead missile.  This is useful at the start of the
// game to ensure that player and plane missiles aren't moving before they
// should.
void missile_init_dead(missile_t *missile){
    missile->currentState = dead_st;
    // init_general(missile);
}

// Initialize the missile as an enemy missile.  This will randomly choose the
// origin and destination of the missile.  The origin should be somewhere near
// the top of the screen, and the destination should be the very bottom of the
// screen.
void missile_init_enemy(missile_t *missile){
    missile->type = MISSILE_TYPE_ENEMY;
    // Set x,y origin to random place near the top of the screen (top quarter? – you choose!)
    // missile->y_origin = ENEMY_MISSILE_START_HEIGHT;
    missile->y_origin = rand() % (SCREEN_HEIGHT/4);
    missile->x_origin = rand() % SCREEN_WIDTH;
    // Set x,y destination to random location along the bottom of the screen
    missile->y_dest = SCREEN_HEIGHT;
    missile->x_dest = rand() % SCREEN_WIDTH;
    // Set missile speed
    float a = 1.5;
    float speed_multiplier = (((float)rand()/(float)(RAND_MAX)) * a);
    missile->speed = speed_multiplier;
    // Set current state
    missile->currentState = init_st;
    init_general(missile); //General Init
}

//Return the closest launch site x value
uint16_t getClosestLaunchSite(uint16_t x_dest){
    uint16_t launchSiteOne = LAUNCH_SITE_ONE;
    uint16_t launchSiteTwo = LAUNCH_SITE_TWO;
    uint16_t launchSiteThree = LAUNCH_SITE_THREE;
    uint16_t difference = SCREEN_WIDTH;
    uint16_t min = SCREEN_WIDTH; //Set default min bigger than all choices

    //Check which of the 3 launch sites it is closest to and return it
    if(abs(launchSiteThree - x_dest) <= difference){
        difference = abs(launchSiteThree - x_dest);
        min = launchSiteThree;
    }
    if(abs(launchSiteTwo - x_dest) <= difference){
        difference = abs(launchSiteTwo - x_dest);
        min = launchSiteTwo;
    }
    if(abs(launchSiteOne - x_dest) <= difference){
        difference = abs(launchSiteOne - x_dest);
        min = launchSiteOne;
    }
    return min;
}

// Initialize the missile as a player missile.  This function takes an (x, y)
// destination of the missile (where the user touched on the touchscreen).  The
// origin should be the closest "firing location" to the destination (there are
// three firing locations evenly spaced along the bottom of the screen).
void missile_init_player(missile_t *missile, uint16_t x_dest, uint16_t y_dest){
    missile->type = MISSILE_TYPE_PLAYER;
    // Set x,y origin to closest missile launch site
    missile->x_origin = getClosestLaunchSite(x_dest);
    missile->y_origin = SCREEN_HEIGHT;
    // x,y destination is provided (touched location)
    missile->x_dest = x_dest;
    missile->y_dest = y_dest;
    // Set current state
    missile->currentState = init_st;
    init_general(missile);
}

//Returns the division of the current length/total length
double getPercentage(missile_t *missile){
    double percent = ((missile->length)/(missile->total_length));
    return percent;
}

//Update the length of the missile when ticked from SM depending on its type
void updateLength(missile_t *missile){
    if (DEBUG_FLAG){
        printf("Before Update : %f\n", missile->length);
        printf("Previous percentage : %f\n", getPercentage(missile));
        printf("Previous Total Length : %d\n", missile->total_length);
    }
    switch(missile->type){ //Increments missile length
        case MISSILE_TYPE_PLAYER:
            missile->length = ((missile->length) + (CONFIG_PLAYER_MISSILE_DISTANCE_PER_TICK * DOUBLE_SPEED));
            break;
        case MISSILE_TYPE_ENEMY:
            missile->length = (((missile->length) + (missile->speed + CONFIG_ENEMY_MISSILE_DISTANCE_PER_TICK) * DOUBLE_SPEED));
            break;
        case MISSILE_TYPE_PLANE:
            missile->length = ((missile->length) + (CONFIG_ENEMY_MISSILE_DISTANCE_PER_TICK * DOUBLE_SPEED));
            break;
    }
    if (DEBUG_FLAG){
        printf("After Update : %f\n", missile->length);
        printf("Post percentage : %f\n", getPercentage(missile));
        printf("Post Total Length : %d\n", missile->total_length);
    }
}

//Draw Moving Line, erases if alive is false
void drawMovingLine(missile_t *missile, bool alive){
    if(alive){
        if(missile->type == MISSILE_TYPE_PLAYER){ //Player Missile
            display_drawLine(missile->x_origin, missile->y_origin, missile->x_current, missile->y_current, DISPLAY_GREEN);
        }
        else if (missile->type == MISSILE_TYPE_ENEMY){
            display_drawLine(missile->x_origin, missile->y_origin, missile->x_current, missile->y_current, DISPLAY_RED);
        }
        else if (missile->type == MISSILE_TYPE_PLANE){
            display_drawLine(missile->x_origin, missile->y_origin, missile->x_current, missile->y_current, DISPLAY_WHITE);
        }
    }
    else{ //Dead line
        display_drawLine(missile->x_origin, missile->y_origin, missile->x_current, missile->y_current, DISPLAY_BLACK); //Erase old
    }
}

//Calculate new x and y given percentage
void updateLocation(missile_t *missile, double percentage){
    //Update X
    missile->x_current = (missile->x_origin + (percentage * (missile->x_dest - missile->x_origin)));
    //Update Y
    missile->y_current = (missile->y_origin + (percentage * (missile->y_dest - missile->y_origin)));
}

//Increase radius for growing missile explosion
void increaseRadius(missile_t *missile){
    missile->radius = (missile->radius + (CONFIG_EXPLOSION_RADIUS_CHANGE_PER_TICK * TRIPLE_SPEED));
}

//Decrease radius for shrinking missile explosion
void decreaseRadius(missile_t *missile){
    missile->radius = (missile->radius - (CONFIG_EXPLOSION_RADIUS_CHANGE_PER_TICK * DOUBLE_SPEED));
}

//Draw a circle if true, else erase a circle
void drawCircle(missile_t *missile, bool draw){
    if(draw){
        if(missile->type == MISSILE_TYPE_PLAYER){
            display_fillCircle(missile->x_current, missile->y_current, missile->radius, DISPLAY_GREEN);
        }
        else if(missile->type == MISSILE_TYPE_PLANE){
            display_fillCircle(missile->x_current, missile->y_current, missile->radius, DISPLAY_WHITE);
        }
        else{
            display_fillCircle(missile->x_current, missile->y_current, missile->radius, DISPLAY_RED);
        }
    }
    else{ //erase case
        display_fillCircle(missile->x_current, missile->y_current, missile->radius, DISPLAY_BLACK);
    }
}

// This is a debug state print routine. It will print the names of the states each
// time tick() is called. It only prints states if they are different than the
// previous state.
void debugStatePrintMissiles(uint8_t currentState) {
  static enum missile_st previousState;
  static bool firstPass = true;
  // Only print the message if:
  // 1. This the first pass and the value for previousState is unknown.
  // 2. previousState != currentState - this prevents reprinting the same state name over and over.
  if (previousState != currentState || firstPass) {
    firstPass = false;                // previousState will be defined, firstPass is false.
    previousState = currentState;     // keep track of the last state that you were in.
    switch(currentState) {            // This prints messages based upon the state that you were in.
      case init_st:
            printf(init_st_msg);
            break;
        case move_st:
            printf(move_st_msg);
            break;
        case explode_grow_st:
            printf(explode_grow_st_msg);
            break;
        case explode_shrink_st:
            printf(explode_shrink_st_msg);
            break;
        case dead_st:
            printf(dead_st_msg);
            break;
        default:
        // print an error message here.
            break;
     }
  }
}


// Initialize the missile as a plane missile.  This function takes an (x, y)
// location of the plane which will be used as the origin.  The destination can
// be randomly chosed along the bottom of the screen.
void missile_init_plane(missile_t *missile, int16_t plane_x, int16_t plane_y){
    missile->type = MISSILE_TYPE_PLANE;
    //x,y origin provided (plane location)
    missile->x_origin = plane_x;
    missile->y_origin = plane_y;
    //x,y destination chosen randomly along the bottom
    missile->y_dest = SCREEN_HEIGHT;
    missile->x_dest = rand() % SCREEN_WIDTH;
    //Set current state
    missile->currentState = init_st;
    init_general(missile);
}

////////// State Machine TICK Function //////////
void missile_tick(missile_t *missile){
    if(DEBUG_FLAG){
        debugStatePrintMissiles(missile->currentState); //Debug SM
    }
    // Perform state update first.
    switch(missile->currentState) { //Mealy
        case init_st:
            missile->currentState = move_st; //Move to next state
            break;
        case move_st:
            if(missile->explode_me == true){ //If we explode mid path
                missile->currentState = explode_grow_st; //Go into exploding state
                drawMovingLine(missile, false); //Erase path
                break;
            }
            if(getPercentage(missile) >= 1){ //Did it reach its destination?
                if((missile->type == MISSILE_TYPE_ENEMY) || (missile->type == MISSILE_TYPE_PLANE)){ //If enemy, it reached its end and should die
                    missile->currentState = explode_grow_st;//explode on impact
                    drawMovingLine(missile, false); //Erase path
                    missile->impacted = true; //Used for counting number of impacted missiles
                }
                else{ //Player missiles should explode at end
                    missile->currentState = explode_grow_st;//explode
                    drawMovingLine(missile, false); //Erase path
                }
            }
            break;
        case explode_grow_st:
            if(missile->radius >= CONFIG_EXPLOSION_MAX_RADIUS){ //Max radius reached
                missile->currentState = explode_shrink_st; //Start shrinking
            }
            break;
        case explode_shrink_st:
            if(missile->radius <= 0){ //Explosion gone, move to dead state
                missile->currentState = dead_st;
                drawCircle(missile, false); //Erase leftovers
            }
            break;
        case dead_st:
            break;
        default:
        // print an error message here.
            break;
    }
    
    // Perform state action next.
    switch(missile->currentState) { //Moore
        case init_st:
            break;
        case move_st:
            drawMovingLine(missile, false); //Erase old line
            updateLength(missile);//Update length
            updateLocation(missile, getPercentage(missile));//Calculate new x and y
            drawMovingLine(missile, true); //Draw new line
            break;
        case explode_grow_st:
            increaseRadius(missile); //Increase explosion radius
            drawCircle(missile, true); //Draw new circle
            break;
        case explode_shrink_st:
            drawCircle(missile, false); //Erase old circle
            decreaseRadius(missile); //Decrease radius of explosion
            drawCircle(missile, true); //Draw new circle
            break;
        case dead_st:
            break;
        default:
        // print an error message here.
            break;
    }  
}
