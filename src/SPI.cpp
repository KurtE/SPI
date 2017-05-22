/*
 * Copyright (c) 2010 by Cristian Maglie <c.maglie@bug.st>
 * SPI Master library for arduino.
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 */

#include "SPI.h"
#include "pins_arduino.h"

/**********************************************************/
/*     8 bit AVR-based boards				  */
/**********************************************************/

#if defined(__AVR__)

SPIClass SPI;

uint8_t SPIClass::interruptMode = 0;
uint8_t SPIClass::interruptMask = 0;
uint8_t SPIClass::interruptSave = 0;
#ifdef SPI_TRANSACTION_MISMATCH_LED
uint8_t SPIClass::inTransactionFlag = 0;
#endif
uint16_t SPIClass::_transferWriteFill = 0;

void SPIClass::begin()
{
	// Set SS to high so a connected chip will be "deselected" by default
	digitalWrite(SS, HIGH);

	// When the SS pin is set as OUTPUT, it can be used as
	// a general purpose output port (it doesn't influence
	// SPI operations).
	pinMode(SS, OUTPUT);

	// Warning: if the SS pin ever becomes a LOW INPUT then SPI
	// automatically switches to Slave, so the data direction of
	// the SS pin MUST be kept as OUTPUT.
	SPCR |= _BV(MSTR);
	SPCR |= _BV(SPE);

	// Set direction register for SCK and MOSI pin.
	// MISO pin automatically overrides to INPUT.
	// By doing this AFTER enabling SPI, we avoid accidentally
	// clocking in a single bit since the lines go directly
	// from "input" to SPI control.
	// http://code.google.com/p/arduino/issues/detail?id=888
	pinMode(SCK, OUTPUT);
	pinMode(MOSI, OUTPUT);
}

void SPIClass::end() {
	SPCR &= ~_BV(SPE);
}

// mapping of interrupt numbers to bits within SPI_AVR_EIMSK
#if defined(__AVR_ATmega32U4__)
  #define SPI_INT0_MASK	 (1<<INT0)
  #define SPI_INT1_MASK	 (1<<INT1)
  #define SPI_INT2_MASK	 (1<<INT2)
  #define SPI_INT3_MASK	 (1<<INT3)
  #define SPI_INT4_MASK	 (1<<INT6)
#elif defined(__AVR_AT90USB646__) || defined(__AVR_AT90USB1286__)
  #define SPI_INT0_MASK	 (1<<INT0)
  #define SPI_INT1_MASK	 (1<<INT1)
  #define SPI_INT2_MASK	 (1<<INT2)
  #define SPI_INT3_MASK	 (1<<INT3)
  #define SPI_INT4_MASK	 (1<<INT4)
  #define SPI_INT5_MASK	 (1<<INT5)
  #define SPI_INT6_MASK	 (1<<INT6)
  #define SPI_INT7_MASK	 (1<<INT7)
#elif defined(EICRA) && defined(EICRB) && defined(EIMSK)
  #define SPI_INT0_MASK	 (1<<INT4)
  #define SPI_INT1_MASK	 (1<<INT5)
  #define SPI_INT2_MASK	 (1<<INT0)
  #define SPI_INT3_MASK	 (1<<INT1)
  #define SPI_INT4_MASK	 (1<<INT2)
  #define SPI_INT5_MASK	 (1<<INT3)
  #define SPI_INT6_MASK	 (1<<INT6)
  #define SPI_INT7_MASK	 (1<<INT7)
#else
  #ifdef INT0
  #define SPI_INT0_MASK	 (1<<INT0)
  #endif
  #ifdef INT1
  #define SPI_INT1_MASK	 (1<<INT1)
  #endif
  #ifdef INT2
  #define SPI_INT2_MASK	 (1<<INT2)
  #endif
#endif

void SPIClass::usingInterrupt(uint8_t interruptNumber)
{
	uint8_t stmp, mask;

	if (interruptMode > 1) return;

	stmp = SREG;
	noInterrupts();
	switch (interruptNumber) {
	#ifdef SPI_INT0_MASK
	case 0: mask = SPI_INT0_MASK; break;
	#endif
	#ifdef SPI_INT1_MASK
	case 1: mask = SPI_INT1_MASK; break;
	#endif
	#ifdef SPI_INT2_MASK
	case 2: mask = SPI_INT2_MASK; break;
	#endif
	#ifdef SPI_INT3_MASK
	case 3: mask = SPI_INT3_MASK; break;
	#endif
	#ifdef SPI_INT4_MASK
	case 4: mask = SPI_INT4_MASK; break;
	#endif
	#ifdef SPI_INT5_MASK
	case 5: mask = SPI_INT5_MASK; break;
	#endif
	#ifdef SPI_INT6_MASK
	case 6: mask = SPI_INT6_MASK; break;
	#endif
	#ifdef SPI_INT7_MASK
	case 7: mask = SPI_INT7_MASK; break;
	#endif
	default:
		interruptMode = 2;
		SREG = stmp;
		return;
	}
	interruptMode = 1;
	interruptMask |= mask;
	SREG = stmp;
}

