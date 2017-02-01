/*
 * SH1106.c
 *
 *  Created on: 6 nov. 2016
 *      Author: Erik
 */
#include "SH1106.h"

uint8_t Data[9];
SPI_HandleTypeDef *SH1106_HSPI = NULL;

int SH1106_Init (SPI_HandleTypeDef *spi)
{
	SH1106_HSPI = spi;

	// Reset display
	HAL_GPIO_WritePin(SH1106_RES, GPIO_PIN_SET);
	HAL_Delay (20);
	HAL_GPIO_WritePin(SH1106_RES, GPIO_PIN_RESET);
	HAL_Delay (1);
	HAL_GPIO_WritePin(SH1106_RES, GPIO_PIN_SET);
	HAL_Delay (20);

	// Stel Command mode in
	HAL_GPIO_WritePin(SH1106_DC, GPIO_PIN_RESET);

	SH1106_WriteByte(0xAE);    /*display off*/
	SH1106_WriteByte(0x02);    /*set lower column address*/
	SH1106_WriteByte(0x10);    /*set higher column address*/
	SH1106_WriteByte(0x40);    /*set display start line*/
	SH1106_WriteByte(0xB0);    /*set page address*/
	SH1106_WriteByte(0xA1);    /*set segment remap*/
	//SH1106_WriteByte(0xA5);    /*entire display ON*/
	SH1106_WriteByte(0xA6);    /*normal / reverse*/
	SH1106_WriteByte(0xA8);    /*multiplex ratio*/
	SH1106_WriteByte(0x3F);    /*duty = 1/32*/
	SH1106_WriteByte(0xAD);    /*set charge pump enable*/
	SH1106_WriteByte(0x8B);     /*external VCC   */
	SH1106_WriteByte(0x33);    /*0X30---0X33  set VPP   9V liangdu!!!!*/
	SH1106_WriteByte(0xC8);    /*Com scan direction*/
	SH1106_WriteByte(0xD3);    /*set display offset*/
	SH1106_WriteByte(0x00);   /*   0x20  */
	SH1106_WriteByte(0xD5);    /*set osc division*/
	SH1106_WriteByte(0x80);
	SH1106_WriteByte(0xD9);    /*set pre-charge period*/
	SH1106_WriteByte(0x1F);    /*0x22*/
	SH1106_WriteByte(0xDA);    /*set COM pins*/
	SH1106_WriteByte(0x00);    // 0x12

	SH1106_SetBrightness (0x80);

	// Stel Data mode in
	// HAL_GPIO_WritePin(SH1106_DC, GPIO_PIN_SET);

}

void SH1106_Clear (void)
{
	memset(disp_buffer, 0, 132 * 8);
}

void SH1106_TurnOn (void)
{
	HAL_GPIO_WritePin (SH1106_DC, GPIO_PIN_RESET);	// stel command mode in
	SH1106_WriteByte(0xAF);    /*display ON*/
}

void SH1106_TurnOff (void)
{
	HAL_GPIO_WritePin (SH1106_DC, GPIO_PIN_RESET);	// stel command mode in
	SH1106_WriteByte(0xAE);    /*display OFF*/
}

void SH1106_SetBrightness(uint8_t value)
{
	static uint8_t vcomh;
	uint8_t volt = value >> 6;
	uint8_t bright = value & 0x3F;

	bright ^= (volt & 2) << 5;
	bright ^= (volt & 1) << 5;

	// Stel Command mode in
	HAL_GPIO_WritePin(SH1106_DC, GPIO_PIN_RESET);

	SH1106_WriteByte(0x81);    		// contrast control
	SH1106_WriteByte(bright);    	// 128

	if (vcomh != (value >> 3))
	{
		vcomh = value >> 3;
//		SH1106_WriteByte(0xDB);    		// set vcomh
//		SH1106_WriteByte(vcomh); 	// 0x40
		SH1106_WriteByte(0x30 + (value >> 6));
		SH1106_WriteByte(0xAF);    /*display ON*/
	}
}

int SH1106_DrawChar (char data, uint8_t x, uint8_t y, drawmode clr, uint8_t pitch)
{
	uint16_t bufpos;
	uint8_t bitpos, n;

	if (data < 0x20 || data > 0x7F) return 0;

	bufpos = x + ((y >> 3) * XSIZE);
	bitpos = y & 7;

	for (n = 0; n < 5; n++)
	{
		switch (clr)
		{
			case NORMAL:
				disp_buffer[bufpos + n] |= (ASCII[data - 0x20][n] << bitpos);
				if (bitpos > 0)
					disp_buffer[bufpos + n + XSIZE] |= (ASCII[data - 0x20][n] >> (8 - bitpos));
				break;
			case INVERSE:
				disp_buffer[bufpos + n] |= ((255 ^ ASCII[data - 0x20][n]) << bitpos);
				if (bitpos > 0)
					disp_buffer[bufpos + n + XSIZE] |= ((255 ^ ASCII[data - 0x20][n]) >> (8 - bitpos));
				break;
			case REPLACE:
				disp_buffer[bufpos + n] = (disp_buffer[bufpos + n] & (255 >> (8 - bitpos))) |
																	 (ASCII[data - 0x20][n] << bitpos);
				if (bitpos > 0)
				{
					disp_buffer[bufpos + n + XSIZE] |= (ASCII[data - 0x20][n] >> (8 - bitpos));
					disp_buffer[bufpos + n] = (disp_buffer[bufpos + n] & (255 << bitpos)) |
																		 (ASCII[data - 0x20][n] >> (8 - bitpos));
				}
				break;
			case FAST:
				disp_buffer[bufpos + n] = (ASCII[data - 0x20][n] << bitpos);
				if (bitpos > 0)
					disp_buffer[bufpos + n + XSIZE] = (ASCII[data - 0x20][n] >> (8 - bitpos));
				break;
			case INVERT:
				disp_buffer[bufpos + n] &= ((255 ^ ASCII[data - 0x20][n]) << bitpos);
				if (bitpos > 0)
					disp_buffer[bufpos + n + XSIZE] &= ((255 ^ ASCII[data - 0x20][n]) >> (8 - bitpos));
				break;
		}
	}

	if (clr == INVERSE)
	{
		for (n = 5; n < pitch; n++)
		{
			disp_buffer[bufpos + n] |= (255 << bitpos);
			if (bitpos > 0)
				disp_buffer[bufpos + n + XSIZE] |= (255 >> (8 - bitpos));
		}
	}

	return 0;
}

