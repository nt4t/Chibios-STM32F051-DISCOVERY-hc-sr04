Chibios-STM32F051-DISCOVERY-hc-sr04
==============================

This is my example code for the STM32F0 Discovers using the RTOS ChibiOS.

requirements
------------
* Chibios 2.5.0+ (or a recent development snapshot)
* arm toolchain (e.g. arm-none-eabi from summon-arm)

features
--------
* background blinker thread
* HC-SR04 Ultrasonic Ranging Module measurement via ICU
* control accuracy via ICU clock frequency in icucfg struct

usage
-----
* edit the Makefile and point "CHIBIOS = ../../chibios" to your ChibiOS folder
* make
* connect the STM32F0 Discovery with TTL/RS232 adapter, PA2(TX) and PA3(RX) are routed to USART2 38400-8-N-1.
* connect hc-sr04 trig to portC 11 pin.
* connect hc-sr04 echo to portA 6 pin via simple Voltage Divider http://www.microbuilder.eu/Tutorials/Fundamentals/MeasuringBatteryVoltage.aspx.
* flash the STM32F0
* use your favorite terminal programm to connect to the Serial Port 38400-8n1 

