#include <stdlib.h>

#include "fsl_misc_utilities.h"
#include "fsl_device_registers.h"
#include "fsl_i2c_master_driver.h"
#include "fsl_spi_master_driver.h"
#include "fsl_rtc_driver.h"
#include "fsl_clock_manager.h"
#include "fsl_power_manager.h"
#include "fsl_mcglite_hal.h"
#include "fsl_port_hal.h"

#include "gpio_pins.h"
#include "SEGGER_RTT.h"
#include "warp.h"


extern volatile WarpI2CDeviceState	deviceINA219State;
extern volatile uint32_t		gWarpI2cBaudRateKbps;



void
initINA219(const uint8_t i2cAddress, WarpI2CDeviceState volatile *  deviceStatePointer)
{
	deviceStatePointer->i2cAddress	= i2cAddress;

	return;
}

WarpStatus
readSensorRegisterINA219(uint8_t deviceRegister)
{
	uint8_t cmdBuf[1]	= {0xFF};
	i2c_status_t		status;

	i2c_device_t slave =
	{
		.address = deviceINA219State.i2cAddress,
		.baudRate_kbps = gWarpI2cBaudRateKbps
	};

	cmdBuf[0] = deviceRegister;

	status = I2C_DRV_MasterReceiveDataBlocking(
							0 /* I2C peripheral instance */,
							&slave,
							cmdBuf,
							1,
							(uint8_t *)deviceINA219State.i2cBuffer,
							2,
							500 /* timeout in milliseconds */);

	if (status == kStatus_I2C_Success)
	{
		SEGGER_RTT_printf(0, "\rRegister: 0x%02x --> Value:	[0x%02x,0x%02x]\n", cmdBuf[0], deviceINA219State.i2cBuffer[0], deviceINA219State.i2cBuffer[1]);
	}
	else
	{
		SEGGER_RTT_WriteString(0, "\rRead of INA219 register failed.\n");

		return kWarpStatusDeviceCommunicationFailed;
	}

	return kWarpStatusOK;
}
