void initINA219(const uint8_t i2cAddress, WarpI2CDeviceState volatile *  deviceStatePointer);
WarpStatus readSensorRegisterINA219(uint8_t deviceRegister, int numberOfBytes);
void writeSensorRegisterINA219(uint8_t deviceRegister, uint16_t val);
