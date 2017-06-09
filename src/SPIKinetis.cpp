
 /* Copyright (c) 2010 by Cristian Maglie <c.maglie@bug.st>
 * SPI Master library for arduino.
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 */

#include "SPIKinetis.h"
#include "pins_arduino.h"


/**********************************************************/
/*     32 bit Teensy 3.X			  					  */
/**********************************************************/

#if defined(__arm__) && defined(TEENSYDUINO) && defined(KINETISK)

#if defined(__MK64FX512__)
	// defines wrong in kinetis.h... submited update request
	#ifndef DMAMUX_SOURCE_SPI1
		#define DMAMUX_SOURCE_SPI1			16  // Either RX or TX (but not both at same time)
		#define DMAMUX_SOURCE_SPI2			17  // Either RX or TX (but not both at same time)
	#endif
#endif

// Also for now work around bug in DMAChannel
#ifdef KINETISK
#ifndef TRANSFER_COUNT_FIXED
inline void DMAChanneltransferCount(DMAChannel * dmac, unsigned int len) {
	// note does no validation of length...
	DMABaseClass::TCD_t *tcd = dmac->TCD;
	if (!(tcd->BITER & DMA_TCD_BITER_ELINK)) {
		tcd->BITER = len & 0x7fff;
	} else {
		tcd->BITER = (tcd->BITER & 0xFE00) | (len & 0x1ff);
	}
	tcd->CITER = tcd->BITER; 
}
#else 
inline void DMAChanneltransferCount(DMAChannel * dmac, unsigned int len) {
	dmac->transferCount(len);
}
#endif
#endif

//=============================================================================
// Define the ino specific to instance in hardware
// Hardware clock, Miso, Mosi, SCK, CS, IRQ	
//=============================================================================
// First: t3.0-2
extern void _spi_dma_rxISR0(void);
#if defined(__MK20DX128__) || defined(__MK20DX256__)
const SPIClass::SPI_Hardware_t SPIClass::spi0_hardware = {
	SIM_SCGC6, SIM_SCGC6_SPI0, 
	4, IRQ_SPI0, 
	32767, DMAMUX_SOURCE_SPI0_TX, DMAMUX_SOURCE_SPI0_RX, 
	_spi_dma_rxISR0,  
	12, 8, 
	2, 2, 
	11, 7, 
	2, 2, 
	13, 14, 
	2, 2, 
	10, 2, 9, 6, 20, 23, 21, 22, 15,
	2,  2, 2,  2,  2,  2,  2,  2,  2,
	0x1, 0x1, 0x2, 0x2, 0x4, 0x4, 0x8, 0x8, 0x10
};
SPIClass SPI((uintptr_t)&KINETISK_SPI0, (uintptr_t)&SPIClass::spi0_hardware);

#elif defined(__MK64FX512__) || defined(__MK66FX1M0__)
extern void _spi_dma_rxISR1(void);
extern void _spi_dma_rxISR2(void);
const SPIClass::SPI_Hardware_t SPIClass::spi0_hardware = {
	SIM_SCGC6, SIM_SCGC6_SPI0, 4, IRQ_SPI0,
	32767, DMAMUX_SOURCE_SPI0_TX, DMAMUX_SOURCE_SPI0_RX,
	_spi_dma_rxISR0,
	12, 8, 39, 255,
	2, 2, 2, 0,
	11, 7, 28, 255,
	2, 2, 2, 0,
	13, 14, 27,
	2, 2, 2,
	10, 2, 9, 6, 20, 23, 21, 22, 15, 26, 45,
	2,  2, 2,  2,  2,  2,  2,  2,  2,   2,   3,
	0x1, 0x1, 0x2, 0x2, 0x4, 0x4, 0x8, 0x8, 0x10, 0x1, 0x20
};
const SPIClass::SPI_Hardware_t SPIClass::spi1_hardware = {
	SIM_SCGC6, SIM_SCGC6_SPI1, 1, IRQ_SPI1,
	#if defined(__MK66FX1M0__)
	32767, DMAMUX_SOURCE_SPI1_TX, DMAMUX_SOURCE_SPI1_RX,
	#else
	// T3.5 does not have good DMA support on 1 and 2
	511, 0, DMAMUX_SOURCE_SPI1,
	#endif
	_spi_dma_rxISR1,
	1, 5, 61, 59,
	2, 7, 2, 7,
	0, 21, 61, 59,
	2, 7, 7, 2,
	32, 20, 60,
	2, 7, 2,
	6, 31, 58, 62, 63, 255, 255, 255, 255, 255, 255,
	7,  2,  2,  2,  2,  0,  0,  0,  0,   0,   0,
	0x1, 0x1, 0x2, 0x1, 0x4, 0, 0, 0, 0, 0, 0
};


const SPIClass::SPI_Hardware_t SPIClass::spi2_hardware = {
	SIM_SCGC3, SIM_SCGC3_SPI2, 1,  IRQ_SPI2, 
	#if defined(__MK66FX1M0__)
	32767, DMAMUX_SOURCE_SPI2_TX, DMAMUX_SOURCE_SPI2_RX,
	#else
	// T3.5 does not have good DMA support on 1 and 2
	511, 0, DMAMUX_SOURCE_SPI2,
	#endif
	_spi_dma_rxISR2,
	45, 51, 255, 255,
	2, 2, 0, 0,
	44, 52, 255, 255,
	2, 2, 0, 0, 
	46, 53, 255,
	2, 2, 0, 
	43, 54, 55, 255, 255, 255, 255, 255, 255, 255, 255,  
	2,  2,  2,  0,  0,  0,  0,  0,  0,   0,   0, 
	0x1, 0x2, 0x1, 0, 0, 0, 0, 0, 0, 0, 0
};
SPIClass SPI((uintptr_t)&KINETISK_SPI0, (uintptr_t)&SPIClass::spi0_hardware);
SPIClass SPI1((uintptr_t)&KINETISK_SPI1, (uintptr_t)&SPIClass::spi1_hardware);
SPIClass SPI2((uintptr_t)&KINETISK_SPI2, (uintptr_t)&SPIClass::spi2_hardware);
#endif


void SPIClass::begin()
{
	volatile uint32_t *reg;

	hardware().clock_gate_register |= hardware().clock_gate_mask;
	port().MCR = SPI_MCR_MDIS | SPI_MCR_HALT | SPI_MCR_PCSIS(0x1F);
	port().CTAR0 = SPI_CTAR_FMSZ(7) | SPI_CTAR_PBR(0) | SPI_CTAR_BR(1) | SPI_CTAR_CSSCK(1);
	port().CTAR1 = SPI_CTAR_FMSZ(15) | SPI_CTAR_PBR(0) | SPI_CTAR_BR(1) | SPI_CTAR_CSSCK(1);
	port().MCR = SPI_MCR_MSTR | SPI_MCR_PCSIS(0x1F);
	reg = portConfigRegister(hardware().mosi_pin[mosi_pin_index]);
	*reg = PORT_PCR_MUX(hardware().mosi_mux[mosi_pin_index]);
	reg = portConfigRegister(hardware().miso_pin[miso_pin_index]);
	*reg= PORT_PCR_MUX(hardware().miso_mux[miso_pin_index]);
	reg = portConfigRegister(hardware().sck_pin[sck_pin_index]);
	*reg = PORT_PCR_MUX(hardware().sck_mux[sck_pin_index]);
}

