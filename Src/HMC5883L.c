/*
 * HMC5883L.c
 *
 *  Created on: 18 сент. 2018 г.
 *      Author: Denis Denk
 */

#include "HMC5883L.h"
#include "i2c.h"

void HMC5883L_Init(void) {

	// Something magic for enable magnetometer HMC5883L on GY-86 module
	//Bypass Mode
	I2C_WriteByte(MPU6050_ADDRESS, INT_PIN_CFG, 0x02);
	HAL_Delay(10);
	I2C_WriteByte(MPU6050_ADDRESS, USER_CTRL, 0x00);
	HAL_Delay(10);
	//Disable Sleep Mode
	I2C_WriteByte(MPU6050_ADDRESS, PWR_MGMT_1, 0x00);
	HAL_Delay(10);

	// Magnetometer HMC5883L Init
	// Continuous measurement mode for Magnetometer
	I2C_WriteByte(HMC5883L_ADDRESS, HMC5883L_RA_MODE, 0x00);
	HAL_Delay(10);
}

uint16_t HMC5883L_GetAngle(void) {

	uint8_t	magBuf[12] = {0};
	int16_t mx, my, mz = 0;
	float angle, x, y, z = 0;


	HAL_I2C_Mem_Read(&hi2c1, (uint16_t)HMC5883L_ADDRESS << 1, HMC5883L_RA_DATAX_H, 1, magBuf, 6, 0x100);

	mx = (int16_t)magBuf[0]<<8 | magBuf[1];
	mz = (int16_t)magBuf[2]<<8 | magBuf[3];
	my = (int16_t)magBuf[4]<<8 | magBuf[5];

	x = mx * 0.92;
	y = my * 0.92;
	z = mz * 0.92;

	// Calculate angle
	angle = atan2(y, x);
	if (angle < 0)
		angle += 2*M_PI;

	if (angle > 2*M_PI)
		angle -= 2*M_PI;

	return (uint16_t)(angle * (180 / M_PI));
}
