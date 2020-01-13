#include <stdint.h>

#include "fsl_spi_master_driver.h"
#include "fsl_port_hal.h"

#include "SEGGER_RTT.h"
#include "gpio_pins.h"
#include "warp.h"
#include "devSSD1331.h"

volatile uint8_t	inBuffer[32];
volatile uint8_t	payloadBytes[32];


/*
 *	Override Warp firmware's use of these pins and define new aliases.
 */
enum
{
	kSSD1331PinMOSI		= GPIO_MAKE_PIN(HW_GPIOA, 8),
	kSSD1331PinSCK		= GPIO_MAKE_PIN(HW_GPIOA, 9),
	kSSD1331PinCSn		= GPIO_MAKE_PIN(HW_GPIOA, 12),
	kSSD1331PinDC		= GPIO_MAKE_PIN(HW_GPIOB, 13),
	kSSD1331PinRST		= GPIO_MAKE_PIN(HW_GPIOA, 2),
};

static int
writeCommand(uint8_t commandByte)
{
	spi_status_t status;

	/*
	 *	Drive /CS low.
	 *
	 *	Make sure there is a high-to-low transition by first driving high, delay, then drive low.
	 */
	GPIO_DRV_SetPinOutput(kSSD1331PinCSn);
	OSA_TimeDelay(10);
	GPIO_DRV_ClearPinOutput(kSSD1331PinCSn);

	/*
	 *	Drive DC low (command).
	 */
	GPIO_DRV_ClearPinOutput(kSSD1331PinDC);

	payloadBytes[0] = commandByte;
	status = SPI_DRV_MasterTransferBlocking(0	/* master instance */,
					NULL		/* spi_master_user_config_t */,
					(const uint8_t * restrict)&payloadBytes[0],
					(uint8_t * restrict)&inBuffer[0],
					1		/* transfer size */,
					1000		/* timeout in microseconds (unlike I2C which is ms) */);

	/*
	 *	Drive /CS high
	 */
	GPIO_DRV_SetPinOutput(kSSD1331PinCSn);

	return status;
}