void SPIClass::end()
{
	volatile uint32_t *reg;

	reg = portConfigRegister(hardware().mosi_pin[mosi_pin_index]);
	*reg = 0;
	reg = portConfigRegister(hardware().miso_pin[miso_pin_index]);
	*reg = 0;
	reg = portConfigRegister(hardware().sck_pin[sck_pin_index]);
	*reg = 0;
	port().MCR = SPI_MCR_MDIS | SPI_MCR_HALT | SPI_MCR_PCSIS(0x1F);
}

void SPIClass::usingInterrupt(IRQ_NUMBER_t interruptName)
{
	uint32_t n = (uint32_t)interruptName;

	if (n >= NVIC_NUM_INTERRUPTS) return;

	//Serial.print("usingInterrupt ");
	//Serial.println(n);
	interruptMasksUsed |= (1 << (n >> 5));
	interruptMask[n >> 5] |= (1 << (n & 0x1F));
	//Serial.printf("interruptMasksUsed = %d\n", interruptMasksUsed);
	//Serial.printf("interruptMask[0] = %08X\n", interruptMask[0]);
	//Serial.printf("interruptMask[1] = %08X\n", interruptMask[1]);
	//Serial.printf("interruptMask[2] = %08X\n", interruptMask[2]);
}

void SPIClass::notUsingInterrupt(IRQ_NUMBER_t interruptName)
{
	uint32_t n = (uint32_t)interruptName;
	if (n >= NVIC_NUM_INTERRUPTS) return;
	interruptMask[n >> 5] &= ~(1 << (n & 0x1F));
	if (interruptMask[n >> 5] == 0) {
		interruptMasksUsed &= ~(1 << (n >> 5));
	}
}

const uint16_t SPISettings::ctar_div_table[23] = {
	2, 3, 4, 5, 6, 8, 10, 12, 16, 20, 24, 32, 40,
	56, 64, 96, 128, 192, 256, 384, 512, 640, 768
};
const uint32_t SPISettings::ctar_clock_table[23] = {
	SPI_CTAR_PBR(0) | SPI_CTAR_BR(0) | SPI_CTAR_DBR | SPI_CTAR_CSSCK(0),
	SPI_CTAR_PBR(1) | SPI_CTAR_BR(0) | SPI_CTAR_DBR | SPI_CTAR_CSSCK(0),
	SPI_CTAR_PBR(0) | SPI_CTAR_BR(0) | SPI_CTAR_CSSCK(0),
	SPI_CTAR_PBR(2) | SPI_CTAR_BR(0) | SPI_CTAR_DBR | SPI_CTAR_CSSCK(0),
	SPI_CTAR_PBR(1) | SPI_CTAR_BR(0) | SPI_CTAR_CSSCK(0),
	SPI_CTAR_PBR(0) | SPI_CTAR_BR(1) | SPI_CTAR_CSSCK(1),
	SPI_CTAR_PBR(2) | SPI_CTAR_BR(0) | SPI_CTAR_CSSCK(0),
	SPI_CTAR_PBR(1) | SPI_CTAR_BR(1) | SPI_CTAR_CSSCK(1),
	SPI_CTAR_PBR(0) | SPI_CTAR_BR(3) | SPI_CTAR_CSSCK(2),
	SPI_CTAR_PBR(2) | SPI_CTAR_BR(1) | SPI_CTAR_CSSCK(0),
	SPI_CTAR_PBR(1) | SPI_CTAR_BR(3) | SPI_CTAR_CSSCK(2),
	SPI_CTAR_PBR(0) | SPI_CTAR_BR(4) | SPI_CTAR_CSSCK(3),
	SPI_CTAR_PBR(2) | SPI_CTAR_BR(3) | SPI_CTAR_CSSCK(2),
	SPI_CTAR_PBR(3) | SPI_CTAR_BR(3) | SPI_CTAR_CSSCK(2),
	SPI_CTAR_PBR(0) | SPI_CTAR_BR(5) | SPI_CTAR_CSSCK(4),
	SPI_CTAR_PBR(1) | SPI_CTAR_BR(5) | SPI_CTAR_CSSCK(4),
	SPI_CTAR_PBR(0) | SPI_CTAR_BR(6) | SPI_CTAR_CSSCK(5),
	SPI_CTAR_PBR(1) | SPI_CTAR_BR(6) | SPI_CTAR_CSSCK(5),
	SPI_CTAR_PBR(0) | SPI_CTAR_BR(7) | SPI_CTAR_CSSCK(6),
	SPI_CTAR_PBR(1) | SPI_CTAR_BR(7) | SPI_CTAR_CSSCK(6),
	SPI_CTAR_PBR(0) | SPI_CTAR_BR(8) | SPI_CTAR_CSSCK(7),
	SPI_CTAR_PBR(2) | SPI_CTAR_BR(7) | SPI_CTAR_CSSCK(6),
	SPI_CTAR_PBR(1) | SPI_CTAR_BR(8) | SPI_CTAR_CSSCK(7)
};

void SPIClass::updateCTAR(uint32_t ctar)
{
	if (port().CTAR0 != ctar) {
		uint32_t mcr = port().MCR;
		if (mcr & SPI_MCR_MDIS) {
			port().CTAR0 = ctar;
			port().CTAR1 = ctar | SPI_CTAR_FMSZ(8);
		} else {
			port().MCR = SPI_MCR_MDIS | SPI_MCR_HALT | SPI_MCR_PCSIS(0x1F);
			port().CTAR0 = ctar;
			port().CTAR1 = ctar | SPI_CTAR_FMSZ(8);
			port().MCR = mcr;
		}
	}
}

void SPIClass::setBitOrder(uint8_t bitOrder)
{
	hardware().clock_gate_register |= hardware().clock_gate_mask;
	uint32_t ctar = port().CTAR0;
	if (bitOrder == LSBFIRST) {
		ctar |= SPI_CTAR_LSBFE;
	} else {
		ctar &= ~SPI_CTAR_LSBFE;
	}
	updateCTAR(ctar);
}

void SPIClass::setDataMode(uint8_t dataMode)
{
	hardware().clock_gate_register |= hardware().clock_gate_mask;
	//uint32_t ctar = port().CTAR0;

	// TODO: implement with native code
	//SPCR = (SPCR & ~SPI_MODE_MASK) | dataMode;
}

void SPIClass::setClockDivider_noInline(uint32_t clk)
{
	hardware().clock_gate_register |= hardware().clock_gate_mask;
	uint32_t ctar = port().CTAR0;
	ctar &= (SPI_CTAR_CPOL | SPI_CTAR_CPHA | SPI_CTAR_LSBFE);
	if (ctar & SPI_CTAR_CPHA) {
		clk = (clk & 0xFFFF0FFF) | ((clk & 0xF000) >> 4);
	}
	ctar |= clk;
	updateCTAR(ctar);
}



