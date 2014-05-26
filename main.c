/*
Based on ChibiOS/testhal/STM32F0xx/PWM-ICU,

PA2(TX) and PA3(RX) are routed to USART2 38400-8-N-1.
*/

#include "ch.h"
#include "hal.h"
#include "chprintf.h"

/*
 * This is a periodic thread that does absolutely nothing except flashing
 * a LED.
 */
static WORKING_AREA(blinker_wa, 128);
static msg_t blinker(void *arg) {

  (void)arg;
  chRegSetThreadName("blinker");
  while (TRUE) {
    palSetPad(GPIOC, GPIOC_LED4);
    chThdSleepMilliseconds(500);
    palClearPad(GPIOC, GPIOC_LED4);
    chThdSleepMilliseconds(500);
  }
}

//trig thread
static WORKING_AREA(trig_wa, 128);
static msg_t trig(void *arg) {

  (void)arg;
  chRegSetThreadName("trig");
  while (TRUE) {
    palSetPad(GPIOC, 11);         /* set 11. */
    chThdSleepMilliseconds(20);
    palClearPad(GPIOC, 11);       /* clear 11. */
    chThdSleepMilliseconds(20);
  }
}


icucnt_t last_width, last_period;
static void icuwidthcb(ICUDriver *icup) {

  palSetPad(GPIOC, GPIOC_LED3);
  last_width = icuGetWidth(icup);
}

static void icuperiodcb(ICUDriver *icup) {

  palClearPad(GPIOC, GPIOC_LED3);
  last_period = icuGetPeriod(icup);
}

static void icuoverflowcb(ICUDriver *icup) {

  (void)icup;
}

static ICUConfig icucfg = {
  ICU_INPUT_ACTIVE_HIGH,
  10000,                                    /* 10kHz ICU clock frequency.   */
  icuwidthcb,
  icuperiodcb,
  icuoverflowcb,
  ICU_CHANNEL_1,
  0
};


/*
 * Application entry point.
 */
int main(void) {
  unsigned i;

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();

  icuStart(&ICUD3, &icucfg);
  palSetPadMode(GPIOA, 6, PAL_MODE_ALTERNATE(1));
  icuEnable(&ICUD3);

  palSetPadMode(GPIOC, 11, PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST);

  sdStart( &SD2 , NULL );
  palSetPadMode(GPIOA, 2, PAL_MODE_ALTERNATE(1));      /* USART1 TX.       */
  palSetPadMode(GPIOA, 3, PAL_MODE_ALTERNATE(1));      /* USART1 RX.       */

  /*
   * Starting the blinker thread.
   */
  chThdCreateStatic(blinker_wa, sizeof(blinker_wa),
                    NORMALPRIO-1, blinker, NULL);
  /*
   * Starting the trig thread.
   */
  chThdCreateStatic(trig_wa, sizeof(trig_wa),
                    NORMALPRIO-1, trig, NULL);

  while (TRUE) {

    chThdSleepMilliseconds(50);
    chprintf((BaseSequentialStream *) &SD2, "echo %d\n\r", last_width);
    chThdSleepMilliseconds(50);

  }
  return 0;
}
