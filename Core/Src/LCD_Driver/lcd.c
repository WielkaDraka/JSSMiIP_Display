/*
 * lcd.c
 *
 *  Created on: 27.05.2022
 *      Author: Lukasz Pyzik
 */

#include <stddef.h>
#include <string.h>
#include "stm32f7xx_hal.h"
#include <math.h>
#include "LCD_Driver/lcd.h"

/* SDRAM_REFRESH_COUNT = [(SDRAM self refresh time / number of row) x  SDRAM CLK] – 20
            		   = [(64ms/4096) * 100MHz] - 20 = 1562.5 - 20 ~ 1542 = 0x0603*/
#define SDRAM_REFRESH_COUNT							((uint32_t)0x0603) // SDRAM refresh counter (100Mhz SD clock)
#define SDRAM_TIMEOUT								((uint32_t)0xFFFF)

/* mode register definition */
#define SDRAM_MODEREG_BURST_LENGTH_1             	((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2             	((uint16_t)0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4             	((uint16_t)0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8             	((uint16_t)0x0004)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL      	((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED     	((uint16_t)0x0008)
#define SDRAM_MODEREG_CAS_LATENCY_2              	((uint16_t)0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3              	((uint16_t)0x0030)
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD    	((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED	((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE     	((uint16_t)0x0200)

extern LTDC_HandleTypeDef hltdc;
extern SDRAM_HandleTypeDef hsdram1;
static FMC_SDRAM_CommandTypeDef Command;
static callback_draw_event ltdc_draw_event;

/**
  * @brief  Gets the LCD X size.
  * @param  layer: layer we want to get X size from
  * @retval Used LCD X size
  */
uint32_t LCD_get_x_size(uint32_t layer){
	return hltdc.LayerCfg[layer].ImageWidth;
}

/**
  * @brief  Gets the LCD Y size.
  * @param  layer: layer we want to get Y size from
  * @retval Used LCD Y size
  */
uint32_t LCD_get_y_size(uint32_t layer){
	return hltdc.LayerCfg[layer].ImageHeight;
}

/**
  * @brief  Clears the hole LCD.
  * @param  color:	color of the layer
  * @param	addr: 	addres of the layer (background, layer1 or layer2)
  * @retval None
  */
void LCD_clear(uint32_t color, uint32_t addr){
	for(uint16_t h=0; h<272; h++){
		for(uint16_t w=0; w<480; w++){
			*(__IO uint32_t*)(addr + (4*(h*480 + w))) = color;
		}
	}
}

/**
  * @brief  Function to draw point.
  * @param	x, y:	the coordinates of the point
  * @param	thickness: point thickness
  * @param  color:	color of the layer
  * @param	addr: 	addres of the layer (background, layer1 or layer2)
  * @retval None
  */
void LCD_draw_point(uint16_t x, uint16_t y, uint16_t thickness, uint32_t color, uint32_t addr)
{
	for(uint16_t h=y-thickness/2; h<y+thickness/2; h++){
			for(uint16_t w=x-thickness/2; w<x+thickness/2; w++){
				*(__IO uint32_t*)(addr + (4*(h*480 + w))) = color;
			}
		}
}

/**
  * @brief  Used Bresenham's algorithm to draw a line
  * @param	x_1, y_1:	the coordinates of the first point on the display
  * @param	x_2, y_2:	the coordinates of the second point on the display
  * @param  thickness:	line thickness
  * @param  color: 		color of the layer
  * @param	addr: 		addres of the layer (background, layer1 or layer2)
  * @retval None
  */
void LCD_draw_line(uint16_t x_1, uint16_t y_1, uint16_t x_2, uint16_t y_2, uint16_t thickness, uint32_t color, uint32_t addr)
{
	uint16_t dx, dy, stepX, stepY;
	uint16_t x, y;
	int16_t incrE;
	int16_t incrNE;
	int16_t d;
	if(x_1 < x_2){
		stepX = 1;
		dx = x_2 - x_1;
	}
	else{
		stepX = -1;
		dx = x_1 - x_2;
	}
	if(y_1 < y_2){
		stepY = 1;
		dy = y_2 - y_1;
	}
	else{
		stepY = -1;
		dy = y_1 - y_2;
	}
	x = x_1;
	y = y_1;
	for(int t =x  - thickness/2 ; t <= x + thickness/2; t++)
		*(__IO uint32_t*)(addr + (4*(y*480 + t))) = color;


	if(dx > dy){
		incrE = dy * 2;
		incrNE = (dx - dy) * 2;
		d = dy * 2 - dx;

		while (x != x_2)
		{
			x += stepX;
			if(d <= 0){
				d += incrE;
			}
			else{
				d += incrNE;
                y += stepY;
			}
			for(int t =x  - thickness/2 ; t <= x + thickness/2; t++)
				*(__IO uint32_t*)(addr + (4*(y*480 + t))) = color;
		}
	}
	else{
		incrE = dx * 2;
		incrNE = (dx - dy) * 2;
		d = dx * 2 - dy;
		while (y != y_2)
		{
			y += stepY;
			if (d < 0){
				d += incrE;
			}
			else{
				d += incrNE;
				x += stepX;
			}
			for(int t =x  - thickness/2 ; t <= x + thickness/2; t++)
				*(__IO uint32_t*)(addr + (4*(y*480 + t))) = color;
		}
	}

}

/**
  * @brief  Used Bresenham's algorithm to draw a line
  * @param	x_s, y_s:	the coordinates of the center point of circle
  * @param	radius:		radius of circle
  * @param  thickness:	line thickness
  * @param  color: 		color of the layer
  * @param	addr: 		addres of the layer (background, layer1 or layer2)
  * @retval None
  */
void LCD_draw_circle(uint16_t x_s, uint16_t y_s, uint16_t radius, uint16_t thickness, uint32_t color, uint32_t addr)
{
	for(uint16_t h=y_s-radius-thickness; h<y_s+radius+thickness; h++){
			for(uint16_t w=x_s-radius-thickness; w<x_s+radius+thickness; w++){
				if(((w - x_s)*(w - x_s) + (h - y_s)*(h - y_s) >=  (((radius - thickness/2) * (radius - thickness/2)))) && ((w - x_s)*(w - x_s) + (h - y_s)*(h - y_s) <=  (((radius + thickness) * (radius + thickness)))))
				*(__IO uint32_t*)(addr + (4*(h*480 + w))) = color;
			}
		}
}

/**
  * @brief  To draw horizontal line
  * @param	x_start, y_start:	the first point of line
  * @param	length:		length of line
  * @param  thickness:	line thickness
  * @param  color: 		color of the layer
  * @param	addr: 		addres of the layer (background, layer1 or layer2)
  * @retval None
  */
void LCD_draw_Hline(uint16_t x_start, uint16_t y_start, uint16_t length, uint16_t thickness, uint32_t color, uint32_t addr)
{
	for(uint16_t h=y_start; h<y_start + thickness; h++){
			for(uint16_t w=x_start; w<length + x_start; w++){
				*(__IO uint32_t*)(addr + (4*(h*480 + w))) = color;
			}
		}
}

/**
  * @brief  To draw vertical line
  * @param	x_start, y_start:	the first point of line
  * @param	length:		length of line
  * @param  thickness:	line thickness
  * @param  color: 		color of the layer
  * @param	addr: 		addres of the layer (background, layer1 or layer2)
  * @retval None
  */
void LCD_draw_Vline(uint16_t x_start, uint16_t y_start, uint16_t length, uint16_t thickness, uint32_t color, uint32_t addr)
{
	for(uint16_t h=y_start; h<y_start + length; h++){
			for(uint16_t w=x_start ; w<x_start + thickness; w++){
				*(__IO uint32_t*)(addr+ (4*(h*480 + w))) = color;
			}
		}
}

/**
  * @brief  To draw rectangle
  * @param	x_start, y_start:	the point where the rectangle begins to draw
  * @param	x_end, y_end:		point diagonally to the starting point
  * @param  thickness:			line thickness
  * @param  color: 				color of the layer
  * @param	addr: 				addres of the layer (background, layer1 or layer2)
  * @retval None
  */
void LCD_draw_Rectangle(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end, uint16_t thickness, uint32_t color, uint32_t addr)
{
	LCD_draw_Hline(x_start, y_start, x_end - x_start, thickness, color, addr);
	LCD_draw_Hline(x_start, y_end, x_end - x_start, thickness, color, addr);
	LCD_draw_Vline(x_start, y_start, y_end - y_start, thickness, color, addr);
	LCD_draw_Vline(x_end, y_start, y_end - y_start + thickness, thickness, color, addr);
}

/**
  * @brief  Display text (Draw text) in defined place, with text size, font offset and the layer on which text should be displayed
  * @param  text: text to display
  * @param  x: x from which the text is to be displayed, distance from left edge
  * @param  y: y from which the text is to be displayed, distance from top edge
  * @param  font: is a structure for the corresponding font
  * @param	addr: addres of the layer (background, layer1 or layer2)
  * @retval None
  */
void LCD_draw_text(char* text, uint16_t x, uint16_t y, Font_t* font, uint32_t addr){
	uint8_t len = strlen(text);
	uint32_t offset;
	uint32_t w;
	uint32_t h;

	for(uint32_t s=0; s<len; s++){
		offset = font->offset[text[s] - font->start];
		w = font->bitmap[offset];
		h = font->bitmap[offset + 1];

		for(uint32_t r=0; r<h; r++){
			for(uint32_t c=0; c<w; c++){
				uint32_t index = offset + 2 + r * w + c;
				uint32_t color = font->bitmap[index];

				if(color != 0xFF000000){
					*(__IO uint32_t*)(addr + (4*((y + r)*480 + x + c))) = color;
				}
			}
		}

		x += w;
	}
}

/**
  * @brief  Display text (Draw text) in defined place, with text size, font offset and the layer on which text should be displayed.
  * 		RLE (RUN-LENGTH ENCODING) is algorithm to data compression.
  * @param  text: text to display
  * @param  x: x from which the text is to be displayed, distance from left edge
  * @param  y: y from which the text is to be displayed, distance from top edge
  * @param  font: is a structure for the corresponding font (the data must by RLE)
  * @param	addr: addres of the layer (background, layer1 or layer2)
  * @retval None
  */
void LCD_draw_text_rle(char* text, uint16_t x, uint16_t y, Font_t* font, uint32_t addr){
	uint8_t len = strlen(text);
	uint32_t offset;
	uint32_t w;
	uint32_t h;
	uint16_t c;
	uint16_t pairs;
	uint16_t same;
	uint32_t color;

	for(uint32_t s=0; s<len; s++){
		offset = font->offset[text[s] - font->start];
		w = font->bitmap[offset++];
		h = font->bitmap[offset++];

		for(uint32_t r=0; r<h; r++){
			c = 0;
			pairs = font->bitmap[offset++];

			for(uint32_t p=0; p<pairs; p++){

				same = font->bitmap[offset++];
				color = font->bitmap[offset++];

				for(uint32_t s=0; s<same; s++){
					if(c < w){
						if(color != 0xFF000000){
							*(__IO uint32_t*)(addr + (4*((y + r)*480 + x + c))) = color;
						}
						c++;
					}
				}
			}
		}

		x += w;
	}
}

/**
  * @brief  Function to draw image form file
  * @param	src:		is a pointer to image
  * @param	x, y:		picture position (distance from the top left corner of the screen)
  * @param	addr: 		addres of the layer (background, layer1 or layer2)
  * @param modulate		change the colors of the picture. Default value to write: LCD_COLOR_NONE
  * @retval None
  */
void LCD_draw_image(const uint32_t* src, uint16_t x, uint16_t y, uint32_t addr, uint32_t modulate){
	uint32_t w = src[0];
	uint32_t h = src[1];
	uint32_t color;

	src = src + 2;

	for(uint16_t r=0; r<h; r++){
		for(uint16_t c=0; c<w; c++){

			color = *src++;

			*(__IO uint32_t*)(addr + (4*((y + r)*480 + x + c))) = color & modulate;
		}
	}
}

/**
  * @brief  Function to draw image form file
  * 		RLE (RUN-LENGTH ENCODING) is algorithm to data compression.
  * @param	src:		is a pointer to image
  * @param	x, y:		picture position (distance from the top left corner of the screen)
  * @param	addr: 		addres of the layer (background, layer1 or layer2)
  * @param modulate		change the colors of the picture. Default value to write: LCD_COLOR_NONE
  * @retval None
  */
void LCD_draw_image_rle(const uint32_t* src, uint16_t x, uint16_t y, uint32_t addr, uint32_t modulate){
	uint32_t w = src[0];
	uint32_t h = src[1];
	uint16_t c;
	uint16_t pairs;
	uint16_t same;
	uint32_t color;

	src = src + 2;

	for(uint16_t r=0; r<h; r++){
		c = 0;
		pairs = *src++;

		for(uint16_t p=0; p<pairs; p++){

			same = *src++;
			color = *src++;

			for(uint16_t s=0; s<same; s++){
				if(c < w){
					if(color & 0xFF000000){
						*(__IO uint32_t*)(addr + (4*((y + r)*480 + x + c))) = color & modulate;
					}
					c++;
				}
			}
		}
	}
}

/**
 * @brief	Initialization for FMC
 * @param	RefreshCount: Set the refresh time
 */
void LCD_SDRAM_init(uint32_t RefreshCount){
	__IO uint32_t tmpmrd = 0;
	/* Configuration created from Reference manual RM0385 */

	/* Step 1 and Step 2 already done in HAL_SDRAM_Init() */

	/* Step 3: Configure a clock configuration enable command */
	Command.CommandMode            = FMC_SDRAM_CMD_CLK_ENABLE;
	Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
	Command.AutoRefreshNumber      = 1;
	Command.ModeRegisterDefinition = 0;

	// Send the command
	HAL_SDRAM_SendCommand(&hsdram1, &Command, SDRAM_TIMEOUT);

	/* Step 4: Insert 100 us minimum delay - Min HAL Delay is 1ms */
	HAL_Delay(1);

	/* Step 5: Configure a PALL (precharge all) command */
	Command.CommandMode            = FMC_SDRAM_CMD_PALL;
	Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
	Command.AutoRefreshNumber      = 1;
	Command.ModeRegisterDefinition = 0;

	// Send the command
	HAL_SDRAM_SendCommand(&hsdram1, &Command, SDRAM_TIMEOUT);

	/* Step 6: Configure an Auto Refresh command */
	Command.CommandMode            = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
	Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
	Command.AutoRefreshNumber      = 8;
	Command.ModeRegisterDefinition = 0;

	// Send the command
	HAL_SDRAM_SendCommand(&hsdram1, &Command, SDRAM_TIMEOUT);

	/* Step 7: Program the external memory mode register */
	tmpmrd = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_1          |\
					 SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL   |\
					 SDRAM_MODEREG_CAS_LATENCY_2           |\
					 SDRAM_MODEREG_OPERATING_MODE_STANDARD |\
					 SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;

	Command.CommandMode            = FMC_SDRAM_CMD_LOAD_MODE;
	Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
	Command.AutoRefreshNumber      = 1;
	Command.ModeRegisterDefinition = tmpmrd;

	// Send the command
	HAL_SDRAM_SendCommand(&hsdram1, &Command, SDRAM_TIMEOUT);

	/* Step 8: Set the refresh rate counter - refer to section SDRAM refresh timer register in RM0385 */
	// Set the device refresh rate
	HAL_SDRAM_ProgramRefreshRate(&hsdram1, RefreshCount);
}

/**
 * @brief	Initialization for LCD
 * @param	draw_event: Is a function event
 */
uint32_t LCD_init(callback_draw_event draw_event){
	ltdc_draw_event = draw_event;

	LCD_SDRAM_init(SDRAM_REFRESH_COUNT);

	LCD_clear(0x00000000, LCD_FB_START_ADDRESS + LCD_L0_OFFSET);
	LCD_clear(0x00000000, LCD_FB_START_ADDRESS + LCD_L1_OFFSET);

	return LCD_OK;
}

/**
 * @brief	Function to update layers
 */
uint32_t LCD_update(void){
	static uint8_t swap;
	uint32_t address;
	LCD_LayerCfgTypeDef layer0_config = {0};
	LCD_LayerCfgTypeDef layer1_config = {0};

	address = swap ? LCD_BB_START_ADDRESS : LCD_FB_START_ADDRESS;
	swap ^= 1;

	if(ltdc_draw_event){
		ltdc_draw_event(address);
	}


	layer0_config.WindowX0 = 0;
	layer0_config.WindowX1 = 480;
	layer0_config.WindowY0 = 0;
	layer0_config.WindowY1 = 272;
	layer0_config.PixelFormat = LTDC_PIXEL_FORMAT_ARGB8888;
	layer0_config.Alpha = 255;
	layer0_config.Alpha0 = 255;
	layer0_config.BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA;
	layer0_config.BlendingFactor2 = LTDC_BLENDING_FACTOR2_PAxCA;
	layer0_config.FBStartAdress = address + LCD_L0_OFFSET;
	layer0_config.ImageWidth = 480;
	layer0_config.ImageHeight = 272;
	layer0_config.Backcolor.Blue = 0;
	layer0_config.Backcolor.Green = 0;
	layer0_config.Backcolor.Red = 0;
	if(HAL_LTDC_ConfigLayer(&hltdc, &layer0_config, 0) != HAL_OK){
		return 1;
	}

	layer1_config.WindowX0 = 0;
	layer1_config.WindowX1 = 480;
	layer1_config.WindowY0 = 0;
	layer1_config.WindowY1 = 272;
	layer1_config.PixelFormat = LTDC_PIXEL_FORMAT_ARGB8888;
	layer1_config.Alpha = 255;
	layer1_config.Alpha0 = 0;
	layer1_config.BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA;
	layer1_config.BlendingFactor2 = LTDC_BLENDING_FACTOR2_PAxCA;
	layer1_config.FBStartAdress = address + LCD_L1_OFFSET;
	layer1_config.ImageWidth = 480;
	layer1_config.ImageHeight = 272;
	layer1_config.Backcolor.Blue = 0;
	layer1_config.Backcolor.Green = 0;
	layer1_config.Backcolor.Red = 0;
	if(HAL_LTDC_ConfigLayer(&hltdc, &layer1_config, 1) != HAL_OK){
		return 1;
	}

	return 0;
}