void SPIClass::transfer(const void * buf, void * retbuf, size_t count)
{

	if (count == 0) return;
	if (!(port().CTAR0 & SPI_CTAR_LSBFE)) {
		// We are doing the standard MSB order
		const uint8_t *p_write = (const uint8_t *)buf;
		uint8_t *p_read = (uint8_t *)retbuf;
		size_t count_read = count;

		// Lets clear the reader queue
		port().MCR = SPI_MCR_MSTR | SPI_MCR_CLR_RXF | SPI_MCR_PCSIS(0x1F);

		uint32_t sr;

		// Now lets loop while we still have data to output
		if (count & 1) {
		    if (p_write) {
				if (count > 1)
					port().PUSHR = *p_write++ | SPI_PUSHR_CONT | SPI_PUSHR_CTAS(0);
				else
					port().PUSHR = *p_write++ | SPI_PUSHR_CTAS(0);
			} else {
				if (count > 1)
					port().PUSHR = (_transferWriteFill & 0xff) | SPI_PUSHR_CONT | SPI_PUSHR_CTAS(0);
				else
					port().PUSHR = (_transferWriteFill & 0xff) | SPI_PUSHR_CTAS(0);
			}
			count--;
		}

	    uint16_t w =  (uint16_t)((_transferWriteFill & 0xff) << 8) | (_transferWriteFill & 0xff);

		while (count > 0) {
			// Push out the next byte; 
		    if (p_write) {
		    	w = (*p_write++) << 8;
				w |= *p_write++;
		    }
		    uint16_t queue_full_status_mask = (hardware().queue_size-1) << 12;
			if (count == 2)
				port().PUSHR = w | SPI_PUSHR_CTAS(1);
			else	
				port().PUSHR = w | SPI_PUSHR_CONT | SPI_PUSHR_CTAS(1);
			count -= 2; // how many bytes to output.
			// Make sure queue is not full before pushing next byte out
			do {
				sr = port().SR;
				if (sr & 0xF0)  {
					uint16_t w = port().POPR;  // Read any pending RX bytes in
					if (count_read & 1) {
						if (p_read) {
							*p_read++ = w;  // Read any pending RX bytes in
						} 
						count_read--;
					} else {
						if (p_read) {
							*p_read++ = w >> 8;
							*p_read++ = (w & 0xff);
						}
						count_read -= 2;
					}
				}
			} while ((sr & (15 << 12)) > queue_full_status_mask);

		}

		// now lets wait for all of the read bytes to be returned...
		while (count_read) {
			sr = port().SR;
			if (sr & 0xF0)  {
				uint16_t w = port().POPR;  // Read any pending RX bytes in
				if (count_read & 1) {
					if (p_read)
						*p_read++ = w;  // Read any pending RX bytes in
					count_read--;
				} else {
					if (p_read) {
						*p_read++ = w >> 8;
						*p_read++ = (w & 0xff);
					}
					count_read -= 2;
				}
			}
		}
	} else {
		// We are doing the less ofen LSB mode
		const uint8_t *p_write = (const uint8_t *)buf;
		uint8_t *p_read = (uint8_t *)retbuf;
		size_t count_read = count;

		// Lets clear the reader queue
		port().MCR = SPI_MCR_MSTR | SPI_MCR_CLR_RXF | SPI_MCR_PCSIS(0x1F);

		uint32_t sr;

		// Now lets loop while we still have data to output
		if (count & 1) {
		    if (p_write) {
				if (count > 1)
					port().PUSHR = *p_write++ | SPI_PUSHR_CONT | SPI_PUSHR_CTAS(0);
				else
					port().PUSHR = *p_write++ | SPI_PUSHR_CTAS(0);
			} else {
				if (count > 1)
					port().PUSHR = (_transferWriteFill & 0xff) | SPI_PUSHR_CONT | SPI_PUSHR_CTAS(0);
				else
					port().PUSHR = (_transferWriteFill & 0xff) | SPI_PUSHR_CTAS(0);
			}
			count--;
		}

	    uint16_t w = _transferWriteFill;

		while (count > 0) {
			// Push out the next byte; 
		    if (p_write) {
				w = *p_write++;
		    	w |= ((*p_write++) << 8);
		    }
		    uint16_t queue_full_status_mask = (hardware().queue_size-1) << 12;
			if (count == 2)
				port().PUSHR = w | SPI_PUSHR_CTAS(1);
			else	
				port().PUSHR = w | SPI_PUSHR_CONT | SPI_PUSHR_CTAS(1);
			count -= 2; // how many bytes to output.
			// Make sure queue is not full before pushing next byte out
			do {
				sr = port().SR;
				if (sr & 0xF0)  {
					uint16_t w = port().POPR;  // Read any pending RX bytes in
					if (count_read & 1) {
						if (p_read) {
							*p_read++ = w;  // Read any pending RX bytes in
						} 
						count_read--;
					} else {
						if (p_read) {
							*p_read++ = (w & 0xff);
							*p_read++ = w >> 8;
						}
						count_read -= 2;
					}
				}
			} while ((sr & (15 << 12)) > queue_full_status_mask);

		}

		// now lets wait for all of the read bytes to be returned...
		while (count_read) {
			sr = port().SR;
			if (sr & 0xF0)  {
				uint16_t w = port().POPR;  // Read any pending RX bytes in
				if (count_read & 1) {
					if (p_read)
						*p_read++ = w;  // Read any pending RX bytes in
					count_read--;
				} else {
					if (p_read) {
						*p_read++ = (w & 0xff);
						*p_read++ = w >> 8;
					}
					count_read -= 2;
				}
			}
		}
	}
}

void SPIClass::transfer16(const uint16_t * buf, uint16_t * retbuf, size_t count)
{
	if (count == 0) return;
	size_t count_read = count;

	// Lets clear the reader queue
	port().MCR = SPI_MCR_MSTR | SPI_MCR_CLR_RXF | SPI_MCR_PCSIS(0x1F);

	uint32_t sr;

	// Now lets loop while we still have data to output
	
    uint16_t w =  _transferWriteFill;

	while (count > 0) {
		// Push out the next byte; 
	    if (buf) {
	    	w = *buf++;
	    }
	    uint16_t queue_full_status_mask = (hardware().queue_size-1) << 12;
		if (count == 1)
			port().PUSHR = w | SPI_PUSHR_CTAS(1);
		else	
			port().PUSHR = w | SPI_PUSHR_CONT | SPI_PUSHR_CTAS(1);
		count--; // how many words to output.
		// Make sure queue is not full before pushing next byte out
		do {
			sr = port().SR;
			if (sr & 0xF0)  {
				uint16_t w = port().POPR;  // Read any pending RX bytes in
				if (retbuf) {
					*retbuf++ = w ;
				}
				count_read--;
			}
		} while ((sr & (15 << 12)) > queue_full_status_mask);

	}

	// now lets wait for all of the read bytes to be returned...
	while (count_read) {
		sr = port().SR;
		if (sr & 0xF0)  {
			uint16_t w = port().POPR;  // Read any pending RX bytes in
			if (retbuf) {
				*retbuf++ = w;  // Read any pending RX bytes in
			}
			count_read--;
		}
	}
}

//=============================================================================
// ASYNCH Support
//=============================================================================
//=========================================================================
// Try Transfer using DMA.
//=========================================================================

void _spi_dma_rxISR0(void) {SPI.dma_rxisr();}
#if defined(__MK64FX512__) || defined(__MK66FX1M0__)
void _spi_dma_rxISR1(void) {SPI1.dma_rxisr();}
void _spi_dma_rxISR2(void) {SPI2.dma_rxisr();}
#endif


static uint8_t bit_bucket;
static uint16_t bit_bucket16;

#if defined (SPI_DEBUG_ASYNC_T3X)
// Lets define a TCD structure to print data from

typedef struct  __attribute__((packed, aligned(4))) {
		uint32_t SADDR;
		int16_t SOFF;
		uint16_t ATTR;
		uint32_t NBYTES;
		int32_t SLAST;
		uint32_t DADDR;
		int16_t DOFF;
		uint16_t CITER;
		int32_t DLASTSGA;
		uint16_t CSR;
		uint16_t BITER;
	} TCD_DEBUG;

