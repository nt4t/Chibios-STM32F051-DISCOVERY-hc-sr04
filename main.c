/*
Based on ChibiOS/testhal/STM32F0xx/PWM-ICU,

PA2(TX) and PA3(RX) are routed to USART2 38400-8-N-1.
*/

#include "ch.h"
#include "hal.h"
#include "chprintf.h"

#define MB_SIZE 2

typedef struct {
    int junk;
    int thread;
    unsigned long cnt;
} msgLED;

static char buffer[sizeof(msgLED)*MB_SIZE];

//MemoryPool pool;

static MEMORYPOOL_DECL(pool, sizeof(msgLED)*MB_SIZE, NULL);
static MAILBOX_DECL(mbox, buffer, sizeof(buffer));

// align data, this depends on your compiler. this works for GCC
msgLED data[MB_SIZE] __attribute__((aligned(sizeof(stkalign_t))));


/*
 * This is a periodic thread that does absolutely nothing except flashing
 * a LED.
 */
static WORKING_AREA(blinker_wa, 128);
static msg_t blinker(void *arg) {
//  (void)arg;
msg_t msg1;
msgLED msg2;
    
  chRegSetThreadName("blinker");

while (TRUE) {
    // Wait on a message from the working threads to display
    msg1 = chMBFetch(&mbox, (msg_t *)&msg2, TIME_INFINITE);

    if (msg1 == RDY_OK) {
	// If we get here, we have a message
	if (msg2.thread == 1) {
//	    chprintf((BaseSequentialStream *) &SD2, "echo1 %d\n\r", msg2.cnt);
	    palSetPad(GPIOC, GPIOC_LED3);
	    chThdSleepMilliseconds(msg2.cnt * 1);
	    palClearPad(GPIOC, GPIOC_LED3);
	    chThdSleepMilliseconds(msg2.cnt * 1);
	}
	if (msg2.thread == 2) {
	    chprintf((BaseSequentialStream *) &SD2, "echo2 %d\n\r", msg2.cnt);
	}
	} else {
	    chprintf((BaseSequentialStream *) &SD2, "Fetch status %d\n\r", msg1);
    }
// Now we have to free the memory pool object
    chPoolFree(&pool, (void *)&msg2);
}
return 0;
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

  palSetPad(GPIOC, GPIOC_LED4);
  last_width = icuGetWidth(icup);

}

static void icuperiodcb(ICUDriver *icup) {

  palClearPad(GPIOC, GPIOC_LED4);
  last_period = icuGetPeriod(icup);
}

static void icuoverflowcb(ICUDriver *icup) {

  (void)icup;
}

static ICUConfig icucfg = {
  ICU_INPUT_ACTIVE_HIGH,
  50000,                                    /* 10kHz ICU clock frequency.   */
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

  //Initialize the mailbox
  chMBInit(&mbox, (msg_t *)buffer, MB_SIZE);
  chMBReset(&mbox);
  chThdSleepMilliseconds(2000);

  msg_t msg1;
  msgLED msg, msg2;

  chMBPost(&mbox, (msg_t)&msg, TIME_INFINITE);
  chMBPost(&mbox, (msg_t)&msg2, TIME_INFINITE);

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

    //grab a memory pool item
    msgLED *msg = (msgLED *)chPoolAlloc(&pool);
    if (msg != NULL) {
	msg->thread = 1;
	msg->cnt = last_width;
    // Here we'll send something to the blinker thread
//    chMBPost(&mbox, (msg_t)msg, TIME_IMMEDIATE);
	chMBPostAhead(&mbox, (msg_t)msg, TIME_IMMEDIATE);
    }
    chThdSleepMilliseconds(20);
  }
  return 0;
}
