/*
 * SH1106.c
 *
 *  Created on: 6 nov. 2016
 *      Author: Erik
 */
#include "SH1106.h"

uint8_t Data[9];
SPI_HandleTypeDef *SH1106_HSPI = NULL;

static uint8_t pagenum = 0;

int SH1106_Init (SPI_HandleTypeDef *spi)
{
	SH1106_HSPI = spi;

	// Reset display
	// HAL_GPIO_WritePin(SH1106_RES, GPIO_PIN_SET);
	// HAL_Delay (20);
	HAL_GPIO_WritePin(SH1106_RES, GPIO_PIN_RESET);
	HAL_Delay (1);
	HAL_GPIO_WritePin(SH1106_RES, GPIO_PIN_SET);
	HAL_Delay (20);

	// Stel Command mode in
	HAL_GPIO_WritePin(SH1106_DC, GPIO_PIN_RESET);

	// SH1106_WriteByte(0xAE);    /*display off*/
	SH1106_WriteByte(0x02);    /*set lower column address*/
	SH1106_WriteByte(0x10);    /*set higher column address*/
	SH1106_WriteByte(0x40);    /*set display start line*/
	//  SH1106_WriteByte(0xB0);    /*set page address*/
	SH1106_WriteByte(0xA1);    /*set segment remap*/
	// SH1106_WriteByte(0xA5);    /*entire display ON*/
	SH1106_WriteByte(0xA6);    /*normal / reverse*/
	SH1106_WriteByte(0xA8);    /*multiplex ratio*/
	SH1106_WriteByte(0x3F);    /*duty = 1/32*/
	SH1106_WriteByte(0xAD);    /*set charge pump enable*/
	SH1106_WriteByte(0x8B);     /*external VCC   */
	SH1106_WriteByte(0x32);    /*0X30---0X33  set VPP   9V liangdu!!!!*/
	SH1106_WriteByte(0xC8);    /*Com scan direction*/
	SH1106_WriteByte(0xD3);    /*set display offset*/
	SH1106_WriteByte(0x00);   /*   0x20  */
	SH1106_WriteByte(0xD5);    /*set osc division*/
	SH1106_WriteByte(0x80);
	SH1106_WriteByte(0xD9);    /*set pre-charge period*/
	SH1106_WriteByte(0x1F);    /*0x22*/
	SH1106_WriteByte(0xDA);    /*set COM pins*/
	SH1106_WriteByte(0x12);    // 0x12
	SH1106_WriteByte(0xDB);		 // set VCOMD voltage
	SH1106_WriteByte(0x10);		 // 0x35
	SH1106_SetBrightness (0x80);
	// SH1106_TurnOn();

	// Stel Data mode in
	// HAL_GPIO_WritePin(SH1106_DC, GPIO_PIN_SET);

}

void SH1106_Clear (void)
{
	memset(disp_buffer, 0, XSIZE * 8);
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
/*
	SH1106_WriteByte(0x81);    		// contrast control
	SH1106_WriteByte(bright);    	// 128

	if (vcomh != (value >> 3))
	{
		vcomh = value >> 3;
//		SH1106_WriteByte(0xDB);    		// set vcomh
//		SH1106_WriteByte(vcomh); 	// 0x40
		SH1106_WriteByte(0x30 + (value >> 6));
		SH1106_WriteByte(0xAF);    // display ON
	}
*/
	SH1106_WriteByte (0x30 + (value >> 6));	// DC voltage instellen
	SH1106_WriteByte (0x81);
	SH1106_WriteByte (value * 1.375 - (value >> 6) * 32);


}

int SH1106_SetPixel (uint8_t x, uint8_t y, drawmode clr)
{
	uint16_t	offset = (x % XSIZE) + (y >> 3) * XSIZE;
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
	  case XOR:
		disp_buffer[offset] ^= 1 << bitpos;
		break;
	  default:
		break;
	}
	return 0;
}