void dumpDMA_TCD(const char *psz, DMABaseClass *dmabc)
{
	Serial.printf("%s %08x %08x:", psz, (uint32_t)dmabc, (uint32_t)dmabc->TCD);
	TCD_DEBUG *tcd = (TCD_DEBUG*)dmabc->TCD;
	Serial.printf("%08x %04x %04x %08x %08x ", tcd->SADDR, tcd->SOFF, tcd->ATTR, tcd->NBYTES, tcd->SLAST);
	Serial.printf("%08x %04x %04x %08x %04x %04x\n", tcd->DADDR, tcd->DOFF, tcd->CITER, tcd->DLASTSGA, 
					tcd->CSR, tcd->BITER);

}
#endif

#define dontInterruptAtCompletion(dmac) (dmac)->TCD->CSR &= ~DMA_TCD_CSR_INTMAJOR

bool SPIClass::initDMAChannels() {
//	Serial.println("Init DMA Channels"); 

	// Allocate our channels. 
	_dmaTX = new DMAChannel();
	if (_dmaTX == nullptr) {
		return false;
	}

	_dmaRX = new DMAChannel();
	if (_dmaRX == nullptr) {
		delete _dmaTX; // release it
		_dmaTX = nullptr; 
		return false;
	}

	// Let's setup the RX chain
	_dmaRX->disable();
	_dmaRX->source((volatile uint8_t&)port().POPR);
	_dmaRX->disableOnCompletion();
	_dmaRX->triggerAtHardwareEvent(hardware().rx_dma_channel);
	_dmaRX->attachInterrupt(hardware().dma_rxisr);
	_dmaRX->interruptAtCompletion();

	// We may be using settings chain here so lets set it up. 
	// Now lets setup TX chain.  Note if trigger TX is not set
	// we need to have the RX do it for us.
	_dmaTX->disable();
	_dmaTX->destination((volatile uint8_t&)port().PUSHR);
	_dmaTX->disableOnCompletion();

	if (hardware().tx_dma_channel) {
		_dmaTX->triggerAtHardwareEvent(hardware().tx_dma_channel);
	} else {
		Serial.printf("SPI InitDMA tx triger by RX: %x\n", (uint32_t)_dmaRX);
	    _dmaTX->triggerAtTransfersOf(*_dmaRX);
	}


	_dma_state = DMAState::idle;  // Should be first thing set!
	//Serial.println("end First dma call");
	return true;
}


bool SPIClass::transfer(const void *buf, void *retbuf, size_t count, void(*callback)(void)) {

	//Serial.println("Async Transfer called"); Serial.flush();
  	// There are some special casing we may wish to setup.
  	// For logical Write operations: don't use RX but instead just let it stay in ffio

  	// On T3.5 on SPI1/2 we will not set up RX, as only have one DMA channel to use so
  	// will use on TX...
	uint8_t dma_first_byte;
	if (_dma_state == DMAState::notAllocated) {
		if (!initDMAChannels())
			return false;
	}

	if (_dma_state == DMAState::active)
		return false; // already active

	// Now handle the cases where the count > then how many we can output in one DMA request
	if (count > hardware().max_dma_count) {
		_dma_count_remaining = count - hardware().max_dma_count;
		count = hardware().max_dma_count;
	} else {
		_dma_count_remaining = 0;
	}

	// Now See if caller passed in a source buffer. 
	_dmaTX->TCD->ATTR_DST = 0;		// Make sure set for 8 bit mode
	uint8_t *write_data = (uint8_t*) buf;
	if (buf) {
		dma_first_byte = *write_data;
		_dmaTX->sourceBuffer((uint8_t*)write_data+1, count-1);  
		_dmaTX->TCD->SLAST = 0;	// Finish with it pointing to next location
	} else {
		dma_first_byte = (_transferWriteFill & 0xff);
		_dmaTX->source((uint8_t&)_transferWriteFill);   // maybe have setable value
		DMAChanneltransferCount(_dmaTX, count-1);
	}	
	if (retbuf) {
		// On T3.5 must handle SPI1/2 differently as only one DMA channel
		_dmaRX->TCD->ATTR_SRC = 0;		//Make sure set for 8 bit mode...
		_dmaRX->destinationBuffer((uint8_t*)retbuf, count);
		_dmaRX->TCD->DLASTSGA = 0;		// At end point after our bufffer
	} else {
			// Write  only mode
		_dmaRX->TCD->ATTR_SRC = 0;		//Make sure set for 8 bit mode...
		_dmaRX->destination((uint8_t&)bit_bucket);
		DMAChanneltransferCount(_dmaRX, count);
	}

	_dma_callback = callback;

	// Now try to start it?
	// Setup DMA main object
#if defined(SPI_DEBUG_ASYNC_T3X) && defined(SPI_DEBUG_VERBOSE)
	Serial.printf("DMA count: %d, dump TCDs %d %d\n", count, _dmaTX->channel,  _dmaRX->channel);
	dumpDMA_TCD("TX:",_dmaTX);
	dumpDMA_TCD("RX:",_dmaRX);
#endif
	port().MCR = SPI_MCR_MSTR | SPI_MCR_CLR_RXF | SPI_MCR_CLR_TXF | SPI_MCR_PCSIS(0x1F);

	port().SR = 0xFF0F0000;

	// Lets try to output the first byte to make sure that we are in 8 bit mode...
	port().PUSHR = dma_first_byte | SPI_PUSHR_CTAS(0) | SPI_PUSHR_CONT;	

	if (hardware().tx_dma_channel) {
		port().RSER =  SPI_RSER_RFDF_RE | SPI_RSER_RFDF_DIRS | SPI_RSER_TFFF_RE | SPI_RSER_TFFF_DIRS;
		_dmaRX->enable();
		// Get the initial settings. 
		_dmaTX->enable();
	} else {
		//T3.5 SP1 and SPI2 - TX is not triggered by SPI but by RX...
		port().RSER =  SPI_RSER_RFDF_RE | SPI_RSER_RFDF_DIRS ;
	    _dmaTX->triggerAtTransfersOf(*_dmaRX);
		_dmaTX->enable();
		_dmaRX->enable();
	}

	_dma_state = DMAState::active;
	return true;
}