int
devSSD1331init(void)
{
	/*
	 *	Override Warp firmware's use of these pins.
	 *
	 *	Re-configure SPI to be on PTA8 and PTA9 for MOSI and SCK respectively.
	 */
	PORT_HAL_SetMuxMode(PORTA_BASE, 8u, kPortMuxAlt3);
	PORT_HAL_SetMuxMode(PORTA_BASE, 9u, kPortMuxAlt3);

	enableSPIpins();

	/*
	 *	Override Warp firmware's use of these pins.
	 *
	 *	Reconfigure to use as GPIO.
	 */
	PORT_HAL_SetMuxMode(PORTB_BASE, 13u, kPortMuxAsGpio);
	PORT_HAL_SetMuxMode(PORTA_BASE, 12u, kPortMuxAsGpio);
	PORT_HAL_SetMuxMode(PORTB_BASE, 0u, kPortMuxAsGpio);


	/*
	 *	RST high->low->high.
	 */
	GPIO_DRV_SetPinOutput(kSSD1331PinRST);
	OSA_TimeDelay(100);
	GPIO_DRV_ClearPinOutput(kSSD1331PinRST);
	OSA_TimeDelay(100);
	GPIO_DRV_SetPinOutput(kSSD1331PinRST);
	OSA_TimeDelay(100);

	/*
	 *	Initialization sequence, borrowed from https://github.com/adafruit/Adafruit-SSD1331-OLED-Driver-Library-for-Arduino
	 */
	writeCommand(kSSD1331CommandDISPLAYOFF);	// 0xAE
	writeCommand(kSSD1331CommandSETREMAP);		// 0xA0
	writeCommand(0x72);				// RGB Color
	writeCommand(kSSD1331CommandSTARTLINE);		// 0xA1
	writeCommand(0x0);
	writeCommand(kSSD1331CommandDISPLAYOFFSET);	// 0xA2
	writeCommand(0x0);
	writeCommand(kSSD1331CommandNORMALDISPLAY);	// 0xA4
	writeCommand(kSSD1331CommandSETMULTIPLEX);	// 0xA8
	writeCommand(0x3F);				// 0x3F 1/64 duty
	writeCommand(kSSD1331CommandSETMASTER);		// 0xAD
	writeCommand(0x8E);
	writeCommand(kSSD1331CommandPOWERMODE);		// 0xB0
	writeCommand(0x0B);
	writeCommand(kSSD1331CommandPRECHARGE);		// 0xB1
	writeCommand(0x31);
	writeCommand(kSSD1331CommandCLOCKDIV);		// 0xB3
	writeCommand(0xF0);				// 7:4 = Oscillator Frequency, 3:0 = CLK Div Ratio (A[3:0]+1 = 1..16)
	writeCommand(kSSD1331CommandPRECHARGEA);	// 0x8A
	writeCommand(0x64);
	writeCommand(kSSD1331CommandPRECHARGEB);	// 0x8B
	writeCommand(0x78);
	writeCommand(kSSD1331CommandPRECHARGEA);	// 0x8C
	writeCommand(0x64);
	writeCommand(kSSD1331CommandPRECHARGELEVEL);	// 0xBB
	writeCommand(0x3A);
	writeCommand(kSSD1331CommandVCOMH);		// 0xBE
	writeCommand(0x3E);
	writeCommand(kSSD1331CommandMASTERCURRENT);	// 0x87
	writeCommand(0x06);
	writeCommand(kSSD1331CommandCONTRASTA);		// 0x91
	writeCommand(0xFF);
	writeCommand(kSSD1331CommandCONTRASTB);		// 0x50
	writeCommand(0xFF);
	writeCommand(kSSD1331CommandCONTRASTC);		// 0x7D
	writeCommand(0xFF);
	writeCommand(kSSD1331CommandDISPLAYON);		// Turn on oled panel
	// SEGGER_RTT_WriteString(0, "\r\n\tDone with initialization sequence...\n");

	/*
	 *	To use fill commands, you will have to issue a command to the display to enable them. See the manual.
	 */
	writeCommand(kSSD1331CommandFILL);
	writeCommand(0x01);
	// SEGGER_RTT_WriteString(0, "\r\n\tDone with enabling fill...\n");

	/*
	 *	Clear Screen
	 */
	writeCommand(kSSD1331CommandCLEAR);
	writeCommand(0x00);
	writeCommand(0x00);
	writeCommand(0x5F);
	writeCommand(0x3F);
	// SEGGER_RTT_WriteString(0, "\r\n\tDone with screen clear...\n");



	/*
	 *	Read the manual for the SSD1331 (SSD1331_1.2.pdf) to figure
	 *	out how to fill the entire screen with the brightest shade
	 *	of green.
	 */
    writeCommand(kSSD1331CommandDRAWLINE);
    writeCommand(0x19); // Column Address of Start (0 ~ 0x5F)
    writeCommand(0x22); // Row Address of Start (0 ~ 0x3F)
    writeCommand(0x28); // Column Address of End (0 ~ 0x5F)
    writeCommand(0x36); // Row Address of End (0 ~ 0x3F)
    writeCommand(0x00); // Color C of the line
    writeCommand(0xFF); // Color B of the line
    writeCommand(0x00); // Color A of the line

    writeCommand(kSSD1331CommandDRAWLINE);
    writeCommand(0x28); // Column Address of Start (0 ~ 0x5F)
    writeCommand(0x36); // Row Address of Start (0 ~ 0x3F)
    writeCommand(0x54); // Column Address of End (0 ~ 0x5F)
    writeCommand(0x08); // Row Address of End (0 ~ 0x3F)
    writeCommand(0x00); // Color C of the line
    writeCommand(0xFF); // Color B of the line
    writeCommand(0x00); // Color A of the line

    writeCommand(kSSD1331CommandDRAWLINE);
    writeCommand(0x54); // Column Address of Start (0 ~ 0x5F)
    writeCommand(0x08); // Row Address of Start (0 ~ 0x3F)
    writeCommand(0x57); // Column Address of End (0 ~ 0x5F)
    writeCommand(0x0B); // Row Address of End (0 ~ 0x3F)
    writeCommand(0x00); // Color C of the line
    writeCommand(0xFF); // Color B of the line
    writeCommand(0x00); // Color A of the line

    writeCommand(kSSD1331CommandDRAWLINE);
    writeCommand(0x57); // Column Address of Start (0 ~ 0x5F)
    writeCommand(0x0B); // Row Address of Start (0 ~ 0x3F)
    writeCommand(0x28); // Column Address of End (0 ~ 0x5F)
    writeCommand(0x3C); // Row Address of End (0 ~ 0x3F)
    writeCommand(0x00); // Color C of the line
    writeCommand(0xFF); // Color B of the line
    writeCommand(0x00); // Color A of the line

    writeCommand(kSSD1331CommandDRAWLINE);
    writeCommand(0x28); // Column Address of Start (0 ~ 0x5F)
    writeCommand(0x3C); // Row Address of Start (0 ~ 0x3F)
    writeCommand(0x16); // Column Address of End (0 ~ 0x5F)
    writeCommand(0x24); // Row Address of End (0 ~ 0x3F)
    writeCommand(0x00); // Color C of the line
    writeCommand(0xFF); // Color B of the line
    writeCommand(0x00); // Color A of the line

    writeCommand(kSSD1331CommandDRAWLINE);
    writeCommand(0x16); // Column Address of Start (0 ~ 0x5F)
    writeCommand(0x24); // Row Address of Start (0 ~ 0x3F)
    writeCommand(0x19); // Column Address of End (0 ~ 0x5F)
    writeCommand(0x22); // Row Address of End (0 ~ 0x3F)
    writeCommand(0x00); // Color C of the line
    writeCommand(0xFF); // Color B of the line
    writeCommand(0x00); // Color A of the line

	SEGGER_RTT_WriteString(0, "\r\n\tDone with displaying tick\n");



	return 0;
}

