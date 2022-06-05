/*
 * LPS25H.h
 *
 *		This library is for sensor AltIMU10 v5
 *		MEMS pressure sensor: 260-1260 hPa absolute digital output barometer:
 *
 *      Author: Lukasz Pyzik
 */

#include "AltIMU10v5/LPS25H.h"
#include "math.h"

LPS25H alititude;

extern I2C_HandleTypeDef hi2c1;

/**
  * @brief  To read the value from register
  * @param  reg: register addres
  * @retval value: return the read value
  */
uint8_t lps25h_read_reg(uint8_t reg)
{
	uint8_t value = 0;

	HAL_I2C_Mem_Read(&hi2c1, LPS25H_H_ADDR, reg, 1, &value, sizeof(value), HAL_MAX_DELAY);

	return value;
}

/**
  * @brief  To write the value to register
  * @param  reg: register addres
  * @param	value: to be written to the registry
  * @retval None
  */
void lps25h_write_reg(uint8_t reg, uint8_t value)
{
	HAL_I2C_Mem_Write(&hi2c1, LPS25H_H_ADDR, reg, 1, &value, sizeof(value), HAL_MAX_DELAY);

}

/**
  * @brief  Initialization for the lps25h sensor
  * @retval None
  */
void lps25h_Init(void)
{
	//PD = 1 (active mode);  ODR = 011 (12.5 Hz pressure & temperature output data rate)
	lps25h_write_reg(LPS25H_CTRL_REG1, 0xB0);
}

/**
  * @brief  Read pressure data from sensor and returns raw 24-bit sensor output
  * @retval None
  */
void lps25h_readPressure(void)
{
	//HAL_I2C_Mem_Read(&hi2c1, LPS25H_H_ADDR, LPS25H_PRESS_OUT_XL , 1, alititude.Buf_p, sizeof(alititude.Buf_p), HAL_MAX_DELAY);
	HAL_I2C_Mem_Read(&hi2c1, LPS25H_H_ADDR, LPS25H_PRESS_OUT_XL | 0x80, 1, (uint8_t*)&alititude.pressure, sizeof(alititude.Buf_p), HAL_MAX_DELAY);

	//alititude.pressure = (alititude.Buf_p[2]<<16) | (alititude.Buf_p[1]<<8) | alititude.Buf_p[0];
}

/**
  * @brief  Reads pressure in inches
  * @retval None
  */
void lps25h_readPressureInchesHg(void)
{
	lps25h_readPressure();
	alititude.pressure_inches = (float)alititude.pressure/138706.5;
}

/**
  * @brief  Reads pressure in milibars (mbar)/hectopasscals (hPa)
  * @retval None
  */
void lps25h_readPressureMillibars(void)
{
	lps25h_readPressure();
	alititude.pressure_milibars = alititude.pressure/4096;
}

/**
  * @brief  Reads temperature and returns raw 16-bit sensor output
  * @retval None
  */
void lps25h_readTemperature(void)
{
	HAL_I2C_Mem_Read(&hi2c1, LPS25H_H_ADDR, LPS25H_TEMP_OUT_L | 0x80, 1, (uint8_t*)&alititude.temperature, sizeof(alititude.Buf_t), HAL_MAX_DELAY);
	//HAL_I2C_Mem_Read(&hi2c1, LPS25H_H_ADDR, LPS25H_TEMP_OUT_L, 1, alititude.Buf_t, sizeof(alititude.Buf_t), HAL_MAX_DELAY);
	//alititude.temperature = (alititude.Buf_t[1]<<8) | alititude.Buf_t[0];
}

/**
  * @brief  Reads temperature in degrees F
  * @retval None
  */
void lps25h_readTemperatureF(void)
{
	lps25h_readTemperature();
	alititude.temperature_F = (float)108.5f + alititude.temperature / 480.f * 1.8f;
}

/**
  * @brief  Reads temperature in degrees C
  * @retval None
  */
void lps25h_readTemperatureC(void)
{
	lps25h_readTemperature();
	alititude.temperature_C = 42.5f + alititude.temperature / 480.0f;
}

/**
  * @brief  Converts pressure in mbar to altitude in meters, using 1976 US
  * Standard Atmosphere model (note that this formula only applies to a
  * height of 11 km, or about 36000 ft)
  * If altimeter setting (QNH, barometric pressure adjusted to sea
  * level) is given, this function returns an indicated altitude
  * compensated for actual regional pressure; otherwise, it returns
  * the pressure altitude above the standard pressure level of 1013.25 mbar
  * @retval None
  */
void lps25h_pressureToAlitudeFeet(void)
{
	lps25h_readPressureInchesHg();
	lps25h_readTemperatureF();
	alititude.altitude_feet = -29.271769 * alititude.temperature_F * log(alititude.pressure_inches/ENVIRONMENT_PRESSURE_I);
}

/**
  * @brief  Converts pressure in mbar to altitude in meters, using 1976 US
  * Standard Atmosphere model (note that this formula only applies to a
  * height of 11 km, or about 36000 ft)
  * If altimeter setting (QNH, barometric pressure adjusted to sea
  * level) is given, this function returns an indicated altitude
  * compensated for actual regional pressure; otherwise, it returns
  * the pressure altitude above the standard pressure level of 29.9213 inHg
  * @retval None
  */
void lps25h_pressureToAlitudeMeters(void)
{
	lps25h_readPressureMillibars();
	lps25h_readTemperatureC();
	alititude.altitude_meters = -29.271769 * alititude.temperature_C * log(alititude.pressure_milibars/ENVIRONMENT_PRESSURE_M);
}