bool SPIClass::transfer16(const uint16_t *buf, uint16_t *retbuf, size_t count, void(*callback)(void)) {

	//Serial.println("Async Transfer called"); Serial.flush();
  	// There are some special casing we may wish to setup.
  	// For logical Write operations: don't use RX but instead just let it stay in ffio

  	// On T3.5 on SPI1/2 we will not set up RX, as only have one DMA channel to use so
  	// will use on TX...
	uint16_t dma_first_word;
	if (_dma_state == DMAState::notAllocated) {
		if (!initDMAChannels())
			return false;
	}

	if (_dma_state == DMAState::active)
		return false; // already active

	// Now handle the cases where the count > then how many we can output in one DMA request
	if (count > hardware().max_dma_count) {
		_dma_count_remaining = count - hardware().max_dma_count;
		count = hardware().max_dma_count;
	} else {
		_dma_count_remaining = 0;
	}

	// If we are using both 16 and 8 bit transfers, we need to setup pointer to PUSHR/POPR correctly
	_dmaTX->TCD->ATTR_DST = 1;
	_dmaRX->TCD->ATTR_SRC = 1;
	if (buf) {
		dma_first_word = *buf;
		_dmaTX->sourceBuffer((uint16_t*)buf+1, (count-1)*2);  
		_dmaTX->TCD->SLAST = 0;	// Finish with it pointing to next location
	} else {
		dma_first_word = _transferWriteFill;
		_dmaTX->source((uint16_t&)_transferWriteFill);   // maybe have setable value
		DMAChanneltransferCount(_dmaTX, count-1);
	}	
	if (retbuf) {
		// On T3.5 must handle SPI1/2 differently as only one DMA channel

		_dmaRX->destinationBuffer((uint16_t*)retbuf, count*2);
		_dmaRX->TCD->DLASTSGA = 0;	// Finish with it pointing to next location
	} else {
			// Write  only mode
		_dmaRX->destination((uint16_t&)bit_bucket16);
		DMAChanneltransferCount(_dmaRX, count);
	}

	_dma_callback = callback;

	// Now try to start it?
	// Setup DMA main object
#if defined(SPI_DEBUG_ASYNC_T3X) && defined(SPI_DEBUG_VERBOSE)
	Serial.printf("DMA16 %x %x count: %d,  dump TCDs %d %d\n", (uint32_t)buf, (uint32_t)retbuf, count, _dmaTX->channel,  _dmaRX->channel);
	dumpDMA_TCD("TX:",_dmaTX);
	dumpDMA_TCD("RX:",_dmaRX);
#endif
	// Hack - It appears that for DMA SPI on T3.2 and T3.6, that they only work with CTAR 0... SO make
	// CTAR0 16 bit
	port().CTAR0 |= SPI_CTAR_FMSZ(8); 	// Hack convert from 8 bit to 16 bit...

	port().MCR = SPI_MCR_MSTR | SPI_MCR_CLR_RXF | SPI_MCR_PCSIS(0x1F);

	port().SR = 0xFF0F0000;

	// Lets try to output the first byte to make sure that we are in 16 bit mode...
	port().PUSHR = dma_first_word | SPI_PUSHR_CTAS(0) | SPI_PUSHR_CONT;	

	if (hardware().tx_dma_channel) {
		port().RSER =  SPI_RSER_RFDF_RE | SPI_RSER_RFDF_DIRS | SPI_RSER_TFFF_RE | SPI_RSER_TFFF_DIRS;
		_dmaRX->enable();
		// Get the initial settings. 
		_dmaTX->enable();
	} else {
		//T3.5 SP1 and SPI2 - TX is not triggered by SPI but by RX...
		port().RSER =  SPI_RSER_RFDF_RE | SPI_RSER_RFDF_DIRS ;
	    _dmaTX->triggerAtTransfersOf(*_dmaRX);
		_dmaTX->enable();
		_dmaRX->enable();
	}

	_dma_state = DMAState::active;
	return true;
}


void SPIClass::flush(void) 
{
	while (_dma_state == DMAState::active) {
		yield();
	}
}


bool SPIClass::done(void)
{
	return (_dma_state != DMAState::active);
}

//-------------------------------------------------------------------------
// DMA RX ISR
//-------------------------------------------------------------------------
void SPIClass::dma_rxisr(void) {
//  Serial.println("_spi_dma_rxISR");
//	digitalWriteFast(2, HIGH);
#if defined(SPI_DEBUG_ASYNC_T3X) && defined(SPI_DEBUG_VERBOSE)
	Serial.printf("DMA end dump TCDs %d %d Remaining: %d\n", _dmaTX->channel,  _dmaRX->channel, _dma_count_remaining);
	dumpDMA_TCD("TX:",_dmaTX);
	dumpDMA_TCD("RX:",_dmaRX);
#endif

	_dmaRX->clearInterrupt();
	_dmaTX->clearComplete();
	_dmaRX->clearComplete();

	uint8_t should_reenable_tx = true;	// should we re-enable TX maybe not if count will be 0...
	if (_dma_count_remaining) {
		// What do I need to do to start it back up again...
		// We will use the BITR/CITR from RX as TX may have prefed some stuff
		if (_dma_count_remaining > hardware().max_dma_count) {
			// Count does not need to be updated...
//			_dmaTX->transferCount(hardware().max_dma_count);
//			_dmaRX->transferCount(hardware().max_dma_count);
			_dma_count_remaining -= hardware().max_dma_count;
		} else {
			DMAChanneltransferCount(_dmaTX, _dma_count_remaining-1);
			DMAChanneltransferCount(_dmaRX, _dma_count_remaining);
			if (_dma_count_remaining == 1) should_reenable_tx = false;

			_dma_count_remaining = 0;
		}
		// In some cases we need to again start the TX manually to get it to work...
		if (_dmaTX->TCD->SADDR == &_transferWriteFill) {
			if (port().CTAR0  & SPI_CTAR_FMSZ(8)) {
				port().PUSHR = (_transferWriteFill | SPI_PUSHR_CTAS(0) | SPI_PUSHR_CONT);
			} else  {
				port().PUSHR = ((_transferWriteFill & 0xff) | SPI_PUSHR_CTAS(0) | SPI_PUSHR_CONT);
			}
		} else {
			if (port().CTAR0  & SPI_CTAR_FMSZ(8)) {
				// 16 bit mode
				uint16_t w = *((uint16_t*)_dmaTX->TCD->SADDR);
				_dmaTX->TCD->SADDR = (volatile uint8_t*)(_dmaTX->TCD->SADDR) + 2;
				port().PUSHR = (w | SPI_PUSHR_CTAS(0) | SPI_PUSHR_CONT);
			} else  {
				uint8_t w = *((uint8_t*)_dmaTX->TCD->SADDR);
				_dmaTX->TCD->SADDR = (volatile uint8_t*)(_dmaTX->TCD->SADDR) + 1;
				port().PUSHR = (w | SPI_PUSHR_CTAS(0) | SPI_PUSHR_CONT);
			}
		}
		_dmaRX->enable();
		if (should_reenable_tx)
			_dmaTX->enable();
#if defined(SPI_DEBUG_ASYNC_T3X) && defined(SPI_DEBUG_VERBOSE)
		Serial.printf("DMA end restart dump TCDs %d %d Remaining: %d RSER: %x SR:%x MCR: %x\n", _dmaTX->channel,  _dmaRX->channel, 
			_dma_count_remaining, port().RSER, port().SR, port().MCR);
		dumpDMA_TCD("TX:",_dmaTX);
		dumpDMA_TCD("RX:",_dmaRX);
#endif
	} else {

		port().RSER = 0;
		//port().MCR = SPI_MCR_MSTR | SPI_MCR_CLR_RXF | SPI_MCR_PCSIS(0x1F);  // clear out the queue
		port().SR = 0xFF0F0000;
		port().CTAR0  &= ~(SPI_CTAR_FMSZ(8)); 	// Hack restore back to 8 bits

		_dma_state = DMAState::completed;   // set back to 1 in case our call wants to start up dma again
		if (_dma_callback)
			(*_dma_callback)();
	}

//	digitalWriteFast(2, LOW);
}



//=============================================================================

uint8_t SPIClass::pinIsChipSelect(uint8_t pin)
{
	for (uint8_t i = 0; i < sizeof(hardware().cs_pin); i++) {
		if  (pin == hardware().cs_pin[i] ) return hardware().cs_mask[i];
	}
    return 0;
}

bool SPIClass::pinIsChipSelect(uint8_t pin1, uint8_t pin2)
{
	uint8_t pin1_mask, pin2_mask;
	if ((pin1_mask = (uint8_t)pinIsChipSelect(pin1)) == 0) return false;
	if ((pin2_mask = (uint8_t)pinIsChipSelect(pin2)) == 0) return false;
	//Serial.printf("pinIsChipSelect %d %d %x %x\n\r", pin1, pin2, pin1_mask, pin2_mask);
	if ((pin1_mask & pin2_mask) != 0) return false;
	return true;
}

