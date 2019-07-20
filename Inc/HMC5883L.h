/*
 * HMC5883L.h
 *
 *  Created on: 18 сент. 2018 г.
 *      Author: Denis Denk
 */

#ifndef HMC5883L_H_
#define HMC5883L_H_

#include "stdint.h"
#include "math.h"

#include "HMC5883L_RegisterMap.h"


void HMC5883L_Init(void);
uint16_t HMC5883L_GetAngle(void);

#endif /* HMC5883L_H_ */
