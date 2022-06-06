/*
 * app.h
 *
 *  Created on: Jun 4, 2022
 *      Author: Lukasz Pyzik
 */

#ifndef INC_APP_H_
#define INC_APP_H_

#include "LCD_Driver/lcd.h"
#include "LCD_Driver/touch.h"
#include "font.h"

// distance from the axis
#define CHART_HORIZONTAL_NUMBER	245
#define CHART_VERTICAL_0		40

// distance to sign the axis y
#define CHART_TEMPERATURE_10 	172
#define CHART_TEMPERATURE_20	112
#define CHART_TEMPERATURE_30	52

// maximum chart points in x axis
#define CHART_TEMP_VERTICAL		10

// maximum temperature data
#define LOG_TEMP_DATA			10

//enum for number in x axis
enum CHART_VERTICAL_NUMBER
{

	CHART_VERTICAL_NUMBER_1,
	CHART_VERTICAL_NUMBER_2,
	CHART_VERTICAL_NUMBER_3,
	CHART_VERTICAL_NUMBER_4,
	CHART_VERTICAL_NUMBER_5,
	CHART_VERTICAL_NUMBER_6,
	CHART_VERTICAL_NUMBER_7,
	CHART_VERTICAL_NUMBER_8,
	CHART_VERTICAL_NUMBER_9,
	CHART_VERTICAL_NUMBER_10,

};

/**
  * @brief  Callback for drawing in the screen. Works at a frequency of 60 Hz
  * @param	addr: 		addres of the layer (background, layer1 or layer2)
  * @retval None
  */
void on_lcd_draw(uint32_t addr);

/**
  * @brief  Initialization function for all peripherals
  * @retval None
  */
void app_Init(void);

/**
  * @brief  Infinite program loop
  * @retval None
  */
void app_loop(void);

/**
  * @brief  Displays when starting the device
  * @param	addr:	addres of the layer (background, layer1 or layer2)
  * @retval None
  */
void first_display(uint32_t addr);

/**
  * @brief  Detection of a button press to display chart
  * @param	x, y:	Touched coordinates
  * @retval None
  */
void logger_chart(uint16_t x, uint16_t y);

/**
  * @brief  Detection of a button press to display a single value
  * @param	x, y:	Touched coordinates
  * @retval None
  */
void logger_record(uint16_t x, uint16_t y);

/**
  * @brief  For drawing anything related to chart
  * @param	addr:	addres of the layer (background, layer1 or layer2)
  * @retval None
  */
void dashboard_chart(uint32_t addr);

/**
  * @brief  For drawing anything related to single measurement
  * @param	addr:	addres of the layer (background, layer1 or layer2)
  * @retval None
  */
void dashboard_record(uint32_t addr);

#endif /* INC_APP_H_ */