// setCS() is not intended for use from normal Arduino programs/sketches.
uint8_t SPIClass::setCS(uint8_t pin)
{
	for (uint8_t i = 0; i < sizeof(hardware().cs_pin); i++) {
		if  (pin == hardware().cs_pin[i] ) {
			volatile uint32_t *reg = portConfigRegister(hardware().cs_pin[i]);
			uint32_t mux = PORT_PCR_MUX(hardware().cs_mux[i]);
			*reg = mux;

			return hardware().cs_mask[i];
		}
	}
	return 0;
}

void SPIClass::setMOSI(uint8_t pin) {
	if (pin == hardware().mosi_pin[mosi_pin_index]) return;

	for (uint8_t i = 0; i < sizeof(hardware().mosi_pin); i++) {
		if  (pin == hardware().mosi_pin[i] ) {
			mosi_pin_index = i;
			return;
		}
	}
}

void SPIClass::setMISO(uint8_t pin) {
	if (pin == hardware().miso_pin[miso_pin_index]) return;

	for (uint8_t i = 0; i < sizeof(hardware().miso_pin); i++) {
		if  (pin == hardware().miso_pin[i] ) {
			miso_pin_index = i;
			return;
		}
	}
}

void SPIClass::setSCK(uint8_t pin) {
	if (pin == hardware().sck_pin[sck_pin_index]) return;

	for (uint8_t i = 0; i < sizeof(hardware().sck_pin); i++) {
		if  (pin == hardware().sck_pin[i] ) {
			sck_pin_index = i;
			return;
		}
	}
}	

bool SPIClass::pinIsMISO(uint8_t pin) {
	for (uint8_t i = 0; i < sizeof(hardware().miso_pin); i++) {
		if  (pin == hardware().miso_pin[i] ) return true;
	}
	return false;
}

bool SPIClass::pinIsMOSI(uint8_t pin) {
	for (uint8_t i = 0; i < sizeof(hardware().mosi_pin); i++) {
		if  (pin == hardware().mosi_pin[i] ) return true;
	}
	return false;
}

bool SPIClass::pinIsSCK(uint8_t pin) {
	for (uint8_t i = 0; i < sizeof(hardware().sck_pin); i++) {
		if  (pin == hardware().sck_pin[i] ) return true;
	}
	return false;
}



/**********************************************************/
/*     32 bit Teensy-LC                                   */
/**********************************************************/

#elif defined(__arm__) && defined(TEENSYDUINO) && defined(KINETISL)

// Now define our down objects
extern void _spi_dma_rxISR0(void);
const SPIClass::SPI_Hardware_t SPIClass::spi0_hardware = {
	SIM_SCGC4, SIM_SCGC4_SPI0,
	0, // BR index 0
	DMAMUX_SOURCE_SPI0_TX, DMAMUX_SOURCE_SPI0_RX, _spi_dma_rxISR0,
	12, 8,
	2, 2,
	11, 7,
	2, 2,
	13, 14,
	2, 2,
	10, 2,
	2,  2,
	0x1, 0x1
};
SPIClass SPI((uintptr_t)&KINETISL_SPI0, (uintptr_t)&SPIClass::spi0_hardware);

extern void _spi_dma_rxISR1(void);
const SPIClass::SPI_Hardware_t SPIClass::spi1_hardware = {
	SIM_SCGC4, SIM_SCGC4_SPI1,
	1, // BR index 1 in SPI Settings
	DMAMUX_SOURCE_SPI1_TX, DMAMUX_SOURCE_SPI1_RX, _spi_dma_rxISR1,
	1, 5,
	2, 2,
	0, 21,
	2, 2,
	20, 255,
	2, 0,
	6, 255,
	2,  0,
	0x1, 0
};
SPIClass SPI1((uintptr_t)&KINETISL_SPI1, (uintptr_t)&SPIClass::spi1_hardware);


void SPIClass::begin()
{
	volatile uint32_t *reg;

	hardware().clock_gate_register |= hardware().clock_gate_mask;
	port().C1 = SPI_C1_SPE | SPI_C1_MSTR;
	port().C2 = 0;
	uint8_t tmp __attribute__((unused)) = port().S;
	reg = portConfigRegister(hardware().mosi_pin[mosi_pin_index]);
	*reg = PORT_PCR_MUX(hardware().mosi_mux[mosi_pin_index]);
	reg = portConfigRegister(hardware().miso_pin[miso_pin_index]);
	*reg = PORT_PCR_MUX(hardware().miso_mux[miso_pin_index]);
	reg = portConfigRegister(hardware().sck_pin[sck_pin_index]);
	*reg = PORT_PCR_MUX(hardware().sck_mux[sck_pin_index]);
}

void SPIClass::end() {
	volatile uint32_t *reg;
	if (_dmaTX != nullptr) {
		delete _dmaTX;
		_dmaTX = nullptr;
		delete _dmaRX;
		_dmaRX = nullptr;
		_dma_state = DMAState::notAllocated;
	}
	reg = portConfigRegister(hardware().mosi_pin[mosi_pin_index]);
	*reg = 0;
	reg = portConfigRegister(hardware().miso_pin[miso_pin_index]);
	*reg = 0;
	reg = portConfigRegister(hardware().sck_pin[sck_pin_index]);
	*reg = 0;
	port().C1 = 0;
}

#if 1 //def  ALL_IN_ONE_TRANSFER
void SPIClass::transfer(const void * buf, void * retbuf, size_t count) {
	if (count == 0) return;
	const uint8_t *p = (const uint8_t *)buf;
	uint8_t *pret = (uint8_t *)retbuf;
	uint8_t in;

	while (!(port().S & SPI_S_SPTEF)) ; // wait
	uint8_t out = p ? *p++ : (_transferWriteFill & 0xff);
	port().DL = out;
	while (--count > 0) {
		if (p) {
			out = *p++;
		}
		while (!(port().S & SPI_S_SPTEF)) ; // wait
		__disable_irq();
		port().DL = out;
		while (!(port().S & SPI_S_SPRF)) ; // wait
		in = port().DL;
		__enable_irq();
		if (pret)*pret++ = in;
	}
	while (!(port().S & SPI_S_SPRF)) ; // wait
	in = port().DL;
	if (pret)*pret = in;
}
#else
// Suggested version by Thomas Roell - with my own minor tweaks. 
void SPIClass::transfer(const void * buf, void * retbuf, uint32_t count) {
    const uint8_t *tx_data = (const uint8_t *)buf;
    uint8_t *rx_data = (uint8_t *)retbuf;

	if (count == 0) return;


    if (tx_data) {
        if (rx_data) {
        	// Transfer mode
            if (count <= 2) {
                if (count == 2) {
                    port().DL = *tx_data++;
                    while (!(port().S & SPI_S_SPTEF));
                    port().DL = *tx_data++;
                    while (!(port().S & SPI_S_SPRF));
                    *rx_data++ = port().DL;
                    while (!(port().S & SPI_S_SPRF));
                    *rx_data++ = port().DL;
                } else {  // count == 1
                    port().DL = *tx_data++;
                    while (!(port().S & SPI_S_SPRF));
                    *rx_data++ = port().DL;
                } 
            } else {
                const uint8_t *tx_data_e = tx_data + count;

                port().DL = *tx_data++;
                while (!(port().S & SPI_S_SPTEF));
                port().DL = *tx_data++;
                
                do {
                    while (!(port().S & SPI_S_SPRF));
                    *rx_data++ = port().DL;
                    port().DL = *tx_data++;
                } while (tx_data != tx_data_e);
                
                while (!(port().S & SPI_S_SPRF));
                *rx_data++ = port().DL;
                while (!(port().S & SPI_S_SPRF));
                *rx_data++ = port().DL;
            }
        } else {
        	// Write mode
            uint8_t rx_dummy __attribute__((unused));

            if (count <= 2) {
                if (count == 2) {
                    port().DL = *tx_data++;
                    while (!(port().S & SPI_S_SPTEF));
                    port().DL = *tx_data++;
                    while (!(port().S & SPI_S_SPRF));
                    rx_dummy = port().DL;
                    while (!(port().S & SPI_S_SPRF));
                    rx_dummy = port().DL;
                } else  {
                    port().DL = *tx_data++;
                    while (!(port().S & SPI_S_SPRF));
                    rx_dummy = port().DL;
                }
            } else {
                const uint8_t *tx_data_e = tx_data + count;

                port().DL = *tx_data++;
                while (!(port().S & SPI_S_SPTEF));
                port().DL = *tx_data++;
                
                do {
                    while (!(port().S & SPI_S_SPRF));
                    rx_dummy = port().DL;
                    port().DL = *tx_data++;
                } while (tx_data != tx_data_e);
                
                while (!(port().S & SPI_S_SPRF));
                rx_dummy = port().DL;
                while (!(port().S & SPI_S_SPRF));
                rx_dummy = port().DL;
            }
        }
    } else {
        // Read mode
        const uint8_t tx_default = 0x00;

        if (count <= 2) {
            if (count == 2) {
                port().DL = tx_default;
                while (!(port().S & SPI_S_SPTEF));
                port().DL = tx_default;
                while (!(port().S & SPI_S_SPRF));
                *rx_data++ = port().DL;
                while (!(port().S & SPI_S_SPRF));
                *rx_data++ = port().DL;
            } else {
                port().DL = tx_default;
                while (!(port().S & SPI_S_SPRF));
                *rx_data++ = port().DL;
            }
        } else {
            uint8_t *rx_data_e = rx_data + count - 2;

            port().DL = tx_default;
            while (!(port().S & SPI_S_SPTEF));
            port().DL = tx_default;
            
            do {
                while (!(port().S & SPI_S_SPRF));
                *rx_data++ = port().DL;
                port().DL = tx_default;
            } while (rx_data != rx_data_e);
                
            while (!(port().S & SPI_S_SPRF));
            *rx_data++ = port().DL;
            while (!(port().S & SPI_S_SPRF));
            *rx_data++ = port().DL;
        }
    }
}
#endif