int
devSSD1331TurnRed(void)
{
	/*
	 *	Clear Screen
	 */
	writeCommand(kSSD1331CommandCLEAR);
	writeCommand(0x00);
	writeCommand(0x00);
	writeCommand(0x5F);
	writeCommand(0x3F);
	// SEGGER_RTT_WriteString(0, "\r\n\tDone with screen clear...\n");



	/*
	 *	Read the manual for the SSD1331 (SSD1331_1.2.pdf) to figure
	 *	out how to fill the entire screen with the brightest shade
	 *	of green.
	 */
	writeCommand(kSSD1331CommandDRAWRECT);
	writeCommand(0x2C); // Column Address of Start (0 ~ 0x5F)
	writeCommand(0x06); // Row Address of Start (0 ~ 0x3F)
	writeCommand(0x34); // Column Address of End (0 ~ 0x5F)
	writeCommand(0x28); // Row Address of End (0 ~ 0x3F)
	writeCommand(0xFF); // Color C of the line
	writeCommand(0x00); // Color B of the line
	writeCommand(0x00); // Color A of the line
	writeCommand(0xFF); // Color C of the fill area
	writeCommand(0x00); // Color B of the fill area
	writeCommand(0x00); // Color A of the fill area

	writeCommand(kSSD1331CommandDRAWRECT);
	writeCommand(0x2C); // Column Address of Start (0 ~ 0x5F)
	writeCommand(0x30); // Row Address of Start (0 ~ 0x3F)
	writeCommand(0x34); // Column Address of End (0 ~ 0x5F)
	writeCommand(0x38); // Row Address of End (0 ~ 0x3F)
	writeCommand(0xFF); // Color C of the line
	writeCommand(0x00); // Color B of the line
	writeCommand(0x00); // Color A of the line
	writeCommand(0xFF); // Color C of the fill area
	writeCommand(0x00); // Color B of the fill area
	writeCommand(0x00); // Color A of the fill area

	SEGGER_RTT_WriteString(0, "\r\n\tDone with displaying red exclamation mark\n");

	while(1){
		OSA_TimeDelay(1000);
		writeCommand(kSSD1331CommandINVERTDISPLAY);
		OSA_TimeDelay(1000);
		writeCommand(kSSD1331CommandNORMALDISPLAY);
	}

	return 0;
}


