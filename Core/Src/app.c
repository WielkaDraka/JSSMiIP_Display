/*
 * app.c
 *
 *  Created on: Jun 4, 2022
 *      Author: Lukasz Pyzik
 */
#include "app.h"

#include "button/button.h"
#include "images/images.h"
#include "AltIMU10v5/LPS25H.h"
#include <stdio.h>

TS_StateTypeDef touch_state;
extern TIM_HandleTypeDef htim6;
extern LPS25H alititude;

enum DisplayState{
	DISPLAY_RECORD,
	DISPLAY_CHART

};

volatile uint8_t state = DISPLAY_RECORD;
uint16_t temperature = 0;
uint16_t temperature_data[LOG_TEMP_DATA] = {0};
uint8_t log_temp = 0;

uint8_t tim7_flag = 0;

void app_Init(void)
{
	 LCD_init(on_lcd_draw);
	 HAL_Delay(200);
	 first_display(LCD_FB_START_ADDRESS);
	 BSP_TS_Init(480, 272);
	 HAL_TIM_Base_Start_IT(&htim6);
	 lps25h_Init();
}

void app_loop(void)
{
		tim7_flag = 1;
		HAL_Delay(1000);

}

void on_lcd_draw(uint32_t addr){

	switch(state)
	{
	case DISPLAY_RECORD:
		dashboard_record(addr);
		break;
	case DISPLAY_CHART:
		dashboard_chart(addr);
		break;
	}

}


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim == &htim6) {
		LCD_update();
		BSP_TS_GetState(&touch_state);
			if(touch_state.touchDetected)
			{
				uint16_t x = touch_state.touchX[0];
				uint16_t y = touch_state.touchY[0];
				switch(state)
				{
				case DISPLAY_RECORD:
					logger_record(x,y);
					break;
				case DISPLAY_CHART:
					logger_chart(x,y);
					break;
				}

			}
	}
}

void logger_chart(uint16_t x, uint16_t y)
{
	if(x > 360 && y < 60){
		for(uint8_t log_temp = 0; log_temp < LOG_TEMP_DATA; log_temp++)
						temperature_data[log_temp] = 0;
		state = DISPLAY_RECORD;
	}
}
void dashboard_chart(uint32_t addr)
{
	//LCD_clear(0x00000000, addr + LCD_L0_OFFSET);
	LCD_clear(0x00000000, addr + LCD_L1_OFFSET);
	LCD_draw_image_rle(chart, 0, 0, addr + LCD_L0_OFFSET, LCD_COLOR_NONE);
	LCD_draw_image_rle(back_pressed, 380, 20, addr + LCD_L1_OFFSET, LCD_COLOR_NONE);
	for(uint8_t i = CHART_VERTICAL_NUMBER_1; i <= CHART_VERTICAL_NUMBER_10; i++)
	{
		char text[20] = {0};
		sprintf(text, "%d", i +1);
		LCD_draw_text_rle(text, (i + 1) * 35 + CHART_VERTICAL_0, CHART_HORIZONTAL_NUMBER, &font16, addr + LCD_L1_OFFSET);
		//LCD_draw_point((i + 1) * 35 + CHART_VERTICAL_0, 100, 8, LCD_COLOR_WHITE, addr + LCD_L1_OFFSET);
		if (temperature_data[i] != 0)
			LCD_draw_point((i + 1) * 35 + CHART_VERTICAL_0, 232 - (temperature_data[i]*6/1000), 5, LCD_COLOR_WHITE, addr + LCD_L1_OFFSET);
	}
	LCD_draw_text_rle("30", CHART_TEMP_VERTICAL, CHART_TEMPERATURE_30, &font16, addr + LCD_L0_OFFSET);
	LCD_draw_text_rle("20", CHART_TEMP_VERTICAL, CHART_TEMPERATURE_20, &font16, addr + LCD_L0_OFFSET);
	LCD_draw_text_rle("10", CHART_TEMP_VERTICAL, CHART_TEMPERATURE_10, &font16, addr + LCD_L0_OFFSET);


}
void logger_record(uint16_t x, uint16_t y)
{
	if(x < 100 && y < 60)
		state = DISPLAY_CHART;
}
void dashboard_record(uint32_t addr)
{
	LCD_clear(0x00000000, addr + LCD_L0_OFFSET);
	LCD_clear(0x00000000, addr + LCD_L1_OFFSET);
	LCD_draw_text_rle("Temperature", 120, 70, &font30, addr + LCD_L1_OFFSET);
	uint32_t color = 0;
	char text[20] = {0};

	temperature = (int)(alititude.temperature_C*10);

	lps25h_readTemperatureC();
	sprintf(text, "%d,%d", temperature/10, temperature%10);
	temperature = temperature/10;

	if(temperature < 10)
	{
		color = LCD_COLOR_GREEN;
	}
	if(temperature >= 20 && temperature < 30)
	{
		color = LCD_COLOR_YELLOW;
	}
	else
	{
		color = LCD_COLOR_ORANGE;
	}
	LCD_draw_Rectangle(120, 120, 360, 200, 5, color, addr + LCD_L1_OFFSET);
	LCD_draw_text_rle(text, 200, 145, &font30, addr + LCD_L1_OFFSET);
	LCD_draw_image_rle(chart_pressed, 10, 20, addr + LCD_L1_OFFSET, LCD_COLOR_NONE);

}
void first_display(uint32_t addr)
{
	 LCD_draw_text_rle("Przygotowane przez", 50, 80, &font30, addr + LCD_L0_OFFSET);
	 LCD_draw_text_rle("Lukasz Pyzik", 100, 150, &font30, addr + LCD_L0_OFFSET);
	 HAL_Delay(3000);
	 LCD_clear(0x00000000, addr + LCD_L0_OFFSET);
	 LCD_clear(0x00000000, addr + LCD_L1_OFFSET);
	 LCD_draw_image_rle(QRcode, (480-150)/2, (272-150)/2, addr + LCD_L1_OFFSET, LCD_COLOR_NONE);
	 HAL_Delay(3000);
	 LCD_clear(0x00000000, addr + LCD_L0_OFFSET);
	 LCD_clear(0x00000000, addr + LCD_L1_OFFSET);
}