void SPIClass::transfer16(const uint16_t * buf, uint16_t * retbuf, size_t count) {
	if (count == 0) return;

	uint16_t in;


	while (!(port().S & SPI_S_SPTEF)) ; // wait
	uint16_t out = buf ? *buf++ : _transferWriteFill;

	port().C2 = SPI_C2_SPIMODE;	// Set to 16 bit mode
	port().S;

	port().DL = out;
	port().DH = out >> 8;

	while (--count > 0) {
		if (buf) {
			out = *buf++;
		}
		while (!(port().S & SPI_S_SPTEF)) ; // wait
		__disable_irq();
		port().DL = out;
		port().DH = out >> 8;
		while (!(port().S & SPI_S_SPRF)) ; // wait
		in =  port().DL | (port().DH << 8);
		__enable_irq();
		if (retbuf)*retbuf++ = in;
	}
	while (!(port().S & SPI_S_SPRF)) ; // wait
	in = port().DL | (port().DH << 8);
	if (retbuf)*retbuf = in;
	port().C2 = 0;	// turn off 16 bit mode. 
	port().S;
}




//=============================================================================
// ASYNCH Support
//=============================================================================
//=========================================================================
// Try Transfer using DMA.
//=========================================================================
static uint8_t      _dma_dummy_rx;


void _spi_dma_rxISR0(void) {SPI.dma_isr();}
void _spi_dma_rxISR1(void) {SPI1.dma_isr();}

void SPIClass::dma_isr(void) {
	//  Serial.println("_spi_dma_rxISR");
	_dmaRX->clearInterrupt();
	port().C2 = 0;
	uint8_t tmp __attribute__((unused)) = port().S;
	_dmaTX->clearComplete();
	_dmaRX->clearComplete();

	_dma_state = DMAState::completed;   // set back to 1 in case our call wants to start up dma again
	if (_dma_callback)
		(*_dma_callback)();
}

bool SPIClass::initDMAChannels() {
	//Serial.println("First dma call"); Serial.flush();
	_dmaTX = new DMAChannel();
	if (_dmaTX == nullptr) {
		return false;
	}

	_dmaTX->disable();
	_dmaTX->destination((volatile uint8_t&)port().DL);
	_dmaTX->disableOnCompletion();
	_dmaTX->triggerAtHardwareEvent(hardware().tx_dma_channel);


	_dmaRX = new DMAChannel();
	if (_dmaRX == NULL) {
		delete _dmaTX;
		_dmaRX = nullptr;
		return false;
	}
	_dmaRX->disable();
	_dmaRX->source((volatile uint8_t&)port().DL);
	_dmaRX->disableOnCompletion();
	_dmaRX->triggerAtHardwareEvent(hardware().rx_dma_channel);
	_dmaRX->attachInterrupt(hardware().dma_isr);
	_dmaRX->interruptAtCompletion();

	_dma_state = DMAState::idle;  // Should be first thing set!
	//Serial.println("end First dma call");
	return true;
}

bool SPIClass::transfer(const void *buf, void *retbuf, size_t count, void(*callback)(void))
{
	if (_dma_state == DMAState::notAllocated) {
		if (!initDMAChannels()) {
			return false;
		}
	}
  
	if (_dma_state == DMAState::active)
		return false; // already active

	//_dmaTX->destination((volatile uint8_t&)port().DL);
	//_dmaRX->source((volatile uint8_t&)port().DL);
	_dmaTX->CFG->DCR = (_dmaTX->CFG->DCR & ~DMA_DCR_DSIZE(3)) | DMA_DCR_DSIZE(1);
	_dmaRX->CFG->DCR = (_dmaRX->CFG->DCR & ~DMA_DCR_SSIZE(3)) | DMA_DCR_SSIZE(1);  // 8 bit transfer

	// Now see if the user passed in TX buffer to send.
	uint8_t first_char;
	if (buf) {
		uint8_t *data_out = (uint8_t*)buf;
		first_char = *data_out++;
		_dmaTX->sourceBuffer(data_out, count-1);
	} else {
		first_char = (_transferWriteFill & 0xff);
		_dmaTX->source((uint8_t&)_transferWriteFill);   // maybe have setable value
		_dmaTX->transferCount(count-1);
	}

	if (retbuf) {
		_dmaRX->destinationBuffer((uint8_t*)retbuf, count);
	} else {
		_dmaRX->destination(_dma_dummy_rx);    // NULL ?
		_dmaRX->transferCount(count);
	}
	_dma_callback = callback;

	//Serial.println("Before DMA C2");
	// Try pushing the first character
    while (!(port().S & SPI_S_SPTEF));
    port().DL = first_char;

	port().C2 |= SPI_C2_TXDMAE | SPI_C2_RXDMAE;

	// Now  make sure SPI is enabled. 
	port().C1 |= SPI_C1_SPE;
      
    _dmaRX->enable();
    _dmaTX->enable();
    _dma_state = DMAState::active;
    return true;
}