int SH1106_DrawString (char *text, uint8_t x, uint8_t y, drawmode clr, uint8_t *buf)
{
	uint8_t px, py, chrnum;
	for (chrnum = 0; chrnum < strlen(text); chrnum++)
	{
		SH1106_DrawChar (text[chrnum], x + 6 * chrnum, y, clr, 6);
	}
	return 0;
}

int SH1106_DrawStringBold (char *text, uint8_t x, uint8_t y, drawmode clr, uint8_t *buf)
{
	uint8_t		px, py, chrnum;

	for (chrnum = 0; chrnum < strlen(text); chrnum++)
	{
		SH1106_DrawChar (text[chrnum], x + 7 * chrnum, y, clr, 7);
		SH1106_DrawChar (text[chrnum], x + 1 + 7 * chrnum, y, clr, 7);
	}
	return 0;
}

int SH1106_SetPixel (uint8_t x, uint8_t y, drawmode clr)
{
	uint16_t	offset = (x % 132) + (y >> 3) * 132;
	uint8_t		bitpos = y & 7;

	switch (clr)
	{
	  case NORMAL:
		disp_buffer[offset] |= 1 << bitpos;
		break;
	  case INVERT:
		disp_buffer[offset] &= 255 - (1 << bitpos);
		break;
	  case FAST:
		disp_buffer[offset] = 1 << bitpos;
		break;
	  default:
		break;
	}
	return 0;
}

int SH1106_DrawBox (uint8_t x, uint8_t y, uint8_t width, uint8_t height, drawmode border, drawmode fill)
{
	return 0;
}

int SH1106_DrawBitmap (uint8_t x, uint8_t y, uint8_t width, uint8_t height, drawmode clr, char *data)
{
	uint16_t	offset;			// byte offset in schermbuffer
	uint16_t	bitmapoffs;		// byte offset in bitmapbuffer
	uint8_t		xp, yp;
	uint8_t		shift = 7 - ((y + height - 1) & 7);
	uint8_t		writedata;

	for (xp = 0, bitmapoffs = 0; xp < width; xp++)
	{
		for (yp = 0, offset = x + xp + ((y + height - 1) >> 3) * 132; yp < height; yp += 8, bitmapoffs++, offset -= 132)
		{
			writedata = data[bitmapoffs];

			switch (clr)
			{
			  case NORMAL:
				  disp_buffer[offset] |= writedata >> shift;
				  if (shift)
					disp_buffer[offset - 132] |= writedata << (8 - shift);
				  break;

			  default:
				if (yp == 0 || shift == 0)
				  disp_buffer[offset] = writedata >> shift;
				else
				  // Fast modus, maar onze vorige byte niet overschrijven
				  disp_buffer[offset] |= writedata >> shift;

				if (shift)
				  disp_buffer[offset - 132] = writedata << (8 - shift);
				break;
			}
		}
	}
	return 0;
}

int SH1106_WriteByte (uint8_t value)
{
	return HAL_SPI_Transmit (SH1106_HSPI, &value, 1, 1000);
}

int SH1106_WriteData (uint8_t *data, uint8_t size)
{
	return HAL_SPI_Transmit (SH1106_HSPI, data, size, 1000);
}

static uint16_t	framecount = 0;

uint16_t	SH1106_GetFrameCount()
{
	uint16_t	count = framecount;
	framecount = 0;
	return count;
}

int SH1106_PaintScreen ()
{
	uint8_t page;

	for (page = 0; page < 8; page++)
	{
		while (SH1106_HSPI->State != HAL_SPI_STATE_READY);

		// Stel Command mode in
		HAL_GPIO_WritePin(SH1106_DC, GPIO_PIN_RESET);
		SH1106_WriteByte (0xB0 + page);

		// Stel Data mode in
		HAL_GPIO_WritePin(SH1106_DC, GPIO_PIN_SET);
		HAL_SPI_Transmit_DMA (SH1106_HSPI, disp_buffer + page * XSIZE, 132); // sizeof(disp_buffer)
		//SH1106_WriteData (disp_buffer + page * XSIZE, XSIZE);
	}
	framecount++;

	return 0;
}