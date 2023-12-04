#include <stdio.h>

#include "config.h"
#include "display.h"
#include "interrupts.h"
#include "intervalTimer.h"
#include "missile.h"

missile_t missiles[CONFIG_MAX_TOTAL_MISSILES];
missile_t *enemy_missiles = &(missiles[0]);
missile_t *player_missiles = &(missiles[CONFIG_MAX_ENEMY_MISSILES]);

// Interrupt Function for calling FSM ticks
void isr() {
  intervalTimer_ackInterrupt(INTERVAL_TIMER_0);

  // Check for dead enemy missiles and re-initialize
  for (uint16_t i = 0; i < CONFIG_MAX_ENEMY_MISSILES; i++)
    if (missile_is_dead(&enemy_missiles[i])) {
      missile_init_enemy(&enemy_missiles[i]);
    }

  // Check for dead player missiles and re-initialize
  for (uint16_t i = 0; i < CONFIG_MAX_PLAYER_MISSILES; i++)
    if (missile_is_dead(&player_missiles[i])) {
      missile_init_player(&player_missiles[i], rand() % DISPLAY_WIDTH,
                          rand() % DISPLAY_HEIGHT);
    }

  // Tick all missiles
  for (uint16_t i = 0; i < CONFIG_MAX_TOTAL_MISSILES; i++) {
    missile_tick(&missiles[i]);
  }
}

// Milestone 1 test application
int main() {
  display_init();
  display_fillScreen(CONFIG_BACKGROUND_COLOR);

  // Initialize timer interrupts
  interrupts_init();
  interrupts_register(INTERVAL_TIMER_0_INTERRUPT_IRQ, isr);
  interrupts_irq_enable(INTERVAL_TIMER_0_INTERRUPT_IRQ);
  interrupts_irq_enable(INTERVAL_TIMER_1_INTERRUPT_IRQ);

  intervalTimer_initCountDown(INTERVAL_TIMER_0, CONFIG_GAME_TIMER_PERIOD);
  intervalTimer_enableInterrupt(INTERVAL_TIMER_0);
  intervalTimer_start(INTERVAL_TIMER_0);

  // Initialize missiles
  for (uint16_t i = 0; i < CONFIG_MAX_TOTAL_MISSILES; i++)
    missile_init_dead(&missiles[i]);

  // Main game loop
  while (1)
    ;
}