int SH1106_SetByte (uint16_t index, uint8_t value, int8_t shift, drawmode clr, uint8_t width)
{
	uint8_t mask;		// bewerkmasker voor buffer

	if (width > 0 && width < 8)	// Bij 0 gaan we er vanuit dat de breedte niet opgegeven is
		mask = ((1 << width) - 1) << shift;
	else
		mask = 255 << shift;

	if (shift > 0)
	{
		value <<= shift;
		mask = ((1 << width) - 1) << shift;
	}
	else if (shift < 0)
	{
		value >>= -shift;
		mask = ((1 << width) -1) >> (-shift);
	}

	switch (clr)
	{
		case NORMAL:		// OR met buffer
			disp_buffer[index] |= value & mask;
			break;
		case INVERSE:		// value inverteren, OR met buffer
			disp_buffer[index] |= (~value) & mask;
			break;
		case REPLACE:		// buffer vervangen met value
			disp_buffer[index] &= ~mask;
			disp_buffer[index] |= value;
			break;
		case FAST:			// buffer vervangen met value, ongeschreven bits negeren
			disp_buffer[index] = value;
			break;
		case REVERSE:		// value inverteren, ongeschreven bits negeren
			disp_buffer[index] = ~value;
			break;
		case INVERT:		// value inverteren, AND met buffer (voor witte achtergrond)
			disp_buffer[index] &= ~value;
			break;
		case XOR:
			disp_buffer[index] ^= value & mask;
		default:
			// niets doen
			break;
	}

	return 0;
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
		SH1106_SetByte (bufpos + n, ASCII[data - 0x20][n], bitpos, clr, 8);
		if (bitpos > 0)
			SH1106_SetByte (bufpos + n + XSIZE, ASCII[data - 0x20][n], bitpos - 8, clr, 8);

	}

	if (clr == INVERSE || clr == REVERSE)
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

int SH1106_DrawBox (uint8_t x, uint8_t y, uint8_t width, uint8_t height, drawmode clr)
{

	return 0;
}

int SH1106_FillBox (uint8_t x, uint8_t y, uint8_t width, uint8_t height, drawmode clr)
{
	uint8_t		xp, yp, shift;

	shift = y & 7;
	for (yp = 0; yp < height + (y & 7); yp += 8)
	{
		for (xp = x; xp < x + width; xp++)
		{
			SH1106_SetByte (xp + ((y + yp) >> 3) * XSIZE, 255, shift, clr, height + (y & 7) - yp);
		}
		shift = 0;
	}

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
		for (yp = 0, offset = x + xp + ((y + height - 1) >> 3) * XSIZE; yp < height; yp += 8, bitmapoffs++, offset -= XSIZE)
		{
			writedata = data[bitmapoffs];

/*
			switch (clr)
			{
				case INVERSE:
					writedata = ~writedata;
					// continue;
					//fallthrough
			  case NORMAL:
				  disp_buffer[offset] |= writedata >> shift;
				  if (shift)
					disp_buffer[offset - XSIZE] |= writedata << (8 - shift);
				  break;

			  case NONE:
				  break;

			  default:
				if (yp == 0 || shift == 0)
				  disp_buffer[offset] = writedata >> shift;
				else
				  // Fast modus, maar onze vorige byte niet overschrijven
				  disp_buffer[offset] |= writedata >> shift;

				if (shift)
				  disp_buffer[offset - XSIZE] = writedata << (8 - shift);
				break;
			}
*/
			if (clr == FAST && (shift == 0 || yp == 0))
			{
				SH1106_SetByte (offset, writedata, -shift, clr, 8);
			}
			else
			{
				// Fast modus, maar onze vorige byte niet overschrijven
				SH1106_SetByte (offset, writedata, -shift, REPLACE, 8);
			}
			if (shift)
				SH1106_SetByte (offset - XSIZE, writedata, 8 - shift, clr, 8);
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

void SH1106_SPIDMA_Callback (void)
{

  if (SH1106_HSPI->hdmatx->State == HAL_DMA_STATE_READY)
	  if (pagenum < 8)
	  {
			// Stel Command mode in
			HAL_GPIO_WritePin(SH1106_DC, GPIO_PIN_RESET);
			SH1106_WriteByte (0xB0 + pagenum);

			// Stel Data mode in
			HAL_GPIO_WritePin(SH1106_DC, GPIO_PIN_SET);
			HAL_SPI_Transmit_DMA (SH1106_HSPI, spi_buffer + pagenum * XSIZE, XSIZE); // sizeof(disp_buffer)

			pagenum++;
	  }
}

int SH1106_PaintScreen ()
{
	uint8_t page = 0;

	// wacht tot SPI periferie gereed is
	while (SH1106_HSPI->State != HAL_SPI_STATE_READY);

	memcpy (spi_buffer, disp_buffer, sizeof disp_buffer);
	pagenum = 0;
	SH1106_SPIDMA_Callback (); // Pagina schrijffunctie aanroepen

	framecount++;

	return 0;
}
