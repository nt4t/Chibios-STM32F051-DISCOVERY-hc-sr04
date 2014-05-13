/*
Based on ChibiOS/testhal/STM32F0xx/SPI,

PA2(TX) and PA3(RX) are routed to USART2 38400-8-N-1.
*/

#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "lis302dl.h"

/*
 * Low speed SPI configuration (140.625kHz, CPHA=0, CPOL=0, MSb first).
 */
static const SPIConfig ls_spicfg = {
  NULL,
  GPIOB,
  12,
  SPI_CR1_BR_2 | SPI_CR1_BR_1,
  SPI_CR2_DS_2 | SPI_CR2_DS_1 | SPI_CR2_DS_0
};

/*
 * SPI TX and RX buffers.
 */
static uint8_t txbuf[512];
static uint8_t rxbuf[512];

/*
 * SPI bus contender 2.
 */
static WORKING_AREA(spi_thread_2_wa, 256);
static msg_t spi_thread_2(void *p) {

  (void)p;
  chRegSetThreadName("SPI thread 2");
  while (TRUE) {
    spiAcquireBus(&SPID2);              /* Acquire ownership of the bus.    */
    palClearPad(GPIOC, GPIOC_LED4);     /* LED OFF.                         */
    spiStart(&SPID2, &ls_spicfg);       /* Setup transfer parameters.       */
    spiSelect(&SPID2);                  /* Slave Select assertion.          */
    spiExchange(&SPID2, 512,
                txbuf, rxbuf);          /* Atomic transfer operations.      */
    spiUnselect(&SPID2);                /* Slave Select de-assertion.       */
    spiReleaseBus(&SPID2);              /* Ownership release.               */
  }
  return 0;
}
/*
 * This is a periodic thread that does absolutely nothing except flashing
 * a LED.
 */
static WORKING_AREA(blinker_wa, 128);
static msg_t blinker(void *arg) {

  (void)arg;
  chRegSetThreadName("blinker");
  while (TRUE) {
    palSetPad(GPIOC, GPIOC_LED3);
    chThdSleepMilliseconds(500);
    palClearPad(GPIOC, GPIOC_LED3);
    chThdSleepMilliseconds(500);
  }
}

static uint8_t readByteSPI(uint8_t reg)
{
	char txbuf[2] = {0x80 | reg, 0xFF};
	char rxbuf[2];
	spiSelect(&SPID2);
	spiExchange(&SPID2, 2, txbuf, rxbuf);
	spiUnselect(&SPID2);
	return rxbuf[1];
}
static uint8_t writeByteSPI(uint8_t reg, uint8_t val)
{
	char txbuf[2] = {reg, val};
	char rxbuf[2];
	spiSelect(&SPID2);
	spiExchange(&SPID2, 2, txbuf, rxbuf);
	spiUnselect(&SPID2);
	return rxbuf[1];
}

static void initGyro(void)
{
    /* see the L3GD20 Datasheet */
    writeByteSPI(0x20, 0xcF);
}

static uint8_t readGyro(float* data)
{
    /* read from L3GD20 registers and assemble data */
    /* 0xc0 sets read and address increment */
/*    char txbuf[8] = {0xc0 | 0x27, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    char rxbuf[8];
    spiSelect(&SPID1);
    spiExchange(&SPID1, 8, txbuf, rxbuf);
    spiUnselect(&SPID1);
    if (rxbuf[1] & 0x7) {
        int16_t val_x = (rxbuf[3] << 8) | rxbuf[2];
        int16_t val_y = (rxbuf[5] << 8) | rxbuf[4];
        int16_t val_z = (rxbuf[7] << 8) | rxbuf[6];
        data[0] = (((float)val_x) * mdps_per_digit)/1000.0;
        data[1] = (((float)val_y) * mdps_per_digit)/1000.0;
        data[2] = (((float)val_z) * mdps_per_digit)/1000.0;
        return 1;
    }*/
    return 0;
}

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

  /*
   * SPI2 I/O pins setup.
   */
  palSetPadMode(GPIOB, 13, PAL_MODE_ALTERNATE(0) |
                           PAL_STM32_OSPEED_HIGHEST);       /* New SCK.     */
  palSetPadMode(GPIOB, 14, PAL_MODE_ALTERNATE(0) |
                           PAL_STM32_OSPEED_HIGHEST);       /* New MISO.    */
  palSetPadMode(GPIOB, 15, PAL_MODE_ALTERNATE(0) |
                           PAL_STM32_OSPEED_HIGHEST);       /* New MOSI.    */
  palSetPad(GPIOB, 12);
  palSetPadMode(GPIOB, 12, PAL_MODE_OUTPUT_PUSHPULL |
                           PAL_STM32_OSPEED_HIGHEST);       /* New CS.      */

  sdStart( &SD2 , NULL );
  palSetPadMode(GPIOA, 2, PAL_MODE_ALTERNATE(1));      /* USART1 TX.       */
  palSetPadMode(GPIOA, 3, PAL_MODE_ALTERNATE(1));      /* USART1 RX.       */

//  spiAcquireBus(&SPID2);              /* Acquire ownership of the bus.    */
  palSetPad(GPIOC, GPIOC_LED4);       /* LED OFF.                         */
  spiStart(&SPID2, &ls_spicfg);       /* Setup transfer parameters.       */
 
  writeByteSPI(LIS302DL_CTRL_REG1, (1<<PD_CTRL_REG1) );  //init lis302 
  writeByteSPI(LIS302DL_CTRL_REG2, (1<<FDS_CTRL_REG2) ); //enable FDS filter

//  spiSelect(&SPID2);                  /* Slave Select assertion.          */

  float gyroData[3];
  if (readGyro(gyroData)) {
 
  }
 
  //chThdCreateStatic(spi_thread_1_wa, sizeof(spi_thread_1_wa),
  //                    NORMALPRIO + 1, spi_thread_1, NULL);
  //chThdCreateStatic(spi_thread_2_wa, sizeof(spi_thread_2_wa),
  //                    NORMALPRIO + 1, spi_thread_2, NULL);

  /*
   * Starting the blinker thread.
   */
  chThdCreateStatic(blinker_wa, sizeof(blinker_wa),
                    NORMALPRIO-1, blinker, NULL);

  while (TRUE) {
//    chprintf((BaseSequentialStream *) &SD2, "axis X Y Z: %d %d %d \n\r", 255 - readByteSPI(LIS302DL_OUT_X), 255 - readByteSPI(LIS302DL_OUT_Y), 255 - readByteSPI(LIS302DL_OUT_Z));  
    chprintf((BaseSequentialStream *) &SD2, "axis X Y Z: %d %d %d \n\r", readByteSPI(LIS302DL_OUT_X), readByteSPI(LIS302DL_OUT_Y), readByteSPI(LIS302DL_OUT_Z));  
    chThdSleepMilliseconds(100);
  }
  return 0;
}