bool SPIClass::transfer16(const uint16_t * buf, uint16_t * retbuf, size_t count, void(*callback)(void)) 
{
	if (_dma_state == DMAState::notAllocated) {
		if (!initDMAChannels()) {
			return false;
		}
	}
  
	if (_dma_state == DMAState::active)
		return false; // already active

	port().C2 = SPI_C2_SPIMODE;	// turn on 16 bit mode
	port().S;

	// maybe hack up the transfer size?
	//_dmaTX->destination((volatile uint8_t&)port().DL);
	//_dmaRX->source((volatile uint8_t&)port().DL);
	_dmaTX->CFG->DCR = (_dmaTX->CFG->DCR & ~DMA_DCR_DSIZE(3)) | DMA_DCR_DSIZE(2);
	_dmaRX->CFG->DCR = (_dmaRX->CFG->DCR & ~DMA_DCR_SSIZE(3)) | DMA_DCR_SSIZE(2);  // 16 bit transfer

	// Now handle NULL pointers.
	uint16_t first_word;
	if (buf) {
		first_word = *buf++;
		_dmaTX->sourceBuffer(buf, (count-1)*2);
	} else {
		first_word = _transferWriteFill;
		_dmaTX->source(_transferWriteFill);   // maybe have setable value
		_dmaTX->transferCount((count-1)*2);
	}

	if (retbuf) {
		_dmaRX->destinationBuffer(retbuf, count*2);
	} else {
		_dmaRX->destination(_dma_dummy_rx);    // NULL ?
		_dmaRX->transferCount(count*2);
	}
	_dma_callback = callback;

	//Serial.println("Before DMA C2");
	// Try pushing the first character
    while (!(port().S & SPI_S_SPTEF));
    port().DL = first_word & 0xff;
    port().DH = first_word >> 8;

	port().C2 |= SPI_C2_TXDMAE | SPI_C2_RXDMAE;

	// Now  make sure SPI is enabled. 
	port().C1 |= SPI_C1_SPE;
      
    _dmaRX->enable();
    _dmaTX->enable();
    _dma_state = DMAState::active;
    return true;
}


void SPIClass::flush(void) 
{
	while (_dma_state == DMAState::active) {
		yield();
	}
}


bool SPIClass::done(void)
{
	return (_dma_state == DMAState::completed);
}

#ifdef SPI_DEBUG_ASYNC_LC
void dumpDMA_CFG(const char* psz, DMABaseClass *dmabc)
{
	uint32_t* cfg = (uint32_t*)dmabc->CFG;
	Serial.printf("%s %08x %08x: %08x %08x %08x %08x\n", psz, (uint32_t)dmabc, (uint32_t)dmabc->CFG, 
			cfg[0], cfg[1], cfg[2], cfg[3] );
}
#endif

// SPI functions only change when we are not running...
void SPIClass::setMOSI(uint8_t pin) {
	if (pin == hardware().mosi_pin[mosi_pin_index]) return;

	for (uint8_t i = 0; i < sizeof(hardware().mosi_pin); i++) {
		if  (pin == hardware().mosi_pin[i] ) {
			mosi_pin_index = i;
			return;
		}
	}
}

void SPIClass::setMISO(uint8_t pin) {
	if (pin == hardware().miso_pin[miso_pin_index]) return;

	for (uint8_t i = 0; i < sizeof(hardware().miso_pin); i++) {
		if  (pin == hardware().miso_pin[i] ) {
			miso_pin_index = i;
			return;
		}
	}
}

void SPIClass::setSCK(uint8_t pin) {
	if (pin == hardware().sck_pin[sck_pin_index]) return;

	for (uint8_t i = 0; i < sizeof(hardware().sck_pin); i++) {
		if  (pin == hardware().sck_pin[i] ) {
			sck_pin_index = i;
			return;
		}
	}
}	

bool SPIClass::pinIsMISO(uint8_t pin) {
	for (uint8_t i = 0; i < sizeof(hardware().miso_pin); i++) {
		if  (pin == hardware().miso_pin[i] ) return true;
	}
	return false;
}

bool SPIClass::pinIsMOSI(uint8_t pin) {
	for (uint8_t i = 0; i < sizeof(hardware().mosi_pin); i++) {
		if  (pin == hardware().mosi_pin[i] ) return true;
	}
	return false;
}

bool SPIClass::pinIsSCK(uint8_t pin) {
	for (uint8_t i = 0; i < sizeof(hardware().sck_pin); i++) {
		if  (pin == hardware().sck_pin[i] ) return true;
	}
	return false;
}


const uint16_t SPISettings::br_div_table[30] = {
	2, 4, 6, 8, 10, 12, 14, 16, 20, 24,
	28, 32, 40, 48, 56, 64, 80, 96, 112, 128,
	160, 192, 224, 256, 320, 384, 448, 512, 640, 768,
};

const uint8_t SPISettings::br_clock_table[30] = {
	SPI_BR_SPPR(0) | SPI_BR_SPR(0),
	SPI_BR_SPPR(1) | SPI_BR_SPR(0),
	SPI_BR_SPPR(2) | SPI_BR_SPR(0),
	SPI_BR_SPPR(3) | SPI_BR_SPR(0),
	SPI_BR_SPPR(4) | SPI_BR_SPR(0),
	SPI_BR_SPPR(5) | SPI_BR_SPR(0),
	SPI_BR_SPPR(6) | SPI_BR_SPR(0),
	SPI_BR_SPPR(7) | SPI_BR_SPR(0),
	SPI_BR_SPPR(4) | SPI_BR_SPR(1),
	SPI_BR_SPPR(5) | SPI_BR_SPR(1),
	SPI_BR_SPPR(6) | SPI_BR_SPR(1),
	SPI_BR_SPPR(7) | SPI_BR_SPR(1),
	SPI_BR_SPPR(4) | SPI_BR_SPR(2),
	SPI_BR_SPPR(5) | SPI_BR_SPR(2),
	SPI_BR_SPPR(6) | SPI_BR_SPR(2),
	SPI_BR_SPPR(7) | SPI_BR_SPR(2),
	SPI_BR_SPPR(4) | SPI_BR_SPR(3),
	SPI_BR_SPPR(5) | SPI_BR_SPR(3),
	SPI_BR_SPPR(6) | SPI_BR_SPR(3),
	SPI_BR_SPPR(7) | SPI_BR_SPR(3),
	SPI_BR_SPPR(4) | SPI_BR_SPR(4),
	SPI_BR_SPPR(5) | SPI_BR_SPR(4),
	SPI_BR_SPPR(6) | SPI_BR_SPR(4),
	SPI_BR_SPPR(7) | SPI_BR_SPR(4),
	SPI_BR_SPPR(4) | SPI_BR_SPR(5),
	SPI_BR_SPPR(5) | SPI_BR_SPR(5),
	SPI_BR_SPPR(6) | SPI_BR_SPR(5),
	SPI_BR_SPPR(7) | SPI_BR_SPR(5),
	SPI_BR_SPPR(4) | SPI_BR_SPR(6),
	SPI_BR_SPPR(5) | SPI_BR_SPR(6)
};

// setCS() is not intended for use from normal Arduino programs/sketches.
bool SPIClass::pinIsChipSelect(uint8_t pin) 
{
	for (uint8_t i = 0; i < sizeof(hardware().cs_pin); i++) {
		if  (pin == hardware().cs_pin[i] ) return hardware().cs_mask[i];
	}
    return 0;
}


uint8_t SPIClass::setCS(uint8_t pin)
{
	for (uint8_t i = 0; i < sizeof(hardware().cs_pin); i++) {
		if  (pin == hardware().cs_pin[i] ) {
			volatile uint32_t *reg = portConfigRegister(hardware().cs_pin[i]);
			uint32_t mux = PORT_PCR_MUX(hardware().cs_mux[i]);
			*reg = mux;

			return hardware().cs_mask[i];
		}
	}
	return 0;
}


#endif
