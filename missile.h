#ifndef MISSILE
#define MISSILE

#include <stdbool.h>
#include <stdint.h>

/* The same missile structure will be used for all missiles in the game,
so this enum is used to identify the type of missile */
typedef enum {
  MISSILE_TYPE_PLAYER,
  MISSILE_TYPE_ENEMY,
  MISSILE_TYPE_PLANE
} missile_type_t;

/* This struct contains all information about a missile */
typedef struct {

  // Missle type (player, enemy, enemy plane)
  missile_type_t type;

  // Current state (the 'enum' will be defined in your missile.c file, so it's
  // just declared as an integer type here)
  int32_t currentState;

  // Starting x,y of missile
  uint16_t x_origin;
  uint16_t y_origin;

  // Ending x,y of missile, and the total length from origin to destination.
  uint16_t x_dest;
  uint16_t y_dest;
  uint16_t total_length;

  // Used to track the current x,y of missile
  int16_t x_current;
  int16_t y_current;

  // While flying, this tracks the current length of the flight path
  double length;

  // While flying, this flag is used to indicate the missile should be detonated
  bool explode_me;

  // While exploding, this tracks the current radius
  double radius;

  // Used for game statistics, this tracks whether the missile impacted the
  // ground.
  bool impacted;

} missile_t;

////////// State Machine INIT Functions //////////
// Unlike most state machines that have a single `init` function, our missile
// will have different initializers depending on the missile type.

// Initialize the missile as a dead missile.  This is useful at the start of the
// game to ensure that player and plane missiles aren't moving before they
// should.
void missile_init_dead(missile_t *missile);

// Initialize the missile as an enemy missile.  This will randomly choose the
// origin and destination of the missile.  The origin should be somewhere near
// the top of the screen, and the destination should be the very bottom of the
// screen.
void missile_init_enemy(missile_t *missile);

// Initialize the missile as a player missile.  This function takes an (x, y)
// destination of the missile (where the user touched on the touchscreen).  The
// origin should be the closest "firing location" to the destination (there are
// three firing locations evenly spaced along the bottom of the screen).
void missile_init_player(missile_t *missile, uint16_t x_dest, uint16_t y_dest);

// Initialize the missile as a plane missile.  This function takes an (x, y)
// location of the plane which will be used as the origin.  The destination can
// be randomly chosed along the bottom of the screen.
void missile_init_plane(missile_t *missile, int16_t plane_x, int16_t plane_y);

////////// State Machine TICK Function //////////
void missile_tick(missile_t *missile);

// Return whether the given missile is dead.
bool missile_is_dead(missile_t *missile);

// Return whether the given missile is exploding.  This is needed when detecting
// whether a missile hits another exploding missile.
bool missile_is_exploding(missile_t *missile);

// Return whether the given missile is flying.
bool missile_is_flying(missile_t *missile);

// Used to indicate that a flying missile should be detonated.  This occurs when
// an enemy or plane missile is located within an explosion zone.
void missile_trigger_explosion(missile_t *missile);

#endif /* MISSILE */
