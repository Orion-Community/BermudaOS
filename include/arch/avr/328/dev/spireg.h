#ifndef __328SPI_REG_H
#define __328SPI_REG_H

#include <bermuda.h>

#include <arch/avr/io.h>

/**
 * \def SPI_DDR
 * \brief DDR for hardware SPI.
 */
#define SPI_DDR (*(AvrIO->ddrb))

/**
 * \def SPI_PORT
 * \brief SPI I/O port.
 */
#define SPI_PORT (*(AvrIO->portb))

#define SPI_CTRL   SFR_IO8(0x2C)
#define SPI_STATUS SFR_IO8(0x2D)
#define SPI_DATA   SFR_IO8(0x2E)
#define SPI_MAX_DIV 128

/**
 * \def SPI_SCK
 * \brief SCK pin.
 */
#define SPI_SCK  5

/**
 * \def SPI_MISO
 * \brief MISO pin.
 */
#define SPI_MISO 4

/**
 * \def SPI_MOSI
 * \brief MOSI pin.
 */
#define SPI_MOSI 3

/**
 * \def SPI_SS
 * \brief SS pin.
 */
#define SPI_SS   2

#endif /* __328SPI_REG_H */