// Some globals used for Async support
uint8_t *_spi_async_txBuffer;
uint8_t *_spi_async_rxBuffer;
uint32_t  _spi_async_count;
void (*_spi_async_callback)(void);
volatile bool _spi_async_active = false;
volatile uint8_t _spi_transfer16_active_state = 0;


void SPIClass::transfer(const void * buf, void * retbuf, uint32_t count) {
	if (count == 0) return;

	const uint8_t *p = (const uint8_t *)buf;
	uint8_t *pret = (uint8_t *)retbuf;
	uint8_t in;

	uint8_t out = p ? *p++ : (_transferWriteFill & 0xff);
	SPDR = out;
	while (--count > 0) {
		if (p) {
			out = *p++;
		}
		while (!(SPSR & _BV(SPIF))) ;
		in = SPDR;
		SPDR = out;
		if (pret)*pret++ = in;
	}
	while (!(SPSR & _BV(SPIF))) ;
	in = SPDR;
	if (pret)*pret = in;
}

void SPIClass::transfer16(const uint16_t * buf, uint16_t* retbuf, uint32_t count) {
	uint16_t out = _transferWriteFill;
	while (count--) {
		if (buf) {
			out = *buf++;
		}
		uint16_t retval = transfer16(out);
		if (retbuf) {
			*retbuf++ = retval;
		}
	}
}




bool SPIClass::transfer(const void *txBuffer, void *rxBuffer, uint32_t count, void(*callback)(void)) {
	if (count == 0) return false;
	if (_spi_async_active) return false;

 	_spi_async_txBuffer = (uint8_t*)txBuffer;
	_spi_async_rxBuffer = (uint8_t*)rxBuffer;
	_spi_async_count = count;
	_spi_async_callback = callback;

	// setup to output the first byte
	_spi_async_active = true;
	SPDR = _spi_async_txBuffer ? *_spi_async_txBuffer++ : (_transferWriteFill & 0xff);
	attachInterrupt();
	return true;
}

bool SPIClass::transfer16(const uint16_t *txBuffer, uint16_t *rxBuffer, uint32_t count, void(*callback)(void)) {
	if (count == 0) return false;
	if (_spi_async_active) return false;

 	_spi_async_txBuffer = (uint8_t*)txBuffer;
	_spi_async_rxBuffer = (uint8_t*)rxBuffer;
	_spi_async_count = count;
	_spi_async_callback = callback;

	// setup to output the first byte
	_spi_async_active = true;
	_spi_transfer16_active_state = 1; 	// doing MSB
	SPDR = _spi_async_txBuffer ? _spi_async_txBuffer[1] : (_transferWriteFill & 0xff);
	attachInterrupt();
	return true;
}

void SPIClass::flush(void) {
	while (_spi_async_active) {
		yield();
	}

}
bool SPIClass::done(void)  {
	return (_spi_async_active == 0);
}


ISR(SPI_STC_vect)
{
	// Get the return value
	uint8_t in = SPDR;
	if (_spi_transfer16_active_state) {
		// 16 bit mode, LSB/MSB state... 
		if (_spi_async_rxBuffer) {
			if (_spi_transfer16_active_state == 1) {
				_spi_async_rxBuffer[1] = in;
			} else {
				_spi_async_rxBuffer[0] = in;
				_spi_async_rxBuffer += 2;	// advance. 
			}
		}

		// Now see if we need to transfer new item. 
		if (_spi_transfer16_active_state == 1) {
			if (_spi_async_txBuffer) {
				SPDR = _spi_async_txBuffer[0];
				_spi_async_txBuffer += 2;
			} else {
				SPDR = (SPI._transferWriteFill >> 8);
			}
		} else {
			if (--_spi_async_count) {
				SPDR = _spi_async_txBuffer ? _spi_async_txBuffer[1] : (SPI._transferWriteFill & 0xff);
			} else {
				// We are done, so disable the interrupt, plus clear the active flag and then call callback
				_spi_async_active = false;
				_spi_transfer16_active_state = 0;
				SPI.detachInterrupt();
				if (_spi_async_callback)
	    			(*_spi_async_callback)();
	    	}
		}	

	} else {
		// standard 8 bit transfer.
		if (_spi_async_rxBuffer) {
			*_spi_async_rxBuffer++ = in;
		}

		if (--_spi_async_count) {
			if (_spi_async_txBuffer)
				SPDR = *_spi_async_txBuffer++;
			else
				SPDR = (SPI._transferWriteFill & 0xff);
		} else {
			// We are done, so disable the interrupt, plus clear the active flag and then call callback
			_spi_async_active = false;
			SPI.detachInterrupt();
			if (_spi_async_callback)
	    		(*_spi_async_callback)();
		}	
	}


}

#endif


