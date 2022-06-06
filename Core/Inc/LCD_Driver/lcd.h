/*
 * lcd.h
 *
 *  Created on: 27.05.2022
 *      Author: Lukasz Pyzik
 */

#ifndef LCD_H_
#define LCD_H_

#include "stm32f7xx_hal.h"
/* Layers adress */
#define LCD_L0					0
#define LCD_L1					1
#define LCD_FB_START_ADDRESS 	0xC0000000
#define LCD_BB_START_ADDRESS 	0xC0200000
#define LCD_L0_OFFSET 		 	0x00000000
#define LCD_L1_OFFSET 		 	0x00400000

#define LCD_LayerCfgTypeDef    LTDC_LayerCfgTypeDef

/* check status */
#define LCD_OK                 ((uint8_t)0x00)
#define LCD_ERROR              ((uint8_t)0x01)
#define LCD_TIMEOUT            ((uint8_t)0x02)

/* LCD display colors definition */
#define LCD_COLOR_BLUE          ((uint32_t)0xFF0000FF)
#define LCD_COLOR_GREEN         ((uint32_t)0xFF00FF00)
#define LCD_COLOR_RED           ((uint32_t)0xFFFF0000)
#define LCD_COLOR_CYAN          ((uint32_t)0xFF00FFFF)
#define LCD_COLOR_MAGENTA       ((uint32_t)0xFFFF00FF)
#define LCD_COLOR_YELLOW        ((uint32_t)0xFFFFFF00)
#define LCD_COLOR_LIGHTBLUE     ((uint32_t)0xFF8080FF)
#define LCD_COLOR_LIGHTGREEN    ((uint32_t)0xFF80FF80)
#define LCD_COLOR_LIGHTRED      ((uint32_t)0xFFFF8080)
#define LCD_COLOR_LIGHTCYAN     ((uint32_t)0xFF80FFFF)
#define LCD_COLOR_LIGHTMAGENTA  ((uint32_t)0xFFFF80FF)
#define LCD_COLOR_LIGHTYELLOW   ((uint32_t)0xFFFFFF80)
#define LCD_COLOR_DARKBLUE      ((uint32_t)0xFF000080)
#define LCD_COLOR_DARKGREEN     ((uint32_t)0xFF008000)
#define LCD_COLOR_DARKRED       ((uint32_t)0xFF800000)
#define LCD_COLOR_DARKCYAN      ((uint32_t)0xFF008080)
#define LCD_COLOR_DARKMAGENTA   ((uint32_t)0xFF800080)
#define LCD_COLOR_DARKYELLOW    ((uint32_t)0xFF808000)
#define LCD_COLOR_WHITE         ((uint32_t)0xFFFFFFFF)
#define LCD_COLOR_LIGHTGRAY     ((uint32_t)0xFFD3D3D3)
#define LCD_COLOR_GRAY          ((uint32_t)0xFF808080)
#define LCD_COLOR_DARKGRAY      ((uint32_t)0xFF404040)
#define LCD_COLOR_BLACK         ((uint32_t)0xFF000000)
#define LCD_COLOR_BROWN         ((uint32_t)0xFFA52A2A)
#define LCD_COLOR_ORANGE        ((uint32_t)0xFFFFA500)
#define LCD_COLOR_TRANSPARENT   ((uint32_t)0xFF000000)
#define LCD_COLOR_NONE			((uint32_t)0xFFFFFFFF)

typedef void(*callback_draw_event)(uint32_t addr);

typedef struct{
	char start;
	char end;
	const uint32_t* bitmap;
	const uint32_t* offset;
}Font_t;

uint32_t LCD_init(callback_draw_event draw_event);

uint32_t LCD_get_x_size(uint32_t layer);
uint32_t LCD_get_y_size(uint32_t layer);

void LCD_clear(uint32_t color, uint32_t addr);
uint32_t LCD_update(void);

void LCD_draw_point(uint16_t x, uint16_t y, uint16_t thickness, uint32_t color, uint32_t addr);
void LCD_draw_line(uint16_t x_1, uint16_t y_1, uint16_t x_2, uint16_t y_2, uint16_t thickness, uint32_t color, uint32_t addr);
void LCD_draw_Hline(uint16_t x_start, uint16_t y_start, uint16_t length, uint16_t thickness, uint32_t color, uint32_t addr);
void LCD_draw_Vline(uint16_t x_start, uint16_t y_start, uint16_t length, uint16_t thickness, uint32_t color, uint32_t addr);
void LCD_draw_Rectangle(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end, uint16_t thickness, uint32_t color, uint32_t addr);
void LCD_draw_circle(uint16_t x_s, uint16_t y_s, uint16_t radius, uint16_t thickness, uint32_t color, uint32_t addr);

void LCD_draw_text(char* text, uint16_t x, uint16_t y, Font_t* font, uint32_t addr);
void LCD_draw_image(const uint32_t* src, uint16_t x, uint16_t y, uint32_t addr, uint32_t modulate);

void LCD_draw_text_rle(char* text, uint16_t x, uint16_t y, Font_t* font, uint32_t addr);
void LCD_draw_image_rle(const uint32_t* src, uint16_t x, uint16_t y, uint32_t addr, uint32_t modulate);

#endif /* LCD_H_ */
