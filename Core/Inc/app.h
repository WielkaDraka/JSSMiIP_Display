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

#define CHART_HORIZONTAL_NUMBER	245
#define CHART_VERTICAL_0	40

#define CHART_TEMPERATURE_10 	172
#define CHART_TEMPERATURE_20	112
#define CHART_TEMPERATURE_30	52

#define CHART_TEMP_VERTICAL		10

#define LOG_TEMP_DATA			10
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
void on_lcd_draw(uint32_t addr);

void app_Init(void);
void app_loop(void);

void first_display(uint32_t addr);

void logger_chart(uint16_t x, uint16_t y);
void logger_record(uint16_t x, uint16_t y);

void dashboard_chart(uint32_t addr);
void dashboard_record(uint32_t addr);

#endif /* INC_APP_H_ */
